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
#include "cdb/cdbpath.h"
#include "cdb/cdbutil.h"
#include "optimizer/pathnode.h"


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
cdb_create_grouping_paths(PlannerInfo *root,
						  RelOptInfo *input_rel,
						  RelOptInfo *output_rel,
						  PathTarget *target,
						  PathTarget *partial_grouping_target,
						  const AggClauseCosts *agg_costs,
						  const AggClauseCosts *agg_partial_costs,
						  const AggClauseCosts *agg_final_costs)
{
	Query	   *parse = root->parse;
	Path	   *cheapest_path = input_rel->cheapest_total_path;
	bool		has_ordered_aggs = agg_costs->numPureOrderedAggs > 0;

	/* The caller should've checked these already */
	Assert(parse->hasAggs || parse->groupClause);
	Assert(!agg_costs->hasNonPartial && !agg_costs->hasNonSerial);
	Assert(root->config->gp_enable_multiphase_agg);

	/* The caller already constructed a one-stage plan. */


	/*
	 * This prohibition could be relaxed if we tracked missing combine
	 * functions per DQA and were willing to plan some DQAs as single and
	 * some as multiple phases.  Not currently, however.
	 */
	if (agg_costs->hasNonCombine || agg_costs->hasNonSerial)
		return;

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

	/* Consider 2-phase aggs */
	if (parse->groupClause)
	{

	}
	else
	{
		/*
		 * AGG -> MOTION -> AGG
		 */
		Path	   *path;
		CdbPathLocus locus;

		CdbPathLocus_MakeSingleQE(&locus, getgpsegmentCount());

		path = cheapest_path;

		path = (Path *) create_agg_path(root,
										output_rel,
										path,
										partial_grouping_target,
										AGG_PLAIN,
										AGGSPLIT_INITIAL_SERIAL,
										false, /* streaming */
										NIL, /* groupClause */
										NIL,
										agg_partial_costs,
										1 /* dNumPartialGroups */);

		path = cdbpath_create_motion_path(root, path, NIL, false, locus);

		path = (Path *) create_agg_path(root,
										output_rel,
										path,
										target,
										AGG_PLAIN,
										AGGSPLIT_FINAL_DESERIAL,
										false, /* streaming */
										NIL, /* groupClause */
										(List *) parse->havingQual,
										agg_final_costs,
										1 /* dNumGroups */);

		add_path(output_rel, path);
	}
}
