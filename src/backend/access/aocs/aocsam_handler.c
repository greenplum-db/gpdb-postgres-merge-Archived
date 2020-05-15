/*--------------------------------------------------------------------------
 *
 * aocsam_handler.c
 *	  Append only columnar access methods handler
 *
 * Portions Copyright (c) 2009-2010, Greenplum Inc.
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/backend/access/aocs/aocsam_handler.c
 *
 *--------------------------------------------------------------------------
 */
#include "postgres.h"
#include "access/tableam.h"
#include "cdb/cdbaocsam.h"

#include "utils/builtins.h"
#include "access/aomd.h"
#include "access/appendonlywriter.h"
#include "access/genam.h"
#include "access/heapam.h"
#include "access/multixact.h"
#include "access/rewriteheap.h"
#include "access/tableam.h"
#include "access/tsmapi.h"
#include "access/tuptoaster.h"
#include "access/xact.h"
#include "catalog/pg_appendonly_fn.h"
#include "catalog/catalog.h"
#include "catalog/heap.h"
#include "catalog/index.h"
#include "catalog/storage.h"
#include "catalog/storage_xlog.h"
#include "cdb/cdbappendonlyam.h"
#include "cdb/cdbvars.h"
#include "commands/progress.h"
#include "commands/vacuum.h"
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
#include "utils/faultinjector.h"
#include "utils/rel.h"
#include "nodes/nodeFuncs.h"

typedef struct AOCODMLState
{
	Oid relationOid;
	AOCSInsertDesc insertDesc;
	AOCSDeleteDesc deleteDesc;
} AOCODMLState;

static void reset_state_cb(void *arg);
/*
 * GPDB_12_MERGE_FIXME: This is a temporary state of things. A locally stored
 * state is needed currently because there is no viable place to store this
 * information outside of the table access method. Ideally the caller should be
 * responsible for initializing a state and passing it over using the table
 * access method api.
 *
 * Until this is in place, the local state is not to be accessed directly but
 * only via the *_dml_state functions.
 * It contains:
 *		a quick look up member for the common case
 *		a hash table which keeps per relation information
 *		a memory context that should be long lived enough and is
 *			responsible for reseting the state via its reset cb
 */
static struct AOCOLocal
{
	AOCODMLState           *last_used_state;
	HTAB				   *dmlDescriptorTab;

	MemoryContext			stateCxt;
	MemoryContextCallback	cb;
} aocoLocal = {
	.last_used_state  = NULL,
	.dmlDescriptorTab = NULL,

	.stateCxt		  = NULL,
	.cb				  = {
		.func	= reset_state_cb,
		.arg	= NULL
	},
};

/*
 * There are two cases that we are called from, during context destruction
 * after a successful completion and after a transaction abort. Only in the
 * second case we should not have cleaned up the DML state and the entries in
 * the hash table. We need to reset our global state. The actual clean up is
 * taken care elsewhere.
 */
static void
reset_state_cb(void *arg)
{
	aocoLocal.dmlDescriptorTab = NULL;
	aocoLocal.last_used_state = NULL;
	aocoLocal.stateCxt = NULL;
}

static void
init_dml_local_state(void)
{
	HASHCTL hash_ctl;

	if (!aocoLocal.dmlDescriptorTab)
	{
		Assert(aocoLocal.stateCxt == NULL);
		aocoLocal.stateCxt = AllocSetContextCreate(
			CurrentMemoryContext,
			"AppendOnly DML State Context",
			ALLOCSET_SMALL_SIZES);
		MemoryContextRegisterResetCallback(
			aocoLocal.stateCxt,
			&aocoLocal.cb);

		memset(&hash_ctl, 0, sizeof(hash_ctl));
		hash_ctl.keysize = sizeof(Oid);
		hash_ctl.entrysize = sizeof(AOCODMLState);
		hash_ctl.hcxt = aocoLocal.stateCxt;
		aocoLocal.dmlDescriptorTab =
			hash_create("AppendOnly DML state", 128, &hash_ctl,
			            HASH_CONTEXT | HASH_ELEM | HASH_BLOBS);
	}
}

/*
 * Create and insert a state entry for a relation. The actual descriptors will
 * be created lazily when/if needed.
 *
 * Should be called exactly once per relation.
 */
static inline AOCODMLState *
enter_dml_state(const Oid relationOid)
{
	AOCODMLState *state;
	bool				found;

	Assert(aocoLocal.dmlDescriptorTab);

	state = (AOCODMLState *) hash_search(
		aocoLocal.dmlDescriptorTab,
		&relationOid,
		HASH_ENTER,
		&found);

	Assert(!found);

	state->insertDesc = NULL;
	state->deleteDesc = NULL;

	aocoLocal.last_used_state = state;
	return state;
}

/*
 * Retrieve the state information for a relation.
 * It is required that the state has been created before hand.
 */
static inline AOCODMLState *
find_dml_state(const Oid relationOid)
{
	AOCODMLState *state;
	Assert(aocoLocal.dmlDescriptorTab);

	if (aocoLocal.last_used_state &&
		aocoLocal.last_used_state->relationOid == relationOid)
		return aocoLocal.last_used_state;

	state = (AOCODMLState *) hash_search(
		aocoLocal.dmlDescriptorTab,
		&relationOid,
		HASH_FIND,
		NULL);

	Assert(state);

	aocoLocal.last_used_state = state;
	return state;
}

/*
 * Remove the state information for a relation.
 * It is required that the state has been created before hand.
 *
 * Should be called exactly once per relation.
 */
static inline AOCODMLState *
remove_dml_state(const Oid relationOid)
{
	AOCODMLState *state;
	Assert(aocoLocal.dmlDescriptorTab);

	state = (AOCODMLState *) hash_search(
		aocoLocal.dmlDescriptorTab,
		&relationOid,
		HASH_REMOVE,
		NULL);

	Assert(state);

	if (aocoLocal.last_used_state &&
		aocoLocal.last_used_state->relationOid == relationOid)
		aocoLocal.last_used_state = NULL;

	return state;
}

/*
 * Although the operation param is superfluous at the momment, the signature of
 * the function is such for balance between the init and finish.
 *
 * This function should be called exactly once per relation.
 */
void
aoco_dml_init(Relation relation, CmdType operation)
{
	init_dml_local_state();
	(void) enter_dml_state(RelationGetRelid(relation));
}

/*
 * This function should be called exactly once per relation.
 */
void
aoco_dml_finish(Relation relation, CmdType operation)
{
	AOCODMLState *state;

	state = remove_dml_state(RelationGetRelid(relation));

	if (state->deleteDesc)
	{
		aocs_delete_finish(state->deleteDesc);
		state->deleteDesc = NULL;

		/*
		 * Bump up the modcount. If we inserted something (meaning that
		 * this was an UPDATE), we can skip this, as the insertion bumped
		 * up the modcount already.
		 */
		if (!state->insertDesc)
			AORelIncrementModCount(relation);
	}

	if (state->insertDesc)
	{
		Assert(state->insertDesc->aoi_rel == relation);
		aocs_insert_finish(state->insertDesc);
		state->insertDesc = NULL;
	}

}

/*
 * Retrieve the insertDescriptor for a relation. Initialize it if needed.
 */
static AOCSInsertDesc
get_insert_descriptor(const Relation relation, bool for_update)
{
	AOCODMLState *state;

	state = find_dml_state(RelationGetRelid(relation));

	if (state->insertDesc == NULL)
	{
		MemoryContext oldcxt;

		oldcxt = MemoryContextSwitchTo(aocoLocal.stateCxt);
		state->insertDesc = aocs_insert_init(relation,
		                                     ChooseSegnoForWrite(relation),
		                                     for_update);
		MemoryContextSwitchTo(oldcxt);
	}

	return state->insertDesc;
}

static Datum
tts_aocovirtual_getsysattr (TupleTableSlot *slot, int attnum, bool *isnull)
{
	Datum result;

	if (attnum == GpSegmentIdAttributeNumber)
	{
		*isnull = false;
		result = Int32GetDatum(GpIdentity.segindex);
	}
	else
		result = TTSOpsVirtual.getsysattr(slot, attnum, isnull);

	return result;
}

/*
 * AOCO access method uses virtual tuples with some minor modifications.
 */
static const TupleTableSlotOps *
aoco_slot_callbacks(Relation relation)
{
	TupleTableSlotOps *aoSlotOps = palloc(sizeof(TupleTableSlotOps));

	*aoSlotOps = TTSOpsVirtual;
	aoSlotOps->getsysattr = tts_aocovirtual_getsysattr;
	return (const TupleTableSlotOps*) aoSlotOps;
}

static TableScanDesc
aoco_beginscan(Relation relation,
               Snapshot snapshot,
               int nkeys, struct ScanKeyData *key,
               ParallelTableScanDesc pscan,
               uint32 flags)
{
	Snapshot	aocsMetaDataSnapshot;
	AOCSScanDesc aoscan;
	aocsMetaDataSnapshot = snapshot;
	if (aocsMetaDataSnapshot== SnapshotAny)
	{
		/*
		 * the append-only meta data should never be fetched with
		 * SnapshotAny as bogus results are returned.
		 */
		aocsMetaDataSnapshot = GetTransactionSnapshot();
	}

	aoscan = aocs_beginscan(relation,
	                        snapshot,
	                        aocsMetaDataSnapshot,
	                        NULL,
	                        (bool *)key);

	aoscan->rs_base.rs_rd = relation;
	aoscan->rs_base.rs_snapshot = snapshot;
	aoscan->rs_base.rs_nkeys = nkeys;
	aoscan->rs_base.rs_flags = flags;
	aoscan->rs_base.rs_parallel = pscan;

	return (TableScanDesc) aoscan;
}

static void
aoco_endscan(TableScanDesc scan)
{
	aocs_endscan((AOCSScanDesc) scan);
}

static void
aoco_rescan(TableScanDesc scan, ScanKey key,
                  bool set_params, bool allow_strat,
                  bool allow_sync, bool allow_pagemode)
{
	elog(ERROR, "not implemented yet");

}

static bool
aoco_getnextslot(TableScanDesc scan, ScanDirection direction, TupleTableSlot *slot)
{
	AOCSScanDesc  aoscan = (AOCSScanDesc) scan;

	ExecClearTuple(slot);
	if (aocs_getnext(aoscan,direction,slot))
	{
		ExecStoreVirtualTuple(slot);
		pgstat_count_heap_getnext(aoscan->aos_rel);

		return true;
	}
	else
	{
		if (slot)
			ExecClearTuple(slot);

		return false;
	}

}

static Size
aoco_parallelscan_estimate(Relation rel)
{
	elog(ERROR, "parallel SeqScan not implemented for AO or AOCO tables");
}

static Size
aoco_parallelscan_initialize(Relation rel, ParallelTableScanDesc pscan)
{
	elog(ERROR, "parallel SeqScan not implemented for AO or AOCO tables");
}

static void
aoco_parallelscan_reinitialize(Relation rel, ParallelTableScanDesc pscan)
{
	elog(ERROR, "parallel SeqScan not implemented for AO or AOCO tables");
}

static IndexFetchTableData *
aoco_index_fetch_begin(Relation rel)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_index_fetch_reset(IndexFetchTableData *scan)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_index_fetch_end(IndexFetchTableData *scan)
{
	elog(ERROR, "not implemented yet");
}

static bool
aoco_index_fetch_tuple(struct IndexFetchTableData *scan,
                             ItemPointer tid,
                             Snapshot snapshot,
                             TupleTableSlot *slot,
                             bool *call_again, bool *all_dead)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_tuple_insert(Relation relation, TupleTableSlot *slot, CommandId cid,
                        int options, BulkInsertState bistate)
{

	AOCSInsertDesc          insertDesc;
	bool					shouldFree = true;

	insertDesc = get_insert_descriptor(relation, false);

	aocs_insert(insertDesc, slot);

	pgstat_count_heap_insert(relation, 1);
}

static void
aoco_tuple_insert_speculative(Relation relation, TupleTableSlot *slot,
                                    CommandId cid, int options,
                                    BulkInsertState bistate, uint32 specToken)
{
	/* GPDB_12_MERGE_FIXME: not supported. Can this function be left out completely? Or ereport()? */
	elog(ERROR, "speculative insertion not supported on AOCO tables");
}

static void
aoco_tuple_complete_speculative(Relation relation, TupleTableSlot *slot,
                                      uint32 specToken, bool succeeded)
{
	elog(ERROR, "speculative insertion not supported on AOCO tables");
}

/*
 *	aoco_multi_insert	- insert multiple tuples into an ao relation
 *
 * This is like aoco_tuple_insert(), but inserts multiple tuples in one
 * operation. Typicaly used by COPY. This is preferrable than calling
 * aoco_tuple_insert() in a loop because ... WAL??
 */
static void
aoco_multi_insert(Relation relation, TupleTableSlot **slots, int ntuples,
                        CommandId cid, int options, BulkInsertState bistate)
{
	/*
	* GPDB_12_MERGE_FIXME: Poor man's implementation for now in order to make
		* the tests pass. Implement properly.
		*/
	for (int i = 0; i < ntuples; i++)
		aoco_tuple_insert(relation, slots[i], cid, options, bistate);
}

static TM_Result
aoco_tuple_delete(Relation relation, ItemPointer tid, CommandId cid,
                        Snapshot snapshot, Snapshot crosscheck, bool wait,
                        TM_FailureData *tmfd, bool changingPart)
{
	elog(ERROR, "not implemented yet");
}


static TM_Result
aoco_tuple_update(Relation relation, ItemPointer otid, TupleTableSlot *slot,
                        CommandId cid, Snapshot snapshot, Snapshot crosscheck,
                        bool wait, TM_FailureData *tmfd,
                        LockTupleMode *lockmode, bool *update_indexes)
{
	elog(ERROR, "not implemented yet");
}

static TM_Result
aoco_tuple_lock(Relation relation, ItemPointer tid, Snapshot snapshot,
                      TupleTableSlot *slot, CommandId cid, LockTupleMode mode,
                      LockWaitPolicy wait_policy, uint8 flags,
                      TM_FailureData *tmfd)
{
	/* GPDB_12_MERGE_FIXME: not supported. Can this function be left out completely? Or ereport()? */
	elog(ERROR, "speculative insertion not supported on AO tables");
}

static void
aoco_finish_bulk_insert(Relation relation, int options)
{
	elog(ERROR, "not implemented yet");
}




/* ------------------------------------------------------------------------
 * Callbacks for non-modifying operations on individual tuples for heap AM
 * ------------------------------------------------------------------------
 */

static bool
aoco_fetch_row_version(Relation relation,
                             ItemPointer tid,
                             Snapshot snapshot,
                             TupleTableSlot *slot)
{
	/*
	 * This is a generic interface. It is currently used in three distinct
	 * cases, only one of which is currently invoking it for AO tables.
	 * This is DELETE RETURNING. In order to return the slot via the tid for
	 * AO tables one would have to scan the block directory and the visibility
	 * map. A block directory is not guarranteed to exist. Even if it exists, a
	 * state would have to be created and dropped for every tuple look up since
	 * this interface does not allow for the state to be passed around. This is
	 * a very costly operation to be performed per tuple lookup. Furthermore, if
	 * a DELETE operation is currently on the fly, the corresponding visibility
	 * map entries will not have been finalized into a visibility map tuple.
	 *
	 * Error out with feature not supported. Given that this is a generic
	 * interface, we can not really say which feature is that, although we do
	 * know that is DELETE RETURNING.
	 */
	ereport(ERROR,
	        (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		        errmsg("feature not supported on appendoptimized relations")));
}

static void
aoco_get_latest_tid(TableScanDesc sscan,
                          ItemPointer tid)
{
	/*
	 * Tid scans are not supported for appendoptimized relation. This function
	 * should not have been called in the first place, but if it is called,
	 * better to error out.
	 */
	ereport(ERROR,
	        (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		        errmsg("feature not supported on appendoptimized relations")));
}

static bool
aoco_tuple_tid_valid(TableScanDesc scan, ItemPointer tid)
{
	/*
	 * Tid scans are not supported for appendoptimized relation. This function
	 * should not have been called in the first place, but if it is called,
	 * better to error out.
	 */
	ereport(ERROR,
	        (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		        errmsg("feature not supported on appendoptimized relations")));
}

static bool
aoco_tuple_satisfies_snapshot(Relation rel, TupleTableSlot *slot,
                                    Snapshot snapshot)
{
	/*
	 * AOCO table dose not support unique and tidscan yet.
	 */
	ereport(ERROR,
	        (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
		        errmsg("feature not supported on appendoptimized relations")));
}

static TransactionId
aoco_compute_xid_horizon_for_tuples(Relation rel,
                                          ItemPointerData *tids,
                                          int nitems)
{
	// GPDB_12_MERGE_FIXME: vacuum related call back.
	elog(ERROR, "not implemented yet");
}

static void
aoco_relation_nontransactional_truncate(Relation rel)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_relation_copy_data(Relation rel, const RelFileNode *newrnode)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_vacuum_rel(Relation onerel, VacuumParams *params,
                      BufferAccessStrategy bstrategy)
{
	elog(ERROR, "not implemented yet");
}

static void
aoco_relation_copy_for_cluster(Relation OldHeap, Relation NewHeap,
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
	elog(ERROR, "CLUSTER not supported on AOCO tables");
}

static bool
aoco_scan_analyze_next_block(TableScanDesc scan, BlockNumber blockno,
                                   BufferAccessStrategy bstrategy)
{
	return false;
}

static bool
aoco_scan_analyze_next_tuple(TableScanDesc scan, TransactionId OldestXmin,
                                   double *liverows, double *deadrows,
                                   TupleTableSlot *slot)
{
	return false;
}

static double
aoco_index_build_range_scan(Relation heapRelation,
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
	elog(ERROR, "not implemented yet");
}

static void
aoco_index_validate_scan(Relation heapRelation,
                               Relation indexRelation,
                               IndexInfo *indexInfo,
                               Snapshot snapshot,
                               ValidateIndexState *state)
{
	elog(ERROR, "not implemented yet");
}

/* ------------------------------------------------------------------------
 * Miscellaneous callbacks for the heap AM
 * ------------------------------------------------------------------------
 */

/*
 * This pretends that the all the space is taken by the main fork.
 */
static uint64
aoco_relation_size(Relation rel, ForkNumber forkNumber)
{
	return BLCKSZ;
}

static bool
aoco_relation_needs_toast_table(Relation rel)
{

	/*
	 * GPDB_12_MERGE_FIXME: does the AOCO relation really needs a toast table?
	 * Deactivate for now.
	 */
	return false;
}


/* ------------------------------------------------------------------------
 * Planner related callbacks for the heap AM
 * ------------------------------------------------------------------------
 */
static void
aoco_estimate_rel_size(Relation rel, int32 *attr_widths,
                             BlockNumber *pages, double *tuples,
                             double *allvisfrac)
{
	/*
	 * GPDB_12_MERGE_FIXME
	 *
	 * GetAllFileSegInfo()
	 *    sum up eof values
	 *    sum up tuple counts
	 *
	 * Find a suitable visimap interface to compute allvisfrac
	 *
	 * QD has no content, so the above information needs to be obtained from
	 * QEs.  Obtaining from only one QE should be ok, given that this is an
	 * estimate.
	 */
	*pages = 1;
	*tuples = 1;
	*allvisfrac = 0;
}

/* ------------------------------------------------------------------------
 * Executor related callbacks for the heap AM
 * ------------------------------------------------------------------------
 */
static bool
aoco_scan_bitmap_next_block(TableScanDesc scan,
                                  TBMIterateResult *tbmres)
{
	elog(ERROR, "not implemented yet");
}

static bool
aoco_scan_bitmap_next_tuple(TableScanDesc scan,
                                  TBMIterateResult *tbmres,
                                  TupleTableSlot *slot)
{
	elog(ERROR, "not implemented yet");
}

static bool
aoco_scan_sample_next_block(TableScanDesc scan, SampleScanState *scanstate)
{
	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "sample scan on AOCO table not yet implemented");
}

static bool
aoco_scan_sample_next_tuple(TableScanDesc scan, SampleScanState *scanstate,
                                  TupleTableSlot *slot)
{
	// GPDB_12_MERGE_FIXME: re-implement this
	elog(ERROR, "sample scan on AOCO table not yet implemented");
}
/* ------------------------------------------------------------------------
 * Definition of the AOCO table access method.
 * ------------------------------------------------------------------------
 */
static const TableAmRoutine aoco_methods = {
	.type = T_TableAmRoutine,
	.slot_callbacks = aoco_slot_callbacks,

	.scan_begin = aoco_beginscan,
	.scan_end = aoco_endscan,
	.scan_rescan = aoco_rescan,
	.scan_getnextslot = aoco_getnextslot,

	.parallelscan_estimate = aoco_parallelscan_estimate,
	.parallelscan_initialize = aoco_parallelscan_initialize,
	.parallelscan_reinitialize = aoco_parallelscan_reinitialize,

	.index_fetch_begin = aoco_index_fetch_begin,
	.index_fetch_reset = aoco_index_fetch_reset,
	.index_fetch_end = aoco_index_fetch_end,
	.index_fetch_tuple = aoco_index_fetch_tuple,

	.tuple_insert = aoco_tuple_insert,
	.tuple_insert_speculative = aoco_tuple_insert_speculative,
	.tuple_complete_speculative = aoco_tuple_complete_speculative,
	.multi_insert = aoco_multi_insert,
	.tuple_delete = aoco_tuple_delete,
	.tuple_update = aoco_tuple_update,
	.tuple_lock = aoco_tuple_lock,
	.finish_bulk_insert = aoco_finish_bulk_insert,

	.tuple_fetch_row_version = aoco_fetch_row_version,
	.tuple_get_latest_tid = aoco_get_latest_tid,
	.tuple_tid_valid = aoco_tuple_tid_valid,
	.tuple_satisfies_snapshot = aoco_tuple_satisfies_snapshot,
	.compute_xid_horizon_for_tuples = aoco_compute_xid_horizon_for_tuples,

	.relation_set_new_filenode = appendonly_relation_set_new_filenode, /* shared with AO */
	.relation_nontransactional_truncate = aoco_relation_nontransactional_truncate,
	.relation_copy_data = aoco_relation_copy_data,
	.relation_copy_for_cluster = aoco_relation_copy_for_cluster,
	.relation_vacuum = aoco_vacuum_rel,
	.scan_analyze_next_block = aoco_scan_analyze_next_block,
	.scan_analyze_next_tuple = aoco_scan_analyze_next_tuple,
	.index_build_range_scan = aoco_index_build_range_scan,
	.index_validate_scan = aoco_index_validate_scan,

	.relation_size = aoco_relation_size,
	.relation_needs_toast_table = aoco_relation_needs_toast_table,

	.relation_estimate_size = aoco_estimate_rel_size,

	.scan_bitmap_next_block = aoco_scan_bitmap_next_block,
	.scan_bitmap_next_tuple = aoco_scan_bitmap_next_tuple,
	.scan_sample_next_block = aoco_scan_sample_next_block,
	.scan_sample_next_tuple = aoco_scan_sample_next_tuple
};

Datum
aoco_tableam_handler(PG_FUNCTION_ARGS)
{
#ifdef GPDB_12_MERGE_FIXME
	/*
	 * The aoco access method is currently under active development and
	 * unstable. This causes many hangs and cores to be generated during tests.
	 * While stabilizing the access method, keep it turned off so the rest of
	 * the tests can run till completion.
	 */
	PG_RETURN_POINTER(&aoco_methods);
#else
	elog(ERROR, "aoco accesss method not implented yet");
#endif
}

typedef struct neededColumnContext
{
	bool *mask;
	int n;
} neededColumnContext;

static bool
neededColumnContextWalker(Node *node, neededColumnContext *c)
{
	if (node == NULL)
		return false;

	if (IsA(node, Var))
	{
		Var *var = (Var *)node;

		if (IS_SPECIAL_VARNO(var->varno))
			return false;

		if (var->varattno > 0 && var->varattno <= c->n)
			c->mask[var->varattno - 1] = true;

			/*
			 * If all attributes are included,
			 * set all entries in mask to true.
			 */
		else if (var->varattno == 0)
		{
			int i;

			for (i=0; i < c->n; i++)
				c->mask[i] = true;
		}

		return false;
	}
	return expression_tree_walker(node, neededColumnContextWalker, (void * )c);
}
/*
 * n specifies the number of allowed entries in mask: we use
 * it for bounds-checking in the walker above.
 */
/* GPDB_12_MERGE_FIXME: this used to be in execQual.c Where does it belong now? */
static void
GetNeededColumnsForScan(Node *expr, bool *mask, int n)
{
	neededColumnContext c;

	c.mask = mask;
	c.n = n;

	neededColumnContextWalker(expr, &c);
}

void
InitAOCSScanOpaque(SeqScanState *scanstate, Relation currentRelation, bool **proj)
{
	/* Initialize AOCS projection info */
	int			ncol = currentRelation->rd_att->natts;
	int			i;
	MemoryContext oldcxt;

	Assert(currentRelation != NULL);

	*proj = palloc0(ncol * sizeof(bool));

	ncol = currentRelation->rd_att->natts;
	GetNeededColumnsForScan((Node *) scanstate->ss.ps.plan->targetlist, *proj, ncol);
	GetNeededColumnsForScan((Node *) scanstate->ss.ps.plan->qual, *proj, ncol);

	for (i = 0; i < ncol; i++)
	{
		if ((*proj)[i])
			break;
	}

	/*
	 * In some cases (for example, count(*)), no columns are specified.
	 * We always scan the first column.
	 */
	if (i == ncol)
		(*proj)[0] = true;
}
