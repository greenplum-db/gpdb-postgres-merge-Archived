/*
 * subtrans.h
 *
 * PostgreSQL subtransaction-log manager
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/subtrans.h,v 1.12 2009/01/01 17:23:56 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/subtrans.h,v 1.11 2008/01/01 19:45:56 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef SUBTRANS_H
#define SUBTRANS_H

/* Number of SLRU buffers to use for subtrans */
#define NUM_SUBTRANS_BUFFERS	32

typedef struct SubTransData
{
	TransactionId parent;
	TransactionId topMostParent;
} SubTransData;

extern void SubTransSetParent(TransactionId xid, TransactionId parent);
extern TransactionId SubTransGetParent(TransactionId xid);
extern TransactionId SubTransGetTopmostTransaction(TransactionId xid);

extern Size SUBTRANSShmemSize(void);
extern void SUBTRANSShmemInit(void);
extern void BootStrapSUBTRANS(void);
extern void StartupSUBTRANS(TransactionId oldestActiveXID);
extern void ShutdownSUBTRANS(void);
extern void CheckPointSUBTRANS(void);
extern void ExtendSUBTRANS(TransactionId newestXact);
extern void TruncateSUBTRANS(TransactionId oldestXact);

#endif   /* SUBTRANS_H */
