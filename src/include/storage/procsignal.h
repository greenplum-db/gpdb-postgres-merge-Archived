/*-------------------------------------------------------------------------
 *
 * procsignal.h
 *	  Routines for interprocess signalling
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/procsignal.h
=======
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/procsignal.h,v 1.6 2010/02/26 02:01:28 momjian Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 *
 *-------------------------------------------------------------------------
 */
#ifndef PROCSIGNAL_H
#define PROCSIGNAL_H

#include "storage/backendid.h"


/*
 * Reasons for signalling a Postgres child process (a backend or an auxiliary
 * process, like checkpointer).  We can cope with concurrent signals for different
 * reasons.  However, if the same reason is signaled multiple times in quick
 * succession, the process is likely to observe only one notification of it.
 * This is okay for the present uses.
 *
 * Also, because of race conditions, it's important that all the signals be
 * defined so that no harm is done if a process mistakenly receives one.
 */
typedef enum
{
	PROCSIG_CATCHUP_INTERRUPT,	/* sinval catchup interrupt */
	PROCSIG_NOTIFY_INTERRUPT,	/* listen/notify interrupt */

<<<<<<< HEAD
	PROCSIG_QUERY_FINISH,		/* query finish */
=======
	/* Recovery conflict reasons */
	PROCSIG_RECOVERY_CONFLICT_DATABASE,
	PROCSIG_RECOVERY_CONFLICT_TABLESPACE,
	PROCSIG_RECOVERY_CONFLICT_LOCK,
	PROCSIG_RECOVERY_CONFLICT_SNAPSHOT,
	PROCSIG_RECOVERY_CONFLICT_BUFFERPIN,
	PROCSIG_RECOVERY_CONFLICT_STARTUP_DEADLOCK,
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

	NUM_PROCSIGNALS				/* Must be last! */
} ProcSignalReason;

/*
 * prototypes for functions in procsignal.c
 */
extern Size ProcSignalShmemSize(void);
extern void ProcSignalShmemInit(void);

extern void ProcSignalInit(int pss_idx);
extern int SendProcSignal(pid_t pid, ProcSignalReason reason,
			   BackendId backendId);

extern void procsignal_sigusr1_handler(SIGNAL_ARGS);
extern bool AmIInSIGUSR1Handler(void);

#endif   /* PROCSIGNAL_H */
