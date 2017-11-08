/*-------------------------------------------------------------------------
 *
 * freespace.c
 *	  POSTGRES free space map for quickly finding free space in relations
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/storage/freespace/freespace.c,v 1.70 2008/12/03 12:22:53 heikki Exp $
 *
 *
 * NOTES:
 *
 *  Free Space Map keeps track of the amount of free space on pages, and
 *  allows quickly searching for a page with enough free space. The FSM is
 *  stored in a dedicated relation fork of all heap relations, and those
 *  index access methods that need it (see also indexfsm.c). See README for
 *  more information.
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/htup.h"
#include "access/xlogutils.h"
#include "storage/bufpage.h"
#include "storage/bufmgr.h"
#include "storage/freespace.h"
#include "storage/fsm_internals.h"
#include "storage/lmgr.h"
#include "storage/lwlock.h"
#include "storage/smgr.h"
#include "utils/rel.h"
#include "utils/inval.h"
#include "miscadmin.h"

<<<<<<< HEAD
#include "cdb/cdbvars.h"

/*----------
 * During database shutdown, we store the contents of FSM into a disk file,
 * which is re-read during startup.  This way we don't have a startup
 * transient condition where FSM isn't really functioning.
=======
/*
 * We use just one byte to store the amount of free space on a page, so we
 * divide the amount of free space a page can have into 256 different
 * categories. The highest category, 255, represents a page with at least
 * MaxFSMRequestSize bytes of free space, and the second highest category
 * represents the range from 254 * FSM_CAT_STEP, inclusive, to
 * MaxFSMRequestSize, exclusive.
 *
 * MaxFSMRequestSize depends on the architecture and BLCKSZ, but assuming
 * default 8k BLCKSZ, and that MaxFSMRequestSize is 24 bytes, the categories
 * look like this
 *
>>>>>>> 38e9348282e
 *
 * Range     Category
 * 0    - 31   0
 * 32   - 63   1
 * ...    ...  ...
 * 8096 - 8127 253
 * 8128 - 8163 254
 * 8164 - 8192 255
 *
 * The reason that MaxFSMRequestSize is special is that if MaxFSMRequestSize
 * isn't equal to a range boundary, a page with exactly MaxFSMRequestSize
 * bytes of free space wouldn't satisfy a request for MaxFSMRequestSize
 * bytes. If there isn't more than MaxFSMRequestSize bytes of free space on a
 * completely empty page, that would mean that we could never satisfy a
 * request of exactly MaxFSMRequestSize bytes.
 */
<<<<<<< HEAD

/* Name of FSM cache file (relative to $PGDATA) */
#define FSM_CACHE_FILENAME	"global/pg_fsm.cache"

/* Fixed values in header */
#define FSM_CACHE_LABEL		"FSM"
#define FSM_CACHE_ENDIAN	0x01020304
#define FSM_CACHE_VERSION	20030305

/* File header layout */
typedef struct FsmCacheFileHeader
{
	char		label[4];
	uint32		endian;
	uint32		version;
	int32		numRels;
} FsmCacheFileHeader;

/* Per-relation header */
typedef struct FsmCacheRelHeader
{
	RelFileNode key;			/* hash key (must be first) */
	bool		isIndex;		/* if true, we store only page numbers */
	uint32		avgRequest;		/* moving average of space requests */
	BlockNumber interestingPages;		/* # of pages with useful free space */
	int32		storedPages;	/* # of pages stored in arena */
} FsmCacheRelHeader;

int			MaxFSMRelations;	/* these are set by guc.c */
int			MaxFSMPages;

static FSMHeader *FreeSpaceMap; /* points to FSMHeader in shared memory */
static HTAB *FreeSpaceMapRelHash;		/* points to (what used to be)
										 * FSMHeader->relHash */


static void CheckFreeSpaceMapStatistics(int elevel, int numRels,
							double needed);
static FSMRelation *lookup_fsm_rel(RelFileNode *rel);
static FSMRelation *create_fsm_rel(RelFileNode *rel);
static void delete_fsm_rel(FSMRelation *fsmrel);
static int realloc_fsm_rel(FSMRelation *fsmrel, BlockNumber interestingPages,
				bool isIndex);
static void link_fsm_rel_usage(FSMRelation *fsmrel);
static void unlink_fsm_rel_usage(FSMRelation *fsmrel);
static void link_fsm_rel_storage(FSMRelation *fsmrel);
static void unlink_fsm_rel_storage(FSMRelation *fsmrel);
static BlockNumber find_free_space(FSMRelation *fsmrel, Size spaceNeeded);
static BlockNumber find_index_free_space(FSMRelation *fsmrel);
static void fsm_record_free_space(FSMRelation *fsmrel, BlockNumber page,
					  Size spaceAvail);
static bool lookup_fsm_page_entry(FSMRelation *fsmrel, BlockNumber page,
					  int *outPageIndex);
static void compact_fsm_storage(void);
static void push_fsm_rels_after(FSMRelation *afterRel);
static void pack_incoming_pages(FSMPageData *newLocation, int newPages,
					FSMPageData *pageSpaces, int nPages);
static void pack_existing_pages(FSMPageData *newLocation, int newPages,
					FSMPageData *oldLocation, int oldPages);
static int	fsm_calc_request(FSMRelation *fsmrel);
static int	fsm_calc_request_unclamped(FSMRelation *fsmrel);
static int	fsm_calc_target_allocation(int myRequest);
static int	fsm_current_chunks(FSMRelation *fsmrel);
static int	fsm_current_allocation(FSMRelation *fsmrel);

/*
 * List of relations that are in the vacuum process and have not been
 * commited.
 */
static List *vacuumRels = NIL;
=======
#define FSM_CATEGORIES	256
#define FSM_CAT_STEP	(BLCKSZ / FSM_CATEGORIES)
#define MaxFSMRequestSize	MaxHeapTupleSize
>>>>>>> 38e9348282e

/*
 * Depth of the on-disk tree. We need to be able to address 2^32-1 blocks,
 * and 1626 is the smallest number that satisfies X^3 >= 2^32-1. Likewise,
 * 216 is the smallest number that satisfies X^4 >= 2^32-1. In practice,
 * this means that 4096 bytes is the smallest BLCKSZ that we can get away
 * with a 3-level tree, and 512 is the smallest we support.
 */
#define FSM_TREE_DEPTH	((SlotsPerFSMPage >= 1626) ? 3 : 4)

#define FSM_ROOT_LEVEL	(FSM_TREE_DEPTH - 1)
#define FSM_BOTTOM_LEVEL 0

/*
 * The internal FSM routines work on a logical addressing scheme. Each
 * level of the tree can be thought of as a separately addressable file.
 */
typedef struct
{
	int level;		/* level */
	int logpageno;	/* page number within the level */
} FSMAddress;

/* Address of the root page. */
static const FSMAddress FSM_ROOT_ADDRESS = { FSM_ROOT_LEVEL, 0 };

/* functions to navigate the tree */
static FSMAddress fsm_get_child(FSMAddress parent, uint16 slot);
static FSMAddress fsm_get_parent(FSMAddress child, uint16 *slot);
static FSMAddress fsm_get_location(BlockNumber heapblk, uint16 *slot);
static BlockNumber fsm_get_heap_blk(FSMAddress addr, uint16 slot);
static BlockNumber fsm_logical_to_physical(FSMAddress addr);

static Buffer fsm_readbuf(Relation rel, FSMAddress addr, bool extend);
static void fsm_extend(Relation rel, BlockNumber fsm_nblocks);

/* functions to convert amount of free space to a FSM category */
static uint8 fsm_space_avail_to_cat(Size avail);
static uint8 fsm_space_needed_to_cat(Size needed);
static Size  fsm_space_cat_to_avail(uint8 cat);

/* workhorse functions for various operations */
static int fsm_set_and_search(Relation rel, FSMAddress addr, uint16 slot,
							  uint8 newValue, uint8 minValue);
static BlockNumber fsm_search(Relation rel, uint8 min_cat);
static uint8 fsm_vacuum_page(Relation rel, FSMAddress addr, bool *eof);


/******** Public API ********/

/*
 * GetPageWithFreeSpace - try to find a page in the given relation with
 *		at least the specified amount of free space.
 *
 * If successful, return the block number; if not, return InvalidBlockNumber.
 *
 * The caller must be prepared for the possibility that the returned page
 * will turn out to have too little space available by the time the caller
 * gets a lock on it.  In that case, the caller should report the actual
 * amount of free space available on that page and then try again (see
 * RecordAndGetPageWithFreeSpace).	If InvalidBlockNumber is returned,
 * extend the relation.
 */
BlockNumber
GetPageWithFreeSpace(Relation rel, Size spaceNeeded)
{
	uint8 min_cat = fsm_space_needed_to_cat(spaceNeeded);
	return fsm_search(rel, min_cat);
}

/*
 * RecordAndGetPageWithFreeSpace - update info about a page and try again.
 *
 * We provide this combo form to save some locking overhead, compared to
 * separate RecordPageWithFreeSpace + GetPageWithFreeSpace calls. There's
 * also some effort to return a page close to the old page; if there's a
 * page with enough free space on the same FSM page where the old one page
 * is located, it is preferred.
 */
BlockNumber
RecordAndGetPageWithFreeSpace(Relation rel, BlockNumber oldPage,
							  Size oldSpaceAvail, Size spaceNeeded)
{
	int			old_cat = fsm_space_avail_to_cat(oldSpaceAvail);
	int			search_cat = fsm_space_needed_to_cat(spaceNeeded);
	FSMAddress	addr;
	uint16		slot;
	int			search_slot;

	/* Get the location of the FSM byte representing the heap block */
	addr = fsm_get_location(oldPage, &slot);

	search_slot = fsm_set_and_search(rel, addr, slot, old_cat, search_cat);

	/*
	 * If fsm_set_and_search found a suitable new block, return that.
	 * Otherwise, search as usual.
	 */
	if (search_slot != -1)
		return fsm_get_heap_blk(addr, search_slot);
	else
		return fsm_search(rel, search_cat);
}

/*
 * RecordPageWithFreeSpace - update info about a page.
 *
 * Note that if the new spaceAvail value is higher than the old value stored
 * in the FSM, the space might not become visible to searchers until the next
 * FreeSpaceMapVacuum call, which updates the upper level pages.
 */
void
RecordPageWithFreeSpace(Relation rel, BlockNumber heapBlk, Size spaceAvail)
{
	int			new_cat = fsm_space_avail_to_cat(spaceAvail);
	FSMAddress	addr;
	uint16		slot;

	/* Get the location of the FSM byte representing the heap block */
	addr = fsm_get_location(heapBlk, &slot);

	fsm_set_and_search(rel, addr, slot, new_cat, 0);
}

/*
 * XLogRecordPageWithFreeSpace - like RecordPageWithFreeSpace, for use in
 *		WAL replay
 */
void
XLogRecordPageWithFreeSpace(RelFileNode rnode, BlockNumber heapBlk,
							Size spaceAvail)
{
	int			new_cat = fsm_space_avail_to_cat(spaceAvail);
	FSMAddress	addr;
	uint16		slot;
	BlockNumber blkno;
	Buffer		buf;
	Page		page;

	/* Get the location of the FSM byte representing the heap block */
	addr = fsm_get_location(heapBlk, &slot);
	blkno = fsm_logical_to_physical(addr);

	/* If the page doesn't exist already, extend */
	buf = XLogReadBufferExtended(rnode, FSM_FORKNUM, blkno, RBM_ZERO_ON_ERROR);
	page = BufferGetPage(buf);
	if (PageIsNew(page))
		PageInit(page, BLCKSZ, 0);

	if (fsm_set_avail(page, slot, new_cat))
		MarkBufferDirty(buf);
	UnlockReleaseBuffer(buf);
}

/*
 * GetRecordedFreePage - return the amount of free space on a particular page,
 *		according to the FSM.
 */
Size
GetRecordedFreeSpace(Relation rel, BlockNumber heapBlk)
{
	FSMAddress	addr;
	uint16		slot;
	Buffer		buf;
	uint8		cat;

	/* Get the location of the FSM byte representing the heap block */
	addr = fsm_get_location(heapBlk, &slot);

	buf = fsm_readbuf(rel, addr, false);
	if (!BufferIsValid(buf))
		return 0;
	cat = fsm_get_avail(BufferGetPage(buf), slot);
	ReleaseBuffer(buf);

	return fsm_space_cat_to_avail(cat);
}

/*
 * FreeSpaceMapTruncateRel - adjust for truncation of a relation.
 *
 * The caller must hold AccessExclusiveLock on the relation, to ensure
 * that other backends receive the relcache invalidation event that this
 * function sends, before accessing the FSM again.
 *
 * nblocks is the new size of the heap.
 */
void
FreeSpaceMapTruncateRel(Relation rel, BlockNumber nblocks)
{
	BlockNumber	new_nfsmblocks;
	FSMAddress	first_removed_address;
	uint16		first_removed_slot;
	Buffer		buf;

	RelationOpenSmgr(rel);

	/*
	 * If no FSM has been created yet for this relation, there's nothing to
	 * truncate.
	 */
	if (!smgrexists(rel->rd_smgr, FSM_FORKNUM))
		return;

	/* Get the location in the FSM of the first removed heap block */
	first_removed_address = fsm_get_location(nblocks, &first_removed_slot);

	/*
	 * Zero out the tail of the last remaining FSM page. If the slot
	 * representing the first removed heap block is at a page boundary, as
	 * the first slot on the FSM page that first_removed_address points to,
	 * we can just truncate that page altogether.
	 */
	if (first_removed_slot > 0)
	{
		buf = fsm_readbuf(rel, first_removed_address, false);
		if (!BufferIsValid(buf))
			return; /* nothing to do; the FSM was already smaller */
		LockBuffer(buf, BUFFER_LOCK_EXCLUSIVE);
		fsm_truncate_avail(BufferGetPage(buf), first_removed_slot);
		MarkBufferDirty(buf);
		UnlockReleaseBuffer(buf);

		new_nfsmblocks = fsm_logical_to_physical(first_removed_address) + 1;
	}
	else
	{
		new_nfsmblocks = fsm_logical_to_physical(first_removed_address);
		if (smgrnblocks(rel->rd_smgr, FSM_FORKNUM) <= new_nfsmblocks)
			return; /* nothing to do; the FSM was already smaller */
	}

	/* Truncate the unused FSM pages */
	smgrtruncate(rel->rd_smgr, FSM_FORKNUM, new_nfsmblocks, rel->rd_istemp);

	/*
	 * Need to invalidate the relcache entry, because rd_fsm_nblocks
	 * seen by other backends is no longer valid.
	 */
	if (!InRecovery)
		CacheInvalidateRelcache(rel);

	rel->rd_fsm_nblocks = new_nfsmblocks;
}

/*
<<<<<<< HEAD
 * FreeSpaceMapForgetDatabase - forget all relations of a database.
 *
 * This is called during DROP DATABASE and DROP TABLESPACE.  As above,
 * might as well reclaim map space sooner instead of later.
 */
void
FreeSpaceMapForgetDatabase(Oid tblspc, Oid dbid)
=======
 * FreeSpaceMapVacuum - scan and fix any inconsistencies in the FSM
 */
void
FreeSpaceMapVacuum(Relation rel)
>>>>>>> 38e9348282e
{
	bool dummy;

<<<<<<< HEAD
	LWLockAcquire(FreeSpaceLock, LW_EXCLUSIVE);
	for (fsmrel = FreeSpaceMap->usageList; fsmrel; fsmrel = nextrel)
	{
		nextrel = fsmrel->nextUsage;	/* in case we delete it */
		if (!OidIsValid(tblspc) ||
			fsmrel->key.spcNode == tblspc)
		{
			if (fsmrel->key.dbNode == dbid)
				delete_fsm_rel(fsmrel);
		}
	}
	LWLockRelease(FreeSpaceLock);
=======
	/*
	 * Traverse the tree in depth-first order. The tree is stored physically
	 * in depth-first order, so this should be pretty I/O efficient.
	 */
	fsm_vacuum_page(rel, FSM_ROOT_ADDRESS, &dummy);
>>>>>>> 38e9348282e
}

/******** Internal routines ********/

/*
 * Return category corresponding x bytes of free space
 */
static uint8
fsm_space_avail_to_cat(Size avail)
{
	int cat;

	Assert(avail < BLCKSZ);

	if (avail >= MaxFSMRequestSize)
		return 255;

	cat = avail / FSM_CAT_STEP;

	/*
	 * The highest category, 255, is reserved for MaxFSMRequestSize bytes or
	 * more.
	 */
	if (cat > 254)
		cat = 254;

<<<<<<< HEAD
	/* Copy other stats before dropping lock */
	numRels = FreeSpaceMap->numRels;
	LWLockRelease(FreeSpaceLock);

	/* Convert stats to actual number of page slots needed */
	needed = (sumRequests + numRels) * CHUNKPAGES;

	ereport(elevel,
			(errmsg("free space map contains %d pages in %d relations",
					storedPages, numRels),
	errdetail("A total of %.0f page slots are in use (including overhead).\n"
			  "%.0f page slots are required to track all free space.\n"
		  "Current limits are:  %d page slots, %d relations, using %.0f kB.",
			  Min(needed, MaxFSMPages),
			  needed,
			  MaxFSMPages, MaxFSMRelations,
			  (double) FreeSpaceShmemSize() / 1024.0)));

	if (Gp_role != GP_ROLE_DISPATCH) /* not interesting on the dispatch node */
		CheckFreeSpaceMapStatistics(NOTICE, numRels, needed);
	/* Print to server logs too because is deals with a config variable. */
	CheckFreeSpaceMapStatistics(LOG, numRels, needed);
=======
	return (uint8) cat;
>>>>>>> 38e9348282e
}

/*
 * Return the lower bound of the range of free space represented by given
 * category.
 */
static Size
fsm_space_cat_to_avail(uint8 cat)
{
	/* The highest category represents exactly MaxFSMRequestSize bytes. */
	if (cat == 255)
		return MaxFSMRequestSize;
	else
		return cat * FSM_CAT_STEP;
}

/*
 * Which category does a page need to have, to accommodate x bytes of data?
 * While fsm_size_to_avail_cat() rounds down, this needs to round up.
 */
static uint8
fsm_space_needed_to_cat(Size needed)
{
	int cat;

	/* Can't ask for more space than the highest category represents */
	if (needed > MaxFSMRequestSize)
			elog(ERROR, "invalid FSM request size %lu",
				 (unsigned long) needed);

	if (needed == 0)
		return 1;

	cat = (needed + FSM_CAT_STEP - 1) / FSM_CAT_STEP;

	if (cat > 255)
		cat = 255;

	return (uint8) cat;
}

/*
 * Returns the physical block number an FSM page
 */
static BlockNumber
fsm_logical_to_physical(FSMAddress addr)
{
	BlockNumber pages;
	int leafno;
	int l;

	/*
	 * Calculate the logical page number of the first leaf page below the
	 * given page.
	 */
	leafno = addr.logpageno;
	for (l = 0; l < addr.level; l++)
		leafno *= SlotsPerFSMPage;

	/* Count upper level nodes required to address the leaf page */
	pages = 0;
	for (l = 0; l < FSM_TREE_DEPTH; l++)
	{
		pages += leafno + 1;
		leafno /= SlotsPerFSMPage;
	}

	/*
	 * If the page we were asked for wasn't at the bottom level, subtract
	 * the additional lower level pages we counted above.
	 */
	pages -= addr.level;

	/* Turn the page count into 0-based block number */
	return pages - 1;
}

/*
 * Return the FSM location corresponding to given heap block.
 */
static FSMAddress
fsm_get_location(BlockNumber heapblk, uint16 *slot)
{
	FSMAddress addr;

	addr.level = FSM_BOTTOM_LEVEL;
	addr.logpageno = heapblk / SlotsPerFSMPage;
	*slot = heapblk % SlotsPerFSMPage;

	return addr;
}

/*
 * Return the heap block number corresponding to given location in the FSM.
 */
static BlockNumber
fsm_get_heap_blk(FSMAddress addr, uint16 slot)
{
	Assert(addr.level == FSM_BOTTOM_LEVEL);
	return ((unsigned int) addr.logpageno) * SlotsPerFSMPage + slot;
}

/*
 * Given a logical address of a child page, get the logical page number of
 * the parent, and the slot within the parent corresponding to the child.
 */
static FSMAddress
fsm_get_parent(FSMAddress child, uint16 *slot)
{
	FSMAddress parent;

	Assert(child.level < FSM_ROOT_LEVEL);

	parent.level = child.level + 1;
	parent.logpageno = child.logpageno / SlotsPerFSMPage;
	*slot = child.logpageno % SlotsPerFSMPage;

	return parent;
}

/*
 * Given a logical address of a parent page, and a slot number get the
 * logical address of the corresponding child page.
 */
static FSMAddress
fsm_get_child(FSMAddress parent, uint16 slot)
{
	FSMAddress child;

	Assert(parent.level > FSM_BOTTOM_LEVEL);

	child.level = parent.level - 1;
	child.logpageno = parent.logpageno * SlotsPerFSMPage + slot;

	return child;
}

/*
 * Read a FSM page.
 *
 * If the page doesn't exist, InvalidBuffer is returned, or if 'extend' is
 * true, the FSM file is extended.
 */
static Buffer
fsm_readbuf(Relation rel, FSMAddress addr, bool extend)
{
	BlockNumber blkno = fsm_logical_to_physical(addr);
	Buffer buf;

	RelationOpenSmgr(rel);

	/* If we haven't cached the size of the FSM yet, check it first */
	if (rel->rd_fsm_nblocks == InvalidBlockNumber)
	{
		if (smgrexists(rel->rd_smgr, FSM_FORKNUM))
			rel->rd_fsm_nblocks = smgrnblocks(rel->rd_smgr, FSM_FORKNUM);
		else
			rel->rd_fsm_nblocks = 0;
	}

	/* Handle requests beyond EOF */
	if (blkno >= rel->rd_fsm_nblocks)
	{
		if (extend)
			fsm_extend(rel, blkno + 1);
		else
			return InvalidBuffer;
	}

	/*
	 * Use ZERO_ON_ERROR mode, and initialize the page if necessary. The FSM
	 * information is not accurate anyway, so it's better to clear corrupt
	 * pages than error out. Since the FSM changes are not WAL-logged, the
	 * so-called torn page problem on crash can lead to pages with corrupt
	 * headers, for example.
	 */
	buf = ReadBufferExtended(rel, FSM_FORKNUM, blkno, RBM_ZERO_ON_ERROR, NULL);
	if (PageIsNew(BufferGetPage(buf)))
		PageInit(BufferGetPage(buf), BLCKSZ, 0);
	return buf;
}

/*
 * Ensure that the FSM fork is at least fsm_nblocks long, extending
 * it if necessary with empty pages. And by empty, I mean pages filled
 * with zeros, meaning there's no free space.
 */
static void
fsm_extend(Relation rel, BlockNumber fsm_nblocks)
{
	BlockNumber fsm_nblocks_now;
	Page pg;

	pg = (Page) palloc(BLCKSZ);
	PageInit(pg, BLCKSZ, 0);

	/*
	 * We use the relation extension lock to lock out other backends
	 * trying to extend the FSM at the same time. It also locks out
	 * extension of the main fork, unnecessarily, but extending the
	 * FSM happens seldom enough that it doesn't seem worthwhile to
	 * have a separate lock tag type for it.
	 *
	 * Note that another backend might have extended or created the
	 * relation before we get the lock.
	 */
	LockRelationForExtension(rel, ExclusiveLock);

	/* Create the FSM file first if it doesn't exist */
	if ((rel->rd_fsm_nblocks == 0 || rel->rd_fsm_nblocks == InvalidBlockNumber)
		&& !smgrexists(rel->rd_smgr, FSM_FORKNUM))
	{
		smgrcreate(rel->rd_smgr, FSM_FORKNUM, false);
		fsm_nblocks_now = 0;
	}
	else
		fsm_nblocks_now = smgrnblocks(rel->rd_smgr, FSM_FORKNUM);

	while (fsm_nblocks_now < fsm_nblocks)
	{
		smgrextend(rel->rd_smgr, FSM_FORKNUM, fsm_nblocks_now,
				   (char *) pg, rel->rd_istemp);
		fsm_nblocks_now++;
	}

	UnlockRelationForExtension(rel, ExclusiveLock);

	pfree(pg);

	/* Update the relcache with the up-to-date size */
	if (!InRecovery)
		CacheInvalidateRelcache(rel);
	rel->rd_fsm_nblocks = fsm_nblocks_now;
}

/*
 * Set value in given FSM page and slot.
 *
 * If minValue > 0, the updated page is also searched for a page with at
 * least minValue of free space. If one is found, its slot number is
 * returned, -1 otherwise.
 */
static int
fsm_set_and_search(Relation rel, FSMAddress addr, uint16 slot,
				   uint8 newValue, uint8 minValue)
{
	Buffer		buf;
	Page		page;
	int			newslot = -1;

	buf = fsm_readbuf(rel, addr, true);
	LockBuffer(buf, BUFFER_LOCK_EXCLUSIVE);

	page = BufferGetPage(buf);

	if (fsm_set_avail(page, slot, newValue))
		MarkBufferDirty(buf);

	if (minValue != 0)
	{
		/* Search while we still hold the lock */
		newslot = fsm_search_avail(buf, minValue,
								   addr.level == FSM_BOTTOM_LEVEL,
								   true);
	}

	UnlockReleaseBuffer(buf);

	return newslot;
}

/*
 * Search the tree for a heap page with at least min_cat of free space
 */
static BlockNumber
fsm_search(Relation rel, uint8 min_cat)
{
	int restarts = 0;
	FSMAddress addr = FSM_ROOT_ADDRESS;

	for (;;)
	{
		int slot;
		Buffer buf;
		uint8 max_avail = 0;

		/* Read the FSM page. */
		buf = fsm_readbuf(rel, addr, false);

		/* Search within the page */
		if (BufferIsValid(buf))
		{
			LockBuffer(buf, BUFFER_LOCK_SHARE);
			slot = fsm_search_avail(buf, min_cat,
									(addr.level == FSM_BOTTOM_LEVEL),
									false);
			if (slot == -1)
				max_avail = fsm_get_max_avail(BufferGetPage(buf));
			UnlockReleaseBuffer(buf);
		}
		else
			slot = -1;

		if (slot != -1)
		{
			/*
			 * Descend the tree, or return the found block if we're at the
			 * bottom.
			 */
			if (addr.level == FSM_BOTTOM_LEVEL)
				return fsm_get_heap_blk(addr, slot);

			addr = fsm_get_child(addr, slot);
		}
		else if (addr.level == FSM_ROOT_LEVEL)
		{
			/*
			 * At the root, failure means there's no page with enough free
			 * space in the FSM. Give up.
			 */
			return InvalidBlockNumber;
		}
		else
		{
			uint16 parentslot;
			FSMAddress parent;

			/*
			 * At lower level, failure can happen if the value in the upper-
			 * level node didn't reflect the value on the lower page. Update
			 * the upper node, to avoid falling into the same trap again, and
			 * start over.
			 *
			 * There's a race condition here, if another backend updates this
			 * page right after we release it, and gets the lock on the parent
			 * page before us. We'll then update the parent page with the now
			 * stale information we had. It's OK, because it should happen
			 * rarely, and will be fixed by the next vacuum.
			 */
			parent = fsm_get_parent(addr, &parentslot);
			fsm_set_and_search(rel, parent, parentslot, max_avail, 0);

			/*
			 * If the upper pages are badly out of date, we might need to
			 * loop quite a few times, updating them as we go. Any
			 * inconsistencies should eventually be corrected and the loop
			 * should end. Looping indefinitely is nevertheless scary, so
			 * provide an emergency valve.
			 */
			if (restarts++ > 10000)
				return InvalidBlockNumber;

			/* Start search all over from the root */
			addr = FSM_ROOT_ADDRESS;
		}
	}
}


/*
 * Recursive guts of FreeSpaceMapVacuum
 */
static uint8
fsm_vacuum_page(Relation rel, FSMAddress addr, bool *eof_p)
{
	Buffer buf;
	Page page;
	uint8 max_avail;

	/* Read the page if it exists, or return EOF */
	buf = fsm_readbuf(rel, addr, false);
	if (!BufferIsValid(buf))
	{
		*eof_p = true;
		return 0;
	}
	else
		*eof_p = false;

	page = BufferGetPage(buf);

	/*
	 * Recurse into children, and fix the information stored about them
	 * at this level.
	 */
	if (addr.level > FSM_BOTTOM_LEVEL)
	{
		int slot;
		bool eof = false;

<<<<<<< HEAD
/*
 * Calculate target allocation (number of chunks) for a rel
 *
 * Parameter is the result from fsm_calc_request().  The global sumRequests
 * and numRels totals must be up-to-date already.
 *
 * See notes at top of file for details.
 */
static int
fsm_calc_target_allocation(int myRequest)
{
	double		spareChunks;
	int			extra;

	spareChunks = FreeSpaceMap->totalChunks - FreeSpaceMap->numRels;
	Assert(spareChunks > 0);
	if (spareChunks >= FreeSpaceMap->sumRequests)
	{
		/* We aren't oversubscribed, so allocate exactly the request */
		extra = myRequest;
	}
	else
	{
		extra = (int) rint(spareChunks * myRequest / FreeSpaceMap->sumRequests);
		if (extra < 0)			/* shouldn't happen, but make sure */
			extra = 0;
	}
	return 1 + extra;
}

/*
 * Calculate number of chunks actually used to store current data
 */
static int
fsm_current_chunks(FSMRelation *fsmrel)
{
	int			chunkCount;

	/* Make sure storedPages==0 produces right answer */
	if (fsmrel->storedPages <= 0)
		return 0;
	/* Convert page count to chunk count */
	if (fsmrel->isIndex)
		chunkCount = (fsmrel->storedPages - 1) / INDEXCHUNKPAGES + 1;
	else
		chunkCount = (fsmrel->storedPages - 1) / CHUNKPAGES + 1;
	return chunkCount;
}

/*
 * Calculate current actual allocation (number of chunks) for a rel
 */
static int
fsm_current_allocation(FSMRelation *fsmrel)
{
	if (fsmrel->nextPhysical != NULL)
		return fsmrel->nextPhysical->firstChunk - fsmrel->firstChunk;
	else if (fsmrel == FreeSpaceMap->lastRel)
		return FreeSpaceMap->usedChunks - fsmrel->firstChunk;
	else
	{
		/* it's not in the storage-order list */
		Assert(fsmrel->firstChunk < 0 && fsmrel->storedPages == 0);
		return 0;
	}
}


/*
 * Return the FreeSpaceMap structure for examination.
 */
FSMHeader *
GetFreeSpaceMap(void)
{

	return FreeSpaceMap;
}

/*
 * Append relations that are in the vacuum process to the vacuumRels list.
 *
 * Only support non-index relations.
 */
List *
AppendRelToVacuumRels(Relation rel)
{
	RelFileNode *relfilenode = palloc(sizeof(RelFileNode));

	/* This relation should not be an index. */
	Assert(!OidIsValid(rel->rd_rel->relam));

	relfilenode->spcNode = (rel->rd_node).spcNode;
	relfilenode->dbNode = (rel->rd_node).dbNode;
	relfilenode->relNode = (rel->rd_node).relNode;
	vacuumRels = lappend(vacuumRels, relfilenode);

	elog(DEBUG2, "Add relation %d/%d/%d to VacuumRels",
		 relfilenode->spcNode, relfilenode->dbNode, relfilenode->relNode);

	return vacuumRels;
}

/*
 * Remove all relations in the vacuumRels list. This should be called during
 * commit.
 */
void
ResetVacuumRels()
{
	if (vacuumRels == NULL)
		return;
	
	list_free_deep(vacuumRels);
	vacuumRels = NIL;
	elog(DEBUG2, "Reset VacuumRels");
}

/*
 * Clear the freespace map entries for relations that are in vacuumRels.
 */
void
ClearFreeSpaceForVacuumRels()
{
	ListCell *lc;

	if (vacuumRels == NULL)
		return;
	
	foreach (lc, vacuumRels)
	{
		RelFileNode *relfilenode = (RelFileNode *)lfirst(lc);
		RecordRelationFreeSpace(relfilenode, 0, 0, NULL);

		elog(DEBUG2, "Clear the freespace map entry for relation %d/%d/%d in VacuumRels",
			 relfilenode->spcNode, relfilenode->dbNode, relfilenode->relNode);
	}

	ResetVacuumRels();
}

#ifdef FREESPACE_DEBUG
/*
 * Dump contents of freespace map for debugging.
 *
 * We assume caller holds the FreeSpaceLock, or is otherwise unconcerned
 * about other processes.
 */
void
DumpFreeSpace(void)
{
	FSMRelation *fsmrel;
	FSMRelation *prevrel = NULL;
	int			relNum = 0;
	int			nPages;

	for (fsmrel = FreeSpaceMap->usageList; fsmrel; fsmrel = fsmrel->nextUsage)
	{
		relNum++;
		fprintf(stderr, "Map %d: rel %u/%u/%u isIndex %d avgRequest %u interestingPages %u nextPage %d\nMap= ",
				relNum,
				fsmrel->key.spcNode, fsmrel->key.dbNode, fsmrel->key.relNode,
				(int) fsmrel->isIndex, fsmrel->avgRequest,
				fsmrel->interestingPages, fsmrel->nextPage);
		if (fsmrel->isIndex)
=======
		for (slot = 0; slot < SlotsPerFSMPage; slot++)
>>>>>>> 38e9348282e
		{
			int child_avail;

			/* After we hit end-of-file, just clear the rest of the slots */
			if (!eof)
				child_avail = fsm_vacuum_page(rel, fsm_get_child(addr, slot), &eof);
			else
				child_avail = 0;

			/* Update information about the child */
			if (fsm_get_avail(page, slot) != child_avail)
			{
				LockBuffer(buf, BUFFER_LOCK_EXCLUSIVE);
				fsm_set_avail(BufferGetPage(buf), slot, child_avail);
				MarkBufferDirty(buf);
				LockBuffer(buf, BUFFER_LOCK_UNLOCK);
			}
		}
	}

	max_avail = fsm_get_max_avail(BufferGetPage(buf));

	/*
	 * Reset the next slot pointer. This encourages the use of low-numbered
	 * pages, increasing the chances that a later vacuum can truncate the
	 * relation.
	 */
	((FSMPage) PageGetContents(page))->fp_next_slot = 0;

	ReleaseBuffer(buf);

	return max_avail;
}
