/*-------------------------------------------------------------------------
 *
 * gistscan.c
 *	  routines to manage scans on GiST index relations
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/access/gist/gistscan.c,v 1.74 2008/12/04 11:08:46 teodor Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/genam.h"
#include "access/gist_private.h"
#include "access/gistscan.h"
#include "access/relscan.h"
#include "storage/bufmgr.h"
#include "utils/memutils.h"

static void gistfreestack(GISTSearchStack *s);

Datum
gistbeginscan(PG_FUNCTION_ARGS)
{
	Relation	r = (Relation) PG_GETARG_POINTER(0);
	int			nkeys = PG_GETARG_INT32(1);
	ScanKey		key = (ScanKey) PG_GETARG_POINTER(2);
	IndexScanDesc scan;

	scan = RelationGetIndexScan(r, nkeys, key);

	PG_RETURN_POINTER(scan);
}

Datum
gistrescan(PG_FUNCTION_ARGS)
{
	IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	ScanKey		key = (ScanKey) PG_GETARG_POINTER(1);
	GISTScanOpaque so;
	int			i;

	so = (GISTScanOpaque) scan->opaque;
	if (so != NULL)
	{
		/* rescan an existing indexscan --- reset state */
		gistfreestack(so->stack);
		so->stack = NULL;
		/* drop pins on buffers -- no locks held */
		if (BufferIsValid(so->curbuf))
		{
			ReleaseBuffer(so->curbuf);
			so->curbuf = InvalidBuffer;
		}
<<<<<<< HEAD
		if (BufferIsValid(so->markbuf))
		{
			ReleaseBuffer(so->markbuf);
			so->markbuf = InvalidBuffer;
		}

=======
>>>>>>> 38e9348282e
	}
	else
	{
		/* initialize opaque data */
		so = (GISTScanOpaque) palloc(sizeof(GISTScanOpaqueData));
		so->stack = NULL;
		so->tempCxt = createTempGistContext();
		so->curbuf = InvalidBuffer;
		so->giststate = (GISTSTATE *) palloc(sizeof(GISTSTATE));
		initGISTstate(so->giststate, scan->indexRelation);

		scan->opaque = so;
	}

	so->nPageData = so->curPageData = 0;

	/*
	 * Clear all the pointers.
	 */
	ItemPointerSetInvalid(&so->curpos);
<<<<<<< HEAD
	ItemPointerSetInvalid(&so->markpos);
=======
>>>>>>> 38e9348282e
	so->nPageData = so->curPageData = 0;

	so->qual_ok = true;

	/* Update scan key, if a new one is given */
	if (key && scan->numberOfKeys > 0)
	{
		memmove(scan->keyData, key,
				scan->numberOfKeys * sizeof(ScanKeyData));

		/*
		 * Modify the scan key so that all the Consistent method is called for
		 * all comparisons. The original operator is passed to the Consistent
		 * function in the form of its strategy number, which is available
		 * from the sk_strategy field, and its subtype from the sk_subtype
		 * field.
		 *
		 * Next, if any of keys is a NULL and that key is not marked with
		 * SK_SEARCHNULL then nothing can be found.
		 */
		for (i = 0; i < scan->numberOfKeys; i++) {
			scan->keyData[i].sk_func = so->giststate->consistentFn[scan->keyData[i].sk_attno - 1];

			if ( scan->keyData[i].sk_flags & SK_ISNULL ) {
				if ( (scan->keyData[i].sk_flags & SK_SEARCHNULL) == 0 )
					so->qual_ok = false;
			}
		}
	}

	PG_RETURN_VOID();
}

Datum
gistmarkpos(PG_FUNCTION_ARGS)
{
<<<<<<< HEAD
	IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	GISTScanOpaque so;
	GISTSearchStack *o,
			   *n,
			   *tmp;

	so = (GISTScanOpaque) scan->opaque;
	so->markpos = so->curpos;
	if (so->flags & GS_CURBEFORE)
		so->flags |= GS_MRKBEFORE;
	else
		so->flags &= ~GS_MRKBEFORE;

	o = NULL;
	n = so->stack;

	/* copy the parent stack from the current item data */
	while (n != NULL)
	{
		tmp = (GISTSearchStack *) palloc(sizeof(GISTSearchStack));
		tmp->lsn = n->lsn;
		tmp->parentlsn = n->parentlsn;
		tmp->block = n->block;
		tmp->next = o;
		o = tmp;
		n = n->next;
	}

	gistfreestack(so->markstk);
	so->markstk = o;

	/* Update markbuf: make sure to bump ref count on curbuf */
	if (BufferIsValid(so->markbuf))
	{
		ReleaseBuffer(so->markbuf);
		so->markbuf = InvalidBuffer;
	}
	if (BufferIsValid(so->curbuf))
	{
		IncrBufferRefCount(so->curbuf);
		so->markbuf = so->curbuf;
	}

	so->markNPageData = so->nPageData;
	so->markCurPageData = so->curPageData;
	if ( so->markNPageData > 0 )
		memcpy(so->markPageData, so->pageData, sizeof(ItemResult) * so->markNPageData);

=======
	elog(ERROR, "GiST does not support mark/restore");
>>>>>>> 38e9348282e
	PG_RETURN_VOID();
}

Datum
gistrestrpos(PG_FUNCTION_ARGS)
{
<<<<<<< HEAD
	IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	GISTScanOpaque so;
	GISTSearchStack *o,
			   *n,
			   *tmp;

	so = (GISTScanOpaque) scan->opaque;
	so->curpos = so->markpos;
	if (so->flags & GS_MRKBEFORE)
		so->flags |= GS_CURBEFORE;
	else
		so->flags &= ~GS_CURBEFORE;

	o = NULL;
	n = so->markstk;

	/* copy the parent stack from the current item data */
	while (n != NULL)
	{
		tmp = (GISTSearchStack *) palloc(sizeof(GISTSearchStack));
		tmp->lsn = n->lsn;
		tmp->parentlsn = n->parentlsn;
		tmp->block = n->block;
		tmp->next = o;
		o = tmp;
		n = n->next;
	}

	gistfreestack(so->stack);
	so->stack = o;

	/* Update curbuf: be sure to bump ref count on markbuf */
	if (BufferIsValid(so->curbuf))
	{
		ReleaseBuffer(so->curbuf);
		so->curbuf = InvalidBuffer;
	}
	if (BufferIsValid(so->markbuf))
	{
		IncrBufferRefCount(so->markbuf);
		so->curbuf = so->markbuf;
	}

	so->nPageData = so->markNPageData;
	so->curPageData = so->markNPageData;
	if ( so->markNPageData > 0 )
		memcpy(so->pageData, so->markPageData, sizeof(ItemResult) * so->markNPageData);

=======
	elog(ERROR, "GiST does not support mark/restore");
>>>>>>> 38e9348282e
	PG_RETURN_VOID();
}

Datum
gistendscan(PG_FUNCTION_ARGS)
{
	IndexScanDesc scan = (IndexScanDesc) PG_GETARG_POINTER(0);
	GISTScanOpaque so;

	so = (GISTScanOpaque) scan->opaque;

	if (so != NULL)
	{
		gistfreestack(so->stack);
		if (so->giststate != NULL)
		{
			freeGISTstate(so->giststate);
			pfree(so->giststate);
		}
		/* drop pins on buffers -- we aren't holding any locks */
		if (BufferIsValid(so->curbuf))
			ReleaseBuffer(so->curbuf);
		MemoryContextDelete(so->tempCxt);
		pfree(so);
	}

	PG_RETURN_VOID();
}

static void
gistfreestack(GISTSearchStack *s)
{
	while (s != NULL)
	{
		GISTSearchStack *p = s->next;

		pfree(s);
		s = p;
	}
}
