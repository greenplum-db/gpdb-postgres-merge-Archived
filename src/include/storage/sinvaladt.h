/*-------------------------------------------------------------------------
 *
 * sinvaladt.h
 *	  POSTGRES shared cache invalidation data manager.
 *
<<<<<<< HEAD
=======
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/sinvaladt.h,v 1.45 2008/01/01 19:45:59 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef SINVALADT_H
#define SINVALADT_H

#include "storage/sinval.h"


/*
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * The shared cache invalidation manager is responsible for transmitting
 * invalidation messages between backends.	Any message sent by any backend
 * must be delivered to all already-running backends before it can be
 * forgotten.  (If we run out of space, we instead deliver a "RESET"
 * message to backends that have fallen too far behind.)
 *
 * The struct type SharedInvalidationMessage, defining the contents of
 * a single message, is defined in sinval.h.
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/sinvaladt.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef SINVALADT_H
#define SINVALADT_H

<<<<<<< HEAD
#include "storage/sinval.h"
=======
#define MAXNUMMESSAGES 4096
#define MSGNUMWRAPAROUND (MAXNUMMESSAGES * 4096)


/* Per-backend state in shared invalidation structure */
typedef struct ProcState
{
	/* nextMsgNum is -1 in an inactive ProcState array entry. */
	int			nextMsgNum;		/* next message number to read, or -1 */
	bool		resetState;		/* true, if backend has to reset its state */
} ProcState;

/* Shared cache invalidation memory segment */
typedef struct SISeg
{
	/*
	 * General state information
	 */
	int			minMsgNum;		/* oldest message still needed */
	int			maxMsgNum;		/* next message number to be assigned */
	int			lastBackend;	/* index of last active procState entry, +1 */
	int			maxBackends;	/* size of procState array */
	int			freeBackends;	/* number of empty procState slots */

	/*
	 * Next LocalTransactionId to use for each idle backend slot.  We keep
	 * this here because it is indexed by BackendId and it is convenient to
	 * copy the value to and from local memory when MyBackendId is set.
	 */
	LocalTransactionId *nextLXID;		/* array of maxBackends entries */

	/*
	 * Circular buffer holding shared-inval messages
	 */
	SharedInvalidationMessage buffer[MAXNUMMESSAGES];

	/*
	 * Per-backend state info.
	 *
	 * We declare procState as 1 entry because C wants a fixed-size array, but
	 * actually it is maxBackends entries long.
	 */
	ProcState	procState[1];	/* reflects the invalidation state */
} SISeg;


extern SISeg *shmInvalBuffer;	/* pointer to the shared inval buffer */

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

/*
 * prototypes for functions in sinvaladt.c
 */
extern Size SInvalShmemSize(void);
extern void CreateSharedInvalidationState(void);
extern void SharedInvalBackendInit(bool sendOnly);

extern void SIInsertDataEntries(const SharedInvalidationMessage *data, int n);
extern int	SIGetDataEntries(SharedInvalidationMessage *data, int datasize);
extern void SICleanupQueue(bool callerHasWriteLock, int minFree);

extern LocalTransactionId GetNextLocalTransactionId(void);

#endif   /* SINVALADT_H */
