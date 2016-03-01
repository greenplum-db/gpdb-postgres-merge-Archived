/*-------------------------------------------------------------------------
 *
 * tstoreReceiver.h
 *	  prototypes for tstoreReceiver.c
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/tstoreReceiver.h,v 1.8.2.1 2008/12/01 17:06:35 tgl Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/tstoreReceiver.h,v 1.10.2.1 2008/12/01 17:06:27 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */

#ifndef TSTORE_RECEIVER_H
#define TSTORE_RECEIVER_H

#include "tcop/dest.h"
#include "utils/tuplestore.h"


extern DestReceiver *CreateTuplestoreDestReceiver(Tuplestorestate *tStore,
							 MemoryContext tContext);

extern void SetTuplestoreDestReceiverDeToast(DestReceiver *self,
											 bool detoast);

#endif   /* TSTORE_RECEIVER_H */
