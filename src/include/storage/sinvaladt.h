/*-------------------------------------------------------------------------
 *
 * sinvaladt.h
 *	  POSTGRES shared cache invalidation data manager.
 *
 * The shared cache invalidation manager is responsible for transmitting
 * invalidation messages between backends.	Any message sent by any backend
 * must be delivered to all already-running backends before it can be
 * forgotten.  (If we run out of space, we instead deliver a "RESET"
 * message to backends that have fallen too far behind.)
<<<<<<< HEAD
 *
=======
 * 
>>>>>>> 49f001d81e
 * The struct type SharedInvalidationMessage, defining the contents of
 * a single message, is defined in sinval.h.
 *
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
<<<<<<< HEAD
 * src/include/storage/sinvaladt.h
=======
 * $PostgreSQL: pgsql/src/include/storage/sinvaladt.h,v 1.49 2008/07/01 02:09:34 tgl Exp $
>>>>>>> 49f001d81e
 *
 *-------------------------------------------------------------------------
 */
#ifndef SINVALADT_H
#define SINVALADT_H

#include "storage/sinval.h"

/*
 * prototypes for functions in sinvaladt.c
 */
extern Size SInvalShmemSize(void);
extern void CreateSharedInvalidationState(void);
<<<<<<< HEAD
extern void SharedInvalBackendInit(bool sendOnly);

extern void SIInsertDataEntries(const SharedInvalidationMessage *data, int n);
extern int	SIGetDataEntries(SharedInvalidationMessage *data, int datasize);
=======
extern void SharedInvalBackendInit(void);
extern bool BackendIdIsActive(int backendID);

extern void SIInsertDataEntries(const SharedInvalidationMessage *data, int n);
extern int SIGetDataEntries(SharedInvalidationMessage *data, int datasize);
>>>>>>> 49f001d81e
extern void SICleanupQueue(bool callerHasWriteLock, int minFree);

extern LocalTransactionId GetNextLocalTransactionId(void);

#endif   /* SINVALADT_H */
