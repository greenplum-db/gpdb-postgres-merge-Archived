/*-------------------------------------------------------------------------
 *
 * smgr_gp.h
 *	  GPDB extensions to storage manager
 *
 *
 * Portions Copyright (c) 2006-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/storage/sgmr_gp.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef SMGR_GP_H
#define SMGR_GP_H

#include "storage/relfilenode.h"
#include "storage/dbdirnode.h"
#include "access/persistentendxactrec.h"
#include "access/filerepdefs.h"

/*
 * Whether the object is being dropped in the original transaction, crash recovery, or
 * as part of (re)create / (re)drop during resynchronize.
 */
typedef enum StorageManagerMirrorMode
{
	StorageManagerMirrorMode_None = 0,
	StorageManagerMirrorMode_PrimaryOnly = 1,
	StorageManagerMirrorMode_Both = 2,
	StorageManagerMirrorMode_MirrorOnly = 3,
	MaxStorageManagerMirrorMode /* must always be last */
} StorageManagerMirrorMode;

/* prototypes for functions in smgr_ao.c */
extern void AppendOnlyMirrorResyncEofs_HashTableInit(void);

extern void AppendOnlyMirrorResyncEofs_Merge(
	RelFileNode		*relFileNode,
	int32			segmentFileNum,
	int				nestLevel,		/* Transaction nesting level. */
	char			*relationName,
	ItemPointer 	persistentTid,
	int64			persistentSerialNum,
	bool						mirrorCatchupRequired,
	MirrorDataLossTrackingState mirrorDataLossTrackingState,
	int64						mirrorDataLossTrackingSessionNum,
	int64			mirrorNewEof);

extern void AppendOnlyMirrorResyncEofs_RemoveForDrop(
	RelFileNode		*relFileNode,
	int32			segmentFileNum,
	int				nestLevel);		/* Transaction nesting level. */

extern void AppendOnlyMirrorResyncEofs_HashTableRemove(char *procName);

extern void smgrDoAppendOnlyResyncEofs(bool forCommit);
extern void smgrAppendOnlySubTransAbort(void);
extern void AtSubCommit_smgr_appendonly(void);

#endif   /* SMGR_GP_H */
