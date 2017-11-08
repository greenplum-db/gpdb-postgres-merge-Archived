/*-------------------------------------------------------------------------
 *
 * freespace.h
 *	  POSTGRES free space map for quickly finding free space in relations
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/storage/freespace.h,v 1.32 2008/12/12 22:56:00 alvherre Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef FREESPACE_H_
#define FREESPACE_H_

#include "storage/block.h"
#include "storage/bufpage.h"
<<<<<<< HEAD
#include "storage/itemptr.h"
#include "nodes/pg_list.h"
#include "storage/relfilenode.h"
#include "utils/relcache.h"
#include "utils/rel.h"
=======
#include "storage/relfilenode.h"
#include "utils/relcache.h"
>>>>>>> 38e9348282e

/* prototypes for public functions in freespace.c */
extern Size GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk);
extern BlockNumber GetPageWithFreeSpace(Relation rel, Size spaceNeeded);
extern BlockNumber RecordAndGetPageWithFreeSpace(Relation rel,
							  BlockNumber oldPage,
							  Size oldSpaceAvail,
							  Size spaceNeeded);
extern void RecordPageWithFreeSpace(Relation rel, BlockNumber heapBlk,
									Size spaceAvail);
extern void XLogRecordPageWithFreeSpace(RelFileNode rnode, BlockNumber heapBlk,
										Size spaceAvail);

extern void FreeSpaceMapTruncateRel(Relation rel, BlockNumber nblocks);
extern void FreeSpaceMapVacuum(Relation rel);

<<<<<<< HEAD
extern void FreeSpaceMapTruncateRel(RelFileNode *rel, BlockNumber nblocks);
extern void FreeSpaceMapForgetRel(RelFileNode *rel);
extern void FreeSpaceMapForgetDatabase(Oid tblspc, Oid dbid);

extern void PrintFreeSpaceMapStatistics(int elevel);

extern void DumpFreeSpaceMap(int code, Datum arg);
extern void LoadFreeSpaceMap(void);

extern List *AppendRelToVacuumRels(Relation rel);
extern void ResetVacuumRels(void);
extern void ClearFreeSpaceForVacuumRels(void);

#ifdef FREESPACE_DEBUG
extern void DumpFreeSpace(void);
#endif

#endif   /* FREESPACE_H */
=======
#endif   /* FREESPACE_H_ */
>>>>>>> 38e9348282e
