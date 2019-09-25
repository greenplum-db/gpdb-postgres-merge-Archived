/*-------------------------------------------------------------------------
 *
 * cdbgroupingpaths.c
 *	  Routines to aid in planning grouping queries for parallel
 *    execution.  This is, essentially, an extension of the file
 *    optimizer/prep/planner.c, although some functions are not
 *    externalized.
 *
 * Portions Copyright (c) 2019-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/backend/cdb/cdbgroupingpaths.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "cdb/cdbgroupingpaths.h"
#include "cdb/cdbhash.h"
#include "cdb/cdbpath.h"
#include "cdb/cdbutil.h"
#include "nodes/nodeFuncs.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/tlist.h"

/*
 * Function: cdb_grouping_planner
 *
 * This is basically an extension of the function create_grouping_paths() from
 * planner.c.  It creates two- and three-stage Paths to implement aggregates
 * and/or GROUP BY.
 *
 *
 * GPDB_96_MERGE_FIXME: This is roughly equivalent to the old
 * cdb_grouping_planner() function in cdbgroup.c, but for the brave
 * new upper-planner pathification world. Remove all the dead code from
 * cdbgroup.c when this is finished. Currently, this only handles a few
 * simple path types. None of the DQA stuff that was in cdbgroup.c, for
 * example.
 */
void
cdb_create_twostage_grouping_paths(PlannerInfo *root,
								   RelOptInfo *input_rel,
								   RelOptInfo *output_rel,
								   PathTarget *target,
								   PathTarget *partial_grouping_target,
								   bool can_sort,
								   bool consider_hash,
								   double dNumGroups,
								   const AggClauseCosts *agg_costs,
								   const AggClauseCosts *agg_partial_costs,
								   const AggClauseCosts *agg_final_costs)
{
	Query	   *parse = root->parse;
	Path	   *cheapest_path = input_rel->cheapest_total_path;
	bool		has_ordered_aggs = agg_costs->numPureOrderedAggs > 0;

	/* The caller should've checked these already */
	Assert(parse->hasAggs || parse->groupClause);
	/*
	 * This prohibition could be relaxed if we tracked missing combine
	 * functions per DQA and were willing to plan some DQAs as single and
	 * some as multiple phases.  Not currently, however.
	 */
	Assert(!agg_costs->hasNonCombine && !agg_costs->hasNonSerial);
	Assert(root->config->gp_enable_multiphase_agg);

	/* The caller already constructed a one-stage plan. */

	/*
	 * Ordered aggregates need to run the transition function on the
	 * values in sorted order, which in turn translates into single phase
	 * aggregation.
	 */
	if (has_ordered_aggs)
		return;

	/*
	 * We are currently unwilling to redistribute a gathered intermediate
	 * across the cluster.  This might change one day.
	 */
	if (!CdbPathLocus_IsPartitioned(cheapest_path->locus))
		return;


	/* GPDB_96_MERGE_FIXME: having more than 1 distinct agg would require
	 * three-phase strategy, like we used to have in cdbgroup.c
	 */
	if (list_length(agg_costs->dqaArgs) > 1)
		return;

	/*
	 * Consider 2-phase aggs
	 *
	 * AGG_PLAIN  -> MOTION -> AGG_PLAIN
	 * AGG_SORTED -> MOTION -> AGG_SORTED
	 * AGG_SORTED -> MOTION -> AGG_HASHED
	 * AGG_HASHED -> MOTION -> AGG_HASHED
	 */
	if (can_sort)
	{
		ListCell   *lc;

		foreach(lc, input_rel->pathlist)
		{
			Path	   *path = (Path *) lfirst(lc);
			bool		is_sorted;

			is_sorted = pathkeys_contained_in(root->group_pathkeys,
											  path->pathkeys);
			if (path == cheapest_path || is_sorted)
			{
				CdbPathLocus locus;
				bool		need_redistribute;
				Path	   *initial_agg_path;

				locus = cdb_choose_grouping_locus(root, path, target,
												  NIL, NIL,
												  &need_redistribute);
				if (!need_redistribute)
				{
					/*
					 * If the distribution of this path matches the GROUP BY clause,
					 * there's no need for two-stage aggregation, we can run the aggregates
					 * directly in one stage where the data already is.
					 */
					continue;
				}

				if (list_length(agg_costs->dqaArgs) > 0)
				{
					/*
					 * the input distribution must match the DISTINCT argument,
					 * otherwise we can't do it in two phases
					 */
					if (!CdbPathLocus_IsHashed(path->locus) ||
						!cdbpathlocus_is_hashed_on_exprs(path->locus, agg_costs->dqaArgs, true))
						continue;
				}

				if (!is_sorted)
				{
					path = (Path *) create_sort_path(root,
													 output_rel,
													 path,
													 root->group_pathkeys,
													 -1.0);
				}

				initial_agg_path = (Path *) create_agg_path(root,
												output_rel,
												path,
												partial_grouping_target,
								 parse->groupClause ? AGG_SORTED : AGG_PLAIN,
												AGGSPLIT_INITIAL_SERIAL,
												false, /* streaming */
												parse->groupClause,
												NIL,
												agg_partial_costs,
												dNumGroups);

				/*
				 * GroupAgg -> GATHER MOTION -> GroupAgg.
				 *
				 * This has the advantage that it retains the input order. The
				 * downside is that it gathers everything to a single node. If that's
				 * where the final result is needed anyway, that's quite possibly better
				 * than scattering the partial aggregate results and having another
				 * motion to gather the final results, though,
				 */
				{
					CdbPathLocus singleQE_locus;

					CdbPathLocus_MakeSingleQE(&singleQE_locus, getgpsegmentCount());
					path = cdbpath_create_motion_path(root, initial_agg_path, path->pathkeys, false, singleQE_locus);

					/* GPDB_96_MERGE_FIXME: also consider paths that send the output
					 * to a single QE, and retain the order. That might be beneficial
					 * if there's an ORDER BY in the query, so that we don't need to
					 * re-sort.
					 *
					 * Actually, it's probably often better to gather the results in
					 * the QD anyway, even if it means that we cannot do the Finalize
					 * Aggregate stage in parallel, because it avoids a final gather
					 * motion.
					 */
					path = (Path *) create_agg_path(root,
													output_rel,
													path,
													target,
													parse->groupClause ? AGG_SORTED : AGG_PLAIN,
													AGGSPLIT_FINAL_DESERIAL,
													false, /* streaming */
													parse->groupClause,
													(List *) parse->havingQual,
													agg_final_costs,
													dNumGroups);
					add_path(output_rel, path);
				}
			}
		}
	}

	if (consider_hash)
	{
		CdbPathLocus locus;
		bool		need_redistribute;
		Path	   *initial_agg_path;
		bool		doit = true;

		locus = cdb_choose_grouping_locus(root, cheapest_path, target,
										  NIL, NIL,
										  &need_redistribute);
		/*
		 * If the distribution of this path is suitable, two-stage aggregation
		 * is not applicable.
		 */
		if (!need_redistribute)
			doit = false;

		if (doit && list_length(agg_costs->dqaArgs) > 0)
		{
			/*
			 * the input distribution must match the DISTINCT argument,
			 * otherwise we can't do it in two phases
			 */
			if (!CdbPathLocus_IsHashed(cheapest_path->locus) ||
				!cdbpathlocus_is_hashed_on_exprs(cheapest_path->locus, agg_costs->dqaArgs, true))
				doit = false;
		}

		if (doit)
		{
			Path	   *path;

			initial_agg_path = (Path *) create_agg_path(root,
														output_rel,
														cheapest_path,
														partial_grouping_target,
														AGG_HASHED,
														AGGSPLIT_INITIAL_SERIAL,
														false, /* streaming */
														parse->groupClause,
														NIL,
														agg_partial_costs,
														dNumGroups);

			/*
			 * GroupAgg -> GATHER MOTION -> GroupAgg.
			 *
			 * This has the advantage that it retains the input order. The
			 * downside is that it gathers everything to a single node. If that's
			 * where the final result is needed anyway, that's quite possibly better
			 * than scattering the partial aggregate results and having another
			 * motion to gather the final results, though,
			 */
			path = cdbpath_create_motion_path(root, initial_agg_path, NIL, false,
											  locus);

			/* GPDB_96_MERGE_FIXME: also consider paths that send the output
			 * to a single QE, and retain the order. That might be beneficial
			 * if there's an ORDER BY in the query, so that we don't need to
			 * re-sort.
			 *
			 * Actually, it's probably often better to gather the results in
			 * the QD anyway, even if it means that we cannot do the Finalize
			 * Aggregate stage in parallel, because it avoids a final gather
			 * motion.
			 */
			path = (Path *) create_agg_path(root,
											output_rel,
											path,
											target,
											AGG_HASHED,
											AGGSPLIT_FINAL_DESERIAL,
											false, /* streaming */
											parse->groupClause,
											(List *) parse->havingQual,
											agg_final_costs,
											dNumGroups);
			add_path(output_rel, path);
		}
	}
}


/*
 * Figure out the desired data distribution to perform the grouping.
 *
 * In case of a simple GROUP BY, we prefer to distribute the data according to
 * the GROUP BY. With multiple grouping sets, identify the set of common
 * entries, and distribute based on that. For example, if you do
 * GROUP BY GROUPING SETS ((a, b, c), (b, c)), the common cols are b and c.
 */
CdbPathLocus
cdb_choose_grouping_locus(PlannerInfo *root, Path *path,
					  PathTarget *target,
					  List *rollup_lists,
					  List *rollup_groupclauses,
					  bool *need_redistribute_p)
{
	List	   *tlist = make_tlist_from_pathtarget(target);
	CdbPathLocus locus;
	bool		need_redistribute;
	List	   *hash_exprs;
	List	   *hash_opfamilies;
	ListCell   *lc;

	if (!CdbPathLocus_IsBottleneck(path->locus))
	{
		ListCell   *lcl, *lcc;
		Bitmapset  *common_groupcols = NULL;
		bool		first = true;
		int			x;

		if (rollup_lists)
		{
			forboth(lcl, rollup_lists, lcc, rollup_groupclauses)
			{
				List *rlist = (List *) lfirst(lcl);
				List *rclause = (List *) lfirst(lcc);
				List *last_list = (List *) llast(rlist);
				Bitmapset *this_groupcols = NULL;

				this_groupcols = NULL;
				foreach (lc, last_list)
				{
					SortGroupClause *sc = list_nth(rclause, lfirst_int(lc));

					this_groupcols = bms_add_member(this_groupcols, sc->tleSortGroupRef);
				}

				if (first)
					common_groupcols = this_groupcols;
				else
				{
					common_groupcols = bms_int_members(common_groupcols, this_groupcols);
					bms_free(this_groupcols);
				}
				first = false;
			}
		}
		else
		{
			List	   *rclause = root->parse->groupClause;

			foreach(lc, rclause)
			{
				SortGroupClause *sc = lfirst(lc);

				common_groupcols = bms_add_member(common_groupcols, sc->tleSortGroupRef);
			}
		}

		x = -1;
		hash_exprs = NIL;
		while ((x = bms_next_member(common_groupcols, x)) >= 0)
		{
			TargetEntry *tle = get_sortgroupref_tle(x, tlist);

			hash_exprs = lappend(hash_exprs, tle->expr);
		}

		if (!hash_exprs)
			need_redistribute = true;
		else
			need_redistribute = !cdbpathlocus_is_hashed_on_exprs(path->locus, hash_exprs, true);
	}
	else
	{
		need_redistribute = false;
		hash_exprs = NIL;
	}

	hash_opfamilies = NIL;
	foreach(lc, hash_exprs)
	{
		Node	   *expr = lfirst(lc);
		Oid			opfamily;

		opfamily = cdb_default_distribution_opfamily_for_type(exprType(expr));
		hash_opfamilies = lappend_oid(hash_opfamilies, opfamily);
	}

	if (need_redistribute)
	{
		if (hash_exprs)
			locus = cdbpathlocus_from_exprs(root, hash_exprs, hash_opfamilies, getgpsegmentCount());
		else
			CdbPathLocus_MakeSingleQE(&locus, getgpsegmentCount());
	}
	else
		CdbPathLocus_MakeNull(&locus, getgpsegmentCount());

	*need_redistribute_p = need_redistribute;
	return locus;
}
