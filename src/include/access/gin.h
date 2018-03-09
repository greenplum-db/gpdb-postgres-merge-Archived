/*--------------------------------------------------------------------------
 * gin.h
 *	  Public header file for Generalized Inverted Index access method.
 *
 *	Copyright (c) 2006-2011, PostgreSQL Global Development Group
 *
 *	src/include/access/gin.h
 *--------------------------------------------------------------------------
 */
#ifndef GIN_H
#define GIN_H

<<<<<<< HEAD
#include "access/relscan.h"
#include "access/sdir.h"
#include "access/xlogdefs.h"
#include "storage/bufpage.h"
#include "storage/off.h"
#include "utils/rel.h"
#include "access/genam.h"
#include "access/itup.h"
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
#include "access/xlog.h"
#include "storage/block.h"
#include "utils/relcache.h"


/*
 * amproc indexes for inverted indexes.
 */
#define GIN_COMPARE_PROC			   1
#define GIN_EXTRACTVALUE_PROC		   2
#define GIN_EXTRACTQUERY_PROC		   3
#define GIN_CONSISTENT_PROC			   4
#define GIN_COMPARE_PARTIAL_PROC	   5
#define GINNProcs					   5

/*
 * searchMode settings for extractQueryFn.
 */
#define GIN_SEARCH_MODE_DEFAULT			0
#define GIN_SEARCH_MODE_INCLUDE_EMPTY	1
#define GIN_SEARCH_MODE_ALL				2
#define GIN_SEARCH_MODE_EVERYTHING		3		/* for internal use only */

/*
 * GinStatsData represents stats data for planner use
 */
typedef struct GinStatsData
{
	BlockNumber nPendingPages;
	BlockNumber nTotalPages;
	BlockNumber nEntryPages;
	BlockNumber nDataPages;
	int64		nEntries;
	int32		ginVersion;
} GinStatsData;

/* GUC parameter */
extern PGDLLIMPORT int GinFuzzySearchLimit;

/* ginutil.c */
extern void ginGetStats(Relation index, GinStatsData *stats);
extern void ginUpdateStats(Relation index, const GinStatsData *stats);

/* ginxlog.c */
extern void gin_redo(XLogRecPtr beginLoc, XLogRecPtr lsn, XLogRecord *record);
extern void gin_desc(StringInfo buf, XLogRecPtr beginLoc, XLogRecord *record);
extern void gin_xlog_startup(void);
extern void gin_xlog_cleanup(void);
extern bool gin_safe_restartpoint(void);

<<<<<<< HEAD
extern void gin_mask(char *pagedata, BlockNumber blkno);


/* ginbtree.c */

typedef struct GinBtreeStack
{
	BlockNumber blkno;
	Buffer		buffer;
	OffsetNumber off;
	/* predictNumber contains prediction number of pages on current level */
	uint32		predictNumber;
	struct GinBtreeStack *parent;
} GinBtreeStack;

typedef struct GinBtreeData *GinBtree;

typedef struct GinBtreeData
{
	/* search methods */
	BlockNumber (*findChildPage) (GinBtree, GinBtreeStack *);
	bool		(*isMoveRight) (GinBtree, Page);
	bool		(*findItem) (GinBtree, GinBtreeStack *);

	/* insert methods */
	OffsetNumber (*findChildPtr) (GinBtree, Page, BlockNumber, OffsetNumber);
	BlockNumber (*getLeftMostPage) (GinBtree, Page);
	bool		(*isEnoughSpace) (GinBtree, Buffer, OffsetNumber);
	void		(*placeToPage) (GinBtree, Buffer, OffsetNumber, XLogRecData **);
	Page		(*splitPage) (GinBtree, Buffer, Buffer, OffsetNumber, XLogRecData **);
	void		(*fillRoot) (GinBtree, Buffer, Buffer, Buffer);

	bool		searchMode;

	Relation	index;
	GinState   *ginstate;
	bool		fullScan;
	bool		isBuild;

	BlockNumber rightblkno;

	/* Entry options */
	OffsetNumber entryAttnum;
	Datum		entryValue;
	IndexTuple	entry;
	bool		isDelete;

	/* Data (posting tree) option */
	ItemPointerData *items;
	uint32		nitem;
	uint32		curitem;

	PostingItem pitem;
} GinBtreeData;

extern GinBtreeStack *ginPrepareFindLeafPage(GinBtree btree, BlockNumber blkno);
extern GinBtreeStack *ginFindLeafPage(GinBtree btree, GinBtreeStack *stack);
extern void freeGinBtreeStack(GinBtreeStack *stack);
extern void ginInsertValue(GinBtree btree, GinBtreeStack *stack);
extern void findParents(GinBtree btree, GinBtreeStack *stack, BlockNumber rootBlkno);

/* ginentrypage.c */
extern IndexTuple GinFormTuple(Relation index, GinState *ginstate,
			 OffsetNumber attnum, Datum key,
			 ItemPointerData *ipd, uint32 nipd, bool errorTooBig);
extern void GinShortenTuple(IndexTuple itup, uint32 nipd);
extern void prepareEntryScan(GinBtree btree, Relation index, OffsetNumber attnum,
				 Datum value, GinState *ginstate);
extern void entryFillRoot(GinBtree btree, Buffer root, Buffer lbuf, Buffer rbuf);
extern IndexTuple ginPageGetLinkItup(Buffer buf);

/* gindatapage.c */
extern int	compareItemPointers(ItemPointer a, ItemPointer b);
extern uint32 MergeItemPointers(ItemPointerData *dst,
				  ItemPointerData *a, uint32 na,
				  ItemPointerData *b, uint32 nb);

extern void GinDataPageAddItem(Page page, void *data, OffsetNumber offset);
extern void PageDeletePostingItem(Page page, OffsetNumber offset);

typedef struct
{
	GinBtreeData btree;
	GinBtreeStack *stack;
} GinPostingTreeScan;

extern GinPostingTreeScan *prepareScanPostingTree(Relation index,
					   BlockNumber rootBlkno, bool searchMode);
extern void insertItemPointer(GinPostingTreeScan *gdi,
				  ItemPointerData *items, uint32 nitem);
extern Buffer scanBeginPostingTree(GinPostingTreeScan *gdi);
extern void dataFillRoot(GinBtree btree, Buffer root, Buffer lbuf, Buffer rbuf);
extern void prepareDataScan(GinBtree btree, Relation index);

/* ginscan.c */

typedef struct GinScanEntryData *GinScanEntry;

typedef struct GinScanEntryData
{
	/* link to the equals entry in current scan key */
	GinScanEntry master;

	/*
	 * link to values reported to consistentFn, points to
	 * GinScanKey->entryRes[i]
	 */
	bool	   *pval;

	/* entry, got from extractQueryFn */
	Datum		entry;
	OffsetNumber attnum;
	Pointer		extra_data;

	/* Current page in posting tree */
	Buffer		buffer;

	/* current ItemPointer to heap */
	ItemPointerData curItem;

	/* partial match support */
	bool		isPartialMatch;
	TIDBitmap  *partialMatch;
	TBMIterator *partialMatchIterator;
	TBMIterateResult *partialMatchResult;
	StrategyNumber strategy;

	/* used for Posting list and one page in Posting tree */
	ItemPointerData *list;
	uint32		nlist;
	OffsetNumber offset;

	bool		isFinished;
	bool		reduceResult;
	uint32		predictNumberResult;
} GinScanEntryData;

typedef struct GinScanKeyData
{
	/* Number of entries in query (got by extractQueryFn) */
	uint32		nentries;

	/* array of ItemPointer result, reported to consistentFn */
	bool	   *entryRes;

	/* array of scans per entry */
	GinScanEntry scanEntry;
	Pointer    *extra_data;

	/* for calling consistentFn(GinScanKey->entryRes, strategy, query) */
	StrategyNumber strategy;
	Datum		query;
	OffsetNumber attnum;

	ItemPointerData curItem;
	bool		firstCall;
	bool		isFinished;
} GinScanKeyData;

typedef GinScanKeyData *GinScanKey;

typedef struct GinScanOpaqueData
{
	MemoryContext tempCtx;
	GinState	ginstate;

	GinScanKey	keys;
	uint32		nkeys;
	bool		isVoidRes;		/* true if ginstate.extractQueryFn guarantees
								 * that nothing will be found */
} GinScanOpaqueData;

typedef GinScanOpaqueData *GinScanOpaque;

extern Datum ginbeginscan(PG_FUNCTION_ARGS);
extern Datum ginendscan(PG_FUNCTION_ARGS);
extern Datum ginrescan(PG_FUNCTION_ARGS);
extern Datum ginmarkpos(PG_FUNCTION_ARGS);
extern Datum ginrestrpos(PG_FUNCTION_ARGS);
extern void newScanKey(IndexScanDesc scan);

/* ginget.c */
extern PGDLLIMPORT int GinFuzzySearchLimit;

extern Datum gingetbitmap(PG_FUNCTION_ARGS);

/* ginvacuum.c */
extern Datum ginbulkdelete(PG_FUNCTION_ARGS);
extern Datum ginvacuumcleanup(PG_FUNCTION_ARGS);

/* ginarrayproc.c */
extern Datum ginarrayextract(PG_FUNCTION_ARGS);
extern Datum ginqueryarrayextract(PG_FUNCTION_ARGS);
extern Datum ginarrayconsistent(PG_FUNCTION_ARGS);

/* ginbulk.c */
typedef struct EntryAccumulator
{
	Datum		value;
	uint32		length;
	uint32		number;
	OffsetNumber attnum;
	bool		shouldSort;
	ItemPointerData *list;
} EntryAccumulator;

typedef struct
{
	GinState   *ginstate;
	long		allocatedMemory;
	uint32		length;
	EntryAccumulator *entryallocator;
	ItemPointerData *tmpList;
	RBTree	   *tree;
	RBTreeIterator *iterator;
} BuildAccumulator;

extern void ginInitBA(BuildAccumulator *accum);
extern void ginInsertRecordBA(BuildAccumulator *accum,
				  ItemPointer heapptr,
				  OffsetNumber attnum, Datum *entries, int32 nentry);
extern ItemPointerData *ginGetEntry(BuildAccumulator *accum, OffsetNumber *attnum, Datum *entry, uint32 *n);

/* ginfast.c */

typedef struct GinTupleCollector
{
	IndexTuple *tuples;
	uint32		ntuples;
	uint32		lentuples;
	uint32		sumsize;
} GinTupleCollector;

extern void ginHeapTupleFastInsert(Relation index, GinState *ginstate,
					   GinTupleCollector *collector);
extern uint32 ginHeapTupleFastCollect(Relation index, GinState *ginstate,
						GinTupleCollector *collector,
						OffsetNumber attnum, Datum value, ItemPointer item);
extern void ginInsertCleanup(Relation index, GinState *ginstate,
				 bool vac_delay, IndexBulkDeleteResult *stats);

=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
#endif   /* GIN_H */
