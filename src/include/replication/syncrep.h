/*-------------------------------------------------------------------------
 *
 * syncrep.h
 *	  Exports from replication/syncrep.c.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 2010-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * IDENTIFICATION
 *		src/include/replication/syncrep.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef _SYNCREP_H
#define _SYNCREP_H

<<<<<<< HEAD
#include "access/xlogdefs.h"
#include "utils/guc.h"

#define SyncRepRequested() \
	(max_wal_senders > 0)

/* SyncRepWaitMode */
#define SYNC_REP_NO_WAIT		-1
#define SYNC_REP_WAIT_WRITE		0
#define SYNC_REP_WAIT_FLUSH		1

#define NUM_SYNC_REP_WAIT_MODE	2

/* syncRepState */
#define SYNC_REP_DISABLED			-1
=======
#include "access/xlog.h"
#include "storage/proc.h"
#include "storage/shmem.h"
#include "storage/spin.h"
#include "utils/guc.h"

#define SyncRepRequested() \
	(max_wal_senders > 0 && synchronous_commit > SYNCHRONOUS_COMMIT_LOCAL_FLUSH)

/* syncRepState */
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
#define SYNC_REP_NOT_WAITING		0
#define SYNC_REP_WAITING			1
#define SYNC_REP_WAIT_COMPLETE		2

/* user-settable parameters for synchronous replication */
extern char *SyncRepStandbyNames;

/* called by user backend */
extern void SyncRepWaitForLSN(XLogRecPtr XactCommitLSN);

<<<<<<< HEAD
/* called at backend exit */
extern void SyncRepCleanupAtProcExit(void);
=======
/* callback at backend exit */
extern void SyncRepCleanupAtProcExit(int code, Datum arg);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

/* called by wal sender */
extern void SyncRepInitConfig(void);
extern void SyncRepReleaseWaiters(void);

<<<<<<< HEAD
/* called by checkpointer */
extern void SyncRepUpdateSyncStandbysDefined(void);

/* called by various procs */
extern int     SyncRepWakeQueue(bool all, int mode);

extern const char *check_synchronous_standby_names(const char *newval, bool doit, GucSource source);
=======
/* called by wal writer */
extern void SyncRepUpdateSyncStandbysDefined(void);

/* called by various procs */
extern int	SyncRepWakeQueue(bool all);
extern bool check_synchronous_standby_names(char **newval, void **extra, GucSource source);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* _SYNCREP_H */
