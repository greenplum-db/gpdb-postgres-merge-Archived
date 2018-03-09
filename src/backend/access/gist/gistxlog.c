/*-------------------------------------------------------------------------
 *
 * gistxlog.c
 *	  WAL replay logic for GiST.
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *			 src/backend/access/gist/gistxlog.c
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/bufmask.h"
#include "access/gist_private.h"
#include "access/xlogutils.h"
#include "miscadmin.h"
#include "storage/bufmgr.h"
#include "utils/memutils.h"
#include "utils/rel.h"

typedef struct
{
	gistxlogPage *header;
	IndexTuple *itup;
} NewPage;

typedef struct
{
	gistxlogPageSplit *data;
	NewPage    *page;
} PageSplitRecord;

static MemoryContext opCtx;		/* working memory for operations */

/*
 * Replay the clearing of F_FOLLOW_RIGHT flag.
 */
static void
gistRedoClearFollowRight(RelFileNode node, XLogRecPtr lsn,
						 BlockNumber leftblkno)
{
	Buffer		buffer;

	buffer = XLogReadBuffer(node, leftblkno, false);
	if (BufferIsValid(buffer))
	{
		Page		page = (Page) BufferGetPage(buffer);

		/*
		 * Note that we still update the page even if page LSN is equal to the
		 * LSN of this record, because the updated NSN is not included in the
		 * full page image.
		 */
		if (!XLByteLT(lsn, PageGetLSN(page)))
		{
			GistPageGetOpaque(page)->nsn = lsn;
			GistClearFollowRight(page);

			PageSetLSN(page, lsn);
			PageSetTLI(page, ThisTimeLineID);
			MarkBufferDirty(buffer);
		}
		UnlockReleaseBuffer(buffer);
	}
}

/*
 * redo any page update (except page split)
 */
static void
gistRedoPageUpdateRecord(XLogRecPtr lsn, XLogRecord *record)
{
	char	   *begin = XLogRecGetData(record);
	gistxlogPageUpdate *xldata = (gistxlogPageUpdate *) begin;
	Buffer		buffer;
	Page		page;
	char	   *data;

	if (BlockNumberIsValid(xldata->leftchild))
		gistRedoClearFollowRight(xldata->node, lsn, xldata->leftchild);

<<<<<<< HEAD
	if (!isnewroot && xldata->blkno != GIST_ROOT_BLKNO)
		/* operation with root always finalizes insertion */
		pushIncompleteInsert(xldata->node, lsn, xldata->key,
							 &(xldata->blkno), 1,
							 NULL);

	/* nothing else to do if page was backed up (and no info to do it with) */
	if (IsBkpBlockApplied(record, 0))
=======
	/* nothing more to do if page was backed up (and no info to do it with) */
	if (record->xl_info & XLR_BKP_BLOCK_1)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		return;

	buffer = XLogReadBuffer(xldata->node, xldata->blkno, false);
	if (!BufferIsValid(buffer))
		return;
	page = (Page) BufferGetPage(buffer);

	if (XLByteLE(lsn, PageGetLSN(page)))
	{
		UnlockReleaseBuffer(buffer);
		return;
	}

	data = begin + sizeof(gistxlogPageUpdate);

	/* Delete old tuples */
	if (xldata->ntodelete > 0)
	{
		int			i;
		OffsetNumber *todelete = (OffsetNumber *) data;

		data += sizeof(OffsetNumber) * xldata->ntodelete;

		for (i = 0; i < xldata->ntodelete; i++)
			PageIndexTupleDelete(page, todelete[i]);
		if (GistPageIsLeaf(page))
			GistMarkTuplesDeleted(page);
	}

	/* add tuples */
	if (data - begin < record->xl_len)
	{
		OffsetNumber off = (PageIsEmpty(page)) ? FirstOffsetNumber :
		OffsetNumberNext(PageGetMaxOffsetNumber(page));

		while (data - begin < record->xl_len)
		{
			IndexTuple	itup = (IndexTuple) data;
			Size		sz = IndexTupleSize(itup);
			OffsetNumber l;

			data += sz;

			l = PageAddItem(page, (Item) itup, sz, off, false, false);
			if (l == InvalidOffsetNumber)
				elog(ERROR, "failed to add item to GiST index page, size %d bytes",
					 (int) sz);
			off++;
		}
	}
	else
	{
		/*
		 * special case: leafpage, nothing to insert, nothing to delete, then
		 * vacuum marks page
		 */
		if (GistPageIsLeaf(page) && xldata->ntodelete == 0)
			GistClearTuplesDeleted(page);
	}

	if (!GistPageIsLeaf(page) && PageGetMaxOffsetNumber(page) == InvalidOffsetNumber && xldata->blkno == GIST_ROOT_BLKNO)

		/*
		 * all links on non-leaf root page was deleted by vacuum full, so root
		 * page becomes a leaf
		 */
		GistPageSetLeaf(page);

	GistPageGetOpaque(page)->rightlink = InvalidBlockNumber;
	PageSetLSN(page, lsn);
	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);
}

static void
gistRedoPageDeleteRecord(XLogRecPtr lsn, XLogRecord *record)
{
	gistxlogPageDelete *xldata = (gistxlogPageDelete *) XLogRecGetData(record);
	Buffer		buffer;
	Page		page;

	/* nothing else to do if page was backed up (and no info to do it with) */
	if (IsBkpBlockApplied(record, 0))
		return;

	buffer = XLogReadBuffer(xldata->node, xldata->blkno, false);
	if (!BufferIsValid(buffer))
		return;

	page = (Page) BufferGetPage(buffer);
	GistPageSetDeleted(page);

	PageSetLSN(page, lsn);
	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);
}

static void
decodePageSplitRecord(PageSplitRecord *decoded, XLogRecord *record)
{
	char	   *begin = XLogRecGetData(record),
			   *ptr;
	int			j,
				i = 0;

	decoded->data = (gistxlogPageSplit *) begin;
	decoded->page = (NewPage *) palloc(sizeof(NewPage) * decoded->data->npage);

	ptr = begin + sizeof(gistxlogPageSplit);
	for (i = 0; i < decoded->data->npage; i++)
	{
		Assert(ptr - begin < record->xl_len);
		decoded->page[i].header = (gistxlogPage *) ptr;
		ptr += sizeof(gistxlogPage);

		decoded->page[i].itup = (IndexTuple *)
			palloc(sizeof(IndexTuple) * decoded->page[i].header->num);
		j = 0;
		while (j < decoded->page[i].header->num)
		{
			Assert(ptr - begin < record->xl_len);
			decoded->page[i].itup[j] = (IndexTuple) ptr;
			ptr += IndexTupleSize((IndexTuple) ptr);
			j++;
		}
	}
}

static void
gistRedoPageSplitRecord(XLogRecPtr lsn, XLogRecord *record)
{
	gistxlogPageSplit *xldata = (gistxlogPageSplit *) XLogRecGetData(record);
	PageSplitRecord xlrec;
	Buffer		buffer;
	Page		page;
	int			i;
	bool		isrootsplit = false;

	if (BlockNumberIsValid(xldata->leftchild))
		gistRedoClearFollowRight(xldata->node, lsn, xldata->leftchild);
	decodePageSplitRecord(&xlrec, record);

	/* loop around all pages */
	for (i = 0; i < xlrec.data->npage; i++)
	{
		NewPage    *newpage = xlrec.page + i;
		int			flags;

		if (newpage->header->blkno == GIST_ROOT_BLKNO)
		{
			Assert(i == 0);
			isrootsplit = true;
		}

		buffer = XLogReadBuffer(xlrec.data->node, newpage->header->blkno, true);
		Assert(BufferIsValid(buffer));
		page = (Page) BufferGetPage(buffer);

		/* ok, clear buffer */
		if (xlrec.data->origleaf && newpage->header->blkno != GIST_ROOT_BLKNO)
			flags = F_LEAF;
		else
			flags = 0;
		GISTInitBuffer(buffer, flags);

		/* and fill it */
		gistfillbuffer(page, newpage->itup, newpage->header->num, FirstOffsetNumber);

		if (newpage->header->blkno == GIST_ROOT_BLKNO)
		{
			GistPageGetOpaque(page)->rightlink = InvalidBlockNumber;
			GistPageGetOpaque(page)->nsn = xldata->orignsn;
			GistClearFollowRight(page);
		}
		else
		{
			if (i < xlrec.data->npage - 1)
				GistPageGetOpaque(page)->rightlink = xlrec.page[i + 1].header->blkno;
			else
				GistPageGetOpaque(page)->rightlink = xldata->origrlink;
			GistPageGetOpaque(page)->nsn = xldata->orignsn;
			if (i < xlrec.data->npage - 1 && !isrootsplit)
				GistMarkFollowRight(page);
			else
				GistClearFollowRight(page);
		}

		PageSetLSN(page, lsn);
		MarkBufferDirty(buffer);
		UnlockReleaseBuffer(buffer);
	}
}

static void
gistRedoCreateIndex(XLogRecPtr lsn, XLogRecord *record)
{
	RelFileNode *node = (RelFileNode *) XLogRecGetData(record);
	Buffer		buffer;
	Page		page;

	buffer = XLogReadBuffer(*node, GIST_ROOT_BLKNO, true);
	Assert(BufferIsValid(buffer));
	page = (Page) BufferGetPage(buffer);

	GISTInitBuffer(buffer, F_LEAF);

	PageSetLSN(page, lsn);

	MarkBufferDirty(buffer);
	UnlockReleaseBuffer(buffer);
}

<<<<<<< HEAD
static void
gistRedoCompleteInsert(XLogRecPtr lsn __attribute__((unused)), XLogRecord *record)
{
	char	   *begin = XLogRecGetData(record),
			   *ptr;
	gistxlogInsertComplete *xlrec;

	xlrec = (gistxlogInsertComplete *) begin;

	ptr = begin + sizeof(gistxlogInsertComplete);
	while (ptr - begin < record->xl_len)
	{
		Assert(record->xl_len - (ptr - begin) >= sizeof(ItemPointerData));
		forgetIncompleteInsert(xlrec->node, *((ItemPointerData *) ptr));
		ptr += sizeof(ItemPointerData);
	}
}

=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
void
gist_redo(XLogRecPtr beginLoc, XLogRecPtr lsn, XLogRecord *record)
{
	uint8		info = record->xl_info & ~XLR_INFO_MASK;
	MemoryContext oldCxt;

	/*
	 * GiST indexes do not require any conflict processing. NB: If we ever
	 * implement a similar optimization we have in b-tree, and remove killed
	 * tuples outside VACUUM, we'll need to handle that here.
	 */
	RestoreBkpBlocks(lsn, record, false);

	oldCxt = MemoryContextSwitchTo(opCtx);
	switch (info)
	{
		case XLOG_GIST_PAGE_UPDATE:
			gistRedoPageUpdateRecord(lsn, record);
			break;
		case XLOG_GIST_PAGE_DELETE:
			gistRedoPageDeleteRecord(lsn, record);
			break;
		case XLOG_GIST_PAGE_SPLIT:
			gistRedoPageSplitRecord(lsn, record);
			break;
		case XLOG_GIST_CREATE_INDEX:
			gistRedoCreateIndex(lsn, record);
			break;
		default:
			elog(PANIC, "gist_redo: unknown op code %u", info);
	}

	MemoryContextSwitchTo(oldCxt);
	MemoryContextReset(opCtx);
}

static void
out_target(StringInfo buf, RelFileNode node)
{
	appendStringInfo(buf, "rel %u/%u/%u",
					 node.spcNode, node.dbNode, node.relNode);
}

static void
out_gistxlogPageUpdate(StringInfo buf, gistxlogPageUpdate *xlrec)
{
	out_target(buf, xlrec->node);
	appendStringInfo(buf, "; block number %u", xlrec->blkno);
}

static void
out_gistxlogPageDelete(StringInfo buf, gistxlogPageDelete *xlrec)
{
	appendStringInfo(buf, "page_delete: rel %u/%u/%u; blkno %u",
				xlrec->node.spcNode, xlrec->node.dbNode, xlrec->node.relNode,
					 xlrec->blkno);
}

static void
out_gistxlogPageSplit(StringInfo buf, gistxlogPageSplit *xlrec)
{
	appendStringInfo(buf, "page_split: ");
	out_target(buf, xlrec->node);
	appendStringInfo(buf, "; block number %u splits to %d pages",
					 xlrec->origblkno, xlrec->npage);
}

void
gist_desc(StringInfo buf, XLogRecPtr beginLoc, XLogRecord *record)
{
	uint8		info = record->xl_info & ~XLR_INFO_MASK;
	char		*rec = XLogRecGetData(record);

	switch (info)
	{
		case XLOG_GIST_PAGE_UPDATE:
			appendStringInfo(buf, "page_update: ");
			out_gistxlogPageUpdate(buf, (gistxlogPageUpdate *) rec);
			break;
		case XLOG_GIST_PAGE_DELETE:
			out_gistxlogPageDelete(buf, (gistxlogPageDelete *) rec);
			break;
		case XLOG_GIST_PAGE_SPLIT:
			out_gistxlogPageSplit(buf, (gistxlogPageSplit *) rec);
			break;
		case XLOG_GIST_CREATE_INDEX:
			appendStringInfo(buf, "create_index: rel %u/%u/%u",
							 ((RelFileNode *) rec)->spcNode,
							 ((RelFileNode *) rec)->dbNode,
							 ((RelFileNode *) rec)->relNode);
			break;
		default:
			appendStringInfo(buf, "unknown gist op code %u", info);
			break;
	}
}

<<<<<<< HEAD
IndexTuple
gist_form_invalid_tuple(BlockNumber blkno)
{
	/*
	 * we don't alloc space for null's bitmap, this is invalid tuple, be
	 * carefull in read and write code
	 */
	Size		size = IndexInfoFindDataOffset(0);
	IndexTuple	tuple = (IndexTuple) palloc0(size);

	tuple->t_info |= size;

	ItemPointerSetBlockNumber(&(tuple->t_tid), blkno);
	GistTupleSetInvalid(tuple);

	return tuple;
}


static void
gistxlogFindPath(Relation index, gistIncompleteInsert *insert)
{
	GISTInsertStack *top;

	insert->pathlen = 0;
	insert->path = NULL;

	if ((top = gistFindPath(index, insert->origblkno)) != NULL)
	{
		int			i;
		GISTInsertStack *ptr;

		for (ptr = top; ptr; ptr = ptr->parent)
			insert->pathlen++;

		insert->path = (BlockNumber *) palloc(sizeof(BlockNumber) * insert->pathlen);

		i = 0;
		for (ptr = top; ptr; ptr = ptr->parent)
			insert->path[i++] = ptr->blkno;
	}
	else
		elog(ERROR, "lost parent for block %u", insert->origblkno);
}

static SplitedPageLayout *
gistMakePageLayout(Buffer *buffers, int nbuffers)
{
	SplitedPageLayout *res = NULL,
			   *resptr;

	while (nbuffers-- > 0)
	{
		Page		page = BufferGetPage(buffers[nbuffers]);
		IndexTuple *vec;
		int			veclen;

		resptr = (SplitedPageLayout *) palloc0(sizeof(SplitedPageLayout));

		resptr->block.blkno = BufferGetBlockNumber(buffers[nbuffers]);
		resptr->block.num = PageGetMaxOffsetNumber(page);

		vec = gistextractpage(page, &veclen);
		resptr->list = gistfillitupvec(vec, veclen, &(resptr->lenlist));

		resptr->next = res;
		res = resptr;
	}

	return res;
}

/*
 * Mask a Gist page before running consistency checks on it.
 */
void
gist_mask(char *pagedata, BlockNumber blkno)
{
	Page		page = (Page) pagedata;

	mask_page_lsn_and_checksum(page);

	mask_page_hint_bits(page);
	mask_unused_space(page);

	/*
	 * NSN is nothing but a special purpose LSN. Hence, mask it for the same
	 * reason as mask_page_lsn.
	 */
	PageXLogRecPtrSet(GistPageGetOpaque(page)->nsn, (uint64) MASK_MARKER);

#if PG_VERSION_NUM >= 90100
	/*
	 * We update F_FOLLOW_RIGHT flag on the left child after writing WAL
	 * record. Hence, mask this flag. See gistplacetopage() for details.
	 */
	GistMarkFollowRight(page);
#endif

	if (GistPageIsLeaf(page))
	{
		/*
		 * In gist leaf pages, it is possible to modify the LP_FLAGS without
		 * emitting any WAL record. Hence, mask the line pointer flags. See
		 * gistkillitems() for details.
		 */
		mask_lp_flags(page);
	}

#if PG_VERSION_NUM >= 90600
	/*
	 * During gist redo, we never mark a page as garbage. Hence, mask it to
	 * ignore any differences.
	 */
	GistClearPageHasGarbage(page);
#endif
}

/*
 * Continue insert after crash.  In normal situations, there aren't any
 * incomplete inserts, but if a crash occurs partway through an insertion
 * sequence, we'll need to finish making the index valid at the end of WAL
 * replay.
 *
 * Note that we assume the index is now in a valid state, except for the
 * unfinished insertion.  In particular it's safe to invoke gistFindPath();
 * there shouldn't be any garbage pages for it to run into.
 *
 * To complete insert we can't use basic insertion algorithm because
 * during insertion we can't call user-defined support functions of opclass.
 * So, we insert 'invalid' tuples without real key and do it by separate algorithm.
 * 'invalid' tuple should be updated by vacuum full.
 */
static void
gistContinueInsert(gistIncompleteInsert *insert)
{
	IndexTuple *itup;
	int			i,
				lenitup;
	Relation	index;

	index = CreateFakeRelcacheEntry(insert->node);

	/*
	 * needed vector itup never will be more than initial lenblkno+2, because
	 * during this processing Indextuple can be only smaller
	 */
	lenitup = insert->lenblk;
	itup = (IndexTuple *) palloc(sizeof(IndexTuple) * (lenitup + 2 /* guarantee root split */ ));

	for (i = 0; i < insert->lenblk; i++)
		itup[i] = gist_form_invalid_tuple(insert->blkno[i]);

	/*
	 * any insertion of itup[] should make LOG message about
	 */

	if (insert->origblkno == GIST_ROOT_BLKNO)
	{
		/*
		 * it was split root, so we should only make new root. it can't be
		 * simple insert into root, we should replace all content of root.
		 */
		Buffer		buffer = XLogReadBuffer(insert->node, GIST_ROOT_BLKNO, true);

		gistnewroot(index, buffer, itup, lenitup, NULL);
		UnlockReleaseBuffer(buffer);
	}
	else
	{
		Buffer	   *buffers;
		Page	   *pages;
		int			numbuffer;
		OffsetNumber *todelete;

		/* construct path */
		gistxlogFindPath(index, insert);

		Assert(insert->pathlen > 0);

		buffers = (Buffer *) palloc(sizeof(Buffer) * (insert->lenblk + 2 /* guarantee root split */ ));
		pages = (Page *) palloc(sizeof(Page) * (insert->lenblk + 2 /* guarantee root split */ ));
		todelete = (OffsetNumber *) palloc(sizeof(OffsetNumber) * (insert->lenblk + 2 /* guarantee root split */ ));

		for (i = 0; i < insert->pathlen; i++)
		{
			int			j,
						k,
						pituplen = 0;
			uint8		xlinfo;
			XLogRecData *rdata;
			XLogRecPtr	recptr;
			Buffer		tempbuffer = InvalidBuffer;
			int			ntodelete = 0;

			numbuffer = 1;
			buffers[0] = ReadBuffer(index, insert->path[i]);
			LockBuffer(buffers[0], GIST_EXCLUSIVE);

			/*
			 * we check buffer, because we restored page earlier
			 */
			gistcheckpage(index, buffers[0]);

			pages[0] = BufferGetPage(buffers[0]);
			Assert(!GistPageIsLeaf(pages[0]));

			pituplen = PageGetMaxOffsetNumber(pages[0]);

			/* find remove old IndexTuples to remove */
			for (j = 0; j < pituplen && ntodelete < lenitup; j++)
			{
				BlockNumber blkno;
				ItemId		iid = PageGetItemId(pages[0], j + FirstOffsetNumber);
				IndexTuple	idxtup = (IndexTuple) PageGetItem(pages[0], iid);

				blkno = ItemPointerGetBlockNumber(&(idxtup->t_tid));

				for (k = 0; k < lenitup; k++)
					if (ItemPointerGetBlockNumber(&(itup[k]->t_tid)) == blkno)
					{
						todelete[ntodelete] = j + FirstOffsetNumber - ntodelete;
						ntodelete++;
						break;
					}
			}

			if (ntodelete == 0)
				elog(PANIC, "gistContinueInsert: cannot find pointer to page(s)");

			/*
			 * we check space with subtraction only first tuple to delete,
			 * hope, that wiil be enough space....
			 */

			if (gistnospace(pages[0], itup, lenitup, *todelete, 0))
			{

				/* no space left on page, so we must split */
				buffers[numbuffer] = ReadBuffer(index, P_NEW);
				LockBuffer(buffers[numbuffer], GIST_EXCLUSIVE);
				GISTInitBuffer(buffers[numbuffer], 0);
				pages[numbuffer] = BufferGetPage(buffers[numbuffer]);
				gistfillbuffer(pages[numbuffer], itup, lenitup, FirstOffsetNumber);
				numbuffer++;

				if (BufferGetBlockNumber(buffers[0]) == GIST_ROOT_BLKNO)
				{
					Buffer		tmp;

					/*
					 * we split root, just copy content from root to new page
					 */

					/* sanity check */
					if (i + 1 != insert->pathlen)
						elog(PANIC, "unexpected pathlen in index \"%s\"",
							 RelationGetRelationName(index));

					/* fill new page, root will be changed later */
					tempbuffer = ReadBuffer(index, P_NEW);
					LockBuffer(tempbuffer, GIST_EXCLUSIVE);
					memcpy(BufferGetPage(tempbuffer), pages[0], BufferGetPageSize(tempbuffer));

					/* swap buffers[0] (was root) and temp buffer */
					tmp = buffers[0];
					buffers[0] = tempbuffer;
					tempbuffer = tmp;	/* now in tempbuffer GIST_ROOT_BLKNO,
										 * it is still unchanged */

					pages[0] = BufferGetPage(buffers[0]);
				}

				START_CRIT_SECTION();

				for (j = 0; j < ntodelete; j++)
					PageIndexTupleDelete(pages[0], todelete[j]);

				xlinfo = XLOG_GIST_PAGE_SPLIT;
				rdata = formSplitRdata(index->rd_node, insert->path[i],
									   false, &(insert->key),
									 gistMakePageLayout(buffers, numbuffer));

			}
			else
			{
				START_CRIT_SECTION();

				for (j = 0; j < ntodelete; j++)
					PageIndexTupleDelete(pages[0], todelete[j]);
				gistfillbuffer(pages[0], itup, lenitup, InvalidOffsetNumber);

				xlinfo = XLOG_GIST_PAGE_UPDATE;
				rdata = formUpdateRdata(index->rd_node, buffers[0],
										todelete, ntodelete,
										itup, lenitup, &(insert->key));
			}

			/*
			 * use insert->key as mark for completion of insert (form*Rdata()
			 * above) for following possible replays
			 */

			/* write pages, we should mark it dirty befor XLogInsert() */
			for (j = 0; j < numbuffer; j++)
			{
				GistPageGetOpaque(pages[j])->rightlink = InvalidBlockNumber;
				MarkBufferDirty(buffers[j]);
			}
			recptr = XLogInsert(RM_GIST_ID, xlinfo, rdata);
			for (j = 0; j < numbuffer; j++)
			{
				PageSetLSN(pages[j], recptr);
			}

			END_CRIT_SECTION();

			lenitup = numbuffer;
			for (j = 0; j < numbuffer; j++)
			{
				itup[j] = gist_form_invalid_tuple(BufferGetBlockNumber(buffers[j]));
				UnlockReleaseBuffer(buffers[j]);
			}

			if (tempbuffer != InvalidBuffer)
			{
				/*
				 * it was a root split, so fill it by new values
				 */
				gistnewroot(index, tempbuffer, itup, lenitup, &(insert->key));
				UnlockReleaseBuffer(tempbuffer);
			}
		}
	}

	FreeFakeRelcacheEntry(index);

	ereport(LOG,
			(errmsg("index %u/%u/%u needs VACUUM FULL or REINDEX to finish crash recovery",
			insert->node.spcNode, insert->node.dbNode, insert->node.relNode),
		   errdetail("Incomplete insertion detected during crash replay.")));
}

=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
void
gist_xlog_startup(void)
{
	opCtx = createTempGistContext();
}

void
gist_xlog_cleanup(void)
{
	MemoryContextDelete(opCtx);
}

/*
 * Write WAL record of a page split.
 */
XLogRecPtr
gistXLogSplit(RelFileNode node, BlockNumber blkno, bool page_is_leaf,
			  SplitedPageLayout *dist,
			  BlockNumber origrlink, GistNSN orignsn,
			  Buffer leftchildbuf)
{
	XLogRecData *rdata;
	gistxlogPageSplit xlrec;
	SplitedPageLayout *ptr;
	int			npage = 0,
				cur;
	XLogRecPtr	recptr;

	for (ptr = dist; ptr; ptr = ptr->next)
		npage++;

	rdata = (XLogRecData *) palloc(sizeof(XLogRecData) * (npage * 2 + 2));

	xlrec.node = node;
	xlrec.origblkno = blkno;
	xlrec.origrlink = origrlink;
	xlrec.orignsn = orignsn;
	xlrec.origleaf = page_is_leaf;
	xlrec.npage = (uint16) npage;
	xlrec.leftchild =
		BufferIsValid(leftchildbuf) ? BufferGetBlockNumber(leftchildbuf) : InvalidBlockNumber;

	rdata[0].data = (char *) &xlrec;
	rdata[0].len = sizeof(gistxlogPageSplit);
	rdata[0].buffer = InvalidBuffer;

	cur = 1;

	/*
	 * Include a full page image of the child buf. (only necessary if a
	 * checkpoint happened since the child page was split)
	 */
	if (BufferIsValid(leftchildbuf))
	{
		rdata[cur - 1].next = &(rdata[cur]);
		rdata[cur].data = NULL;
		rdata[cur].len = 0;
		rdata[cur].buffer = leftchildbuf;
		rdata[cur].buffer_std = true;
		cur++;
	}

	for (ptr = dist; ptr; ptr = ptr->next)
	{
		rdata[cur - 1].next = &(rdata[cur]);
		rdata[cur].buffer = InvalidBuffer;
		rdata[cur].data = (char *) &(ptr->block);
		rdata[cur].len = sizeof(gistxlogPage);
		cur++;

		rdata[cur - 1].next = &(rdata[cur]);
		rdata[cur].buffer = InvalidBuffer;
		rdata[cur].data = (char *) (ptr->list);
		rdata[cur].len = ptr->lenlist;
		cur++;
	}
	rdata[cur - 1].next = NULL;

	recptr = XLogInsert(RM_GIST_ID, XLOG_GIST_PAGE_SPLIT, rdata);

	pfree(rdata);
	return recptr;
}

/*
 * Write XLOG record describing a page update. The update can include any
 * number of deletions and/or insertions of tuples on a single index page.
 *
 * If this update inserts a downlink for a split page, also record that
 * the F_FOLLOW_RIGHT flag on the child page is cleared and NSN set.
 *
 * Note that both the todelete array and the tuples are marked as belonging
 * to the target buffer; they need not be stored in XLOG if XLogInsert decides
 * to log the whole buffer contents instead.  Also, we take care that there's
 * at least one rdata item referencing the buffer, even when ntodelete and
 * ituplen are both zero; this ensures that XLogInsert knows about the buffer.
 */
XLogRecPtr
gistXLogUpdate(RelFileNode node, Buffer buffer,
			   OffsetNumber *todelete, int ntodelete,
			   IndexTuple *itup, int ituplen,
			   Buffer leftchildbuf)
{
	XLogRecData *rdata;
	gistxlogPageUpdate *xlrec;
	int			cur,
				i;
	XLogRecPtr	recptr;

	rdata = (XLogRecData *) palloc(sizeof(XLogRecData) * (4 + ituplen));
	xlrec = (gistxlogPageUpdate *) palloc(sizeof(gistxlogPageUpdate));

	xlrec->node = node;
	xlrec->blkno = BufferGetBlockNumber(buffer);
	xlrec->ntodelete = ntodelete;
	xlrec->leftchild =
		BufferIsValid(leftchildbuf) ? BufferGetBlockNumber(leftchildbuf) : InvalidBlockNumber;

	rdata[0].buffer = buffer;
	rdata[0].buffer_std = true;
	rdata[0].data = NULL;
	rdata[0].len = 0;
	rdata[0].next = &(rdata[1]);

	rdata[1].data = (char *) xlrec;
	rdata[1].len = sizeof(gistxlogPageUpdate);
	rdata[1].buffer = InvalidBuffer;
	rdata[1].next = &(rdata[2]);

	rdata[2].data = (char *) todelete;
	rdata[2].len = sizeof(OffsetNumber) * ntodelete;
	rdata[2].buffer = buffer;
	rdata[2].buffer_std = true;

	cur = 3;

	/* new tuples */
	for (i = 0; i < ituplen; i++)
	{
		rdata[cur - 1].next = &(rdata[cur]);
		rdata[cur].data = (char *) (itup[i]);
		rdata[cur].len = IndexTupleSize(itup[i]);
		rdata[cur].buffer = buffer;
		rdata[cur].buffer_std = true;
		cur++;
	}

	/*
	 * Include a full page image of the child buf. (only necessary if a
	 * checkpoint happened since the child page was split)
	 */
	if (BufferIsValid(leftchildbuf))
	{
		rdata[cur - 1].next = &(rdata[cur]);
		rdata[cur].data = NULL;
		rdata[cur].len = 0;
		rdata[cur].buffer = leftchildbuf;
		rdata[cur].buffer_std = true;
		cur++;
	}
	rdata[cur - 1].next = NULL;

	recptr = XLogInsert(RM_GIST_ID, XLOG_GIST_PAGE_UPDATE, rdata);

	pfree(rdata);
	return recptr;
}
