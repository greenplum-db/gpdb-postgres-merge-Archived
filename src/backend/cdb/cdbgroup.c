/*-------------------------------------------------------------------------
 *
 * cdbgroup.c
 *	  Routines to aid in planning grouping queries for parallel
 *    execution.  This is, essentially, an extension of the file
 *    optimizer/prep/planner.c, although some functions are not
 *    externalized.
 *
 * Portions Copyright (c) 2005-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	    src/backend/cdb/cdbgroup.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include <limits.h>
#include <math.h>

#include "catalog/pg_collation.h"
#include "catalog/pg_constraint.h"
#include "catalog/indexing.h"
#include "catalog/pg_operator.h"
#include "catalog/pg_type.h"
#include "executor/executor.h"
#include "executor/nodeAgg.h"
#include "miscadmin.h"
#include "nodes/makefuncs.h"
#include "nodes/print.h"
#include "optimizer/clauses.h"
#include "optimizer/cost.h"
#include "optimizer/pathnode.h"
#include "optimizer/paths.h"
#include "optimizer/planmain.h"
#include "optimizer/planner.h"
#include "optimizer/planpartition.h"
#include "optimizer/planshare.h"
#include "optimizer/prep.h"
#include "optimizer/restrictinfo.h"
#include "optimizer/subselect.h"
#include "optimizer/tlist.h"
#include "optimizer/var.h"
#include "parser/parsetree.h"
#include "parser/parse_agg.h"
#include "parser/parse_clause.h"
#include "parser/parse_coerce.h"
#include "parser/parse_expr.h"
#include "parser/parse_oper.h"
#include "parser/parse_relation.h"
#include "utils/array.h"
#include "utils/fmgroids.h"
#include "utils/lsyscache.h"
#include "utils/selfuncs.h"
#include "utils/syscache.h"
#include "utils/tqual.h"
#include "utils/typcache.h"
#include "catalog/pg_aggregate.h"
#include "catalog/pg_statistic.h"

#include "cdb/cdbllize.h"
#include "cdb/cdbpath.h"
#include "cdb/cdbplan.h"
#include "cdb/cdbpullup.h"
#include "cdb/cdbutil.h"
#include "cdb/cdbvars.h"
#include "cdb/cdbhash.h"

#include "cdb/cdbsetop.h"
#include "cdb/cdbgroup.h"


/*
 * MppGroupPrep represents a strategy by which to precondition the
 * argument to  a parallel aggregation plan.
 *
 * MPP_GRP_PREP_NONE
 *    Use the input plan as is.
 * MPP_GRP_PREP_HASH_GROUPS
 *    Redistribute the input to collocate groups.
 * MPP_GRP_PREP_HASH_DISTINCT
 *    Redistribute the input on the sole distinct expression used as
 *    an aggregate argument.
 * MPP_GRP_PREP_FOCUS_QE
 *    Focus a partitioned input on the utility QE.
 * MPP_GRP_PREP_FOCUS_QD
 *    Focus a partitioned input on the QD.
 * MPP_GRP_PREP_BROADCAST
 *    Broadcast to convert a partitioned input into a replicated one.
 */
typedef enum MppGroupPrep
{
	MPP_GRP_PREP_NONE = 0,
	MPP_GRP_PREP_HASH_GROUPS,
	MPP_GRP_PREP_HASH_DISTINCT,
	MPP_GRP_PREP_FOCUS_QE,
	MPP_GRP_PREP_FOCUS_QD,
	MPP_GRP_PREP_BROADCAST
} MppGroupPrep;

/*
 * MppGroupType represents a stategy by which to implement parallel
 * aggregation on a possibly preconditioned input plan.
 *
 * MPP_GRP_TYPE_NONE
 *    No parallel plan found.
 * MPP_GRP_TYPE_BASEPLAN
 *    Use the sequential plan as is.
 * MPP_GRP_TYPE_PLAIN_2STAGE,
 *    Ungrouped, 2-stage aggregation.
 * MPP_GRP_TYPE_GROUPED_2STAGE,
 *    Grouped, 2-stage aggregation.
 * MPP_GRP_TYPE_PLAIN_DQA_2STAGE,
 *    Ungrouped, 2-stage aggregation.
 * MPP_GRP_TYPE_GROUPED_DQA_2STAGE,
 *    Grouped, 2-stage aggregation.
 *
 * TODO: Add types for min-max optimization, when ready:
 *
 * MPP_GRP_TYPE_MINMAX
 *    Use the sequential min-max optimization plan as is.
 * MPP_GRP_TYPE_MINMAX_2STAGE
 *    Use a 2-stage variant of min-max aggregation.
 */
typedef enum MppGroupType
{
	MPP_GRP_TYPE_NONE = 0,
	MPP_GRP_TYPE_BASEPLAN,
	MPP_GRP_TYPE_PLAIN_2STAGE,
	MPP_GRP_TYPE_GROUPED_2STAGE,
	MPP_GRP_TYPE_PLAIN_DQA_2STAGE,
	MPP_GRP_TYPE_GROUPED_DQA_2STAGE,
} MppGroupType;


/* Summary values detailing the post-Motion part of a coplan for
 * three-phase aggreation.  The code is as follows:
 *	S = Sort
 *	G = GroupAgg
 *	H = HashAgg
 *	P = PlainAgg
 *
 * So GSH means (GroupAgg (Sort (HashAgg X))).
 */
typedef enum DqaCoplanType
{
	DQACOPLAN_GGS,
	DQACOPLAN_GSH,
	DQACOPLAN_SHH,
	DQACOPLAN_HH,
	DQACOPLAN_PGS,
	DQACOPLAN_PH
} DqaCoplanType;

typedef enum DqaJoinStrategy
{
	DqaJoinUndefined = 0,
	DqaJoinNone,				/* No join required for solitary DQA argument. */
	DqaJoinCross,				/* Scalar aggregation uses cross product. */
	DqaJoinHash,				/* Hash join (possibly with subsequent sort) */
	DqaJoinMerge,				/* Merge join */

	/*
	 * These last are abstract and will be replaced by DqaJoinHash aor
	 * DqaJoinMerge once planning is complete.
	 */
	DqaJoinSorted,				/* Sorted output required. */
	DqaJoinCheapest,			/* No sort requirement. */
} DqaJoinStrategy;

/* DQA coplan information */
typedef struct DqaInfo
{
	Node	   *distinctExpr;	/* By reference from agg_counts for
								 * convenience. */
	AttrNumber	base_index;		/* Index of attribute in base plan targetlist */
	bool		can_hash;
	double		num_rows;		/* Estimated cardinality of grouping key, dqa
								 * arg */
	Plan	   *coplan;			/* Coplan for this (later this and all prior)
								 * coplan */
	Query	   *parse;			/* Plausible root->parse for the coplan. */
	bool		distinctkey_collocate;	/* Whether the input plan collocates
										 * on this distinct key */

	/*
	 * These fields are for costing and planning.  Before constructing the
	 * coplan for this DQA argument, determine cheapest way to get the answer
	 * and cheapest way to get the answer in grouping key order.
	 */
	bool		use_hashed_preliminary;
	Cost		cost_sorted;
	DqaCoplanType coplan_type_sorted;
	Cost		cost_cheapest;
	DqaCoplanType coplan_type_cheapest;
} DqaInfo;

/* Information about the overall plan for a one-, two- or one coplan of
 * a three-phase aggregation.
 */
typedef struct AggPlanInfo
{
	/*
	 * The input is either represented as a Path or a Plan and a Path. If
	 * input_plan is given, use this plan instead of creating one through
	 * input_path.
	 */
	Path	   *input_path;
	Plan	   *input_plan;

	/* These are the ordinary fields characterizing an aggregation */
	CdbPathLocus input_locus;
	MppGroupPrep group_prep;
	MppGroupType group_type;
	bool		distinctkey_collocate;	/* Whether the input plan collocates
										 * on the distinct key */

	/* These are extra for 3-phase plans */
	DqaJoinStrategy join_strategy;
	bool		use_sharing;

	/* These summarize the status of the structure's cost value. */
	bool		valid;
	Cost		plan_cost;
} AggPlanInfo;

typedef struct MppGroupContext
{
	MppGroupPrep prep;
	MppGroupType type;

	List	   *tlist;			/* The preprocessed targetlist of the original
								 * query. */
	Node	   *havingQual;		/* The proprocessed having qual of the
								 * original query. */
	Path	   *best_path;
	Path	   *cheapest_path;
	Plan	   *subplan;
	AggClauseCosts *agg_costs;
	double		tuple_fraction;
	double	   *p_dNumGroups;	/* Group count estimate shared up the call
								 * tree. */

	List	   *sub_tlist;		/* Derived (in cdb_grouping_planner) input
								 * targetlist. */
	int			numGroupCols;
	AttrNumber *groupColIdx;
	Oid		   *groupOperators;
	int			numDistinctCols;
	AttrNumber *distinctColIdx;
	DqaInfo    *dqaArgs;
	bool		use_hashed_grouping;
	CdbPathLocus input_locus;

	/*
	 * Indicate whether the input plan collocates on the distinct key if any.
	 * It is used for one or two-phase aggregation. For three-phase
	 * aggregation, distinctkey_collocate inside DqaInfo is used.
	 */
	bool		distinctkey_collocate;
	List	   *current_pathkeys;

	/*
	 * Indicate if root->parse has been changed during planning.  Carry in
	 * pointer to root for miscellaneous globals.
	 */
	bool		querynode_changed;
	PlannerInfo *root;

	/* Work space for aggregate/tlist deconstruction and reconstruction */
	Index		final_varno;	/* input */
	bool		use_irefs_tlist;	/* input */
	bool		use_dqa_pruning;	/* input */
	List	   *prefs_tlist;	/* Aggref attributes for prelim_tlist */
	List	   *irefs_tlist;	/* Aggref attributes for optional inter_tlist */
	List	   *frefs_tlist;	/* Aggref attributes for optional join tlists */
	List	   *dqa_tlist;		/* DQA argument attributes for prelim_tlist */
	List	  **dref_tlists;	/* Array of DQA Aggref tlists (dqa_tlist
								 * order) */
	List	   *grps_tlist;		/* Grouping attributes for prelim_tlist */
	List	   *fin_tlist;		/* Final tlist cache. */
	List	   *fin_hqual;		/* Final having qual cache. */
	Index		split_aggref_sortgroupref;	/* for TargetEntrys made in
											 * split_aggref */
	Index		outer_varno;	/* work */
	Index		inner_varno;	/* work */
	int		   *dqa_offsets;	/* work */
	List	   *top_tlist;		/* work - the target list to finalize */

	/* 3-phase DQA decisions */
	DqaJoinStrategy join_strategy;
	bool		use_sharing;

	List	   *wagSortClauses; /* List of List; within-agg multi sort level */
} MppGroupContext;

/* Constants for aggregation approaches.
 */
#define AGG_NONE		0x00

#define AGG_1PHASE		0x01
#define AGG_2PHASE		0x02
#define AGG_2PHASE_DQA	0x04
#define AGG_3PHASE		0x08

#define AGG_SINGLEPHASE	(AGG_1PHASE)
#define AGG_MULTIPHASE	(AGG_2PHASE | AGG_2PHASE_DQA | AGG_3PHASE)

#define AGG_ALL			(AGG_SINGLEPHASE | AGG_MULTIPHASE)


/* Coefficients for cost calculation adjustments: These are candidate GUCs
 * or, perhaps, replacements for the gp_eager_... series.  We wouldn't
 * need these if our statistics and cost calculations were correct, but
 * as of 3.2, they not.
 *
 * Early testing suggested that (1.0, 0.45, 1.7) was about right, but the
 * risk of introducing skew in the initial redistribution of a 1-phase plan
 * is great (especially given the 3.2 tendency to way underestimate the
 * cardinality of joins), so we penalize 1-phase and normalize to the
 * 2-phase cost (approximately).
 */
#ifdef NOT_USED
static const double gp_coefficient_1phase_agg = 20.0;	/* penalty */
static const double gp_coefficient_2phase_agg = 1.0;	/* normalized */
static const double gp_coefficient_3phase_agg = 3.3;	/* increase systematic
														 * underestimate */
#endif

/*---------------------------------------------
 * WITHIN stuff
 *---------------------------------------------*/

/*
 * WithinAggContext is a variable set used throughout plan_within_agg_persort().
 */
typedef struct
{
	bool		use_deduplicate;	/* true to choose deduplicate strategy */
	AttrNumber	pc_pos;			/* resno for peer count in outer tlist */
	AttrNumber	tc_pos;			/* resno for total count in inner tlist */
	List	   *current_pathkeys;	/* pathkeys tracking */
	List	   *inner_pathkeys; /* pathkeys for inner plan */
	List	   *rtable;			/* outer/inner RTE of the output */
} WithinAggContext;


/* Function augment_subplan_tlist
 *
 * Called from make_subplan_tlist, not directly.
 *
 * Make a target list like the input that includes sortgroupref'd entries
 * for the expressions in exprs.  Note that the entries in the input expression
 * list must be distinct.
 *
 * New entries corresponding to the expressions in the input exprs list
 * (if any) are added to the argument list.  Existing entries are modified
 * (if necessary) in place.
 *
 * Return the (modified) input targetlist.
 *
 * Implicitly return an array of resno values for exprs in (pnum, *pcols), if
 * return_resno is true.
 */
List *
augment_subplan_tlist(List *tlist, List *exprs, int *pnum, AttrNumber **pcols,
					  bool return_resno)
{
	int			num;
	AttrNumber *cols = NULL;

	num = list_length(exprs);	/* Known to be distinct. */
	if (num > 0)
	{
		int			keyno = 0;
		ListCell   *lx,
				   *lt;
		TargetEntry *tle,
				   *matched_tle;
		Index		max_sortgroupref = 0;

		foreach(lt, tlist)
		{
			tle = (TargetEntry *) lfirst(lt);
			if (tle->ressortgroupref > max_sortgroupref)
				max_sortgroupref = tle->ressortgroupref;
		}

		if (return_resno)
			cols = (AttrNumber *) palloc(sizeof(AttrNumber) * num);

		foreach(lx, exprs)
		{
			Node	   *expr = (Node *) lfirst(lx);

			matched_tle = NULL;

			foreach(lt, tlist)
			{
				tle = (TargetEntry *) lfirst(lt);

				if (equal(expr, tle->expr))
				{
					matched_tle = tle;
					break;
				}
			}

			if (matched_tle == NULL)
			{
				matched_tle = makeTargetEntry((Expr *) expr,
											  list_length(tlist) + 1,
											  NULL,
											  false);
				tlist = lappend(tlist, matched_tle);
			}

			if (matched_tle->ressortgroupref == 0)
				matched_tle->ressortgroupref = ++max_sortgroupref;

			if (return_resno)
				cols[keyno++] = matched_tle->resno;
		}

		if (return_resno)
		{
			*pnum = num;
			*pcols = cols;
		}
	}
	else
	{
		if (return_resno)
		{
			*pnum = 0;
			*pcols = NULL;
		}
	}

	/*
	 * Note that result is a copy, possibly modified by appending expression
	 * targetlist entries and/or updating sortgroupref values.
	 */
	return tlist;
}

/*
 * Generate targetlist for a SubqueryScan node to wrap the stage-one
 * Agg node (partial aggregation) of a 2-Stage aggregation sequence.
 *
 * varno: varno to use in generated Vars
 * input_tlist: targetlist of this node's input node
 *
 * Result is a "flat" (all simple Var node) targetlist in which
 * varattno and resno match and are sequential.
 *
 * This function also returns a map between the original targetlist
 * entry to new target list entry using resno values. The index
 * positions for resno_map represent the original resnos, while the
 * array elements represent the new resnos. The array is allocated
 * by the caller, which should have length of list_length(input_tlist).
 */
List *
generate_subquery_tlist(Index varno, List *input_tlist,
						bool keep_resjunk, int **p_resno_map)
{
	List	   *tlist = NIL;
	int			resno = 1;
	ListCell   *j;
	TargetEntry *tle;
	Node	   *expr;

	*p_resno_map = (int *) palloc0(list_length(input_tlist) * sizeof(int));

	foreach(j, input_tlist)
	{
		TargetEntry *inputtle = (TargetEntry *) lfirst(j);

		Assert(inputtle->resno == resno && inputtle->resno >= 1);

		/* Don't pull up constants, always use a Var to reference the input. */
		expr = (Node *) makeVar(varno,
								inputtle->resno,
								exprType((Node *) inputtle->expr),
								exprTypmod((Node *) inputtle->expr),
								exprCollation((Node *) inputtle->expr),
								0);

		(*p_resno_map)[inputtle->resno - 1] = resno;

		tle = makeTargetEntry((Expr *) expr,
							  (AttrNumber) resno++,
							  (inputtle->resname == NULL) ?
							  NULL :
							  pstrdup(inputtle->resname),
							  keep_resjunk ? inputtle->resjunk : false);
		tle->ressortgroupref = inputtle->ressortgroupref;
		tlist = lappend(tlist, tle);
	}

	return tlist;
}


/*
 * Function: cdbpathlocus_collocates_pathkeys
 *
 * Is a relation with the given locus guaranteed to collocate tuples with
 * non-distinct values of the key.  The key is a list of PathKeys.
 *
 * Collocation is guaranteed if the locus specifies a single process or
 * if the result is partitioned on a subset of the keys that must be
 * collocated.
 *
 * We ignore other sorts of collocation, e.g., replication or partitioning
 * on a range since these cannot occur at the moment (MPP 2.3).
 */
bool
cdbpathlocus_collocates_pathkeys(PlannerInfo *root, CdbPathLocus locus, List *pathkeys,
								 bool exact_match)
{
	ListCell   *i;
	List	   *pk_eclasses;

	if (CdbPathLocus_IsBottleneck(locus))
		return true;

	if (!CdbPathLocus_IsHashed(locus))
	{
		/*
		 * Note: HashedOJ can *not* be used for grouping. In HashedOJ, NULL
		 * values can be located on any segment, so we would end up with
		 * multiple NULL groups.
		 */
		return false;
	}

	if (exact_match && list_length(pathkeys) != list_length(locus.distkey))
		return false;

	/*
	 * Extract a list of eclasses from the pathkeys.
	 */
	pk_eclasses = NIL;
	foreach(i, pathkeys)
	{
		PathKey    *pathkey = (PathKey *) lfirst(i);
		EquivalenceClass *ec;

		ec = pathkey->pk_eclass;
		while (ec->ec_merged != NULL)
			ec = ec->ec_merged;

		pk_eclasses = lappend(pk_eclasses, ec);
	}

	/*
	 * Check for containment of locus in pk_eclasses.
	 *
	 * We ignore constants in the locus hash key. A constant has the same
	 * value everywhere, so it doesn't affect collocation.
	 */
	return cdbpathlocus_is_hashed_on_eclasses(locus, pk_eclasses, true);
}


/*
 * Function: cdbpathlocus_collocates_expressions
 *
 * Like cdbpathlocus_collocates_pathkeys, but the key list is given as a list
 * of plain expressions, instead of PathKeys.
 */
bool
cdbpathlocus_collocates_expressions(PlannerInfo *root, CdbPathLocus locus, List *exprs,
								   bool exact_match)
{
	if (CdbPathLocus_IsBottleneck(locus))
		return true;

	if (!CdbPathLocus_IsHashed(locus))
	{
		/*
		 * Note: HashedOJ can *not* be used for grouping. In HashedOJ, NULL
		 * values can be located on any segment, so we would end up with
		 * multiple NULL groups.
		 */
		return false;
	}

	if (exact_match && list_length(exprs) != list_length(locus.distkey))
		return false;

	/*
	 * Check for containment of locus in pk_eclasses.
	 *
	 * We ignore constants in the locus hash key. A constant has the same
	 * value everywhere, so it doesn't affect collocation.
	 */
	return cdbpathlocus_is_hashed_on_exprs(locus, exprs, true);
}


/*
 * Function: cdbpathlocus_from_flow
 *
 * Generate a locus from a flow.  Since the information needed to produce
 * canonical path keys is unavailable, this function will never return a
 * hashed locus.
 */
CdbPathLocus
cdbpathlocus_from_flow(Flow *flow)
{
	CdbPathLocus locus;

	CdbPathLocus_MakeNull(&locus, flow->numsegments);

	if (!flow)
		return locus;

	switch (flow->flotype)
	{
		case FLOW_SINGLETON:
			if (flow->segindex == -1)
				/* FIXME: should numsegments be set to flow->numsegments? */
				CdbPathLocus_MakeEntry(&locus);
			else
				CdbPathLocus_MakeSingleQE(&locus, flow->numsegments);
			break;
		case FLOW_REPLICATED:
			CdbPathLocus_MakeReplicated(&locus, flow->numsegments);
			break;
		case FLOW_PARTITIONED:
			CdbPathLocus_MakeStrewn(&locus, flow->numsegments);
			break;
		case FLOW_UNDEFINED:
		default:
			Insist(0);
	}
	return locus;
}

/*
 * Determines, given the range table in use by the query, whether one Var has a
 * functional dependency on another. "Functional dependency" is defined in the
 * same way as check_functional_grouping() in pg_constraint, and this function
 * is a copy of that logic. This is used by find_group_dependent_targets() to
 * discover columns that need to be added back to the grouping target list.
 *
 * GPDB_91_MERGE_FIXME: This duplication is brittle. We should modify our
 * grouping planner to remove the assumptions on target list structure, and find
 * a better way to construct a correct series of Aggref stages.
 */
static bool
has_functional_dependency(Var *from, Var *to, List *rangeTable)
{
	Oid			relid;
	bool		to_is_primary = false;
	Relation	pg_constraint;
	HeapTuple	tuple;
	SysScanDesc scan;
	ScanKeyData skey[1];
	RangeTblEntry *rte;

	if (from->varno != to->varno)
	{
		/* These aren't part of the same table; get out. */
		return false;
	}

	/* Look up the relid of the "to" Var from the range table. */
	rte = list_nth(rangeTable, to->varno - 1);
	Assert(rte);

	if (rte->rtekind != RTE_RELATION)
	{
		/* We don't have functional dependencies on subquery results. */
		return false;
	}

	relid = rte->relid;

	/* Scan pg_constraint for constraints of the target rel */
	pg_constraint = heap_open(ConstraintRelationId, AccessShareLock);

	ScanKeyInit(&skey[0],
				Anum_pg_constraint_conrelid,
				BTEqualStrategyNumber, F_OIDEQ,
				ObjectIdGetDatum(relid));

	scan = systable_beginscan(pg_constraint, ConstraintRelidIndexId, true,
							  NULL, 1, skey);

	while (HeapTupleIsValid(tuple = systable_getnext(scan)))
	{
		Form_pg_constraint con = (Form_pg_constraint) GETSTRUCT(tuple);
		Datum		adatum;
		bool		isNull;
		ArrayType  *arr;
		int16	   *attnums;
		int			numkeys;
		int			i;
		bool		found_col;

		/* Only PK constraints are of interest for now, see comment above */
		if (con->contype != CONSTRAINT_PRIMARY)
			continue;
		/* Constraint must be non-deferrable */
		if (con->condeferrable)
			continue;

		/* Extract the conkey array, ie, attnums of PK's columns */
		adatum = heap_getattr(tuple, Anum_pg_constraint_conkey,
							  RelationGetDescr(pg_constraint), &isNull);
		if (isNull)
			elog(ERROR, "null conkey for constraint %u",
				 HeapTupleGetOid(tuple));
		arr = DatumGetArrayTypeP(adatum);		/* ensure not toasted */
		numkeys = ARR_DIMS(arr)[0];
		if (ARR_NDIM(arr) != 1 ||
			numkeys < 0 ||
			ARR_HASNULL(arr) ||
			ARR_ELEMTYPE(arr) != INT2OID)
			elog(ERROR, "conkey is not a 1-D smallint array");
		attnums = (int16 *) ARR_DATA_PTR(arr);

		found_col = false;
		for (i = 0; i < numkeys; i++)
		{
			AttrNumber	attnum = attnums[i];

			found_col = false;

			if (IsA(to, Var) &&
				to->varno == from->varno &&
				to->varlevelsup == 0 &&
				to->varattno == attnum)
			{
				found_col = true;
			}

			if (!found_col)
				break;
		}

		if (found_col)
		{
			to_is_primary = true;
			break;
		}
	}

	systable_endscan(scan);

	heap_close(pg_constraint, AccessShareLock);

	return to_is_primary;
}

/*
 * State struct for find_group_dependent_targets. See that function for usage.
 */
struct groupdep_ctx
{
	List	*grps_tlist;
	List	*rangeTable;
	List	*add_tlist;
};

/*
 * Callback for expression_tree_walker. Walks the node tree searching for Vars
 * that have a functional dependency (as defined by has_functional_dependency,
 * above) on Vars in the grouping target list. Any such Vars that are found are
 * wrapped in a TargetEntry and added to the context's output target list.
 *
 * Set up the context struct as follows:
 *
 * ctx->grps_tlist: a List of TargetEntry nodes to be searched for targets of
 *                  functional dependencies
 * ctx->rangeTable: the current range table in use by the query, used to match
 *                  Vars to the relation they reference
 * ctx->add_tlist: the output List of TargetEntry nodes; set this to NIL
 */
static bool
find_group_dependent_targets(Node *node, struct groupdep_ctx *ctx)
{
	if (!node)
		return false;

	if (IsA(node, Var))
	{
		Var		   *var = (Var *) node;
		ListCell   *grp_lc;
		bool		has_dep = false;

		/* Search the grouping target list for Vars this one is dependent on. */
		foreach (grp_lc, ctx->grps_tlist)
		{
			TargetEntry *grp_tle = lfirst(grp_lc);
			Expr		*grp_expr = grp_tle->expr;
			Var			*grp_var;

			if (!IsA(grp_expr, Var))
			{
				/*
				 * Ignore any expressions that aren't Vars; they can't have
				 * functional dependencies.
				 */
				continue;
			}

			grp_var = (Var *) grp_expr;

			if (equal(var, grp_var))
			{
				/* Don't add duplicates. */
				has_dep = false;
				break;
			}

			if (!has_dep && has_functional_dependency(var, grp_var,
													  ctx->rangeTable))
			{
				has_dep = true;
			}
		}

		if (has_dep)
		{
			TargetEntry *copy;

			/* Copy the entry so that it can later be added to grps_tlist. */
			copy = makeTargetEntry(copyObject(var),
								   list_length(ctx->grps_tlist)
								   + list_length(ctx->add_tlist) + 1,
								   NULL,
								   false);

			ctx->add_tlist = lappend(ctx->add_tlist, copy);
		}

		return false;
	}

	return expression_tree_walker(node, find_group_dependent_targets, ctx);
}

/**
 * Update the scatter clause before a query tree's targetlist is about to
 * be modified. The scatter clause of a query tree will correspond to
 * old targetlist entries. If the query tree is modified and the targetlist
 * is to be modified, we must call this method to ensure that the scatter clause
 * is kept in sync with the new targetlist.
 */
void
UpdateScatterClause(Query *query, List *newtlist)
{
	Assert(query);
	Assert(query->targetList);
	Assert(newtlist);

	if (query->scatterClause
		&& list_nth(query->scatterClause, 0) != NULL	/* scattered randomly */
		)
	{
		Assert(list_length(query->targetList) == list_length(newtlist));
		List	   *scatterClause = NIL;
		ListCell   *lc = NULL;

		foreach(lc, query->scatterClause)
		{
			Expr	   *o = (Expr *) lfirst(lc);

			Assert(o);
			TargetEntry *tle = tlist_member((Node *) o, query->targetList);

			Assert(tle);
			TargetEntry *ntle = list_nth(newtlist, tle->resno - 1);

			scatterClause = lappend(scatterClause, copyObject(ntle->expr));
		}
		query->scatterClause = scatterClause;
	}
}

/*
 * reconstruct_pathkeys
 *
 * Reconstruct the given pathkeys based on the given mapping from the original
 * targetlist to a new targetlist.
 */
List *
reconstruct_pathkeys(PlannerInfo *root, List *pathkeys, int *resno_map,
					 List *orig_tlist, List *new_tlist)
{
	List	   *new_pathkeys;
	ListCell   *i,
			   *j;

	new_pathkeys = NIL;
	foreach(i, pathkeys)
	{
		PathKey    *pathkey = (PathKey *) lfirst(i);
		bool		found = false;

		foreach(j, pathkey->pk_eclass->ec_members)
		{
			EquivalenceMember *em = (EquivalenceMember *) lfirst(j);
			TargetEntry *tle = tlist_member((Node *) em->em_expr, orig_tlist);

			if (tle)
			{
				TargetEntry *new_tle;
				EquivalenceClass *new_eclass;
				PathKey    *new_pathkey;

				new_tle = get_tle_by_resno(new_tlist, resno_map[tle->resno - 1]);
				if (!new_tle)
					elog(ERROR, "could not find path key expression in constructed subquery's target list");

				/*
				 * The param 'rel' is only used on making and findding EC in childredrels.
				 * But I think the situation does not happen in adding cdb path, So Null is
				 * ok.
				 */
				new_eclass = get_eclass_for_sort_expr(root,
													  new_tle->expr,
													  em->em_nullable_relids,
													  pathkey->pk_eclass->ec_opfamilies,
													  em->em_datatype,
													  exprCollation((Node *) tle->expr),
													  0,
													  NULL,
													  true);
				new_pathkey = copyObject(pathkey);
				new_pathkey->pk_eclass = new_eclass;
				new_pathkeys = lappend(new_pathkeys, new_pathkey);
				found = true;
				break;
			}
		}
		if (!found)
		{
			new_pathkeys = lappend(new_pathkeys, copyObject(pathkey));
		}
	}

	return new_pathkeys;
}
