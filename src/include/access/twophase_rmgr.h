/*-------------------------------------------------------------------------
 *
 * twophase_rmgr.h
 *	  Two-phase-commit resource managers definition
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/twophase_rmgr.h,v 1.7 2009/01/01 17:23:56 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/twophase_rmgr.h,v 1.6.2.1 2009/11/23 09:59:00 heikki Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef TWOPHASE_RMGR_H
#define TWOPHASE_RMGR_H

typedef void (*TwoPhaseCallback) (TransactionId xid, uint16 info,
											  void *recdata, uint32 len);
typedef uint8 TwoPhaseRmgrId;

/*
 * Built-in resource managers
 */
#define TWOPHASE_RM_END_ID			0
#define TWOPHASE_RM_LOCK_ID			1
#define TWOPHASE_RM_INVAL_ID		2
#define TWOPHASE_RM_FLATFILES_ID	3
#define TWOPHASE_RM_NOTIFY_ID		4
#define TWOPHASE_RM_PGSTAT_ID		5
<<<<<<< HEAD
#define TWOPHASE_RM_MAX_ID			TWOPHASE_RM_PGSTAT_ID
=======
#define TWOPHASE_RM_MULTIXACT_ID	6
#define TWOPHASE_RM_MAX_ID			TWOPHASE_RM_MULTIXACT_ID
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

extern const TwoPhaseCallback twophase_recover_callbacks[];
extern const TwoPhaseCallback twophase_postcommit_callbacks[];
extern const TwoPhaseCallback twophase_postabort_callbacks[];


extern void RegisterTwoPhaseRecord(TwoPhaseRmgrId rmid, uint16 info,
					   const void *data, uint32 len);

#endif   /* TWOPHASE_RMGR_H */
