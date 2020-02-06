/*-------------------------------------------------------------------------
 *
 * nodeBitmapHeapscan.c
 *	  Routines to support bitmapped scans of relations
 *
 * NOTE: it is critical that this plan type only be used with MVCC-compliant
 * snapshots (ie, regular snapshots, not SnapshotAny or one of the other
 * special snapshots).  The reason is that since index and heap scans are
 * decoupled, there can be no assurance that the index tuple prompting a
 * visit to a particular heap TID still exists when the visit is made.
 * Therefore the tuple might not exist anymore either (which is OK because
 * heap_fetch will cope) --- but worse, the tuple slot could have been
 * re-used for a newer tuple.  With an MVCC snapshot the newer tuple is
 * certain to fail the time qual and so it will not be mistakenly returned,
 * but with anything else we might return a tuple that doesn't meet the
 * required index qual conditions.
 *
 * In GPDB, this also deals with AppendOnly and AOCS tables. The prefetching
 * hasn't been implemented for them, though.
 *
 * This can also be used in "Dynamic" mode.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 * Portions Copyright (c) 2008-2009, Greenplum Inc.
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	  src/backend/executor/nodeBitmapHeapscan.c
 *
 *-------------------------------------------------------------------------
 */
/*
 * INTERFACE ROUTINES
 *		ExecBitmapHeapScan			scans a relation using bitmap info
 *		ExecBitmapHeapNext			workhorse for above
 *		ExecInitBitmapHeapScan		creates and initializes state info.
 *		ExecReScanBitmapHeapScan	prepares to rescan the plan.
 *		ExecEndBitmapHeapScan		releases all storage.
 */
#include "postgres.h"

#include <math.h>

#include "access/relscan.h"
#include "access/tableam.h"
#include "access/transam.h"
#include "access/visibilitymap.h"
#include "executor/execdebug.h"
#include "executor/nodeBitmapHeapscan.h"
#include "miscadmin.h"
#include "pgstat.h"
#include "storage/bufmgr.h"
#include "storage/predicate.h"
#include "utils/memutils.h"
#include "parser/parsetree.h"
#include "nodes/tidbitmap.h"
#include "utils/rel.h"
#include "utils/spccache.h"
#include "utils/snapmgr.h"

#include "cdb/cdbappendonlyam.h"
#include "cdb/cdbaocsam.h"
#include "cdb/cdbvars.h" /* gp_select_invisible */

static TupleTableSlot *BitmapHeapNext(BitmapHeapScanState *node);
<<<<<<< HEAD
static TupleTableSlot *BitmapAppendOnlyNext(BitmapHeapScanState *node);
static void bitgetpage(HeapScanDesc scan, TBMIterateResult *tbmres);
=======
static inline void BitmapDoneInitializingSharedState(
													 ParallelBitmapHeapState *pstate);
static inline void BitmapAdjustPrefetchIterator(BitmapHeapScanState *node,
												TBMIterateResult *tbmres);
static inline void BitmapAdjustPrefetchTarget(BitmapHeapScanState *node);
static inline void BitmapPrefetch(BitmapHeapScanState *node,
								  TableScanDesc scan);
static bool BitmapShouldInitializeSharedState(
											  ParallelBitmapHeapState *pstate);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

static void ExecEagerFreeBitmapHeapScan(BitmapHeapScanState *node);

/*
 * Initialize the fetch descriptor, if it is not initialized.
 */
static void
initFetchDesc(BitmapHeapScanState *scanstate)
{
	BitmapHeapScan *node = (BitmapHeapScan *)(scanstate->ss.ps.plan);
	Relation	currentRelation = scanstate->ss.ss_currentRelation;
	EState	   *estate = scanstate->ss.ps.state;

	if (RelationIsAoRows(currentRelation))
	{
		if (scanstate->bhs_currentAOFetchDesc == NULL)
		{
			Snapshot	appendOnlyMetaDataSnapshot;

			appendOnlyMetaDataSnapshot = estate->es_snapshot;
			if (appendOnlyMetaDataSnapshot == SnapshotAny)
			{
				/*
				 * the append-only meta data should never be fetched with
				 * SnapshotAny as bogus results are returned.
				 */
				appendOnlyMetaDataSnapshot = GetTransactionSnapshot();
			}
			scanstate->bhs_currentAOFetchDesc =
				appendonly_fetch_init(currentRelation,
									  estate->es_snapshot,
									  appendOnlyMetaDataSnapshot);
			scanstate->bhs_currentAOCSFetchDesc = NULL;
			scanstate->bhs_currentAOCSLossyFetchDesc = NULL;
		}
	}
	else if (RelationIsAoCols(currentRelation))
	{
		if (scanstate->bhs_currentAOCSFetchDesc == NULL)
		{
			bool	   *proj;
			bool	   *projLossy;
			int			colno;
			Snapshot	appendOnlyMetaDataSnapshot;

			appendOnlyMetaDataSnapshot = estate->es_snapshot;
			if (appendOnlyMetaDataSnapshot == SnapshotAny)
			{
				/*
				 * the append-only meta data should never be fetched with
				 * SnapshotAny as bogus results are returned.
				 */
				appendOnlyMetaDataSnapshot = GetTransactionSnapshot();
			}

			/*
			 * Obtain the projection.
			 */
			Assert(currentRelation->rd_att != NULL);

			proj = (bool *) palloc0(currentRelation->rd_att->natts * sizeof(bool));
			projLossy = (bool *) palloc0(currentRelation->rd_att->natts * sizeof(bool));

			GetNeededColumnsForScan((Node *) node->scan.plan.targetlist, proj, currentRelation->rd_att->natts);
			GetNeededColumnsForScan((Node *) node->scan.plan.qual, proj, currentRelation->rd_att->natts);

			memcpy(projLossy,proj, currentRelation->rd_att->natts * sizeof(bool));

			GetNeededColumnsForScan((Node *) node->bitmapqualorig, projLossy, currentRelation->rd_att->natts);

			for (colno = 0; colno < currentRelation->rd_att->natts; colno++)
			{
				if (proj[colno])
					break;
			}

			/*
			 * At least project one column. Since the tids stored in the index may not have
			 * a correponding tuple any more (because of previous crashes, for example), we
			 * need to read the tuple to make sure.
			 */
			if (colno == currentRelation->rd_att->natts)
				proj[0] = true;

			scanstate->bhs_currentAOCSFetchDesc =
				aocs_fetch_init(currentRelation, estate->es_snapshot, appendOnlyMetaDataSnapshot, proj);

			for (colno = 0; colno < currentRelation->rd_att->natts; colno++)
			{
				if (projLossy[colno])
					break;
			}

			/*
			 * At least project one column. Since the tids stored in the index may not have
			 * a correponding tuple any more (because of previous crashes, for example), we
			 * need to read the tuple to make sure.
			 */
			if (colno == currentRelation->rd_att->natts)
				projLossy[0] = true;

			scanstate->bhs_currentAOCSLossyFetchDesc =
				aocs_fetch_init(currentRelation, estate->es_snapshot, appendOnlyMetaDataSnapshot, projLossy);
		}
	}
	else
	{
		if (scanstate->bhs_currentScanDesc_heap == NULL)
		{
			/*
			 * Even though we aren't going to do a conventional seqscan, it is useful
			 * to create a HeapScanDesc --- most of the fields in it are usable.
			 */
			scanstate->bhs_currentScanDesc_heap = heap_beginscan_bm(currentRelation,
																	estate->es_snapshot,
																	0,
																	NULL);
		}
	}
}

/*
 * Free fetch descriptor.
 */
static void
freeFetchDesc(BitmapHeapScanState *scanstate)
{
	if (scanstate->bhs_currentAOFetchDesc != NULL)
	{
		appendonly_fetch_finish(scanstate->bhs_currentAOFetchDesc);
		pfree(scanstate->bhs_currentAOFetchDesc);
		scanstate->bhs_currentAOFetchDesc = NULL;
	}

	if (scanstate->bhs_currentAOCSFetchDesc != NULL)
	{
		aocs_fetch_finish(scanstate->bhs_currentAOCSFetchDesc);
		pfree(scanstate->bhs_currentAOCSFetchDesc);
		scanstate->bhs_currentAOCSFetchDesc = NULL;
	}

	if (scanstate->bhs_currentAOCSLossyFetchDesc != NULL)
	{
		aocs_fetch_finish(scanstate->bhs_currentAOCSLossyFetchDesc);
		pfree(scanstate->bhs_currentAOCSLossyFetchDesc);
		scanstate->bhs_currentAOCSLossyFetchDesc = NULL;
	}
	if (scanstate->bhs_currentScanDesc_heap != NULL)
	{
		heap_endscan(scanstate->bhs_currentScanDesc_heap);
		scanstate->bhs_currentScanDesc_heap = NULL;
	}
}

/*
 * Free the state relevant to bitmaps
 */
static void
freeBitmapState(BitmapHeapScanState *scanstate)
{
	/* BitmapIndexScan is the owner of the bitmap memory. Don't free it here */
	scanstate->tbm = NULL;
	/* Likewise, the tbmres member is owned by the iterator. It'll be freed
	 * during end_iterate. */
	scanstate->tbmres = NULL;
	if (scanstate->tbmiterator)
		tbm_generic_end_iterate(scanstate->tbmiterator);
	scanstate->tbmiterator = NULL;
	if (scanstate->prefetch_iterator)
		tbm_generic_end_iterate(scanstate->prefetch_iterator);
	scanstate->prefetch_iterator = NULL;
}

/* ----------------------------------------------------------------
 *		BitmapHeapNext
 *
 *		Retrieve next tuple from the BitmapHeapScan node's currentRelation
 * ----------------------------------------------------------------
 */
static TupleTableSlot *
BitmapHeapNext(BitmapHeapScanState *node)
{
	ExprContext *econtext;
<<<<<<< HEAD
	HeapScanDesc scan;
	Node  		*tbm;
	GenericBMIterator *tbmiterator;
	TBMIterateResult *tbmres;
#ifdef USE_PREFETCH
	GenericBMIterator *prefetch_iterator;
#endif

	OffsetNumber targoffset;
=======
	TableScanDesc scan;
	TIDBitmap  *tbm;
	TBMIterator *tbmiterator = NULL;
	TBMSharedIterator *shared_tbmiterator = NULL;
	TBMIterateResult *tbmres;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
	TupleTableSlot *slot;
	ParallelBitmapHeapState *pstate = node->pstate;
	dsa_area   *dsa = node->ss.ps.state->es_query_dsa;

	initFetchDesc(node);

	/*
	 * extract necessary information from index scan node
	 */
	econtext = node->ss.ps.ps_ExprContext;
	slot = node->ss.ss_ScanTupleSlot;
	scan = node->bhs_currentScanDesc_heap;

	tbm = node->tbm;
	if (pstate == NULL)
		tbmiterator = node->tbmiterator;
	else
		shared_tbmiterator = node->shared_tbmiterator;
	tbmres = node->tbmres;

	/*
	 * If we haven't yet performed the underlying index scan, do it, and begin
	 * the iteration over the bitmap.
	 *
	 * For prefetching, we use *two* iterators, one for the pages we are
	 * actually scanning and another that runs ahead of the first for
	 * prefetching.  node->prefetch_pages tracks exactly how many pages ahead
	 * the prefetch iterator is.  Also, node->prefetch_target tracks the
	 * desired prefetch distance, which starts small and increases up to the
	 * node->prefetch_maximum.  This is to avoid doing a lot of prefetching in
	 * a scan that stops after a few tuples because of a LIMIT.
	 */
	if (!node->initialized)
	{
<<<<<<< HEAD
		tbm = (Node *) MultiExecProcNode(outerPlanState(node));

		if (!tbm || !(IsA(tbm, TIDBitmap) || IsA(tbm, StreamBitmap)))
			elog(ERROR, "unrecognized result from subplan");

		node->tbm = tbm;
		node->tbmiterator = tbmiterator = tbm_generic_begin_iterate(tbm);
		node->tbmres = tbmres = NULL;
=======
		if (!pstate)
		{
			tbm = (TIDBitmap *) MultiExecProcNode(outerPlanState(node));

			if (!tbm || !IsA(tbm, TIDBitmap))
				elog(ERROR, "unrecognized result from subplan");

			node->tbm = tbm;
			node->tbmiterator = tbmiterator = tbm_begin_iterate(tbm);
			node->tbmres = tbmres = NULL;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#ifdef USE_PREFETCH
			if (node->prefetch_maximum > 0)
			{
				node->prefetch_iterator = tbm_begin_iterate(tbm);
				node->prefetch_pages = 0;
				node->prefetch_target = -1;
			}
#endif							/* USE_PREFETCH */
		}
		else
		{
<<<<<<< HEAD
			node->prefetch_iterator = prefetch_iterator = tbm_generic_begin_iterate(tbm);
			node->prefetch_pages = 0;
			node->prefetch_target = -1;
=======
			/*
			 * The leader will immediately come out of the function, but
			 * others will be blocked until leader populates the TBM and wakes
			 * them up.
			 */
			if (BitmapShouldInitializeSharedState(pstate))
			{
				tbm = (TIDBitmap *) MultiExecProcNode(outerPlanState(node));
				if (!tbm || !IsA(tbm, TIDBitmap))
					elog(ERROR, "unrecognized result from subplan");

				node->tbm = tbm;

				/*
				 * Prepare to iterate over the TBM. This will return the
				 * dsa_pointer of the iterator state which will be used by
				 * multiple processes to iterate jointly.
				 */
				pstate->tbmiterator = tbm_prepare_shared_iterate(tbm);
#ifdef USE_PREFETCH
				if (node->prefetch_maximum > 0)
				{
					pstate->prefetch_iterator =
						tbm_prepare_shared_iterate(tbm);

					/*
					 * We don't need the mutex here as we haven't yet woke up
					 * others.
					 */
					pstate->prefetch_pages = 0;
					pstate->prefetch_target = -1;
				}
#endif

				/* We have initialized the shared state so wake up others. */
				BitmapDoneInitializingSharedState(pstate);
			}

			/* Allocate a private iterator and attach the shared state to it */
			node->shared_tbmiterator = shared_tbmiterator =
				tbm_attach_shared_iterate(dsa, pstate->tbmiterator);
			node->tbmres = tbmres = NULL;

#ifdef USE_PREFETCH
			if (node->prefetch_maximum > 0)
			{
				node->shared_prefetch_iterator =
					tbm_attach_shared_iterate(dsa, pstate->prefetch_iterator);
			}
#endif							/* USE_PREFETCH */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
		}
		node->initialized = true;
	}

	for (;;)
	{
		bool		skip_fetch;

		CHECK_FOR_INTERRUPTS();

		if (tbmres == NULL || tbmres->ntuples == 0)
		{
<<<<<<< HEAD
			CHECK_FOR_INTERRUPTS();

			if (QueryFinishPending)
				return NULL;

			node->tbmres = tbmres = tbm_generic_iterate(tbmiterator);

=======
			if (!pstate)
				node->tbmres = tbmres = tbm_iterate(tbmiterator);
			else
				node->tbmres = tbmres = tbm_shared_iterate(shared_tbmiterator);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
			if (tbmres == NULL)
			{
				/* no more entries in the bitmap */
				break;
			}

<<<<<<< HEAD
#ifdef USE_PREFETCH
			if (node->prefetch_pages > 0)
			{
				/* The main iterator has closed the distance by one page */
				node->prefetch_pages--;
			}
			else if (prefetch_iterator)
			{
				/* Do not let the prefetch iterator get behind the main one */
				TBMIterateResult *tbmpre = tbm_generic_iterate(prefetch_iterator);

				if (tbmpre == NULL || tbmpre->blockno != tbmres->blockno)
					elog(ERROR, "prefetch and main iterators are out of sync");
			}
#endif   /* USE_PREFETCH */

			/*
			 * Ignore any claimed entries past what we think is the end of
			 * the relation.  (This is probably not necessary given that we
			 * got at least AccessShareLock on the table before performing
			 * any of the indexscans, but let's be safe.)
=======
			BitmapAdjustPrefetchIterator(node, tbmres);

			/*
			 * We can skip fetching the heap page if we don't need any fields
			 * from the heap, and the bitmap entries don't need rechecking,
			 * and all tuples on the page are visible to our transaction.
			 *
			 * XXX: It's a layering violation that we do these checks above
			 * tableam, they should probably moved below it at some point.
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
			 */
			skip_fetch = (node->can_skip_fetch &&
						  !tbmres->recheck &&
						  VM_ALL_VISIBLE(node->ss.ss_currentRelation,
										 tbmres->blockno,
										 &node->vmbuffer));

			if (skip_fetch)
			{
<<<<<<< HEAD
				tbmres->ntuples = 0;
				continue;
			}

			/* If tbmres contains no tuples, continue. */
			if (tbmres->ntuples == 0)
				continue;

			/*
			 * Fetch the current heap page and identify candidate tuples.
			 */
			bitgetpage(scan, tbmres);

=======
				/* can't be lossy in the skip_fetch case */
				Assert(tbmres->ntuples >= 0);

				/*
				 * The number of tuples on this page is put into
				 * node->return_empty_tuples.
				 */
				node->return_empty_tuples = tbmres->ntuples;
			}
			else if (!table_scan_bitmap_next_block(scan, tbmres))
			{
				/* AM doesn't think this block is valid, skip */
				continue;
			}

>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
			if (tbmres->ntuples >= 0)
				node->exact_pages++;
			else
				node->lossy_pages++;

<<<<<<< HEAD
			CheckSendPlanStateGpmonPkt(&node->ss.ps);

			/*
			 * Set rs_cindex to first slot to examine
			 */
			scan->rs_cindex = 0;

#ifdef USE_PREFETCH

			/*
			 * Increase prefetch target if it's not yet at the max.  Note that
			 * we will increase it to zero after fetching the very first
			 * page/tuple, then to one after the second tuple is fetched, then
			 * it doubles as later pages are fetched.
			 */
			if (node->prefetch_target >= node->prefetch_maximum)
				 /* don't increase any further */ ;
			else if (node->prefetch_target >= node->prefetch_maximum / 2)
				node->prefetch_target = node->prefetch_maximum;
			else if (node->prefetch_target > 0)
				node->prefetch_target *= 2;
			else
				node->prefetch_target++;
#endif   /* USE_PREFETCH */
=======
			/* Adjust the prefetch target */
			BitmapAdjustPrefetchTarget(node);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
		}
		else
		{
			/*
			 * Continuing in previously obtained page.
			 */
<<<<<<< HEAD
			scan->rs_cindex++;
			tbmres->ntuples--;
=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#ifdef USE_PREFETCH

			/*
			 * Try to prefetch at least a few pages even before we get to the
			 * second page if we don't stop reading after the first tuple.
			 */
			if (!pstate)
			{
				if (node->prefetch_target < node->prefetch_maximum)
					node->prefetch_target++;
			}
			else if (pstate->prefetch_target < node->prefetch_maximum)
			{
				/* take spinlock while updating shared state */
				SpinLockAcquire(&pstate->mutex);
				if (pstate->prefetch_target < node->prefetch_maximum)
					pstate->prefetch_target++;
				SpinLockRelease(&pstate->mutex);
			}
#endif							/* USE_PREFETCH */
		}

		/*
<<<<<<< HEAD
		 * Out of range?  If so, nothing more to look at on this page
		 */
		if (scan->rs_cindex < 0 || scan->rs_cindex >= scan->rs_ntuples)
		{
			tbmres->ntuples = 0;
			continue;
		}

#ifdef USE_PREFETCH

		/*
=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
		 * We issue prefetch requests *after* fetching the current page to try
		 * to avoid having prefetching interfere with the main I/O. Also, this
		 * should happen only when we have determined there is still something
		 * to do on the current page, else we may uselessly prefetch the same
		 * page we are just about to request for real.
		 *
		 * XXX: It's a layering violation that we do these checks above
		 * tableam, they should probably moved below it at some point.
		 */
		BitmapPrefetch(node, scan);

		if (node->return_empty_tuples > 0)
		{
			/*
			 * If we don't have to fetch the tuple, just return nulls.
			 */
			ExecStoreAllNullTuple(slot);

			if (--node->return_empty_tuples == 0)
			{
<<<<<<< HEAD
				TBMIterateResult *tbmpre = tbm_generic_iterate(prefetch_iterator);

				if (tbmpre == NULL)
				{
					/* No more pages to prefetch */
					tbm_generic_end_iterate(prefetch_iterator);
					node->prefetch_iterator = prefetch_iterator = NULL;
					break;
				}
				node->prefetch_pages++;
				PrefetchBuffer(scan->rs_rd, MAIN_FORKNUM, tbmpre->blockno);
			}
		}
#endif   /* USE_PREFETCH */

		/*
		 * Okay to fetch the tuple
		 */
		targoffset = scan->rs_vistuples[scan->rs_cindex];
		dp = (Page) BufferGetPage(scan->rs_cbuf);
		lp = PageGetItemId(dp, targoffset);
		Assert(ItemIdIsNormal(lp));

		scan->rs_ctup.t_data = (HeapTupleHeader) PageGetItem((Page) dp, lp);
		scan->rs_ctup.t_len = ItemIdGetLength(lp);
#if 0
		scan->rs_ctup.t_tableOid = scan->rs_rd->rd_id;
#endif
		ItemPointerSet(&scan->rs_ctup.t_self, tbmres->blockno, targoffset);

		pgstat_count_heap_fetch(scan->rs_rd);

		/*
		 * Set up the result slot to point to this tuple. Note that the slot
		 * acquires a pin on the buffer.
		 */
		ExecStoreHeapTuple(&scan->rs_ctup,
					   slot,
					   scan->rs_cbuf,
					   false);

		/*
		 * If we are using lossy info, we have to recheck the qual conditions
		 * at every tuple.
		 */
		if (tbmres->recheck)
=======
				/* no more tuples to return in the next round */
				node->tbmres = tbmres = NULL;
			}
		}
		else
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
		{
			/*
			 * Attempt to fetch tuple from AM.
			 */
			if (!table_scan_bitmap_next_tuple(scan, tbmres, slot))
			{
				/* nothing more to look at on this page */
				node->tbmres = tbmres = NULL;
				continue;
			}

			/*
			 * If we are using lossy info, we have to recheck the qual
			 * conditions at every tuple.
			 */
			if (tbmres->recheck)
			{
				econtext->ecxt_scantuple = slot;
				if (!ExecQualAndReset(node->bitmapqualorig, econtext))
				{
					/* Fails recheck, so drop it and loop back for another */
					InstrCountFiltered2(node, 1);
					ExecClearTuple(slot);
					continue;
				}
			}
		}

		/* OK to return this tuple */
		return slot;
	}

	ExecEagerFreeBitmapHeapScan(node);

	/*
	 * if we get here it means we are at the end of the scan..
	 */
	return ExecClearTuple(slot);
}

/*
 *	BitmapDoneInitializingSharedState - Shared state is initialized
 *
 *	By this time the leader has already populated the TBM and initialized the
 *	shared state so wake up other processes.
 */
static inline void
BitmapDoneInitializingSharedState(ParallelBitmapHeapState *pstate)
{
	SpinLockAcquire(&pstate->mutex);
	pstate->state = BM_FINISHED;
	SpinLockRelease(&pstate->mutex);
	ConditionVariableBroadcast(&pstate->cv);
}

/*
 *	BitmapAdjustPrefetchIterator - Adjust the prefetch iterator
 */
static inline void
BitmapAdjustPrefetchIterator(BitmapHeapScanState *node,
							 TBMIterateResult *tbmres)
{
#ifdef USE_PREFETCH
	ParallelBitmapHeapState *pstate = node->pstate;

	if (pstate == NULL)
	{
		TBMIterator *prefetch_iterator = node->prefetch_iterator;

		if (node->prefetch_pages > 0)
		{
			/* The main iterator has closed the distance by one page */
			node->prefetch_pages--;
		}
		else if (prefetch_iterator)
		{
			/* Do not let the prefetch iterator get behind the main one */
			TBMIterateResult *tbmpre = tbm_iterate(prefetch_iterator);

			if (tbmpre == NULL || tbmpre->blockno != tbmres->blockno)
				elog(ERROR, "prefetch and main iterators are out of sync");
		}
		return;
	}

	if (node->prefetch_maximum > 0)
	{
		TBMSharedIterator *prefetch_iterator = node->shared_prefetch_iterator;

		SpinLockAcquire(&pstate->mutex);
		if (pstate->prefetch_pages > 0)
		{
			pstate->prefetch_pages--;
			SpinLockRelease(&pstate->mutex);
		}
		else
		{
<<<<<<< HEAD
			ItemId		lp;
			HeapTupleData loctup;
			bool		valid;

			lp = PageGetItemId(dp, offnum);
			if (!ItemIdIsNormal(lp))
				continue;
			loctup.t_data = (HeapTupleHeader) PageGetItem((Page) dp, lp);
			loctup.t_len = ItemIdGetLength(lp);
			ItemPointerSet(&loctup.t_self, page, offnum);
			valid = HeapTupleSatisfiesVisibility(scan->rs_rd, &loctup, snapshot, buffer);
			if (valid)
=======
			/* Release the mutex before iterating */
			SpinLockRelease(&pstate->mutex);

			/*
			 * In case of shared mode, we can not ensure that the current
			 * blockno of the main iterator and that of the prefetch iterator
			 * are same.  It's possible that whatever blockno we are
			 * prefetching will be processed by another process.  Therefore,
			 * we don't validate the blockno here as we do in non-parallel
			 * case.
			 */
			if (prefetch_iterator)
				tbm_shared_iterate(prefetch_iterator);
		}
	}
#endif							/* USE_PREFETCH */
}

/*
 * BitmapAdjustPrefetchTarget - Adjust the prefetch target
 *
 * Increase prefetch target if it's not yet at the max.  Note that
 * we will increase it to zero after fetching the very first
 * page/tuple, then to one after the second tuple is fetched, then
 * it doubles as later pages are fetched.
 */
static inline void
BitmapAdjustPrefetchTarget(BitmapHeapScanState *node)
{
#ifdef USE_PREFETCH
	ParallelBitmapHeapState *pstate = node->pstate;

	if (pstate == NULL)
	{
		if (node->prefetch_target >= node->prefetch_maximum)
			 /* don't increase any further */ ;
		else if (node->prefetch_target >= node->prefetch_maximum / 2)
			node->prefetch_target = node->prefetch_maximum;
		else if (node->prefetch_target > 0)
			node->prefetch_target *= 2;
		else
			node->prefetch_target++;
		return;
	}

	/* Do an unlocked check first to save spinlock acquisitions. */
	if (pstate->prefetch_target < node->prefetch_maximum)
	{
		SpinLockAcquire(&pstate->mutex);
		if (pstate->prefetch_target >= node->prefetch_maximum)
			 /* don't increase any further */ ;
		else if (pstate->prefetch_target >= node->prefetch_maximum / 2)
			pstate->prefetch_target = node->prefetch_maximum;
		else if (pstate->prefetch_target > 0)
			pstate->prefetch_target *= 2;
		else
			pstate->prefetch_target++;
		SpinLockRelease(&pstate->mutex);
	}
#endif							/* USE_PREFETCH */
}

/*
 * BitmapPrefetch - Prefetch, if prefetch_pages are behind prefetch_target
 */
static inline void
BitmapPrefetch(BitmapHeapScanState *node, TableScanDesc scan)
{
#ifdef USE_PREFETCH
	ParallelBitmapHeapState *pstate = node->pstate;

	if (pstate == NULL)
	{
		TBMIterator *prefetch_iterator = node->prefetch_iterator;

		if (prefetch_iterator)
		{
			while (node->prefetch_pages < node->prefetch_target)
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
			{
				TBMIterateResult *tbmpre = tbm_iterate(prefetch_iterator);
				bool		skip_fetch;

				if (tbmpre == NULL)
				{
					/* No more pages to prefetch */
					tbm_end_iterate(prefetch_iterator);
					node->prefetch_iterator = NULL;
					break;
				}
				node->prefetch_pages++;

				/*
				 * If we expect not to have to actually read this heap page,
				 * skip this prefetch call, but continue to run the prefetch
				 * logic normally.  (Would it be better not to increment
				 * prefetch_pages?)
				 *
				 * This depends on the assumption that the index AM will
				 * report the same recheck flag for this future heap page as
				 * it did for the current heap page; which is not a certainty
				 * but is true in many cases.
				 */
				skip_fetch = (node->can_skip_fetch &&
							  (node->tbmres ? !node->tbmres->recheck : false) &&
							  VM_ALL_VISIBLE(node->ss.ss_currentRelation,
											 tbmpre->blockno,
											 &node->pvmbuffer));

				if (!skip_fetch)
					PrefetchBuffer(scan->rs_rd, MAIN_FORKNUM, tbmpre->blockno);
			}
		}

		return;
	}

	if (pstate->prefetch_pages < pstate->prefetch_target)
	{
		TBMSharedIterator *prefetch_iterator = node->shared_prefetch_iterator;

		if (prefetch_iterator)
		{
			while (1)
			{
				TBMIterateResult *tbmpre;
				bool		do_prefetch = false;
				bool		skip_fetch;

				/*
				 * Recheck under the mutex. If some other process has already
				 * done enough prefetching then we need not to do anything.
				 */
				SpinLockAcquire(&pstate->mutex);
				if (pstate->prefetch_pages < pstate->prefetch_target)
				{
					pstate->prefetch_pages++;
					do_prefetch = true;
				}
				SpinLockRelease(&pstate->mutex);

				if (!do_prefetch)
					return;

				tbmpre = tbm_shared_iterate(prefetch_iterator);
				if (tbmpre == NULL)
				{
					/* No more pages to prefetch */
					tbm_end_shared_iterate(prefetch_iterator);
					node->shared_prefetch_iterator = NULL;
					break;
				}

				/* As above, skip prefetch if we expect not to need page */
				skip_fetch = (node->can_skip_fetch &&
							  (node->tbmres ? !node->tbmres->recheck : false) &&
							  VM_ALL_VISIBLE(node->ss.ss_currentRelation,
											 tbmpre->blockno,
											 &node->pvmbuffer));

				if (!skip_fetch)
					PrefetchBuffer(scan->rs_rd, MAIN_FORKNUM, tbmpre->blockno);
			}
		}
	}
#endif							/* USE_PREFETCH */
}

/* ----------------------------------------------------------------
 *		BitmapAppendOnlyNext
 *
 *		Like BitmapHeapNext(), but used when the table is an AO or AOCS table.
 * ----------------------------------------------------------------
 */
static TupleTableSlot *
BitmapAppendOnlyNext(BitmapHeapScanState *node)
{
	EState	   *estate;
	ExprContext *econtext;
	AppendOnlyFetchDesc aoFetchDesc;
	AOCSFetchDesc aocsFetchDesc;
	AOCSFetchDesc aocsLossyFetchDesc;
	Index		scanrelid;
	GenericBMIterator *tbmiterator;
	OffsetNumber psuedoHeapOffset;
	ItemPointerData psudeoHeapTid;
	AOTupleId	aoTid;
	TupleTableSlot *slot;

	/*
	 * extract necessary information from index scan node
	 */
	estate = node->ss.ps.state;
	econtext = node->ss.ps.ps_ExprContext;
	slot = node->ss.ss_ScanTupleSlot;

	initFetchDesc(node);

	aoFetchDesc = node->bhs_currentAOFetchDesc;
	aocsFetchDesc = node->bhs_currentAOCSFetchDesc;
	aocsLossyFetchDesc = node->bhs_currentAOCSLossyFetchDesc;
	scanrelid = ((BitmapHeapScan *) node->ss.ps.plan)->scan.scanrelid;
	tbmiterator = node->tbmiterator;

	/*
	 * If we haven't yet performed the underlying index scan, or
	 * we have used up the bitmaps from the previous scan, do the next scan,
	 * and prepare the bitmap to be iterated over.
	 */
	if (tbmiterator == NULL)
	{
		Node *tbm = (Node *) MultiExecProcNode(outerPlanState(node));

		if (!tbm || !(IsA(tbm, TIDBitmap) || IsA(tbm, StreamBitmap)))
			elog(ERROR, "unrecognized result from subplan");

		if (tbm == NULL)
		{
			ExecEagerFreeBitmapHeapScan(node);

			return ExecClearTuple(slot);
		}

		node->tbmiterator = tbmiterator = tbm_generic_begin_iterate(tbm);
	}

	Assert(tbmiterator != NULL);

	for (;;)
	{
		TBMIterateResult *tbmres = node->tbmres;
		bool		need_recheck = false;

		CHECK_FOR_INTERRUPTS();

		if (QueryFinishPending)
			return NULL;

		if (!node->baos_gotpage)
		{
			/*
			 * Obtain the next psuedo-heap-page-info with item bit-map.  Later, we'll
			 * convert the (psuedo) heap block number and item number to an
			 * Append-Only TID.
			 */
			node->tbmres = tbmres = tbm_generic_iterate(tbmiterator);

			if (tbmres == NULL)
			{
				/* no more entries in the bitmap */
				break;
			}

			/* If tbmres contains no tuples, continue. */
			if (tbmres->ntuples == 0)
				continue;

			/* Make sure we never cross 15-bit offset number [MPP-24326] */
			Assert(tbmres->ntuples <= INT16_MAX + 1);
			CheckSendPlanStateGpmonPkt(&node->ss.ps);

			node->baos_gotpage = true;

			/*
			 * Set cindex to first slot to examine
			 */
			node->baos_cindex = 0;

			node->baos_lossy = (tbmres->ntuples < 0);
			if (!node->baos_lossy)
			{
				node->baos_ntuples = tbmres->ntuples;
			}
			else
			{
				/* Iterate over the first 2^15 tuples [MPP-24326] */
				node->baos_ntuples = INT16_MAX + 1;
			}
		}
		else
		{
			/*
			 * Continuing in previously obtained page; advance cindex
			 */
			node->baos_cindex++;
		}

		/*
		 * Out of range?  If so, nothing more to look at on this page
		 */
		if (node->baos_cindex < 0 || node->baos_cindex >= node->baos_ntuples)
		{
			node->baos_gotpage = false;
			continue;
		}

		if (node->baos_lossy || tbmres->recheck)
			need_recheck = true;

		/*
		 * Must account for lossy page info...
		 */
		if (node->baos_lossy)
		{
			/*
			 * +1 to convert index to offset, since TID offsets are not zero
			 * based.
			 */
			psuedoHeapOffset = node->baos_cindex + 1;	// We are iterating through all items.
		}
		else
		{
			Assert(node->baos_cindex <= tbmres->ntuples);
			psuedoHeapOffset = tbmres->offsets[node->baos_cindex];
		}

		/*
		 * Okay to fetch the tuple
		 */
		ItemPointerSet(&psudeoHeapTid,
					   tbmres->blockno,
					   psuedoHeapOffset);

		tbm_convert_appendonly_tid_out(&psudeoHeapTid, &aoTid);

		if (aoFetchDesc != NULL)
		{
			appendonly_fetch(aoFetchDesc, &aoTid, slot);
		}
		else
		{
			if (need_recheck)
			{
				Assert(aocsLossyFetchDesc != NULL);
				aocs_fetch(aocsLossyFetchDesc, &aoTid, slot);
			}
			else
			{
				Assert(aocsFetchDesc != NULL);
				aocs_fetch(aocsFetchDesc, &aoTid, slot);
			}
		}

		if (TupIsNull(slot))
			continue;

		pgstat_count_heap_fetch(node->ss.ss_currentRelation);

		/*
		 * If we are using lossy info, we have to recheck the qual
		 * conditions at every tuple.
		 */
		if (need_recheck)
		{
			econtext->ecxt_scantuple = slot;
			ResetExprContext(econtext);

			if (!ExecQual(node->bitmapqualorig, econtext))
			{
				/* Fails recheck, so drop it and loop back for another */
				ExecClearTuple(slot);
				continue;
			}
		}

		/* OK to return this tuple */
		return slot;
	}

	/*
	 * if we get here it means we are at the end of the scan..
	 */
	ExecEagerFreeBitmapHeapScan(node);

	return ExecClearTuple(slot);
}


/*
 * BitmapHeapRecheck -- access method routine to recheck a tuple in EvalPlanQual
 */
static bool
BitmapHeapRecheck(BitmapHeapScanState *node, TupleTableSlot *slot)
{
	ExprContext *econtext;

	/*
	 * extract necessary information from index scan node
	 */
	econtext = node->ss.ps.ps_ExprContext;

	/* Does the tuple meet the original qual conditions? */
	econtext->ecxt_scantuple = slot;
	return ExecQualAndReset(node->bitmapqualorig, econtext);
}

/* ----------------------------------------------------------------
 *		ExecBitmapHeapScan(node)
 * ----------------------------------------------------------------
 */
static TupleTableSlot *
ExecBitmapHeapScan(PlanState *pstate)
{
<<<<<<< HEAD
	TupleTableSlot *slot = NULL;
	Relation	currentRelation = node->ss.ss_currentRelation;

	if (RelationIsAppendOptimized(currentRelation))
		slot = ExecScan(&node->ss,
						(ExecScanAccessMtd) BitmapAppendOnlyNext,
						(ExecScanRecheckMtd) BitmapHeapRecheck);
	else
		slot = ExecScan(&node->ss,
						(ExecScanAccessMtd) BitmapHeapNext,
						(ExecScanRecheckMtd) BitmapHeapRecheck);

	if (TupIsNull(slot))
	{
		/* next partition, if needed */
	}

	return slot;
=======
	BitmapHeapScanState *node = castNode(BitmapHeapScanState, pstate);

	return ExecScan(&node->ss,
					(ExecScanAccessMtd) BitmapHeapNext,
					(ExecScanRecheckMtd) BitmapHeapRecheck);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
}

/* ----------------------------------------------------------------
 *		ExecReScanBitmapHeapScan(node)
 * ----------------------------------------------------------------
 */
void
ExecReScanBitmapHeapScan(BitmapHeapScanState *node)
{
	PlanState  *outerPlan = outerPlanState(node);

	/* rescan to release any page pin */
<<<<<<< HEAD
	if (node->bhs_currentScanDesc_heap)
		heap_rescan(node->bhs_currentScanDesc_heap, NULL);

	freeBitmapState(node);
=======
	table_rescan(node->ss.ss_currentScanDesc, NULL);

	/* release bitmaps and buffers if any */
	if (node->tbmiterator)
		tbm_end_iterate(node->tbmiterator);
	if (node->prefetch_iterator)
		tbm_end_iterate(node->prefetch_iterator);
	if (node->shared_tbmiterator)
		tbm_end_shared_iterate(node->shared_tbmiterator);
	if (node->shared_prefetch_iterator)
		tbm_end_shared_iterate(node->shared_prefetch_iterator);
	if (node->tbm)
		tbm_free(node->tbm);
	if (node->vmbuffer != InvalidBuffer)
		ReleaseBuffer(node->vmbuffer);
	if (node->pvmbuffer != InvalidBuffer)
		ReleaseBuffer(node->pvmbuffer);
	node->tbm = NULL;
	node->tbmiterator = NULL;
	node->tbmres = NULL;
	node->prefetch_iterator = NULL;
	node->initialized = false;
	node->shared_tbmiterator = NULL;
	node->shared_prefetch_iterator = NULL;
	node->vmbuffer = InvalidBuffer;
	node->pvmbuffer = InvalidBuffer;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	ExecScanReScan(&node->ss);

	/*
	 * if chgParam of subnode is not null then plan will be re-scanned by
	 * first ExecProcNode.
	 */
	if (outerPlan->chgParam == NULL)
		ExecReScan(outerPlan);
}

/* ----------------------------------------------------------------
 *		ExecEndBitmapHeapScan
 * ----------------------------------------------------------------
 */
void
ExecEndBitmapHeapScan(BitmapHeapScanState *node)
{
<<<<<<< HEAD
	Relation	relation;
=======
	TableScanDesc scanDesc;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/*
	 * extract information from the node
	 */
<<<<<<< HEAD
	relation = node->ss.ss_currentRelation;
=======
	scanDesc = node->ss.ss_currentScanDesc;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/*
	 * Free the exprcontext
	 */
	ExecFreeExprContext(&node->ss.ps);

	/*
	 * clear out tuple table slots
	 */
	if (node->ss.ps.ps_ResultTupleSlot)
		ExecClearTuple(node->ss.ps.ps_ResultTupleSlot);
	ExecClearTuple(node->ss.ss_ScanTupleSlot);

	/*
	 * close down subplans
	 */
	ExecEndNode(outerPlanState(node));

	/*
	 * release bitmaps and buffers if any
	 */
<<<<<<< HEAD
	ExecEagerFreeBitmapHeapScan(node);
=======
	if (node->tbmiterator)
		tbm_end_iterate(node->tbmiterator);
	if (node->prefetch_iterator)
		tbm_end_iterate(node->prefetch_iterator);
	if (node->tbm)
		tbm_free(node->tbm);
	if (node->shared_tbmiterator)
		tbm_end_shared_iterate(node->shared_tbmiterator);
	if (node->shared_prefetch_iterator)
		tbm_end_shared_iterate(node->shared_prefetch_iterator);
	if (node->vmbuffer != InvalidBuffer)
		ReleaseBuffer(node->vmbuffer);
	if (node->pvmbuffer != InvalidBuffer)
		ReleaseBuffer(node->pvmbuffer);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/*
	 * close heap scan
	 */
<<<<<<< HEAD
	freeFetchDesc(node);

	/*
	 * close the heap relation.
	 */
	ExecCloseScanRelation(relation);

	EndPlanStateGpmonPkt(&node->ss.ps);
=======
	table_endscan(scanDesc);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
}

/* ----------------------------------------------------------------
 *		ExecInitBitmapHeapScan
 *
 *		Initializes the scan's state information.
 * ----------------------------------------------------------------
 */
BitmapHeapScanState *
ExecInitBitmapHeapScan(BitmapHeapScan *node, EState *estate, int eflags)
{
	Relation	currentRelation;
	BitmapHeapScanState *bhsState;

	/*
	 * open the base relation and acquire appropriate lock on it.
	 */
	currentRelation = ExecOpenScanRelation(estate, node->scan.scanrelid, eflags);

	bhsState =  ExecInitBitmapHeapScanForPartition(node, estate, eflags,
												   currentRelation);

	/*
	 * initialize child nodes
	 *
	 * We do this last because the child nodes will open indexscans on our
	 * relation's indexes, and we want to be sure we have acquired a lock on
	 * the relation first.
	 */
	outerPlanState(bhsState) = ExecInitNode(outerPlan(node), estate, eflags);

	return bhsState;
}

BitmapHeapScanState *
ExecInitBitmapHeapScanForPartition(BitmapHeapScan *node, EState *estate, int eflags,
								   Relation currentRelation)
{

	BitmapHeapScanState *scanstate;
	int			io_concurrency;

	/* check for unsupported flags */
	Assert(!(eflags & (EXEC_FLAG_BACKWARD | EXEC_FLAG_MARK)));

	/*
	 * Assert caller didn't ask for an unsafe snapshot --- see comments at
	 * head of file.
	 *
	 * MPP-4703: the MVCC-snapshot restriction is required for correct results.
	 * our test-mode may deliberately return incorrect results, but that's OK.
	 */
	Assert(IsMVCCSnapshot(estate->es_snapshot) || gp_select_invisible);

	/*
	 * create state structure
	 */
	scanstate = makeNode(BitmapHeapScanState);
	scanstate->ss.ps.plan = (Plan *) node;
	scanstate->ss.ps.state = estate;
	scanstate->ss.ps.ExecProcNode = ExecBitmapHeapScan;

	scanstate->tbm = NULL;
	scanstate->tbmiterator = NULL;
	scanstate->tbmres = NULL;
	scanstate->return_empty_tuples = 0;
	scanstate->vmbuffer = InvalidBuffer;
	scanstate->pvmbuffer = InvalidBuffer;
	scanstate->exact_pages = 0;
	scanstate->lossy_pages = 0;
	scanstate->prefetch_iterator = NULL;
	scanstate->prefetch_pages = 0;
	scanstate->prefetch_target = 0;
	/* may be updated below */
	scanstate->prefetch_maximum = target_prefetch_pages;
	scanstate->pscan_len = 0;
	scanstate->initialized = false;
	scanstate->shared_tbmiterator = NULL;
	scanstate->shared_prefetch_iterator = NULL;
	scanstate->pstate = NULL;

	/*
	 * We can potentially skip fetching heap pages if we do not need any
	 * columns of the table, either for checking non-indexable quals or for
	 * returning data.  This test is a bit simplistic, as it checks the
	 * stronger condition that there's no qual or return tlist at all.  But in
	 * most cases it's probably not worth working harder than that.
	 */
	scanstate->can_skip_fetch = (node->scan.plan.qual == NIL &&
								 node->scan.plan.targetlist == NIL);

	scanstate->baos_gotpage = false;
	scanstate->baos_lossy = false;
	scanstate->baos_cindex = 0;
	scanstate->baos_ntuples = 0;

	/*
	 * Miscellaneous initialization
	 *
	 * create expression context for node
	 */
	ExecAssignExprContext(estate, &scanstate->ss.ps);

	/*
	 * open the scan relation
	 */
	currentRelation = ExecOpenScanRelation(estate, node->scan.scanrelid, eflags);

	/*
	 * initialize child nodes
	 */
	outerPlanState(scanstate) = ExecInitNode(outerPlan(node), estate, eflags);

	/*
	 * get the scan type from the relation descriptor.
	 */
	ExecInitScanTupleSlot(estate, &scanstate->ss,
						  RelationGetDescr(currentRelation),
						  table_slot_callbacks(currentRelation));

	/*
	 * Initialize result type and projection.
	 */
	ExecInitResultTypeTL(&scanstate->ss.ps);
	ExecAssignScanProjectionInfo(&scanstate->ss);

	/*
	 * initialize child expressions
	 */
	scanstate->ss.ps.qual =
		ExecInitQual(node->scan.plan.qual, (PlanState *) scanstate);
	scanstate->bitmapqualorig =
		ExecInitQual(node->bitmapqualorig, (PlanState *) scanstate);

	/*
	 * Determine the maximum for prefetch_target.  If the tablespace has a
	 * specific IO concurrency set, use that to compute the corresponding
	 * maximum value; otherwise, we already initialized to the value computed
	 * by the GUC machinery.
	 */
	io_concurrency =
		get_tablespace_io_concurrency(currentRelation->rd_rel->reltablespace);
	if (io_concurrency != effective_io_concurrency)
	{
		double		maximum;

		if (ComputeIoConcurrency(io_concurrency, &maximum))
			scanstate->prefetch_maximum = rint(maximum);
	}

	scanstate->ss.ss_currentRelation = currentRelation;

<<<<<<< HEAD
	/*
	 * get the scan type from the relation descriptor.
	 */
	ExecAssignScanType(&scanstate->ss, RelationGetDescr(currentRelation));

	/*
	 * Initialize result tuple type and projection info.
	 */
	ExecAssignResultTypeFromTL(&scanstate->ss.ps);
	ExecAssignScanProjectionInfo(&scanstate->ss);

	scanstate->bhs_currentAOFetchDesc = NULL;
	scanstate->bhs_currentAOCSFetchDesc = NULL;
	scanstate->bhs_currentAOCSLossyFetchDesc = NULL;
=======
	scanstate->ss.ss_currentScanDesc = table_beginscan_bm(currentRelation,
														  estate->es_snapshot,
														  0,
														  NULL);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/*
	 * all done.
	 */
	return scanstate;
}

<<<<<<< HEAD
static void
ExecEagerFreeBitmapHeapScan(BitmapHeapScanState *node)
{
	freeFetchDesc(node);
	freeBitmapState(node);
}

void
ExecSquelchBitmapHeapScan(BitmapHeapScanState *node)
{
	ExecEagerFreeBitmapHeapScan(node);
=======
/*----------------
 *		BitmapShouldInitializeSharedState
 *
 *		The first process to come here and see the state to the BM_INITIAL
 *		will become the leader for the parallel bitmap scan and will be
 *		responsible for populating the TIDBitmap.  The other processes will
 *		be blocked by the condition variable until the leader wakes them up.
 * ---------------
 */
static bool
BitmapShouldInitializeSharedState(ParallelBitmapHeapState *pstate)
{
	SharedBitmapState state;

	while (1)
	{
		SpinLockAcquire(&pstate->mutex);
		state = pstate->state;
		if (pstate->state == BM_INITIAL)
			pstate->state = BM_INPROGRESS;
		SpinLockRelease(&pstate->mutex);

		/* Exit if bitmap is done, or if we're the leader. */
		if (state != BM_INPROGRESS)
			break;

		/* Wait for the leader to wake us up. */
		ConditionVariableSleep(&pstate->cv, WAIT_EVENT_PARALLEL_BITMAP_SCAN);
	}

	ConditionVariableCancelSleep();

	return (state == BM_INITIAL);
}

/* ----------------------------------------------------------------
 *		ExecBitmapHeapEstimate
 *
 *		Compute the amount of space we'll need in the parallel
 *		query DSM, and inform pcxt->estimator about our needs.
 * ----------------------------------------------------------------
 */
void
ExecBitmapHeapEstimate(BitmapHeapScanState *node,
					   ParallelContext *pcxt)
{
	EState	   *estate = node->ss.ps.state;

	node->pscan_len = add_size(offsetof(ParallelBitmapHeapState,
										phs_snapshot_data),
							   EstimateSnapshotSpace(estate->es_snapshot));

	shm_toc_estimate_chunk(&pcxt->estimator, node->pscan_len);
	shm_toc_estimate_keys(&pcxt->estimator, 1);
}

/* ----------------------------------------------------------------
 *		ExecBitmapHeapInitializeDSM
 *
 *		Set up a parallel bitmap heap scan descriptor.
 * ----------------------------------------------------------------
 */
void
ExecBitmapHeapInitializeDSM(BitmapHeapScanState *node,
							ParallelContext *pcxt)
{
	ParallelBitmapHeapState *pstate;
	EState	   *estate = node->ss.ps.state;
	dsa_area   *dsa = node->ss.ps.state->es_query_dsa;

	/* If there's no DSA, there are no workers; initialize nothing. */
	if (dsa == NULL)
		return;

	pstate = shm_toc_allocate(pcxt->toc, node->pscan_len);

	pstate->tbmiterator = 0;
	pstate->prefetch_iterator = 0;

	/* Initialize the mutex */
	SpinLockInit(&pstate->mutex);
	pstate->prefetch_pages = 0;
	pstate->prefetch_target = 0;
	pstate->state = BM_INITIAL;

	ConditionVariableInit(&pstate->cv);
	SerializeSnapshot(estate->es_snapshot, pstate->phs_snapshot_data);

	shm_toc_insert(pcxt->toc, node->ss.ps.plan->plan_node_id, pstate);
	node->pstate = pstate;
}

/* ----------------------------------------------------------------
 *		ExecBitmapHeapReInitializeDSM
 *
 *		Reset shared state before beginning a fresh scan.
 * ----------------------------------------------------------------
 */
void
ExecBitmapHeapReInitializeDSM(BitmapHeapScanState *node,
							  ParallelContext *pcxt)
{
	ParallelBitmapHeapState *pstate = node->pstate;
	dsa_area   *dsa = node->ss.ps.state->es_query_dsa;

	/* If there's no DSA, there are no workers; do nothing. */
	if (dsa == NULL)
		return;

	pstate->state = BM_INITIAL;

	if (DsaPointerIsValid(pstate->tbmiterator))
		tbm_free_shared_area(dsa, pstate->tbmiterator);

	if (DsaPointerIsValid(pstate->prefetch_iterator))
		tbm_free_shared_area(dsa, pstate->prefetch_iterator);

	pstate->tbmiterator = InvalidDsaPointer;
	pstate->prefetch_iterator = InvalidDsaPointer;
}

/* ----------------------------------------------------------------
 *		ExecBitmapHeapInitializeWorker
 *
 *		Copy relevant information from TOC into planstate.
 * ----------------------------------------------------------------
 */
void
ExecBitmapHeapInitializeWorker(BitmapHeapScanState *node,
							   ParallelWorkerContext *pwcxt)
{
	ParallelBitmapHeapState *pstate;
	Snapshot	snapshot;

	Assert(node->ss.ps.state->es_query_dsa != NULL);

	pstate = shm_toc_lookup(pwcxt->toc, node->ss.ps.plan->plan_node_id, false);
	node->pstate = pstate;

	snapshot = RestoreSnapshot(pstate->phs_snapshot_data);
	table_scan_update_snapshot(node->ss.ss_currentScanDesc, snapshot);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
}
