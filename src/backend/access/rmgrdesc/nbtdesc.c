/*-------------------------------------------------------------------------
 *
 * nbtdesc.c
 *	  rmgr descriptor routines for access/nbtree/nbtxlog.c
 *
 * Portions Copyright (c) 1996-2015, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/rmgrdesc/nbtdesc.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/nbtree.h"

<<<<<<< HEAD
static void
out_target(StringInfo buf, xl_btreetid *target)
{
	appendStringInfo(buf, "rel %u/%u/%u; tid %u/%u",
			 target->node.spcNode, target->node.dbNode, target->node.relNode,
					 ItemPointerGetBlockNumber(&(target->tid)),
					 ItemPointerGetOffsetNumber(&(target->tid)));
}

/*
 * Print additional information about an INSERT record.
 */
static void
out_insert(StringInfo buf, bool isleaf, bool ismeta, XLogRecord *record)
{
	char			*rec = XLogRecGetData(record);
	xl_btree_insert *xlrec = (xl_btree_insert *) rec;

	char	   *datapos;
	int			datalen;
	xl_btree_metadata md = { InvalidBlockNumber, 0, InvalidBlockNumber, 0 };
	BlockNumber downlink = 0;

	datapos = (char *) xlrec + SizeOfBtreeInsert;
	datalen = record->xl_len - SizeOfBtreeInsert;
	if (!isleaf)
	{
		memcpy(&downlink, datapos, sizeof(BlockNumber));
		datapos += sizeof(BlockNumber);
		datalen -= sizeof(BlockNumber);
	}
	if (ismeta)
	{
		memcpy(&md, datapos, sizeof(xl_btree_metadata));
		datapos += sizeof(xl_btree_metadata);
		datalen -= sizeof(xl_btree_metadata);
	}

	if ((record->xl_info & XLR_BKP_BLOCK(0)) != 0 && !ismeta && isleaf)
	{
		appendStringInfo(buf, "; page %u",
						 ItemPointerGetBlockNumber(&(xlrec->target.tid)));
		return;					/* nothing to do */
	}

	if ((record->xl_info & XLR_BKP_BLOCK(0)) == 0)
	{
		appendStringInfo(buf, "; add length %d item at offset %d in page %u",
						 datalen, 
						 ItemPointerGetOffsetNumber(&(xlrec->target.tid)),
						 ItemPointerGetBlockNumber(&(xlrec->target.tid)));
	}

	if (ismeta)
		appendStringInfo(buf, "; restore metadata page 0 (root page value %u, level %d, fastroot page value %u, fastlevel %d)",
						 md.root, 
						 md.level,
						 md.fastroot, 
						 md.fastlevel);

	/* Forget any split this insertion completes */
//	if (!isleaf)
//		appendStringInfo(buf, "; completes split for page %u",
//		 				 downlink);
}

/*
 * Print additional information about a DELETE record.
 */
static void
out_delete(StringInfo buf, XLogRecord *record)
{
	char			*rec = XLogRecGetData(record);
	xl_btree_delete *xlrec = (xl_btree_delete *) rec;

	if ((record->xl_info & XLR_BKP_BLOCK(0)) != 0)
		return;

	xlrec = (xl_btree_delete *) XLogRecGetData(record);

	if (record->xl_len > SizeOfBtreeDelete)
	{
		OffsetNumber *unused;
		OffsetNumber *unend;

		unused = (OffsetNumber *) ((char *) xlrec + SizeOfBtreeDelete);
		unend = (OffsetNumber *) ((char *) xlrec + record->xl_len);

		appendStringInfo(buf, "; page index (unend - unused = %u)",
						 (unsigned int)(unend - unused));
	}
}

void
btree_desc(StringInfo buf, XLogRecord *record)
{
	char	   *rec = XLogRecGetData(record);
	uint8		xl_info = record->xl_info;
	uint8		info = xl_info & ~XLR_INFO_MASK;
=======
void
btree_desc(StringInfo buf, XLogReaderState *record)
{
	char	   *rec = XLogRecGetData(record);
	uint8		info = XLogRecGetInfo(record) & ~XLR_INFO_MASK;
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

	switch (info)
	{
		case XLOG_BTREE_INSERT_LEAF:
<<<<<<< HEAD
			{
				xl_btree_insert *xlrec = (xl_btree_insert *) rec;

				appendStringInfoString(buf, "insert: ");
				out_target(buf, &(xlrec->target));
				out_insert(buf, /* isleaf */ true, /* ismeta */ false, record);
				break;
			}
		case XLOG_BTREE_INSERT_UPPER:
			{
				xl_btree_insert *xlrec = (xl_btree_insert *) rec;

				appendStringInfoString(buf, "insert_upper: ");
				out_target(buf, &(xlrec->target));
				out_insert(buf, /* isleaf */ false, /* ismeta */ false, record);
				break;
			}
=======
		case XLOG_BTREE_INSERT_UPPER:
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
		case XLOG_BTREE_INSERT_META:
			{
				xl_btree_insert *xlrec = (xl_btree_insert *) rec;

<<<<<<< HEAD
				appendStringInfoString(buf, "insert_meta: ");
				out_target(buf, &(xlrec->target));
				out_insert(buf, /* isleaf */ false, /* ismeta */ true, record);
=======
				appendStringInfo(buf, "off %u", xlrec->offnum);
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
				break;
			}
		case XLOG_BTREE_SPLIT_L:
		case XLOG_BTREE_SPLIT_R:
		case XLOG_BTREE_SPLIT_L_ROOT:
		case XLOG_BTREE_SPLIT_R_ROOT:
			{
				xl_btree_split *xlrec = (xl_btree_split *) rec;

				appendStringInfo(buf, "level %u, firstright %d",
								 xlrec->level, xlrec->firstright);
				break;
			}
		case XLOG_BTREE_VACUUM:
			{
				xl_btree_vacuum *xlrec = (xl_btree_vacuum *) rec;

				appendStringInfo(buf, "lastBlockVacuumed %u",
								 xlrec->lastBlockVacuumed);
				break;
			}
		case XLOG_BTREE_DELETE:
			{
				xl_btree_delete *xlrec = (xl_btree_delete *) rec;

<<<<<<< HEAD
				appendStringInfo(buf, "delete: index %u/%u/%u; iblk %u, heap %u/%u/%u;",
				xlrec->node.spcNode, xlrec->node.dbNode, xlrec->node.relNode,
								 xlrec->block,
								 xlrec->hnode.spcNode, xlrec->hnode.dbNode, xlrec->hnode.relNode);
				out_delete(buf, record);
=======
				appendStringInfo(buf, "%d items", xlrec->nitems);
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
				break;
			}
		case XLOG_BTREE_MARK_PAGE_HALFDEAD:
			{
				xl_btree_mark_page_halfdead *xlrec = (xl_btree_mark_page_halfdead *) rec;

				appendStringInfo(buf, "topparent %u; leaf %u; left %u; right %u",
								 xlrec->topparent, xlrec->leafblk, xlrec->leftblk, xlrec->rightblk);
				break;
			}
		case XLOG_BTREE_UNLINK_PAGE_META:
		case XLOG_BTREE_UNLINK_PAGE:
			{
				xl_btree_unlink_page *xlrec = (xl_btree_unlink_page *) rec;

				appendStringInfo(buf, "left %u; right %u; btpo_xact %u; ",
								 xlrec->leftsib, xlrec->rightsib,
								 xlrec->btpo_xact);
				appendStringInfo(buf, "leafleft %u; leafright %u; topparent %u",
								 xlrec->leafleftsib, xlrec->leafrightsib,
								 xlrec->topparent);
				break;
			}
		case XLOG_BTREE_NEWROOT:
			{
				xl_btree_newroot *xlrec = (xl_btree_newroot *) rec;

				appendStringInfo(buf, "lev %u", xlrec->level);
				break;
			}
		case XLOG_BTREE_REUSE_PAGE:
			{
				xl_btree_reuse_page *xlrec = (xl_btree_reuse_page *) rec;

				appendStringInfo(buf, "rel %u/%u/%u; latestRemovedXid %u",
								 xlrec->node.spcNode, xlrec->node.dbNode,
							   xlrec->node.relNode, xlrec->latestRemovedXid);
				break;
			}
	}
}

const char *
btree_identify(uint8 info)
{
	const char *id = NULL;

	switch (info & ~XLR_INFO_MASK)
	{
		case XLOG_BTREE_INSERT_LEAF:
			id = "INSERT_LEAF";
			break;
		case XLOG_BTREE_INSERT_UPPER:
			id = "INSERT_UPPER";
			break;
		case XLOG_BTREE_INSERT_META:
			id = "INSERT_META";
			break;
		case XLOG_BTREE_SPLIT_L:
			id = "SPLIT_L";
			break;
		case XLOG_BTREE_SPLIT_R:
			id = "SPLIT_R";
			break;
		case XLOG_BTREE_SPLIT_L_ROOT:
			id = "SPLIT_L_ROOT";
			break;
		case XLOG_BTREE_SPLIT_R_ROOT:
			id = "SPLIT_R_ROOT";
			break;
		case XLOG_BTREE_VACUUM:
			id = "VACUUM";
			break;
		case XLOG_BTREE_DELETE:
			id = "DELETE";
			break;
		case XLOG_BTREE_MARK_PAGE_HALFDEAD:
			id = "MARK_PAGE_HALFDEAD";
			break;
		case XLOG_BTREE_UNLINK_PAGE:
			id = "UNLINK_PAGE";
			break;
		case XLOG_BTREE_UNLINK_PAGE_META:
			id = "UNLINK_PAGE_META";
			break;
		case XLOG_BTREE_NEWROOT:
			id = "NEWROOT";
			break;
		case XLOG_BTREE_REUSE_PAGE:
			id = "REUSE_PAGE";
			break;
	}

	return id;
}
