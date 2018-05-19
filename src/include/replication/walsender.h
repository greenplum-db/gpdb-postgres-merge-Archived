/*-------------------------------------------------------------------------
 *
 * walsender.h
 *	  Exports from replication/walsender.c.
 *
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
 *
 * src/include/replication/walsender.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef _WALSENDER_H
#define _WALSENDER_H

#include <signal.h>

#include "fmgr.h"
<<<<<<< HEAD
#include "access/xlogdefs.h"
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/* global state */
extern bool am_walsender;
extern bool am_cascading_walsender;
<<<<<<< HEAD
=======
extern volatile sig_atomic_t walsender_shutdown_requested;
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
extern volatile sig_atomic_t walsender_ready_to_stop;

/* user-settable parameters */
extern int	max_wal_senders;
extern int	replication_timeout;
extern int	repl_catchup_within_range;

extern int	WalSenderMain(void);
extern void InitWalSender(void);
extern void exec_replication_command(const char *query_string);
extern void WalSndErrorCleanup(void);
extern void WalSndSignals(void);
extern Size WalSndShmemSize(void);
extern void WalSndShmemInit(void);
extern void WalSndWakeup(void);
extern void WalSndRqstFileReload(void);
<<<<<<< HEAD
extern XLogRecPtr WalSndCtlGetXLogCleanUpTo(void);
extern void WalSndSetXLogCleanUpTo(XLogRecPtr xlogPtr);
=======

>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
extern Datum pg_stat_get_wal_senders(PG_FUNCTION_ARGS);

#endif   /* _WALSENDER_H */
