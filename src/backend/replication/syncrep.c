/*-------------------------------------------------------------------------
 *
 * syncrep.c
 *
<<<<<<< HEAD
=======
 * Synchronous replication is new as of PostgreSQL 9.1.
 *
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * If requested, transaction commits wait until their commit LSN is
 * acknowledged by the sync standby.
 *
 * This module contains the code for waiting and release of backends.
 * All code in this module executes on the primary. The core streaming
 * replication transport remains within WALreceiver/WALsender modules.
 *
 * The essence of this design is that it isolates all logic about
<<<<<<< HEAD
 * waiting/releasing onto the primary. The primary looks for an active and
 * a sync-requesting standby and then based on that makes the backends wait
 * for completion of replication.
 *
 * Replication is either synchronous or not synchronous (async). If it is
 * async, we just fastpath out of here. If it is sync, then we wait for
 * the write or flush location on the standby before releasing the waiting backend.
 *
 * The best performing way to manage the waiting backends is to have a
 * single ordered queue of waiting backends, so that we can avoid
 * searching them through all waiters each time we receive a reply.
 *
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
=======
 * waiting/releasing onto the primary. The primary defines which standbys
 * it wishes to wait for. The standby is completely unaware of the
 * durability requirements of transactions on the primary, reducing the
 * complexity of the code and streamlining both standby operations and
 * network bandwidth because there is no requirement to ship
 * per-transaction state information.
 *
 * Replication is either synchronous or not synchronous (async). If it is
 * async, we just fastpath out of here. If it is sync, then in 9.1 we wait
 * for the flush location on the standby before releasing the waiting backend.
 * Further complexity in that interaction is expected in later releases.
 *
 * The best performing way to manage the waiting backends is to have a
 * single ordered queue of waiting backends, so that we can avoid
 * searching the through all waiters each time we receive a reply.
 *
 * In 9.1 we support only a single synchronous standby, chosen from a
 * priority list of synchronous_standby_names. Before it can become the
 * synchronous standby it must have caught up with the primary; that may
 * take some time. Once caught up, the current highest priority standby
 * will release waiters from the queue.
 *
 * Portions Copyright (c) 2010-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * IDENTIFICATION
 *	  src/backend/replication/syncrep.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <unistd.h>

#include "access/xact.h"
<<<<<<< HEAD
#include "miscadmin.h"
#include "replication/syncrep.h"
#include "replication/walsender.h"
#include "replication/walsender_private.h"
#include "storage/pmsignal.h"
#include "storage/proc.h"
#include "storage/procsignal.h"
#include "tcop/tcopprot.h"
#include "utils/builtins.h"
#include "utils/ps_status.h"
#include "utils/faultinjector.h"
#include "pgstat.h"
#include "cdb/cdbvars.h"
=======
#include "access/xlog_internal.h"
#include "miscadmin.h"
#include "postmaster/autovacuum.h"
#include "replication/syncrep.h"
#include "replication/walsender.h"
#include "storage/latch.h"
#include "storage/ipc.h"
#include "storage/pmsignal.h"
#include "storage/proc.h"
#include "tcop/tcopprot.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/guc_tables.h"
#include "utils/memutils.h"
#include "utils/ps_status.h"

>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
/* User-settable parameters for sync rep */
char	   *SyncRepStandbyNames;

#define SyncStandbysDefined() \
	(SyncRepStandbyNames != NULL && SyncRepStandbyNames[0] != '\0')

static bool announce_next_takeover = true;

<<<<<<< HEAD
static int	SyncRepWaitMode = SYNC_REP_WAIT_FLUSH;

static void SyncRepQueueInsert(int mode);
=======
static void SyncRepQueueInsert(void);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
static void SyncRepCancelWait(void);

static int	SyncRepGetStandbyPriority(void);

#ifdef USE_ASSERT_CHECKING
<<<<<<< HEAD
static bool SyncRepQueueIsOrderedByLSN(int mode);
=======
static bool SyncRepQueueIsOrderedByLSN(void);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
#endif

/*
 * ===========================================================
 * Synchronous Replication functions for normal user backends
 * ===========================================================
 */

/*
 * Wait for synchronous replication, if requested by user.
 *
 * Initially backends start in state SYNC_REP_NOT_WAITING and then
 * change that state to SYNC_REP_WAITING before adding ourselves
 * to the wait queue. During SyncRepWakeQueue() a WALSender changes
 * the state to SYNC_REP_WAIT_COMPLETE once replication is confirmed.
 * This backend then resets its state to SYNC_REP_NOT_WAITING.
 */
void
SyncRepWaitForLSN(XLogRecPtr XactCommitLSN)
{
<<<<<<< HEAD
	const char	*old_status;
	char		*old_status_saved = NULL;
	int			len=0;
	int			mode = SyncRepWaitMode;
	bool		syncStandbyPresent = false;
	int			i = 0;

	/*
	 * SIGUSR1 is used to wake us up, cannot wait from inside SIGUSR1 handler
	 * as its non-reentrant, so check for the same and avoid waiting.
	 */
	if (AmIInSIGUSR1Handler())
	{
		return;
	}

	elogif(debug_walrepl_syncrep, LOG,
			"syncrep wait -- This backend's commit LSN for syncrep is %X/%X.",
			XactCommitLSN.xlogid,XactCommitLSN.xrecoff);

	/*
	 * Walsenders are not supposed to call this function, but currently
	 * basebackup needs to access catalog, hence open/close transaction.
	 * It doesn't make sense to wait for myself anyway.
	 */
	if (am_walsender)
	{
		elogif(debug_walrepl_syncrep, LOG,
				"syncrep wait -- Not waiting for syncrep as this process is a walsender.");
		return;
	}

	/*
	 * XXX: Some processes (e.g. auxiliary processes) don't need even
	 * check the walsenders.  We just stay away from any of the shared
	 * memory values.  When we move to fully-synchronous replication,
	 * any xlog change needs to be considered to be synchronous,
	 * but for now this is acceptable.
	 */
	if (MyProc->syncRepState == SYNC_REP_DISABLED)
	{
		elogif(debug_walrepl_syncrep, LOG,
				"syncrep wait -- Not waiting for syncrep as synrep state for this process is disabled.");

		return;
	}

	if (GpIdentity.segindex != MASTER_CONTENT_ID)
	{
		/* Fast exit if user has not requested sync replication. */
		if (!SyncRepRequested())
			return;
	}
=======
	char	   *new_status = NULL;
	const char *old_status;

	/*
	 * Fast exit if user has not requested sync replication, or there are no
	 * sync replication standby names defined. Note that those standbys don't
	 * need to be connected.
	 */
	if (!SyncRepRequested() || !SyncStandbysDefined())
		return;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

	Assert(SHMQueueIsDetached(&(MyProc->syncRepLinks)));
	Assert(WalSndCtl != NULL);

<<<<<<< HEAD
	LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);
	Assert(MyProc->syncRepState == SYNC_REP_NOT_WAITING);

	if (GpIdentity.segindex == MASTER_CONTENT_ID)
	{
		/*
		 * There could be a better way to figure out if there is any active
		 * standby.  But currently, let's move ahead by looking at the per WAL
		 * sender structure to see if anyone is really active, streaming (or
		 * still catching up within limits) and wants to be synchronous.
		 */
		for (i = 0; i < max_wal_senders; i++)
		{
			/* use volatile pointer to prevent code rearrangement */
			volatile WalSnd *walsnd = &WalSndCtl->walsnds[i];

			SpinLockAcquire(&walsnd->mutex);
			syncStandbyPresent = (walsnd->pid != 0)
				&& (walsnd->synchronous)
				&& ((walsnd->state == WALSNDSTATE_STREAMING)
					|| (walsnd->state == WALSNDSTATE_CATCHUP &&
						walsnd->caughtup_within_range));
			SpinLockRelease(&walsnd->mutex);

			if (syncStandbyPresent)
				break;
		}

		/* See if we found any active standby connected. If NO, no need to wait.*/
		if (!syncStandbyPresent)
		{
			elogif(debug_walrepl_syncrep, LOG,
					"syncrep wait -- Not waiting for syncrep because no active and synchronous "
					"standby (walsender) was found.");

			LWLockRelease(SyncRepLock);
			return;
		}
	}

=======
	/* Reset the latch before adding ourselves to the queue. */
	ResetLatch(&MyProc->waitLatch);

	LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);
	Assert(MyProc->syncRepState == SYNC_REP_NOT_WAITING);

>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	/*
	 * We don't wait for sync rep if WalSndCtl->sync_standbys_defined is not
	 * set.  See SyncRepUpdateSyncStandbysDefined.
	 *
	 * Also check that the standby hasn't already replied. Unlikely race
	 * condition but we'll be fetching that cache line anyway so its likely to
	 * be a low cost check.
	 */
<<<<<<< HEAD
	if (((GpIdentity.segindex != MASTER_CONTENT_ID) && !WalSndCtl->sync_standbys_defined) ||
		XLByteLE(XactCommitLSN, WalSndCtl->lsn[mode]))
	{
		elogif(debug_walrepl_syncrep, LOG,
				"syncrep wait -- Not waiting for syncrep because xlog upto LSN (%X/%X) which is "
				"greater than this backend's commit LSN (%X/%X) has already "
				"been replicated.",
				WalSndCtl->lsn[mode].xlogid, WalSndCtl->lsn[mode].xrecoff,
				XactCommitLSN.xlogid, XactCommitLSN.xrecoff);

=======
	if (!WalSndCtl->sync_standbys_defined ||
		XLByteLE(XactCommitLSN, WalSndCtl->lsn))
	{
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		LWLockRelease(SyncRepLock);
		return;
	}

	/*
	 * Set our waitLSN so WALSender will know when to wake us, and add
	 * ourselves to the queue.
	 */
	MyProc->waitLSN = XactCommitLSN;
	MyProc->syncRepState = SYNC_REP_WAITING;
<<<<<<< HEAD
	SyncRepQueueInsert(mode);
	Assert(SyncRepQueueIsOrderedByLSN(mode));
	LWLockRelease(SyncRepLock);

	elogif(debug_walrepl_syncrep, LOG,
			"syncrep wait -- This backend is now inserted in the syncrep queue.");

	/* Alter ps display to show waiting for sync rep. */
	if (update_process_title)
	{
		char		activitymsg[35];

		snprintf(activitymsg, sizeof(activitymsg), " waiting for %X/%X replication",
				XactCommitLSN.xlogid, XactCommitLSN.xrecoff);

		old_status = get_ps_display(&len);
		old_status_saved = (char *) palloc(len + 1);
		Assert (old_status_saved);
		memcpy(old_status_saved, old_status, len);
		set_ps_display(activitymsg, false);
	}

	/* Inform this backend is waiting for replication to pg_stat_activity */
	pgstat_report_waiting(PGBE_WAITING_REPLICATION);

=======
	SyncRepQueueInsert();
	Assert(SyncRepQueueIsOrderedByLSN());
	LWLockRelease(SyncRepLock);

	/* Alter ps display to show waiting for sync rep. */
	if (update_process_title)
	{
		int			len;

		old_status = get_ps_display(&len);
		new_status = (char *) palloc(len + 32 + 1);
		memcpy(new_status, old_status, len);
		sprintf(new_status + len, " waiting for %X/%X",
				XactCommitLSN.xlogid, XactCommitLSN.xrecoff);
		set_ps_display(new_status, false);
		new_status[len] = '\0'; /* truncate off " waiting ..." */
	}

>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	/*
	 * Wait for specified LSN to be confirmed.
	 *
	 * Each proc has its own wait latch, so we perform a normal latch
	 * check/wait loop here.
	 */
	for (;;)
	{
		int			syncRepState;

<<<<<<< HEAD
		/* Must reset the latch before testing state. */
		ResetLatch(&MyProc->procLatch);
=======
		/*
		 * Wait on latch for up to 60 seconds. This allows us to check for
		 * postmaster death regularly while waiting. Note that timeout here
		 * does not necessarily release from loop.
		 */
		WaitLatch(&MyProc->waitLatch, 60000000L);

		/* Must reset the latch before testing state. */
		ResetLatch(&MyProc->waitLatch);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

		/*
		 * Try checking the state without the lock first.  There's no
		 * guarantee that we'll read the most up-to-date value, so if it looks
		 * like we're still waiting, recheck while holding the lock.  But if
		 * it looks like we're done, we must really be done, because once
		 * walsender changes the state to SYNC_REP_WAIT_COMPLETE, it will
		 * never update it again, so we can't be seeing a stale value in that
		 * case.
<<<<<<< HEAD
		 *
		 * Note: on machines with weak memory ordering, the acquisition of the
		 * lock is essential to avoid race conditions: we cannot be sure the
		 * sender's state update has reached main memory until we acquire the
		 * lock.  We could get rid of this dance if SetLatch/ResetLatch
		 * contained memory barriers.
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		 */
		syncRepState = MyProc->syncRepState;
		if (syncRepState == SYNC_REP_WAITING)
		{
			LWLockAcquire(SyncRepLock, LW_SHARED);
			syncRepState = MyProc->syncRepState;
			LWLockRelease(SyncRepLock);
		}
		if (syncRepState == SYNC_REP_WAIT_COMPLETE)
<<<<<<< HEAD
		{
			elogif(debug_walrepl_syncrep, LOG,
					"syncrep wait -- This backend's syncrep state is now 'wait complete'.");
			break;
		}
=======
			break;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

		/*
		 * If a wait for synchronous replication is pending, we can neither
		 * acknowledge the commit nor raise ERROR or FATAL.  The latter would
		 * lead the client to believe that that the transaction aborted, which
		 * is not true: it's already committed locally. The former is no good
		 * either: the client has requested synchronous replication, and is
		 * entitled to assume that an acknowledged commit is also replicated,
<<<<<<< HEAD
		 * which might not be true. So in this case we issue a WARNING (which
=======
		 * which may not be true. So in this case we issue a WARNING (which
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		 * some clients may be able to interpret) and shut off further output.
		 * We do NOT reset ProcDiePending, so that the process will die after
		 * the commit is cleaned up.
		 */
		if (ProcDiePending)
		{
			ereport(WARNING,
					(errcode(ERRCODE_ADMIN_SHUTDOWN),
					 errmsg("canceling the wait for synchronous replication and terminating connection due to administrator command"),
<<<<<<< HEAD
					 errdetail("The transaction has already committed locally, but might not have been replicated to the standby.")));
=======
					 errdetail("The transaction has already committed locally, but may not have been replicated to the standby.")));
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			whereToSendOutput = DestNone;
			SyncRepCancelWait();
			break;
		}

		/*
<<<<<<< HEAD
		 * GPDB: There are multiple code paths going through this function,
		 * e.g. prepare, commit, and abort. To ensure MPP cluster consistency,
		 * if primary already changed, then this backend has to wait for the
		 * xlog record replicate to the mirror to avoid inconsistency between
		 * the primary and the mirror, since they are under synced replication.
		 *
		 * If the mirror is indeed offline and prevents xlog to be synced, FTS
		 * will detect the mirror goes down, and failure handling will kick-in
		 * and mark the mirror down and out-of-sync with the primary to prevent
		 * failover. Then the syncrep will be turned off by the FTS to unblock
		 * backends waiting here.
=======
		 * It's unclear what to do if a query cancel interrupt arrives.  We
		 * can't actually abort at this point, but ignoring the interrupt
		 * altogether is not helpful, so we just terminate the wait with a
		 * suitable warning.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		 */
		if (QueryCancelPending)
		{
			QueryCancelPending = false;
			ereport(WARNING,
<<<<<<< HEAD
					(errmsg("ignoring query cancel request for synchronous replication to ensure cluster consistency."),
					 errdetail("The transaction has already changed locally, it has to be replicated to standby.")));
			SIMPLE_FAULT_INJECTOR(SyncRepQueryCancel);
=======
					(errmsg("canceling wait for synchronous replication due to user request"),
					 errdetail("The transaction has already committed locally, but may not have been replicated to the standby.")));
			SyncRepCancelWait();
			break;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		}

		/*
		 * If the postmaster dies, we'll probably never get an
		 * acknowledgement, because all the wal sender processes will exit. So
		 * just bail out.
		 */
		if (!PostmasterIsAlive(true))
		{
			ProcDiePending = true;
			whereToSendOutput = DestNone;
			SyncRepCancelWait();
			break;
		}
<<<<<<< HEAD

		elogif(debug_walrepl_syncrep, LOG,
				"syncrep wait -- This backend's syncrep state is still 'waiting'. "
				"Hence it will wait on a latch until awakend.")
		/*
		 * Wait on latch.  Any condition that should wake us up will set the
		 * latch, so no need for timeout.
		 */
		WaitLatch(&MyProc->procLatch, WL_LATCH_SET | WL_POSTMASTER_DEATH, -1);
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}

	/*
	 * WalSender has checked our LSN and has removed us from queue. Clean up
	 * state and leave.  It's OK to reset these shared memory fields without
	 * holding SyncRepLock, because any walsenders will ignore us anyway when
	 * we're not on the queue.
	 */
	Assert(SHMQueueIsDetached(&(MyProc->syncRepLinks)));
	MyProc->syncRepState = SYNC_REP_NOT_WAITING;
	MyProc->waitLSN.xlogid = 0;
	MyProc->waitLSN.xrecoff = 0;

<<<<<<< HEAD
	if (update_process_title)
	{
		Assert(old_status_saved);
		set_ps_display(old_status_saved, false);
		pfree(old_status_saved);
	}

	/* Now inform no more waiting for replication */
	pgstat_report_waiting(PGBE_WAITING_NONE);
}

/*
 * Insert MyProc into the specified SyncRepQueue, maintaining sorted invariant.
=======
	if (new_status)
	{
		/* Reset ps display */
		set_ps_display(new_status, false);
		pfree(new_status);
	}
}

/*
 * Insert MyProc into SyncRepQueue, maintaining sorted invariant.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * Usually we will go at tail of queue, though it's possible that we arrive
 * here out of order, so start at tail and work back to insertion point.
 */
static void
<<<<<<< HEAD
SyncRepQueueInsert(int mode)
{
	PGPROC	   *proc;

	Assert(mode >= 0 && mode < NUM_SYNC_REP_WAIT_MODE);
	proc = (PGPROC *) SHMQueuePrev(&(WalSndCtl->SyncRepQueue[mode]),
								   &(WalSndCtl->SyncRepQueue[mode]),
=======
SyncRepQueueInsert(void)
{
	PGPROC	   *proc;

	proc = (PGPROC *) SHMQueuePrev(&(WalSndCtl->SyncRepQueue),
								   &(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
								   offsetof(PGPROC, syncRepLinks));

	while (proc)
	{
		/*
		 * Stop at the queue element that we should after to ensure the queue
		 * is ordered by LSN.
		 */
		if (XLByteLT(proc->waitLSN, MyProc->waitLSN))
			break;

<<<<<<< HEAD
		proc = (PGPROC *) SHMQueuePrev(&(WalSndCtl->SyncRepQueue[mode]),
=======
		proc = (PGPROC *) SHMQueuePrev(&(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
									   &(proc->syncRepLinks),
									   offsetof(PGPROC, syncRepLinks));
	}

	if (proc)
		SHMQueueInsertAfter(&(proc->syncRepLinks), &(MyProc->syncRepLinks));
	else
<<<<<<< HEAD
		SHMQueueInsertAfter(&(WalSndCtl->SyncRepQueue[mode]), &(MyProc->syncRepLinks));
=======
		SHMQueueInsertAfter(&(WalSndCtl->SyncRepQueue), &(MyProc->syncRepLinks));
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
}

/*
 * Acquire SyncRepLock and cancel any wait currently in progress.
 */
static void
SyncRepCancelWait(void)
{
	LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);
	if (!SHMQueueIsDetached(&(MyProc->syncRepLinks)))
		SHMQueueDelete(&(MyProc->syncRepLinks));
	MyProc->syncRepState = SYNC_REP_NOT_WAITING;
	LWLockRelease(SyncRepLock);
}

void
<<<<<<< HEAD
SyncRepCleanupAtProcExit(void)
=======
SyncRepCleanupAtProcExit(int code, Datum arg)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
{
	if (!SHMQueueIsDetached(&(MyProc->syncRepLinks)))
	{
		LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);
		SHMQueueDelete(&(MyProc->syncRepLinks));
		LWLockRelease(SyncRepLock);
	}
<<<<<<< HEAD
=======

	DisownLatch(&MyProc->waitLatch);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
}

/*
 * ===========================================================
 * Synchronous Replication functions for wal sender processes
 * ===========================================================
 */

/*
 * Take any action required to initialise sync rep state from config
 * data. Called at WALSender startup and after each SIGHUP.
 */
void
SyncRepInitConfig(void)
{
	int			priority;

	/*
	 * Determine if we are a potential sync standby and remember the result
	 * for handling replies from standby.
	 */
	priority = SyncRepGetStandbyPriority();
	if (MyWalSnd->sync_standby_priority != priority)
	{
		LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);
		MyWalSnd->sync_standby_priority = priority;
		LWLockRelease(SyncRepLock);
<<<<<<< HEAD
		ereport(LOG,
			(errmsg("standby now has synchronous standby priority %u", priority)));
=======
		ereport(DEBUG1,
			(errmsg("standby \"%s\" now has synchronous standby priority %u",
					application_name, priority)));
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}
}

/*
 * Update the LSNs on each queue based upon our latest state. This
 * implements a simple policy of first-valid-standby-releases-waiter.
 *
<<<<<<< HEAD
 * Other policies are possible, which would change what we do here and
=======
 * Other policies are possible, which would change what we do here and what
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * perhaps also which information we store as well.
 */
void
SyncRepReleaseWaiters(void)
{
	volatile WalSndCtlData *walsndctl = WalSndCtl;
	volatile WalSnd *syncWalSnd = NULL;
<<<<<<< HEAD
	int			numwrite = 0;
	int			numflush = 0;
=======
	int			numprocs = 0;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	int			priority = 0;
	int			i;

	/*
<<<<<<< HEAD
	 * If this WALSender doesn't have any priority then we have nothing to do.
	 * If we are still starting up, still running base backup or the current
	 * flush position is still invalid, then leave quickly also.
	 */
	if (MyWalSnd->sync_standby_priority == 0 ||
		MyWalSnd->state < WALSNDSTATE_STREAMING ||
		XLogRecPtrIsInvalid(MyWalSnd->flush))
=======
	 * If this WALSender is serving a standby that is not on the list of
	 * potential standbys then we have nothing to do. If we are still starting
	 * up or still running base backup, then leave quickly also.
	 */
	if (MyWalSnd->sync_standby_priority == 0 ||
		MyWalSnd->state < WALSNDSTATE_STREAMING)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		return;

	/*
	 * We're a potential sync standby. Release waiters if we are the highest
	 * priority standby. If there are multiple standbys with same priorities
	 * then we use the first mentioned standby. If you change this, also
	 * change pg_stat_get_wal_senders().
	 */
	LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);

	for (i = 0; i < max_wal_senders; i++)
	{
		/* use volatile pointer to prevent code rearrangement */
		volatile WalSnd *walsnd = &walsndctl->walsnds[i];

		if (walsnd->pid != 0 &&
<<<<<<< HEAD
			walsnd->state == WALSNDSTATE_STREAMING &&
			walsnd->sync_standby_priority > 0 &&
			(priority == 0 ||
			 priority > walsnd->sync_standby_priority) &&
			(!XLogRecPtrIsInvalid(walsnd->flush)))
=======
			walsnd->sync_standby_priority > 0 &&
			(priority == 0 ||
			 priority > walsnd->sync_standby_priority))
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		{
			priority = walsnd->sync_standby_priority;
			syncWalSnd = walsnd;
		}
	}

	/*
	 * We should have found ourselves at least.
	 */
	Assert(syncWalSnd);

	/*
	 * If we aren't managing the highest priority standby then just leave.
	 */
	if (syncWalSnd != MyWalSnd)
	{
		LWLockRelease(SyncRepLock);
		announce_next_takeover = true;
		return;
	}

<<<<<<< HEAD
	/*
	 * Set the lsn first so that when we wake backends they will release up to
	 * this location.
	 */
	if (XLByteLT(walsndctl->lsn[SYNC_REP_WAIT_WRITE], MyWalSnd->write))
	{
		walsndctl->lsn[SYNC_REP_WAIT_WRITE] = MyWalSnd->write;
		numwrite = SyncRepWakeQueue(false, SYNC_REP_WAIT_WRITE);
	}
	if (XLByteLT(walsndctl->lsn[SYNC_REP_WAIT_FLUSH], MyWalSnd->flush))
	{
		walsndctl->lsn[SYNC_REP_WAIT_FLUSH] = MyWalSnd->flush;
		numflush = SyncRepWakeQueue(false, SYNC_REP_WAIT_FLUSH);
=======
	if (XLByteLT(walsndctl->lsn, MyWalSnd->flush))
	{
		/*
		 * Set the lsn first so that when we wake backends they will release
		 * up to this location.
		 */
		walsndctl->lsn = MyWalSnd->flush;
		numprocs = SyncRepWakeQueue(false);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}

	LWLockRelease(SyncRepLock);

<<<<<<< HEAD
	elogif(debug_walrepl_syncrep, LOG,
		"syncrep release -- Released %d processe(s) up to write %X/%X, %d processe(s) up to flush %X/%X",
		 numwrite,
		 MyWalSnd->write.xlogid,
		 MyWalSnd->write.xrecoff,
		 numflush,
=======
	elog(DEBUG3, "released %d procs up to %X/%X",
		 numprocs,
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		 MyWalSnd->flush.xlogid,
		 MyWalSnd->flush.xrecoff);

	/*
	 * If we are managing the highest priority standby, though we weren't
	 * prior to this, then announce we are now the sync standby.
	 */
	if (announce_next_takeover)
	{
		announce_next_takeover = false;
<<<<<<< HEAD
		elogif(debug_walrepl_syncrep, LOG,
			   "syncrep release -- standby is now the synchronous standby with priority %u",
			   MyWalSnd->sync_standby_priority);
=======
		ereport(LOG,
				(errmsg("standby \"%s\" is now the synchronous standby with priority %u",
						application_name, MyWalSnd->sync_standby_priority)));
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}
}

/*
 * Check if we are in the list of sync standbys, and if so, determine
 * priority sequence. Return priority if set, or zero to indicate that
 * we are not a potential sync standby.
 *
<<<<<<< HEAD
 * **Note:- Currently, GPDB does NOT apply the concept of standby priority as
 *   we allow max 1 walsender to be alive at a time. Hence, this function returns
 *   1.**
=======
 * Compare the parameter SyncRepStandbyNames against the application_name
 * for this WALSender, or allow any name if we find a wildcard "*".
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 */
static int
SyncRepGetStandbyPriority(void)
{
<<<<<<< HEAD
	/* Currently set the priority to 1 */
	return 1;
}

/*
 * Walk the specified queue from head.  Set the state of any backends that
 * need to be woken, remove them from the queue, and then wake them.
 * Pass all = true to wake whole queue; otherwise, just wake up to
 * the walsender's LSN.
=======
	char	   *rawstring;
	List	   *elemlist;
	ListCell   *l;
	int			priority = 0;
	bool		found = false;

	/* Need a modifiable copy of string */
	rawstring = pstrdup(SyncRepStandbyNames);

	/* Parse string into list of identifiers */
	if (!SplitIdentifierString(rawstring, ',', &elemlist))
	{
		/* syntax error in list */
		pfree(rawstring);
		list_free(elemlist);
		/* GUC machinery will have already complained - no need to do again */
		return 0;
	}

	foreach(l, elemlist)
	{
		char	   *standby_name = (char *) lfirst(l);

		priority++;

		if (pg_strcasecmp(standby_name, application_name) == 0 ||
			pg_strcasecmp(standby_name, "*") == 0)
		{
			found = true;
			break;
		}
	}

	pfree(rawstring);
	list_free(elemlist);

	return (found ? priority : 0);
}

/*
 * Walk queue from head.  Set the state of any backends that need to be woken,
 * remove them from the queue, and then wake them.	Pass all = true to wake
 * whole queue; otherwise, just wake up to the walsender's LSN.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * Must hold SyncRepLock.
 */
int
<<<<<<< HEAD
SyncRepWakeQueue(bool all, int mode)
=======
SyncRepWakeQueue(bool all)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
{
	volatile WalSndCtlData *walsndctl = WalSndCtl;
	PGPROC	   *proc = NULL;
	PGPROC	   *thisproc = NULL;
	int			numprocs = 0;

<<<<<<< HEAD
	Assert(mode >= 0 && mode < NUM_SYNC_REP_WAIT_MODE);
	Assert(SyncRepQueueIsOrderedByLSN(mode));

	proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue[mode]),
								   &(WalSndCtl->SyncRepQueue[mode]),
=======
	Assert(SyncRepQueueIsOrderedByLSN());

	proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue),
								   &(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
								   offsetof(PGPROC, syncRepLinks));

	while (proc)
	{
		/*
		 * Assume the queue is ordered by LSN
		 */
<<<<<<< HEAD
		if (!all && XLByteLT(walsndctl->lsn[mode], proc->waitLSN))
=======
		if (!all && XLByteLT(walsndctl->lsn, proc->waitLSN))
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			return numprocs;

		/*
		 * Move to next proc, so we can delete thisproc from the queue.
		 * thisproc is valid, proc may be NULL after this.
		 */
		thisproc = proc;
<<<<<<< HEAD
		proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue[mode]),
=======
		proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
									   &(proc->syncRepLinks),
									   offsetof(PGPROC, syncRepLinks));

		/*
<<<<<<< HEAD
		 * Remove thisproc from queue.
		 */
		SHMQueueDelete(&(thisproc->syncRepLinks));

		/*
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		 * Set state to complete; see SyncRepWaitForLSN() for discussion of
		 * the various states.
		 */
		thisproc->syncRepState = SYNC_REP_WAIT_COMPLETE;

		/*
<<<<<<< HEAD
		 * Wake only when we have set state and removed from queue.
		 */
		SetLatch(&(thisproc->procLatch));

		elogif(debug_walrepl_syncrep, LOG,
				"syncrep wakeup queue -- %d procid was removed from syncrep queue. "
				"Its state is changed to 'Wait Complete' and "
				"its latch is now set",
				thisproc->pid);
=======
		 * Remove thisproc from queue.
		 */
		SHMQueueDelete(&(thisproc->syncRepLinks));

		/*
		 * Wake only when we have set state and removed from queue.
		 */
		Assert(SHMQueueIsDetached(&(thisproc->syncRepLinks)));
		Assert(thisproc->syncRepState == SYNC_REP_WAIT_COMPLETE);
		SetLatch(&(thisproc->waitLatch));
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

		numprocs++;
	}

	return numprocs;
}

/*
<<<<<<< HEAD
 * The checkpointer calls this as needed to update the shared
=======
 * The background writer calls this as needed to update the shared
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * sync_standbys_defined flag, so that backends don't remain permanently wedged
 * if synchronous_standby_names is unset.  It's safe to check the current value
 * without the lock, because it's only ever updated by one process.  But we
 * must take the lock to change it.
 */
void
SyncRepUpdateSyncStandbysDefined(void)
{
	bool		sync_standbys_defined = SyncStandbysDefined();

	if (sync_standbys_defined != WalSndCtl->sync_standbys_defined)
	{
		LWLockAcquire(SyncRepLock, LW_EXCLUSIVE);

		/*
		 * If synchronous_standby_names has been reset to empty, it's futile
		 * for backends to continue to waiting.  Since the user no longer
		 * wants synchronous replication, we'd better wake them up.
		 */
		if (!sync_standbys_defined)
<<<<<<< HEAD
		{
			int			i;

			for (i = 0; i < NUM_SYNC_REP_WAIT_MODE; i++)
				SyncRepWakeQueue(true, i);
		}
=======
			SyncRepWakeQueue(true);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

		/*
		 * Only allow people to join the queue when there are synchronous
		 * standbys defined.  Without this interlock, there's a race
		 * condition: we might wake up all the current waiters; then, some
		 * backend that hasn't yet reloaded its config might go to sleep on
		 * the queue (and never wake up).  This prevents that.
		 */
		WalSndCtl->sync_standbys_defined = sync_standbys_defined;

		LWLockRelease(SyncRepLock);
	}
}

#ifdef USE_ASSERT_CHECKING
static bool
<<<<<<< HEAD
SyncRepQueueIsOrderedByLSN(int mode)
=======
SyncRepQueueIsOrderedByLSN(void)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
{
	PGPROC	   *proc = NULL;
	XLogRecPtr	lastLSN;

<<<<<<< HEAD
	Assert(mode >= 0 && mode < NUM_SYNC_REP_WAIT_MODE);

	lastLSN.xlogid = 0;
	lastLSN.xrecoff = 0;

	proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue[mode]),
								   &(WalSndCtl->SyncRepQueue[mode]),
=======
	lastLSN.xlogid = 0;
	lastLSN.xrecoff = 0;

	proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue),
								   &(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
								   offsetof(PGPROC, syncRepLinks));

	while (proc)
	{
		/*
		 * Check the queue is ordered by LSN and that multiple procs don't
		 * have matching LSNs
		 */
		if (XLByteLE(proc->waitLSN, lastLSN))
			return false;

		lastLSN = proc->waitLSN;

<<<<<<< HEAD
		proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue[mode]),
=======
		proc = (PGPROC *) SHMQueueNext(&(WalSndCtl->SyncRepQueue),
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
									   &(proc->syncRepLinks),
									   offsetof(PGPROC, syncRepLinks));
	}

	return true;
}
#endif

<<<<<<< HEAD
const char *
check_synchronous_standby_names(const char *newval, bool doit, GucSource source)
=======
/*
 * ===========================================================
 * Synchronous Replication functions executed by any process
 * ===========================================================
 */

bool
check_synchronous_standby_names(char **newval, void **extra, GucSource source)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
{
	char	   *rawstring;
	List	   *elemlist;

	/* Need a modifiable copy of string */
<<<<<<< HEAD
	rawstring = strdup(newval);
	if (rawstring == NULL)
	{
		ereport(GUC_complaint_elevel(source),
				(errcode(ERRCODE_OUT_OF_MEMORY),
				 errmsg("out of memory")));
		return NULL;
	}
=======
	rawstring = pstrdup(*newval);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

	/* Parse string into list of identifiers */
	if (!SplitIdentifierString(rawstring, ',', &elemlist))
	{
<<<<<<< HEAD
		free(rawstring);
		list_free(elemlist);

		/* syntax error in list */
		ereport(GUC_complaint_elevel(source),
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("invalid list syntax for parameter \"synchronous_standby_names\"")));

		return NULL;
=======
		/* syntax error in list */
		GUC_check_errdetail("List syntax is invalid.");
		pfree(rawstring);
		list_free(elemlist);
		return false;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}

	/*
	 * Any additional validation of standby names should go here.
	 *
	 * Don't attempt to set WALSender priority because this is executed by
	 * postmaster at startup, not WALSender, so the application_name is not
	 * yet correctly set.
	 */
<<<<<<< HEAD
	free(rawstring);
	list_free(elemlist);

	return newval;
=======

	pfree(rawstring);
	list_free(elemlist);

	return true;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
}
