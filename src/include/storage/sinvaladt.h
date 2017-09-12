/*-------------------------------------------------------------------------
 *
 * sinvaladt.h
 *	  POSTGRES shared cache invalidation data manager.
 *
 * The shared cache invalidation manager is responsible for transmitting
 * invalidation messages between backends.	Any message sent by any backend
 * must be delivered to all already-running backends before it can be
<<<<<<< HEAD
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
=======
 * forgotten.
 * 
 * The struct type SharedInvalidationMessage, defining the contents of
 * a single message, is defined in sinval.h.
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/sinvaladt.h,v 1.47 2008/03/17 11:50:27 alvherre Exp $
 *
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
 *-------------------------------------------------------------------------
 */
#ifndef SINVALADT_H
#define SINVALADT_H

#include "storage/sinval.h"
<<<<<<< HEAD
=======

>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238

/*
 * prototypes for functions in sinvaladt.c
 */
extern Size SInvalShmemSize(void);
extern void CreateSharedInvalidationState(void);
<<<<<<< HEAD
extern void SharedInvalBackendInit(bool sendOnly);

extern void SIInsertDataEntries(const SharedInvalidationMessage *data, int n);
extern int	SIGetDataEntries(SharedInvalidationMessage *data, int datasize);
extern void SICleanupQueue(bool callerHasWriteLock, int minFree);
=======
extern void SharedInvalBackendInit(void);

extern bool SIInsertDataEntry(SharedInvalidationMessage *data);
extern int SIGetDataEntry(int backendId, SharedInvalidationMessage *data);
extern void SIDelExpiredDataEntries(bool locked);
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238

extern LocalTransactionId GetNextLocalTransactionId(void);

#endif   /* SINVALADT_H */
