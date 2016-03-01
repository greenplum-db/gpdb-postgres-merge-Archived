/*-------------------------------------------------------------------------
 *
 * pmsignal.h
 *	  routines for signaling the postmaster from its child processes
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/pmsignal.h,v 1.25 2009/06/11 14:49:12 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/pmsignal.h,v 1.19 2008/01/01 19:45:59 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef PMSIGNAL_H
#define PMSIGNAL_H

/*
 * Reasons for signaling the postmaster.  We can cope with simultaneous
 * signals for different reasons.  If the same reason is signaled multiple
 * times in quick succession, however, the postmaster is likely to observe
 * only one notification of it.  This is okay for the present uses.
 */
typedef enum
{
	PMSIGNAL_RECOVERY_STARTED,	/* recovery has started */
	PMSIGNAL_RECOVERY_CONSISTENT,		/* recovery has reached consistent
										 * state */
	PMSIGNAL_PASSWORD_CHANGE,	/* pg_auth file has changed */
	PMSIGNAL_WAKEN_ARCHIVER,	/* send a NOTIFY signal to xlog archiver */
	PMSIGNAL_ROTATE_LOGFILE,	/* send SIGUSR1 to syslogger to rotate logfile */
	PMSIGNAL_START_AUTOVAC_LAUNCHER,	/* start an autovacuum launcher */
	PMSIGNAL_START_AUTOVAC_WORKER,		/* start an autovacuum worker */
<<<<<<< HEAD

	PMSIGNAL_START_AUTOVAC,
	PMSIGNAL_START_WALRECEIVER, /* start a walreceiver */

	PMSIGNAL_FILEREP_STATE_CHANGE,	      /* filerep is reporting state change */ 

	PMSIGNAL_PRIMARY_MIRROR_TRANSITION_RECEIVED, /* a primary mirror transition has been received by a backend */
	PMSIGNAL_PRIMARY_MIRROR_ALL_BACKENDS_SHUTDOWN, /* filerep has shut down all backends */

	/* a filerep subprocess crashed in a way that requires postmaster reset */
    PMSIGNAL_POSTMASTER_RESET_FILEREP,

    /* peer segment requested postmaster reset */
    PMSIGNAL_POSTMASTER_RESET_BY_PEER,

	PMSIGNAL_SEGCONFIG_CHANGE,	/* segment configuration hs changed */
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	NUM_PMSIGNALS				/* Must be last value of enum! */
} PMSignalReason;

/* PMSignalData is an opaque struct, details known only within pmsignal.c */
typedef struct PMSignalData PMSignalData;

/*
 * prototypes for functions in pmsignal.c
 */
extern Size PMSignalShmemSize(void);
extern void PMSignalShmemInit(void);
extern void SendPostmasterSignal(PMSignalReason reason);
extern bool CheckPostmasterSignal(PMSignalReason reason);
extern int	AssignPostmasterChildSlot(void);
extern bool ReleasePostmasterChildSlot(int slot);
extern bool IsPostmasterChildWalSender(int slot);
extern void MarkPostmasterChildActive(void);
extern void MarkPostmasterChildWalSender(void);
extern void MarkPostmasterChildInactive(void);
extern bool PostmasterIsAlive(bool amDirectChild);
extern bool ParentProcIsAlive(void);

#endif   /* PMSIGNAL_H */
