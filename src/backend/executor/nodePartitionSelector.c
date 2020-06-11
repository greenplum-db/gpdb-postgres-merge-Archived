/*-------------------------------------------------------------------------
 *
 * nodePartitionSelector.c
 *	  implement the execution of PartitionSelector for selecting partition
 *	  Oids based on a given set of predicates. It works for both constant
 *	  partition elimination and join partition elimination
 *
 * Copyright (c) 2014-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/backend/executor/nodePartitionSelector.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"
#include "miscadmin.h"

#include "access/table.h"
#include "executor/executor.h"
#include "executor/execPartition.h"
#include "executor/instrument.h"
#include "executor/nodePartitionSelector.h"
#include "partitioning/partdesc.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "utils/partcache.h"
#include "utils/rel.h"

static void LogPartitionSelection(EState *estate, int32 selectorId);

static void
partition_propagation(EState *estate, List *partOids, List *scanIds, int32 selectorId);

PartitionSelectorState *initPartitionSelection(PartitionSelector *node, EState *estate);

static TupleTableSlot *ExecPartitionSelector(PlanState *pstate);

/* ----------------------------------------------------------------
 *		ExecInitPartitionSelector
 *
 *		Create the run-time state information for PartitionSelector node
 *		produced by Orca and initializes outer child if exists.
 *
 * ----------------------------------------------------------------
 */
PartitionSelectorState *
ExecInitPartitionSelector(PartitionSelector *node, EState *estate, int eflags)
{
	PartitionSelectorState *psstate;

	/* check for unsupported flags */
	Assert (!(eflags & (EXEC_FLAG_MARK | EXEC_FLAG_BACKWARD)));

	psstate = makeNode(PartitionSelectorState);
	psstate->ps.plan = (Plan *) node;
	psstate->ps.state = estate;
	//psstate->levelPartRules = (PartitionRule **) palloc0(node->nLevels * sizeof(PartitionRule *));

	/* ExprContext initialization */
	ExecAssignExprContext(estate, &psstate->ps);

	/* initialize ExprState for evaluating expressions */
	/* GPDB_12_MERGE_FIXME: these ways of selecting partitions have not been re-implemented yet */
#if 0
	psstate->levelEqExprStates = ExecInitExprList(node->levelEqExpressions, &psstate->ps);
	psstate->levelExprStateLists = ExecInitExprList(node->levelExpressions, &psstate->ps);

	ExprState *residualPredicateExprState = ExecInitExpr((Expr *) node->residualPredicate,
														 (PlanState *) psstate);
	psstate->residualPredicateExprStateList = list_make1(residualPredicateExprState);
	psstate->propagationExprState = ExecInitExpr((Expr *) node->propagationExpression, (PlanState *) psstate);
#endif

	psstate->ps.ExecProcNode = ExecPartitionSelector;

	/* tuple table initialization */
	ExecInitResultTypeTL(&psstate->ps);
	ExecAssignProjectionInfo(&psstate->ps, NULL);

	/* initialize child nodes */
	/* No inner plan for PartitionSelector */
	Assert(NULL == innerPlan(node));
	if (NULL != outerPlan(node))
	{
		outerPlanState(psstate) = ExecInitNode(outerPlan(node), estate, eflags);
	}

	/*
	 * Initialize expressions to extract the partitioning keys from an input tuple.
	 */
	psstate->partkeyExpressions = ExecInitExprList(node->partkeyExpressions, &psstate->ps);

	/* we should have a lock already */
	psstate->parentrel = ExecGetRangeTableRelation(estate, node->parentRTI);

	return psstate;
}

/* ----------------------------------------------------------------
 *		ExecPartitionSelector(node)
 *
 *		Compute and propagate partition table Oids that will be
 *		used by Dynamic table scan. There are two ways of
 *		executing PartitionSelector.
 *
 *		1. Constant partition elimination
 *		Plan structure:
 *			Sequence
 *				|--PartitionSelector
 *				|--DynamicSeqScan
 *		In this case, PartitionSelector evaluates constant partition
 *		constraints to compute and propagate partition table Oids.
 *		It only need to be called once.
 *
 *		2. Join partition elimination
 *		Plan structure:
 *			...:
 *				|--DynamicSeqScan
 *				|--...
 *					|--PartitionSelector
 *						|--...
 *		In this case, PartitionSelector is in the same slice as
 *		DynamicSeqScan, DynamicIndexScan or DynamicBitmapHeapScan.
 *		It is executed for each tuple coming from its child node.
 *		It evaluates partition constraints with the input tuple and
 *		propagate matched partition table Oids.
 *
 *
 * Instead of a Dynamic Table Scan, there can be other nodes that use
 * a PartSelected qual to filter rows, based on which partitions are
 * selected. Currently, ORCA uses Dynamic Table Scans, while plans
 * produced by the non-ORCA planner use gating Result nodes with
 * PartSelected quals, to exclude unwanted partitions.
 *
 * ----------------------------------------------------------------
 */
static TupleTableSlot *
ExecPartitionSelector(PlanState *pstate)
{
	PartitionSelectorState *node = (PartitionSelectorState *) pstate;
	PartitionSelector *ps = (PartitionSelector *) node->ps.plan;
	EState	   *estate = node->ps.state;
	ExprContext *econtext = node->ps.ps_ExprContext;
	TupleTableSlot *inputSlot = NULL;

	if (ps->staticSelection)
	{
		elog(ERROR, "PartitionSelector static selection not implemented");
#if 0
		/* propagate the part oids obtained via static partition selection */
		partition_propagation(estate, ps->staticPartOids, ps->staticScanIds, ps->selectorId);
		return NULL;
#endif
	}

	/* Retrieve PartitionNode and access method from root table.
	 * We cannot do it during node initialization as
	 * DynamicTableScanInfo is not properly initialized yet.
	 */
	/* GPDB_12_MERGE_FIXME: dead? */
#if 0
	if (NULL == node->rootPartitionNode)
	{
		Assert(NULL != estate->dynamicTableScanInfo);
		getPartitionNodeAndAccessMethod
									(
									ps->relid,
									estate->dynamicTableScanInfo->partsMetadata,
									estate->es_query_cxt,
									&node->rootPartitionNode,
									&node->accessMethods
									);
	}
#endif

	if (NULL != outerPlanState(node))
	{
		/* Join partition elimination */
		/* get tuple from outer children */
		PlanState *outerPlan = outerPlanState(node);
		Assert(outerPlan);
		inputSlot = ExecProcNode(outerPlan);

		if (TupIsNull(inputSlot))
		{
			/* no more tuples from outerPlan */

			/*
			 * Make sure we have an entry for this scan id in
			 * dynamicTableScanInfo. Normally, this would've been done the
			 * first time a partition is selected, but we must ensure that
			 * there is an entry even if no partitions were selected.
			 * (The traditional Postgres planner uses this method.)
			 */
			if (node->partkeyExpressions)
				InsertPidIntoDynamicTableScanInfo(estate, ps->scanId, InvalidOid, ps->selectorId);
			else
				LogPartitionSelection(estate, ps->selectorId);

			return NULL;
		}
	}

	/* partition elimination with the given input tuple */
	ResetExprContext(econtext);
	econtext->ecxt_outertuple = inputSlot;

	/*
	 * If we have a partitioning projection, project the input tuple
	 * into a tuple that looks like tuples from the partitioned table, and use
	 * selectPartitionMulti() to select the partitions. (The traditional
	 * Postgres planner uses this method.)
	 */
	if (node->partkeyExpressions)
	{
		Datum		values[PARTITION_MAX_KEYS];
		bool		isnull[PARTITION_MAX_KEYS];
		int			partidx;
		PartitionKey partkey = RelationGetPartitionKey(node->parentrel);
		PartitionDesc partdesc = RelationGetPartitionDesc(node->parentrel);
		int			i;
		ListCell   *lc;

		Assert(list_length(node->partkeyExpressions) == partkey->partnatts);

		i = 0;
		foreach(lc, node->partkeyExpressions)
		{
			ExprState  *estate = (ExprState *) lfirst(lc);

			values[i] = ExecEvalExpr(estate, econtext, &isnull[i]);
			i++;
		}
		partidx = get_partition_for_tuple(partkey,
										  partdesc,
										  values,
										  isnull);

		InsertPidIntoDynamicTableScanInfo(estate, ps->scanId, partdesc->oids[partidx],
										  ps->selectorId);
	}
	else
	{
		/*
		 * Select the partitions based on levelEqExpressions and
		 * levelExpressions. (ORCA uses this method)
		 */
		elog(ERROR, "PartitionSelector selection with levelEqExpressions not implemented");
#if 0
		SelectedParts *selparts = processLevel(node, 0 /* level */, inputSlot);

		/* partition propagation */
		if (NULL != ps->propagationExpression)
		{
			partition_propagation(estate, selparts->partOids, selparts->scanIds, ps->selectorId);
		}
		list_free(selparts->partOids);
		list_free(selparts->scanIds);
		pfree(selparts);
#endif
	}

	return inputSlot;
}

static void LogSelectedPartitionsForScan(int32 selectorId, HTAB *pidIndex, const int32 scanId);

void LogPartitionSelection(EState *estate, int32 selectorId)
{
	if (optimizer_partition_selection_log == false)
		return;

	DynamicTableScanInfo *dynamicTableScanInfo = estate->dynamicTableScanInfo;

	Assert(dynamicTableScanInfo != NULL);

	HTAB **pidIndexes = dynamicTableScanInfo->pidIndexes;

	for (int32 scanIdMinusOne = 0; scanIdMinusOne < dynamicTableScanInfo->numScans; ++scanIdMinusOne)
	{
		HTAB *const pidIndex = pidIndexes[scanIdMinusOne];
		if (pidIndex == NULL)
			continue;
		const int32 scanId = scanIdMinusOne + 1;

		LogSelectedPartitionsForScan(selectorId, pidIndex, scanId);
	}
}

void LogSelectedPartitionsForScan(int32 selectorId, HTAB *pidIndex, const int32 scanId)
{
	int32 numPartitionsSelected = 0;
	Datum *selectedPartOids = palloc(sizeof(Datum) * hash_get_num_entries(pidIndex));

	HASH_SEQ_STATUS status;
	PartOidEntry *partOidEntry;
	hash_seq_init(&status, pidIndex);

	while ((partOidEntry = hash_seq_search(&status)) != NULL)
	{
		if (list_member_int(partOidEntry->selectorList, selectorId))
			selectedPartOids[numPartitionsSelected++] = ObjectIdGetDatum(partOidEntry->partOid);
	}

	// GPDB_12_MERGE_FIXME: DebugPartitionOid is gone
#if 0
	if (numPartitionsSelected > 0)
	{
		char *debugPartitionOid = DebugPartitionOid(selectedPartOids, numPartitionsSelected);
		ereport(LOG,
				(errmsg_internal("scanId: %d, partitions: %s, selector: %d",
								 scanId,
								 debugPartitionOid,
								 selectorId)));
		pfree(debugPartitionOid);
	}
#endif

	pfree(selectedPartOids);
}

/* ----------------------------------------------------------------
 *		ExecReScanPartitionSelector(node)
 *
 *		ExecReScan routine for PartitionSelector.
 * ----------------------------------------------------------------
 */
void
ExecReScanPartitionSelector(PartitionSelectorState *node)
{
	/* reset PartitionSelectorState */
#if 0
	PartitionSelector *ps = (PartitionSelector *) node->ps.plan;

	// GPDB_12_MERGE_FIXME
	for(int iter = 0; iter < ps->nLevels; iter++)
	{
		node->levelPartRules[iter] = NULL;
	}

	/* free result tuple slot */
	ExecClearTuple(node->ps.ps_ResultTupleSlot);
#endif

	/* If the PartitionSelector is in the inner side of a nest loop join,
	 * it should be constant partition elimination and thus has no child node.*/
#if USE_ASSERT_CHECKING
	PlanState  *outerPlan = outerPlanState(node);
	Assert (NULL == outerPlan);
#endif

}

/* ----------------------------------------------------------------
 *		ExecEndPartitionSelector(node)
 *
 *		ExecEnd routine for PartitionSelector. Free resources
 *		and clear tuple.
 *
 * ----------------------------------------------------------------
 */
void
ExecEndPartitionSelector(PartitionSelectorState *node)
{
	ExecFreeExprContext(&node->ps);

	/* clean child node */
	if (NULL != outerPlanState(node))
	{
		ExecEndNode(outerPlanState(node));
	}
}

/* ----------------------------------------------------------------
 *		partition_propagation
 *
 *		Propagate a list of leaf part Oids to the corresponding dynamic scans
 *
 * ----------------------------------------------------------------
 */
#if 0
static void
partition_propagation(EState *estate, List *partOids, List *scanIds, int32 selectorId)
{
	Assert (list_length(partOids) == list_length(scanIds));

	ListCell *lcOid = NULL;
	ListCell *lcScanId = NULL;
	forboth (lcOid, partOids, lcScanId, scanIds)
	{
		Oid partOid = lfirst_oid(lcOid);
		int scanId = lfirst_int(lcScanId);

		InsertPidIntoDynamicTableScanInfo(estate, scanId, partOid, selectorId);
	}
}
#endif

/* EOF */

