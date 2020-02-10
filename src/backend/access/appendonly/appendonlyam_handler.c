/*-------------------------------------------------------------------------
 *
 * appendonlyam_handler.c
 *	  appendonly table access method code
 *
 * Portions Copyright (c) 2008, Greenplum Inc
 * Portions Copyright (c) 2020-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/appendonly/appendonlyam_handler.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <math.h>

#include "miscadmin.h"

#include "access/genam.h"
#include "access/heapam.h"
#include "access/multixact.h"
#include "access/rewriteheap.h"
#include "access/tableam.h"
#include "access/tsmapi.h"
#include "access/tuptoaster.h"
#include "access/xact.h"
#include "catalog/catalog.h"
#include "catalog/index.h"
#include "catalog/storage.h"
#include "catalog/storage_xlog.h"
#include "cdb/cdbappendonlyam.h"
#include "commands/progress.h"
#include "executor/executor.h"
#include "optimizer/plancat.h"
#include "pgstat.h"
#include "storage/bufmgr.h"
#include "storage/bufpage.h"
#include "storage/bufmgr.h"
#include "storage/lmgr.h"
#include "storage/predicate.h"
#include "storage/procarray.h"
#include "storage/smgr.h"
#include "utils/builtins.h"
#include "utils/rel.h"


static void reform_and_rewrite_tuple(HeapTuple tuple,
									 Relation OldHeap, Relation NewHeap,
									 Datum *values, bool *isnull, RewriteState rwstate);

static const TableAmRoutine appendonly_methods;


/* ------------------------------------------------------------------------
 * Slot related callbacks for appendonly AM
 * ------------------------------------------------------------------------
 */

static const TupleTableSlotOps *
appendonly_slot_callbacks(Relation relation)
{
	return &TTSOpsMemTuple;
}

/* ------------------------------------------------------------------------
 * Seq Scan callbacks for appendonly AM
 *
 * These are in appendonlyam.c
 * ------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------
 * Index Scan Callbacks for appendonly AM
 * ------------------------------------------------------------------------
 */

static IndexFetchTableData *
appendonly_index_fetch_begin(Relation rel)
{
	IndexFetchAppendOnlyData *aoscan = palloc0(sizeof(IndexFetchAppendOnlyData));

	aoscan->xs_base.rel = rel;

	/* aoscan->aofetch is initialized lazily on first fetch */

	return &aoscan->xs_base;
}

static void
appendonly_index_fetch_reset(IndexFetchTableData *scan)
{
	// GPDB_12_MERGE_FIXME: Should we close the underlying AO fetch desc here?
}

static void
appendonly_index_fetch_end(IndexFetchTableData *scan)
{
	IndexFetchAppendOnlyData *aoscan = (IndexFetchAppendOnlyData *) scan;

	if (aoscan->aofetch)
	{
		appendonly_fetch_finish(aoscan->aofetch);
		pfree(aoscan->aofetch);
		aoscan->aofetch = NULL;
	}
}

static bool
appendonly_index_fetch_tuple(struct IndexFetchTableData *scan,
							 ItemPointer tid,
							 Snapshot snapshot,
							 TupleTableSlot *slot,
							 bool *call_again, bool *all_dead)
{
	IndexFetchAppendOnlyData *aoscan = (IndexFetchAppendOnlyData *) scan;

	if (!aoscan->aofetch)
	{
		Snapshot	appendOnlyMetaDataSnapshot;

		appendOnlyMetaDataSnapshot = snapshot;
		if (appendOnlyMetaDataSnapshot == SnapshotAny)
		{
			/*
			 * the append-only meta data should never be fetched with
			 * SnapshotAny as bogus results are returned.
			 */
			appendOnlyMetaDataSnapshot = GetTransactionSnapshot();
		}

		aoscan->aofetch =
			appendonly_fetch_init(aoscan->xs_base.rel,
								  snapshot,
								  appendOnlyMetaDataSnapshot);
	}
	else
	{
		/* GPDB_12_MERGE_FIXME: Is it possible for the 'snapshot' to change
		 * between calls? Add a sanity check for that here. */
	}

	Assert(TTS_IS_MEMTUPLE(slot));

	appendonly_fetch(aoscan->aofetch, (AOTupleId *) tid, slot);

	return !TupIsNull(slot);
}


/* ------------------------------------------------------------------------
 * Callbacks for non-modifying operations on individual tuples for heap AM
 * ------------------------------------------------------------------------
 */

static bool
appendonly_fetch_row_version(Relation relation,
						 ItemPointer tid,
						 Snapshot snapshot,
						 TupleTableSlot *slot)
{
	// GPDB_12_MERGE_FIXME: what should this do?
	elog(ERROR, "not implemented yet");
}

static void
appendonly_get_latest_tid(TableScanDesc sscan,
						  ItemPointer tid)
{
	/* No HOT, so nothing to do here. */
}

static bool
appendonly_tuple_tid_valid(TableScanDesc scan, ItemPointer tid)
{
	// GPDB_12_MERGE_FIXME: what should this do?
	elog(ERROR, "not implemented yet");
}

static bool
appendonly_tuple_satisfies_snapshot(Relation rel, TupleTableSlot *slot,
								Snapshot snapshot)
{
	// GPDB_12_MERGE_FIXME: what should this do?
	elog(ERROR, "not implemented yet");
}

static TransactionId
appendonly_compute_xid_horizon_for_tuples(Relation rel,
										  ItemPointerData *tids,
										  int nitems)
{
	// GPDB_12_MERGE_FIXME: what should this do?
	elog(ERROR, "not implemented yet");
}



/* ----------------------------------------------------------------------------
 *  Functions for manipulations of physical tuples for heap AM.
 * ----------------------------------------------------------------------------
 */

static void
appendonly_tuple_insert(Relation relation, TupleTableSlot *slot, CommandId cid,
					int options, BulkInsertState bistate)
{
	bool		shouldFree = true;
	MemTuple	mtuple = ExecFetchSlotMemTuple(slot, true, &shouldFree);
	AppendOnlyInsertDesc insertDesc;

	// GPDB_12_MERGE_FIXME: Where to store this?
#if 0
	if (resultRelInfo->ri_aoInsertDesc == NULL)
	{
		/* Set the pre-assigned fileseg number to insert into */
		ResultRelInfoSetSegno(resultRelInfo, estate->es_result_aosegnos);

		resultRelInfo->ri_aoInsertDesc =
			appendonly_insert_init(resultRelationDesc,
								   resultRelInfo->ri_aosegno,
								   false);
	}
#endif
	
	/* Update the tuple with table oid */
	slot->tts_tableOid = RelationGetRelid(relation);

	/* Perform the insertion, and copy the resulting ItemPointer */
	appendonly_insert(insertDesc, mtuple, (AOTupleId *) &slot->tts_tid);
	// GPDB_12_MERGE_FIXME
	//(resultRelInfo->ri_aoprocessed)++;

	if (shouldFree)
		pfree(mtuple);
}

static void
appendonly_tuple_insert_speculative(Relation relation, TupleTableSlot *slot,
								CommandId cid, int options,
								BulkInsertState bistate, uint32 specToken)
{
 	/* GPDB_12_MERGE_FIXME: not supported. Can this function be left out completely? Or ereport()? */
	elog(ERROR, "speculative insertion not supported on AO tables");
}

static void
appendonly_tuple_complete_speculative(Relation relation, TupleTableSlot *slot,
								  uint32 specToken, bool succeeded)
{
	elog(ERROR, "speculative insertion not supported on AO tables");
}

static void
appendonly_multi_insert(Relation relation, TupleTableSlot **slots, int ntuples,
						CommandId cid, int options, BulkInsertState bistate)
{
	elog(ERROR, "multi-insert not yet implemented on AO tables");
}

static TM_Result
appendonly_tuple_delete(Relation relation, ItemPointer tid, CommandId cid,
					Snapshot snapshot, Snapshot crosscheck, bool wait,
					TM_FailureData *tmfd, bool changingPart)
{
	AppendOnlyDeleteDesc deleteDesc;

	if (IsolationUsesXactSnapshot())
	{
		/*
		 * say "deletes and updates" because this could be a cross-segment
		 * or cross-partition update.
		 */
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("deletes and updates on append-only tables are not supported in serializable transactions")));
	}

	// GPDB_12_MERGE_FIXME: Where to store this?
#if 0
	if (resultRelInfo->ri_deleteDesc == NULL)
	{
		resultRelInfo->ri_deleteDesc = 
			appendonly_delete_init(resultRelationDesc, GetActiveSnapshot());
	}
#endif
	
	return appendonly_delete(deleteDesc, (AOTupleId *) tid);
}


static TM_Result
appendonly_tuple_update(Relation relation, ItemPointer otid, TupleTableSlot *slot,
					CommandId cid, Snapshot snapshot, Snapshot crosscheck,
					bool wait, TM_FailureData *tmfd,
					LockTupleMode *lockmode, bool *update_indexes)
{
	AppendOnlyUpdateDesc updateDesc;
	bool		shouldFree = true;
	MemTuple	mtuple = ExecFetchSlotMemTuple(slot, true, &shouldFree);
	TM_Result	result;

	if (IsolationUsesXactSnapshot())
	{
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("updates on append-only tables are not supported in serializable transactions")));
	}

	// GPDB_12_MERGE_FIXME: Where to store this?
#if 0
	if (resultRelInfo->ri_updateDesc == NULL)
	{
		ResultRelInfoSetSegno(resultRelInfo, estate->es_result_aosegnos);
		resultRelInfo->ri_updateDesc = (AppendOnlyUpdateDesc)
			appendonly_update_init(resultRelationDesc, GetActiveSnapshot(), resultRelInfo->ri_aosegno);
	}
#endif
	
	/* Update the tuple with table oid */
	slot->tts_tableOid = RelationGetRelid(relation);

	result = appendonly_update(updateDesc,
							   mtuple, (AOTupleId *) otid, (AOTupleId *) &slot->tts_tid);
	// GPDB_12_MERGE_FIXME
	//(resultRelInfo->ri_aoprocessed)++;

	/* No HOT updates with AO tables. */
	*update_indexes = true;

	if (shouldFree)
		pfree(mtuple);

	return result;
}

static TM_Result
appendonly_tuple_lock(Relation relation, ItemPointer tid, Snapshot snapshot,
				  TupleTableSlot *slot, CommandId cid, LockTupleMode mode,
				  LockWaitPolicy wait_policy, uint8 flags,
				  TM_FailureData *tmfd)
{
 	/* GPDB_12_MERGE_FIXME: not supported. Can this function be left out completely? Or ereport()? */
	elog(ERROR, "speculative insertion not supported on AO tables");
}

static void
appendonly_finish_bulk_insert(Relation relation, int options)
{
}


/* ------------------------------------------------------------------------
 * DDL related callbacks for heap AM.
 * ------------------------------------------------------------------------
 */

static void
appendonly_relation_set_new_filenode(Relation rel,
								 const RelFileNode *newrnode,
								 char persistence,
								 TransactionId *freezeXid,
								 MultiXactId *minmulti)
{
	SMgrRelation srel;

	/*
	 * Initialize to the minimum XID that could put tuples in the table. We
	 * know that no xacts older than RecentXmin are still running, so that
	 * will do.
	 */
	*freezeXid = RecentXmin;

	/*
	 * Similarly, initialize the minimum Multixact to the first value that
	 * could possibly be stored in tuples in the table.  Running transactions
	 * could reuse values from their local cache, so we are careful to
	 * consider all currently running multis.
	 *
	 * XXX this could be refined further, but is it worth the hassle?
	 */
	*minmulti = GetOldestMultiXactId();

	// GPDB_12_MERGE_FIXME: RelationCreateStorage() used to take a 'relstorage' arg.
	// Do we need to do something special to tell it that this is an AO table?
	srel = RelationCreateStorage(*newrnode, persistence);

	/*
	 * If required, set up an init fork for an unlogged table so that it can
	 * be correctly reinitialized on restart.  An immediate sync is required
	 * even if the page has been logged, because the write did not go through
	 * shared_buffers and therefore a concurrent checkpoint may have moved the
	 * redo pointer past our xlog record.  Recovery may as well remove it
	 * while replaying, for example, XLOG_DBASE_CREATE or XLOG_TBLSPC_CREATE
	 * record. Therefore, logging is necessary even if wal_level=minimal.
	 */
	if (persistence == RELPERSISTENCE_UNLOGGED)
	{
		Assert(rel->rd_rel->relkind == RELKIND_RELATION ||
			   rel->rd_rel->relkind == RELKIND_MATVIEW ||
			   rel->rd_rel->relkind == RELKIND_TOASTVALUE);
		smgrcreate(srel, INIT_FORKNUM, false);
		log_smgrcreate(newrnode, INIT_FORKNUM);
		smgrimmedsync(srel, INIT_FORKNUM);
	}

	smgrclose(srel);
}

static void
appendonly_relation_nontransactional_truncate(Relation rel)
{
	RelationTruncate(rel, 0);
}

static void
appendonly_relation_copy_data(Relation rel, const RelFileNode *newrnode)
{
	SMgrRelation dstrel;

	dstrel = smgropen(*newrnode, rel->rd_backend);
	RelationOpenSmgr(rel);

	/*
	 * Since we copy the file directly without looking at the shared buffers,
	 * we'd better first flush out any pages of the source relation that are
	 * in shared buffers.  We assume no new changes will be made while we are
	 * holding exclusive lock on the rel.
	 */
	FlushRelationBuffers(rel);

	/*
	 * Create and copy all forks of the relation, and schedule unlinking of
	 * old physical files.
	 *
	 * NOTE: any conflict in relfilenode value will be caught in
	 * RelationCreateStorage().
	 */
	RelationCreateStorage(*newrnode, rel->rd_rel->relpersistence);

	/* copy main fork */
	RelationCopyStorage(rel->rd_smgr, dstrel, MAIN_FORKNUM,
						rel->rd_rel->relpersistence);

	/* copy those extra forks that exist */
	for (ForkNumber forkNum = MAIN_FORKNUM + 1;
		 forkNum <= MAX_FORKNUM; forkNum++)
	{
		if (smgrexists(rel->rd_smgr, forkNum))
		{
			smgrcreate(dstrel, forkNum, false);

			/*
			 * WAL log creation if the relation is persistent, or this is the
			 * init fork of an unlogged relation.
			 */
			if (rel->rd_rel->relpersistence == RELPERSISTENCE_PERMANENT ||
				(rel->rd_rel->relpersistence == RELPERSISTENCE_UNLOGGED &&
				 forkNum == INIT_FORKNUM))
				log_smgrcreate(newrnode, forkNum);
			RelationCopyStorage(rel->rd_smgr, dstrel, forkNum,
								rel->rd_rel->relpersistence);
		}
	}


	/* drop old relation, and close new one */
	RelationDropStorage(rel);
	smgrclose(dstrel);
}

static void
appendonly_relation_copy_for_cluster(Relation OldHeap, Relation NewHeap,
								 Relation OldIndex, bool use_sort,
								 TransactionId OldestXmin,
								 TransactionId *xid_cutoff,
								 MultiXactId *multi_cutoff,
								 double *num_tuples,
								 double *tups_vacuumed,
								 double *tups_recently_dead)
{
 	/*
	 * GPDB_12_MERGE_FIXME: not sure what to do here. We used to not
	 * support CLUSTER on AO tables, and there's a check for that in
	 * cluster.c. It would be nicer to check it here. Then again, why not
	 * support it? VACUUM FULL is implemented as CLUSTER, too.
	 */
	elog(ERROR, "CLUSTER not supported on AO tables");
}

static bool
appendonly_scan_analyze_next_block(TableScanDesc scan, BlockNumber blockno,
							   BufferAccessStrategy bstrategy)
{
	// GPDB_12_MERGE_FIXME: Re-implement the logic from acquire_sample_rows_ao()
	// in these two functions.
	return false;
}

static bool
appendonly_scan_analyze_next_tuple(TableScanDesc scan, TransactionId OldestXmin,
							   double *liverows, double *deadrows,
							   TupleTableSlot *slot)
{
	return false;
}

static double
appendonly_index_build_range_scan(Relation heapRelation,
							  Relation indexRelation,
							  IndexInfo *indexInfo,
							  bool allow_sync,
							  bool anyvisible,
							  bool progress,
							  BlockNumber start_blockno,
							  BlockNumber numblocks,
							  IndexBuildCallback callback,
							  void *callback_state,
							  TableScanDesc scan)
{
	AppendOnlyScanDesc aoscan;
	bool		is_system_catalog;
	bool		checking_uniqueness;
	Datum		values[INDEX_MAX_KEYS];
	bool		isnull[INDEX_MAX_KEYS];
	double		reltuples;
	ExprState  *predicate;
	TupleTableSlot *slot;
	EState	   *estate;
	ExprContext *econtext;
	Snapshot	snapshot;
	bool		need_unregister_snapshot = false;
	TransactionId OldestXmin;

	/*
	 * sanity checks
	 */
	Assert(OidIsValid(indexRelation->rd_rel->relam));

	/* Remember if it's a system catalog */
	is_system_catalog = IsSystemRelation(heapRelation);

	/* See whether we're verifying uniqueness/exclusion properties */
	checking_uniqueness = (indexInfo->ii_Unique ||
						   indexInfo->ii_ExclusionOps != NULL);

	/*
	 * "Any visible" mode is not compatible with uniqueness checks; make sure
	 * only one of those is requested.
	 */
	Assert(!(anyvisible && checking_uniqueness));

	/*
	 * Need an EState for evaluation of index expressions and partial-index
	 * predicates.  Also a slot to hold the current tuple.
	 */
	estate = CreateExecutorState();
	econtext = GetPerTupleExprContext(estate);
	slot = table_slot_create(heapRelation, NULL);

	/* Arrange for econtext's scan tuple to be the tuple under test */
	econtext->ecxt_scantuple = slot;

	/* Set up execution state for predicate, if any. */
	predicate = ExecPrepareQual(indexInfo->ii_Predicate, estate);

	/*
	 * Prepare for scan of the base relation.  In a normal index build, we use
	 * SnapshotAny because we must retrieve all tuples and do our own time
	 * qual checks (because we have to index RECENTLY_DEAD tuples). In a
	 * concurrent build, or during bootstrap, we take a regular MVCC snapshot
	 * and index whatever's live according to that.
	 */
	OldestXmin = InvalidTransactionId;

	/* okay to ignore lazy VACUUMs here */
	if (!IsBootstrapProcessingMode() && !indexInfo->ii_Concurrent)
		OldestXmin = GetOldestXmin(heapRelation, PROCARRAY_FLAGS_VACUUM);

	if (!scan)
	{
		/*
		 * Serial index build.
		 *
		 * Must begin our own heap scan in this case.  We may also need to
		 * register a snapshot whose lifetime is under our direct control.
		 */
		if (!TransactionIdIsValid(OldestXmin))
		{
			snapshot = RegisterSnapshot(GetTransactionSnapshot());
			need_unregister_snapshot = true;
		}
		else
			snapshot = SnapshotAny;

		scan = table_beginscan_strat(heapRelation,	/* relation */
									 snapshot,	/* snapshot */
									 0, /* number of keys */
									 NULL,	/* scan key */
									 true,	/* buffer access strategy OK */
									 allow_sync);	/* syncscan OK? */
	}
	else
	{
		/*
		 * Parallel index build.
		 *
		 * Parallel case never registers/unregisters own snapshot.  Snapshot
		 * is taken from parallel heap scan, and is SnapshotAny or an MVCC
		 * snapshot, based on same criteria as serial case.
		 */
		Assert(!IsBootstrapProcessingMode());
		Assert(allow_sync);
		snapshot = scan->rs_snapshot;
	}

	aoscan = (AppendOnlyScanDesc) scan;

	/* GPDB_12_MERGE_FIXME */
#if 0
	/* Publish number of blocks to scan */
	if (progress)
	{
		BlockNumber nblocks;

		if (aoscan->rs_base.rs_parallel != NULL)
		{
			ParallelBlockTableScanDesc pbscan;

			pbscan = (ParallelBlockTableScanDesc) aoscan->rs_base.rs_parallel;
			nblocks = pbscan->phs_nblocks;
		}
		else
			nblocks = aoscan->rs_nblocks;

		pgstat_progress_update_param(PROGRESS_SCAN_BLOCKS_TOTAL,
									 nblocks);
	}
#endif
	
	/*
	 * Must call GetOldestXmin() with SnapshotAny.  Should never call
	 * GetOldestXmin() with MVCC snapshot. (It's especially worth checking
	 * this for parallel builds, since ambuild routines that support parallel
	 * builds must work these details out for themselves.)
	 */
	Assert(snapshot == SnapshotAny || IsMVCCSnapshot(snapshot));
	Assert(snapshot == SnapshotAny ? TransactionIdIsValid(OldestXmin) :
		   !TransactionIdIsValid(OldestXmin));
	Assert(snapshot == SnapshotAny || !anyvisible);

	/* set our scan endpoints */
	if (!allow_sync)
		heap_setscanlimits(scan, start_blockno, numblocks);
	else
	{
		/* syncscan can only be requested on whole relation */
		Assert(start_blockno == 0);
		Assert(numblocks == InvalidBlockNumber);
	}

	reltuples = 0;

	/*
	 * Scan all tuples in the base relation.
	 */
	while (appendonly_getnextslot(&aoscan->rs_base, ForwardScanDirection, slot))
	{
		bool		tupleIsAlive;
		HeapTuple	heapTuple;
		bool		shouldFree;

		CHECK_FOR_INTERRUPTS();

	/* GPDB_12_MERGE_FIXME */
#if 0
		/* Report scan progress, if asked to. */
		if (progress)
		{
			BlockNumber blocks_done = appendonly_scan_get_blocks_done(aoscan);

			if (blocks_done != previous_blkno)
			{
				pgstat_progress_update_param(PROGRESS_SCAN_BLOCKS_DONE,
											 blocks_done);
				previous_blkno = blocks_done;
			}
		}
#endif
		
		/*
		 * appendonly_getnext did the time qual check
		 *
		 * GPDB_12_MERGE_FIXME: in heapam, we do visibility checks in SnapshotAny case
		 * here. Is that not needed with AO tables?
		 */
		tupleIsAlive = true;
		reltuples += 1;

		MemoryContextReset(econtext->ecxt_per_tuple_memory);

		/*
		 * In a partial index, discard tuples that don't satisfy the
		 * predicate.
		 */
		if (predicate != NULL)
		{
			if (!ExecQual(predicate, econtext))
				continue;
		}

		/*
		 * For the current heap tuple, extract all the attributes we use in
		 * this index, and note which are null.  This also performs evaluation
		 * of any expressions needed.
		 */
		FormIndexDatum(indexInfo,
					   slot,
					   estate,
					   values,
					   isnull);

		/*
		 * You'd think we should go ahead and build the index tuple here, but
		 * some index AMs want to do further processing on the data first.  So
		 * pass the values[] and isnull[] arrays, instead.
		 */

		/* Call the AM's callback routine to process the tuple */
		/* GPDB_12_MERGE_FIXME: need to extract the tuple into a HeapTuple for this.
		 * That's inefficient. Previously, we had modified the callback function in GPDB
		 * to take just ItemPointer argument. */
		heapTuple = ExecFetchSlotHeapTuple(slot, true, &shouldFree);

		callback(indexRelation, heapTuple, values, isnull, tupleIsAlive,
				 callback_state);

		if (shouldFree)
			pfree(heapTuple);
	}

	/* GPDB_12_MERGE_FIXME */
#if 0
	/* Report scan progress one last time. */
	if (progress)
	{
		BlockNumber blks_done;

		if (aoscan->rs_base.rs_parallel != NULL)
		{
			ParallelBlockTableScanDesc pbscan;

			pbscan = (ParallelBlockTableScanDesc) aoscan->rs_base.rs_parallel;
			blks_done = pbscan->phs_nblocks;
		}
		else
			blks_done = aoscan->rs_nblocks;

		pgstat_progress_update_param(PROGRESS_SCAN_BLOCKS_DONE,
									 blks_done);
	}
#endif

	table_endscan(scan);

	/* we can now forget our snapshot, if set and registered by us */
	if (need_unregister_snapshot)
		UnregisterSnapshot(snapshot);

	ExecDropSingleTupleTableSlot(slot);

	FreeExecutorState(estate);

	/* These may have been pointing to the now-gone estate */
	indexInfo->ii_ExpressionsState = NIL;
	indexInfo->ii_PredicateState = NULL;

	return reltuples;
}

static void
appendonly_index_validate_scan(Relation heapRelation,
						   Relation indexRelation,
						   IndexInfo *indexInfo,
						   Snapshot snapshot,
						   ValidateIndexState *state)
{
	TableScanDesc scan;
	AppendOnlyScanDesc aoscan;
	HeapTuple	heapTuple;
	Datum		values[INDEX_MAX_KEYS];
	bool		isnull[INDEX_MAX_KEYS];
	ExprState  *predicate;
	TupleTableSlot *slot;
	EState	   *estate;
	ExprContext *econtext;
	BlockNumber root_blkno = InvalidBlockNumber;
	OffsetNumber root_offsets[MaxHeapTuplesPerPage];
	bool		in_index[MaxHeapTuplesPerPage];

	/* state variables for the merge */
	ItemPointer indexcursor = NULL;
	ItemPointerData decoded;
	bool		tuplesort_empty = false;

	/*
	 * sanity checks
	 */
	Assert(OidIsValid(indexRelation->rd_rel->relam));

	/*
	 * Need an EState for evaluation of index expressions and partial-index
	 * predicates.  Also a slot to hold the current tuple.
	 */
	estate = CreateExecutorState();
	econtext = GetPerTupleExprContext(estate);
	slot = MakeSingleTupleTableSlot(RelationGetDescr(heapRelation),
									&TTSOpsHeapTuple);

	/* Arrange for econtext's scan tuple to be the tuple under test */
	econtext->ecxt_scantuple = slot;

	/* Set up execution state for predicate, if any. */
	predicate = ExecPrepareQual(indexInfo->ii_Predicate, estate);

	/*
	 * Prepare for scan of the base relation.  We need just those tuples
	 * satisfying the passed-in reference snapshot.  We must disable syncscan
	 * here, because it's critical that we read from block zero forward to
	 * match the sorted TIDs.
	 */
	scan = table_beginscan_strat(heapRelation,	/* relation */
								 snapshot,	/* snapshot */
								 0, /* number of keys */
								 NULL,	/* scan key */
								 true,	/* buffer access strategy OK */
								 false);	/* syncscan not OK */
	aoscan = (AppendOnlyScanDesc) scan;

	/* GPDB_12_MERGE_FIXME */
#if 0
	pgstat_progress_update_param(PROGRESS_SCAN_BLOCKS_TOTAL,
								 aoscan->rs_nblocks);
#endif
	
	/*
	 * Scan all tuples matching the snapshot.
	 */
	while ((heapTuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
	{
		ItemPointer heapcursor = &heapTuple->t_self;
		ItemPointerData rootTuple;
		OffsetNumber root_offnum;

		CHECK_FOR_INTERRUPTS();

		state->htups += 1;

	/* GPDB_12_MERGE_FIXME */
#if 0
		if ((previous_blkno == InvalidBlockNumber) ||
			(aoscan->rs_cblock != previous_blkno))
		{
			pgstat_progress_update_param(PROGRESS_SCAN_BLOCKS_DONE,
										 aoscan->rs_cblock);
			previous_blkno = aoscan->rs_cblock;
		}
#endif
		
		/* Convert actual tuple TID to root TID */
		rootTuple = *heapcursor;
		root_offnum = ItemPointerGetOffsetNumber(heapcursor);

		if (HeapTupleIsHeapOnly(heapTuple))
		{
			root_offnum = root_offsets[root_offnum - 1];
			if (!OffsetNumberIsValid(root_offnum))
				ereport(ERROR,
						(errcode(ERRCODE_DATA_CORRUPTED),
						 errmsg_internal("failed to find parent tuple for heap-only tuple at (%u,%u) in table \"%s\"",
										 ItemPointerGetBlockNumber(heapcursor),
										 ItemPointerGetOffsetNumber(heapcursor),
										 RelationGetRelationName(heapRelation))));
			ItemPointerSetOffsetNumber(&rootTuple, root_offnum);
		}

		/*
		 * "merge" by skipping through the index tuples until we find or pass
		 * the current root tuple.
		 */
		while (!tuplesort_empty &&
			   (!indexcursor ||
				ItemPointerCompare(indexcursor, &rootTuple) < 0))
		{
			Datum		ts_val;
			bool		ts_isnull;

			if (indexcursor)
			{
				/*
				 * Remember index items seen earlier on the current heap page
				 */
				if (ItemPointerGetBlockNumber(indexcursor) == root_blkno)
					in_index[ItemPointerGetOffsetNumber(indexcursor) - 1] = true;
			}

			tuplesort_empty = !tuplesort_getdatum(state->tuplesort, true,
												  &ts_val, &ts_isnull, NULL);
			Assert(tuplesort_empty || !ts_isnull);
			if (!tuplesort_empty)
			{
				itemptr_decode(&decoded, DatumGetInt64(ts_val));
				indexcursor = &decoded;

				/* If int8 is pass-by-ref, free (encoded) TID Datum memory */
#ifndef USE_FLOAT8_BYVAL
				pfree(DatumGetPointer(ts_val));
#endif
			}
			else
			{
				/* Be tidy */
				indexcursor = NULL;
			}
		}

		/*
		 * If the tuplesort has overshot *and* we didn't see a match earlier,
		 * then this tuple is missing from the index, so insert it.
		 */
		if ((tuplesort_empty ||
			 ItemPointerCompare(indexcursor, &rootTuple) > 0) &&
			!in_index[root_offnum - 1])
		{
			MemoryContextReset(econtext->ecxt_per_tuple_memory);

			/* Set up for predicate or expression evaluation */
			ExecStoreHeapTuple(heapTuple, slot, false);

			/*
			 * In a partial index, discard tuples that don't satisfy the
			 * predicate.
			 */
			if (predicate != NULL)
			{
				if (!ExecQual(predicate, econtext))
					continue;
			}

			/*
			 * For the current heap tuple, extract all the attributes we use
			 * in this index, and note which are null.  This also performs
			 * evaluation of any expressions needed.
			 */
			FormIndexDatum(indexInfo,
						   slot,
						   estate,
						   values,
						   isnull);

			/*
			 * You'd think we should go ahead and build the index tuple here,
			 * but some index AMs want to do further processing on the data
			 * first. So pass the values[] and isnull[] arrays, instead.
			 */

			/*
			 * If the tuple is already committed dead, you might think we
			 * could suppress uniqueness checking, but this is no longer true
			 * in the presence of HOT, because the insert is actually a proxy
			 * for a uniqueness check on the whole HOT-chain.  That is, the
			 * tuple we have here could be dead because it was already
			 * HOT-updated, and if so the updating transaction will not have
			 * thought it should insert index entries.  The index AM will
			 * check the whole HOT-chain and correctly detect a conflict if
			 * there is one.
			 */

			index_insert(indexRelation,
						 values,
						 isnull,
						 &rootTuple,
						 heapRelation,
						 indexInfo->ii_Unique ?
						 UNIQUE_CHECK_YES : UNIQUE_CHECK_NO,
						 indexInfo);

			state->tups_inserted += 1;
		}
	}

	table_endscan(scan);

	ExecDropSingleTupleTableSlot(slot);

	FreeExecutorState(estate);

	/* These may have been pointing to the now-gone estate */
	indexInfo->ii_ExpressionsState = NIL;
	indexInfo->ii_PredicateState = NULL;
}

/* ------------------------------------------------------------------------
 * Miscellaneous callbacks for the heap AM
 * ------------------------------------------------------------------------
 */

/*
 * This pretends that the all the space is taken by the main fork.
 */
static uint64
appendonly_relation_size(Relation rel, ForkNumber forkNumber)
{
	Relation	pg_aoseg_rel;
	TupleDesc	pg_aoseg_dsc;
	HeapTuple	tuple;
	SysScanDesc aoscan;
	int64		result;
	Datum		eof;
	bool		isNull;

	if (forkNumber != MAIN_FORKNUM)
		return 0;

	result = 0;

	pg_aoseg_rel = table_open(rel->rd_appendonly->segrelid, AccessShareLock);
	pg_aoseg_dsc = RelationGetDescr(pg_aoseg_rel);

	aoscan = systable_beginscan(pg_aoseg_rel, InvalidOid, true, NULL, 0, NULL);

	while ((tuple = systable_getnext(aoscan)) != NULL)
	{
		eof = fastgetattr(tuple, Anum_pg_aoseg_eof, pg_aoseg_dsc, &isNull);
		Assert(!isNull);

		result += DatumGetInt64(eof);

		CHECK_FOR_INTERRUPTS();
	}

	systable_endscan(aoscan);
	table_close(pg_aoseg_rel, AccessShareLock);

	return result;
}

/*
 * Check to see whether the table needs a TOAST table.  It does only if
 * (1) there are any toastable attributes, and (2) the maximum length
 * of a tuple could exceed TOAST_TUPLE_THRESHOLD.  (We don't want to
 * create a toast table for something like "f1 varchar(20)".)
 */
static bool
appendonly_relation_needs_toast_table(Relation rel)
{
	int32		data_length = 0;
	bool		maxlength_unknown = false;
	bool		has_toastable_attrs = false;
	TupleDesc	tupdesc = rel->rd_att;
	int32		tuple_length;
	int			i;

	for (i = 0; i < tupdesc->natts; i++)
	{
		Form_pg_attribute att = TupleDescAttr(tupdesc, i);

		if (att->attisdropped)
			continue;
		data_length = att_align_nominal(data_length, att->attalign);
		if (att->attlen > 0)
		{
			/* Fixed-length types are never toastable */
			data_length += att->attlen;
		}
		else
		{
			int32		maxlen = type_maximum_size(att->atttypid,
												   att->atttypmod);

			if (maxlen < 0)
				maxlength_unknown = true;
			else
				data_length += maxlen;
			if (att->attstorage != 'p')
				has_toastable_attrs = true;
		}
	}
	if (!has_toastable_attrs)
		return false;			/* nothing to toast? */
	if (maxlength_unknown)
		return true;			/* any unlimited-length attrs? */
	tuple_length = MAXALIGN(SizeofHeapTupleHeader +
							BITMAPLEN(tupdesc->natts)) +
		MAXALIGN(data_length);
	return (tuple_length > TOAST_TUPLE_THRESHOLD);
}


/* ------------------------------------------------------------------------
 * Planner related callbacks for the heap AM
 * ------------------------------------------------------------------------
 */

static void
appendonly_estimate_rel_size(Relation rel, int32 *attr_widths,
						 BlockNumber *pages, double *tuples,
						 double *allvisfrac)
{
	BlockNumber curpages;
	BlockNumber relpages;
	double		reltuples;
	BlockNumber relallvisible;
	double		density;

	/* it has storage, ok to call the smgr */
	curpages = RelationGetNumberOfBlocks(rel);

	/* coerce values in pg_class to more desirable types */
	relpages = (BlockNumber) rel->rd_rel->relpages;
	reltuples = (double) rel->rd_rel->reltuples;
	relallvisible = (BlockNumber) rel->rd_rel->relallvisible;

	/*
	 * HACK: if the relation has never yet been vacuumed, use a minimum size
	 * estimate of 10 pages.  The idea here is to avoid assuming a
	 * newly-created table is really small, even if it currently is, because
	 * that may not be true once some data gets loaded into it.  Once a vacuum
	 * or analyze cycle has been done on it, it's more reasonable to believe
	 * the size is somewhat stable.
	 *
	 * (Note that this is only an issue if the plan gets cached and used again
	 * after the table has been filled.  What we're trying to avoid is using a
	 * nestloop-type plan on a table that has grown substantially since the
	 * plan was made.  Normally, autovacuum/autoanalyze will occur once enough
	 * inserts have happened and cause cached-plan invalidation; but that
	 * doesn't happen instantaneously, and it won't happen at all for cases
	 * such as temporary tables.)
	 *
	 * We approximate "never vacuumed" by "has relpages = 0", which means this
	 * will also fire on genuinely empty relations.  Not great, but
	 * fortunately that's a seldom-seen case in the real world, and it
	 * shouldn't degrade the quality of the plan too much anyway to err in
	 * this direction.
	 *
	 * If the table has inheritance children, we don't apply this heuristic.
	 * Totally empty parent tables are quite common, so we should be willing
	 * to believe that they are empty.
	 */
	if (curpages < 10 &&
		relpages == 0 &&
		!rel->rd_rel->relhassubclass)
		curpages = 10;

	/* report estimated # pages */
	*pages = curpages;
	/* quick exit if rel is clearly empty */
	if (curpages == 0)
	{
		*tuples = 0;
		*allvisfrac = 0;
		return;
	}

	/* estimate number of tuples from previous tuple density */
	if (relpages > 0)
		density = reltuples / (double) relpages;
	else
	{
		/*
		 * When we have no data because the relation was truncated, estimate
		 * tuple width from attribute datatypes.  We assume here that the
		 * pages are completely full, which is OK for tables (since they've
		 * presumably not been VACUUMed yet) but is probably an overestimate
		 * for indexes.  Fortunately get_relation_info() can clamp the
		 * overestimate to the parent table's size.
		 *
		 * Note: this code intentionally disregards alignment considerations,
		 * because (a) that would be gilding the lily considering how crude
		 * the estimate is, and (b) it creates platform dependencies in the
		 * default plans which are kind of a headache for regression testing.
		 */
		int32		tuple_width;

		tuple_width = get_rel_data_width(rel, attr_widths);
		tuple_width += MAXALIGN(SizeofHeapTupleHeader);
		tuple_width += sizeof(ItemIdData);
		/* note: integer division is intentional here */
		density = (BLCKSZ - SizeOfPageHeaderData) / tuple_width;
	}
	*tuples = rint(density * (double) curpages);

	/*
	 * We use relallvisible as-is, rather than scaling it up like we do for
	 * the pages and tuples counts, on the theory that any pages added since
	 * the last VACUUM are most likely not marked all-visible.  But costsize.c
	 * wants it converted to a fraction.
	 */
	if (relallvisible == 0 || curpages <= 0)
		*allvisfrac = 0;
	else if ((double) relallvisible >= curpages)
		*allvisfrac = 1;
	else
		*allvisfrac = (double) relallvisible / curpages;
}


/* ------------------------------------------------------------------------
 * Executor related callbacks for the heap AM
 * ------------------------------------------------------------------------
 */

static bool
appendonly_scan_bitmap_next_block(TableScanDesc scan,
								  TBMIterateResult *tbmres)
{
	//AppendOnlyScanDesc aoscan = (AppendOnlyScanDesc) scan;

	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "bitmap scan on AO table not yet implemented");
}

static bool
appendonly_scan_bitmap_next_tuple(TableScanDesc scan,
							  TBMIterateResult *tbmres,
							  TupleTableSlot *slot)
{
	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "bitmap scan on AO table not yet implemented");
}

static bool
appendonly_scan_sample_next_block(TableScanDesc scan, SampleScanState *scanstate)
{
	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "sample scan on AO table not yet implemented");
}

static bool
appendonly_scan_sample_next_tuple(TableScanDesc scan, SampleScanState *scanstate,
							  TupleTableSlot *slot)
{
	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "sample scan on AO table not yet implemented");
}


/* ----------------------------------------------------------------------------
 *  Helper functions for the above.
 * ----------------------------------------------------------------------------
 */

/*
 * Reconstruct and rewrite the given tuple
 *
 * We cannot simply copy the tuple as-is, for several reasons:
 *
 * 1. We'd like to squeeze out the values of any dropped columns, both
 * to save space and to ensure we have no corner-case failures. (It's
 * possible for example that the new table hasn't got a TOAST table
 * and so is unable to store any large values of dropped cols.)
 *
 * 2. The tuple might not even be legal for the new table; this is
 * currently only known to happen as an after-effect of ALTER TABLE
 * SET WITHOUT OIDS.
 *
 * So, we must reconstruct the tuple from component Datums.
 */
static void
reform_and_rewrite_tuple(HeapTuple tuple,
						 Relation OldHeap, Relation NewHeap,
						 Datum *values, bool *isnull, RewriteState rwstate)
{
	TupleDesc	oldTupDesc = RelationGetDescr(OldHeap);
	TupleDesc	newTupDesc = RelationGetDescr(NewHeap);
	HeapTuple	copiedTuple;
	int			i;

	heap_deform_tuple(tuple, oldTupDesc, values, isnull);

	/* Be sure to null out any dropped columns */
	for (i = 0; i < newTupDesc->natts; i++)
	{
		if (TupleDescAttr(newTupDesc, i)->attisdropped)
			isnull[i] = true;
	}

	copiedTuple = heap_form_tuple(newTupDesc, values, isnull);

	/* The heap rewrite module does the rest */
	rewrite_heap_tuple(rwstate, tuple, copiedTuple);

	heap_freetuple(copiedTuple);
}


/* ------------------------------------------------------------------------
 * Definition of the appendonly table access method.
 * ------------------------------------------------------------------------
 */

static const TableAmRoutine appendonly_methods = {
	.type = T_TableAmRoutine,

	.slot_callbacks = appendonly_slot_callbacks,

	.scan_begin = appendonly_beginscan,
	.scan_end = appendonly_endscan,
	.scan_rescan = appendonly_rescan,
	.scan_getnextslot = appendonly_getnextslot,

	.parallelscan_estimate = table_block_parallelscan_estimate,
	.parallelscan_initialize = table_block_parallelscan_initialize,
	.parallelscan_reinitialize = table_block_parallelscan_reinitialize,

	.index_fetch_begin = appendonly_index_fetch_begin,
	.index_fetch_reset = appendonly_index_fetch_reset,
	.index_fetch_end = appendonly_index_fetch_end,
	.index_fetch_tuple = appendonly_index_fetch_tuple,

	.tuple_insert = appendonly_tuple_insert,
	.tuple_insert_speculative = appendonly_tuple_insert_speculative,
	.tuple_complete_speculative = appendonly_tuple_complete_speculative,
	.multi_insert = appendonly_multi_insert,
	.tuple_delete = appendonly_tuple_delete,
	.tuple_update = appendonly_tuple_update,
	.tuple_lock = appendonly_tuple_lock,
	.finish_bulk_insert = appendonly_finish_bulk_insert,

	.tuple_fetch_row_version = appendonly_fetch_row_version,
	.tuple_get_latest_tid = appendonly_get_latest_tid,
	.tuple_tid_valid = appendonly_tuple_tid_valid,
	.tuple_satisfies_snapshot = appendonly_tuple_satisfies_snapshot,
	.compute_xid_horizon_for_tuples = appendonly_compute_xid_horizon_for_tuples,

	.relation_set_new_filenode = appendonly_relation_set_new_filenode,
	.relation_nontransactional_truncate = appendonly_relation_nontransactional_truncate,
	.relation_copy_data = appendonly_relation_copy_data,
	.relation_copy_for_cluster = appendonly_relation_copy_for_cluster,
	.relation_vacuum = appendonly_vacuum_rel,
	.scan_analyze_next_block = appendonly_scan_analyze_next_block,
	.scan_analyze_next_tuple = appendonly_scan_analyze_next_tuple,
	.index_build_range_scan = appendonly_index_build_range_scan,
	.index_validate_scan = appendonly_index_validate_scan,

	.relation_size = appendonly_relation_size,
	.relation_needs_toast_table = appendonly_relation_needs_toast_table,

	.relation_estimate_size = appendonly_estimate_rel_size,

	.scan_bitmap_next_block = appendonly_scan_bitmap_next_block,
	.scan_bitmap_next_tuple = appendonly_scan_bitmap_next_tuple,
	.scan_sample_next_block = appendonly_scan_sample_next_block,
	.scan_sample_next_tuple = appendonly_scan_sample_next_tuple
};

Datum
appendoptimized_tableam_handler(PG_FUNCTION_ARGS)
{
	PG_RETURN_POINTER(&appendonly_methods);
}
