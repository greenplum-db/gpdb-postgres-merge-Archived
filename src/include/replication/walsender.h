/*-------------------------------------------------------------------------
 *
 * walsender.h
 *	  Exports from replication/walsender.c.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
 *
 * src/include/replication/walsender.h
=======
 * Portions Copyright (c) 2010-2010, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/include/replication/walsender.h,v 1.4 2010/06/17 00:06:34 itagaki Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 *
 *-------------------------------------------------------------------------
 */
#ifndef _WALSENDER_H
#define _WALSENDER_H

<<<<<<< HEAD
#include <signal.h>

#include "fmgr.h"
#include "access/xlogdefs.h"

/* global state */
extern bool am_walsender;
extern bool am_cascading_walsender;
extern volatile sig_atomic_t walsender_ready_to_stop;

/* user-settable parameters */
extern int	max_wal_senders;
extern int	replication_timeout;
extern int	repl_catchup_within_range;

extern void InitWalSender(void);
extern void exec_replication_command(const char *query_string);
extern void WalSndErrorCleanup(void);
extern void WalSndSignals(void);
extern Size WalSndShmemSize(void);
extern void WalSndShmemInit(void);
extern void WalSndWakeup(void);
extern void WalSndRqstFileReload(void);
extern XLogRecPtr WalSndCtlGetXLogCleanUpTo(void);
extern void WalSndSetXLogCleanUpTo(XLogRecPtr xlogPtr);
extern Datum pg_stat_get_wal_senders(PG_FUNCTION_ARGS);
=======
#include "access/xlog.h"
#include "storage/spin.h"

/*
 * Each walsender has a WalSnd struct in shared memory.
 */
typedef struct WalSnd
{
	pid_t		pid;			/* this walsender's process id, or 0 */
	XLogRecPtr	sentPtr;		/* WAL has been sent up to this point */

	slock_t		mutex;			/* locks shared variables shown above */
} WalSnd;

/* There is one WalSndCtl struct for the whole database cluster */
typedef struct
{
	WalSnd		walsnds[1];		/* VARIABLE LENGTH ARRAY */
} WalSndCtlData;

extern WalSndCtlData *WalSndCtl;

/* global state */
extern bool am_walsender;

/* user-settable parameters */
extern int	WalSndDelay;
extern int	max_wal_senders;

extern int	WalSenderMain(void);
extern void WalSndSignals(void);
extern Size WalSndShmemSize(void);
extern void WalSndShmemInit(void);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

#endif   /* _WALSENDER_H */
