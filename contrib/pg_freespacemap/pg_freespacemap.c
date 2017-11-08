/*-------------------------------------------------------------------------
 *
 * pg_freespacemap.c
 *	  display contents of a free space map
 *
 *	  $PostgreSQL: pgsql/contrib/pg_freespacemap/pg_freespacemap.c,v 1.12 2008/10/02 12:20:50 heikki Exp $
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "funcapi.h"
#include "storage/block.h"
#include "storage/freespace.h"


PG_MODULE_MAGIC;

Datum		pg_freespace(PG_FUNCTION_ARGS);
Datum		pg_freespacedump(PG_FUNCTION_ARGS);

/*
 * Returns the amount of free space on a given page, according to the
 * free space map.
 */
PG_FUNCTION_INFO_V1(pg_freespace);

Datum
pg_freespace(PG_FUNCTION_ARGS)
{
<<<<<<< HEAD
	FuncCallContext *funcctx;
	Datum		result;
	MemoryContext oldcontext;
	FreeSpacePagesContext *fctx;	/* User function context. */
	TupleDesc	tupledesc;
	HeapTuple	tuple;
	FSMHeader  *FreeSpaceMap;	/* FSM main structure. */
	FSMRelation *fsmrel;		/* Individual relation. */

	if (SRF_IS_FIRSTCALL())
	{
		int			i;
		int			nchunks;	/* Size of freespace.c's arena. */
		int			numPages;	/* Max possible no. of pages in map. */
		int			nPages;		/* Mapped pages for a relation. */

		/*
		 * Get the free space map data structure.
		 */
		FreeSpaceMap = GetFreeSpaceMap();

		/* this must match calculation in InitFreeSpaceMap(): */
		nchunks = (MaxFSMPages - 1) / CHUNKPAGES + 1;
		/* Worst case (lots of indexes) could have this many pages: */
		numPages = nchunks * INDEXCHUNKPAGES;

		funcctx = SRF_FIRSTCALL_INIT();

		/* Switch context when allocating stuff to be used in later calls */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/*
		 * Create a function context for cross-call persistence.
		 */
		fctx = (FreeSpacePagesContext *) palloc(sizeof(FreeSpacePagesContext));
		funcctx->user_fctx = fctx;

		/* Construct a tuple descriptor for the result rows. */
		tupledesc = CreateTemplateTupleDesc(NUM_FREESPACE_PAGES_ELEM, false);
		TupleDescInitEntry(tupledesc, (AttrNumber) 1, "reltablespace",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 2, "reldatabase",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 3, "relfilenode",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 4, "relblocknumber",
						   INT8OID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 5, "bytes",
						   INT4OID, -1, 0);

		fctx->tupdesc = BlessTupleDesc(tupledesc);

		/*
		 * Allocate numPages worth of FreeSpacePagesRec records, this is an
		 * upper bound.
		 */
		fctx->record = (FreeSpacePagesRec *) palloc(sizeof(FreeSpacePagesRec) * numPages);

		/* Return to original context when allocating transient memory */
		MemoryContextSwitchTo(oldcontext);

		/*
		 * Lock free space map and scan though all the relations. For each
		 * relation, gets all its mapped pages.
		 */
		LWLockAcquire(FreeSpaceLock, LW_EXCLUSIVE);

		i = 0;

		for (fsmrel = FreeSpaceMap->usageList; fsmrel; fsmrel = fsmrel->nextUsage)
		{
			if (fsmrel->isIndex)
			{
				/* Index relation. */
				IndexFSMPageData *page;

				page = (IndexFSMPageData *)
					(FreeSpaceMap->arena + fsmrel->firstChunk * CHUNKBYTES);

				for (nPages = 0; nPages < fsmrel->storedPages; nPages++)
				{
					fctx->record[i].reltablespace = fsmrel->key.spcNode;
					fctx->record[i].reldatabase = fsmrel->key.dbNode;
					fctx->record[i].relfilenode = fsmrel->key.relNode;
					fctx->record[i].relblocknumber = IndexFSMPageGetPageNum(page);
					fctx->record[i].bytes = 0;
					fctx->record[i].isindex = true;

					page++;
					i++;
				}
			}
			else
			{
				/* Heap relation. */
				FSMPageData *page;

				page = (FSMPageData *)
					(FreeSpaceMap->arena + fsmrel->firstChunk * CHUNKBYTES);

				for (nPages = 0; nPages < fsmrel->storedPages; nPages++)
				{
					fctx->record[i].reltablespace = fsmrel->key.spcNode;
					fctx->record[i].reldatabase = fsmrel->key.dbNode;
					fctx->record[i].relfilenode = fsmrel->key.relNode;
					fctx->record[i].relblocknumber = FSMPageGetPageNum(page);
					fctx->record[i].bytes = FSMPageGetSpace(page);
					fctx->record[i].isindex = false;

					page++;
					i++;
				}
			}
		}

		/* Release free space map. */
		LWLockRelease(FreeSpaceLock);

		/* Set the real no. of calls as we know it now! */
		Assert(i <= numPages);
		funcctx->max_calls = i;
	}

	funcctx = SRF_PERCALL_SETUP();

	/* Get the saved state */
	fctx = funcctx->user_fctx;

	if (funcctx->call_cntr < funcctx->max_calls)
	{
		int			i = funcctx->call_cntr;
		FreeSpacePagesRec *record = &fctx->record[i];
		Datum		values[NUM_FREESPACE_PAGES_ELEM];
		bool		nulls[NUM_FREESPACE_PAGES_ELEM];

		values[0] = ObjectIdGetDatum(record->reltablespace);
		nulls[0] = false;
		values[1] = ObjectIdGetDatum(record->reldatabase);
		nulls[1] = false;
		values[2] = ObjectIdGetDatum(record->relfilenode);
		nulls[2] = false;
		values[3] = Int64GetDatum((int64) record->relblocknumber);
		nulls[3] = false;

		/*
		 * Set (free) bytes to NULL for an index relation.
		 */
		if (record->isindex)
		{
			nulls[4] = true;
		}
		else
		{
			values[4] = UInt32GetDatum(record->bytes);
			nulls[4] = false;
		}

		/* Build and return the tuple. */
		tuple = heap_form_tuple(fctx->tupdesc, values, nulls);
		result = HeapTupleGetDatum(tuple);

		SRF_RETURN_NEXT(funcctx, result);
	}
	else
		SRF_RETURN_DONE(funcctx);
}


/*
 * Function returning relation data from the Free Space Map (FSM).
 */
PG_FUNCTION_INFO_V1(pg_freespacemap_relations);

Datum
pg_freespacemap_relations(PG_FUNCTION_ARGS)
{
	FuncCallContext *funcctx;
	Datum		result;
	MemoryContext oldcontext;
	FreeSpaceRelationsContext *fctx;	/* User function context. */
	TupleDesc	tupledesc;
	HeapTuple	tuple;
	FSMHeader  *FreeSpaceMap;	/* FSM main structure. */
	FSMRelation *fsmrel;		/* Individual relation. */

	if (SRF_IS_FIRSTCALL())
	{
		int			i;
		int			numRelations;		/* Max no. of Relations in map. */

		/*
		 * Get the free space map data structure.
		 */
		FreeSpaceMap = GetFreeSpaceMap();

		numRelations = MaxFSMRelations;

		funcctx = SRF_FIRSTCALL_INIT();

		/* Switch context when allocating stuff to be used in later calls */
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

		/*
		 * Create a function context for cross-call persistence.
		 */
		fctx = (FreeSpaceRelationsContext *) palloc(sizeof(FreeSpaceRelationsContext));
		funcctx->user_fctx = fctx;

		/* Construct a tuple descriptor for the result rows. */
		tupledesc = CreateTemplateTupleDesc(NUM_FREESPACE_RELATIONS_ELEM, false);
		TupleDescInitEntry(tupledesc, (AttrNumber) 1, "reltablespace",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 2, "reldatabase",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 3, "relfilenode",
						   OIDOID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 4, "avgrequest",
						   INT4OID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 5, "interestingpages",
						   INT4OID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 6, "storedpages",
						   INT4OID, -1, 0);
		TupleDescInitEntry(tupledesc, (AttrNumber) 7, "nextpage",
						   INT4OID, -1, 0);

		fctx->tupdesc = BlessTupleDesc(tupledesc);

		/*
		 * Allocate numRelations worth of FreeSpaceRelationsRec records, this
		 * is also an upper bound.
		 */
		fctx->record = (FreeSpaceRelationsRec *) palloc(sizeof(FreeSpaceRelationsRec) * numRelations);

		/* Return to original context when allocating transient memory */
		MemoryContextSwitchTo(oldcontext);

		/*
		 * Lock free space map and scan though all the relations.
		 */
		LWLockAcquire(FreeSpaceLock, LW_EXCLUSIVE);

		i = 0;

		for (fsmrel = FreeSpaceMap->usageList; fsmrel; fsmrel = fsmrel->nextUsage)
		{
			fctx->record[i].reltablespace = fsmrel->key.spcNode;
			fctx->record[i].reldatabase = fsmrel->key.dbNode;
			fctx->record[i].relfilenode = fsmrel->key.relNode;
			fctx->record[i].avgrequest = (int64) fsmrel->avgRequest;
			fctx->record[i].interestingpages = fsmrel->interestingPages;
			fctx->record[i].storedpages = fsmrel->storedPages;
			fctx->record[i].nextpage = fsmrel->nextPage;
			fctx->record[i].isindex = fsmrel->isIndex;

			i++;
		}

		/* Release free space map. */
		LWLockRelease(FreeSpaceLock);

		/* Set the real no. of calls as we know it now! */
		Assert(i <= numRelations);
		funcctx->max_calls = i;
	}

	funcctx = SRF_PERCALL_SETUP();

	/* Get the saved state */
	fctx = funcctx->user_fctx;

	if (funcctx->call_cntr < funcctx->max_calls)
	{
		int			i = funcctx->call_cntr;
		FreeSpaceRelationsRec *record = &fctx->record[i];
		Datum		values[NUM_FREESPACE_RELATIONS_ELEM];
		bool		nulls[NUM_FREESPACE_RELATIONS_ELEM];
=======
	Oid		relid = PG_GETARG_OID(0);
	int64	blkno = PG_GETARG_INT64(1);
	int16	freespace;
	Relation rel;
>>>>>>> 38e9348282e

	rel = relation_open(relid, AccessShareLock);

	if (blkno < 0 || blkno > MaxBlockNumber)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("invalid block number")));

	freespace = GetRecordedFreeSpace(rel, blkno);

	relation_close(rel, AccessShareLock);
	PG_RETURN_INT16(freespace);
}
