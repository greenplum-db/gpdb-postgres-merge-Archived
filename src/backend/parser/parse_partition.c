/*-------------------------------------------------------------------------
 *
 * parse_partition.c
 *	  Expand GPDB legacy partition syntax to PostgreSQL commands.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	src/backend/parser/parse_partition.c
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/table.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "nodes/makefuncs.h"
#include "nodes/parsenodes.h"
#include "parser/parse_utilcmd.h"
#include "utils/lsyscache.h"
#include "utils/partcache.h"
#include "utils/rel.h"

static List *generateRangePartitions(ParseState *pstate, CreateStmt *cstmt,
									 Relation parentrel,
									 GpPartitionElem *elem,
									 const char *queryString);
static List *generateListPartition(ParseState *pstate, CreateStmt *cstmt,
								   Relation parentrel,
								   GpPartitionElem *elem,
								   const char *queryString);
static List *generateDefaultPartition(ParseState *pstate, CreateStmt *cstmt,
									  Relation parentrel,
									  GpPartitionElem *elem,
									  const char *queryString);

static CreateStmt *makePartitionCreateStmt(Relation parentrel, char *partname,
										   PartitionBoundSpec *boundspec);

/*
 * Create a list of CreateStmts, to create partitions based on 'gpPartSpec'
 * specification.
 */
List *
generatePartitions(CreateStmt *cstmt, Oid parentrelid, GpPartitionSpec *gpPartSpec,
				   const char *queryString)
{
	Relation	parentrel;
	List	   *result = NIL;
	char	   *schemaname;
	ListCell   *lc;
	ParseState *pstate;

	pstate = make_parsestate(NULL);
	pstate->p_sourcetext = queryString;

	parentrel = table_open(parentrelid, NoLock);

	schemaname = get_namespace_name(parentrel->rd_rel->relnamespace);

	foreach(lc, gpPartSpec->partElem)
	{
		Node	   *n = lfirst(lc);

		if (IsA(n, GpPartitionElem))
		{
			GpPartitionElem *elem = (GpPartitionElem *) n;
			List	   *new_parts;

			if (elem->boundSpec)
			{
				if (IsA(elem->boundSpec, GpPartitionBoundSpec))
					new_parts = generateRangePartitions(pstate, cstmt, parentrel, elem, queryString);
				else
				{
					Assert(IsA(elem->boundSpec, GpPartitionValuesSpec));
					new_parts = generateListPartition(pstate, cstmt, parentrel, elem, queryString);
				}
			}
			else if (elem->isDefault)
			{
				new_parts = generateDefaultPartition(pstate, cstmt, parentrel, elem, queryString);
			}
			else
			{
				/* GPDB_12_MERGE_FIXME: which cases are we not handling yet? */
				elog(ERROR, "unexpected GpPartitionElem");
			}

			result = list_concat(result, new_parts);
		}
		else if (IsA(n, ColumnReferenceStorageDirective))
		{
			/* GPDB_12_MERGE_FIXME */
			elog(ERROR, "column storage directives not implemented yet");
		}
	}

	free_parsestate(pstate);

	table_close(parentrel, NoLock);

	return result;
}

/* Generate partitions for START (..) END (..) EVERY (..) */
List *
generateRangePartitions(ParseState *pstate,
						CreateStmt *cstmt, Relation parentrel,
						GpPartitionElem *elem,
						const char *queryString)
{
	GpPartitionBoundSpec *boundspec;
	List	   *startconsts = NIL;
	List	   *endconsts = NIL;
	List	   *everyconsts = NIL;
	List	   *result = NIL;
	PartitionKey partkey;

	boundspec = (GpPartitionBoundSpec *) elem->boundSpec;
	Assert(boundspec);

	partkey = RelationGetPartitionKey(parentrel);

	if (partkey->partnatts != 1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
				 errmsg("START/RANGE/EVERY is only supported for tables with a single partitioning key")));

	/* Parse the START/END/EVERY clauses */
	{
		List	   *startvalues = boundspec->partStart;
		ListCell   *lc;
		int			keyidx;

		if (list_length(startvalues) != partkey->partnatts)
			elog(ERROR, "invalid number of start values"); // GPDB_12_MERGE_FIXME: improve message

		keyidx = 0;
		foreach(lc, startvalues)
		{
			Const	   *startconst;

			startconst = transformPartitionBoundValue(pstate,
													  lfirst(lc),
													  "", /* GPDB_12_MERGE_FIXME: colname */
													  get_partition_col_typid(partkey, keyidx),
													  get_partition_col_typmod(partkey, keyidx),
													  get_partition_col_collation(partkey, keyidx));
			if (startconst->consttype != INT4OID)
				elog(ERROR, "fixme: only int4 columns implemented");
			if (startconst->constisnull)
				elog(ERROR, "NULL not allowed in START"); // GPDB_12_MERGE_FIXME: improve message
			startconsts = lappend(startconsts, startconst);
		}
	}
	{
		List	   *endvalues = boundspec->partEnd;
		ListCell   *lc;
		int			keyidx;

		if (list_length(endvalues) != partkey->partnatts)
			elog(ERROR, "invalid number of end values"); // GPDB_12_MERGE_FIXME: improve message

		keyidx = 0;
		foreach(lc, endvalues)
		{
			Const	   *endconst;

			endconst = transformPartitionBoundValue(pstate,
													lfirst(lc),
													"", /* GPDB_12_MERGE_FIXME: colname */
													get_partition_col_typid(partkey, keyidx),
													get_partition_col_typmod(partkey, keyidx),
													get_partition_col_collation(partkey, keyidx));
			if (endconst->consttype != INT4OID)
				elog(ERROR, "fixme: only int4 columns implemented");
			if (endconst->constisnull)
				elog(ERROR, "NULL not allowed in END"); // GPDB_12_MERGE_FIXME: improve message
			endconsts = lappend(endconsts, endconst);
		}
	}

	if (boundspec->partEvery)
	{
		List	   *everyvalues = boundspec->partEvery;
		ListCell   *lc;
		int			keyidx;

		if (list_length(everyvalues) != partkey->partnatts)
			elog(ERROR, "invalid number of every values"); // GPDB_12_MERGE_FIXME: improve message

		keyidx = 0;
		foreach(lc, everyvalues)
		{
			Const	   *everyconst;

			everyconst = transformPartitionBoundValue(pstate,
													  lfirst(lc),
													  "", /* GPDB_12_MERGE_FIXME: colname */
													  get_partition_col_typid(partkey, keyidx),
													  get_partition_col_typmod(partkey, keyidx),
													  get_partition_col_collation(partkey, keyidx));
			if (everyconst->consttype != INT4OID)
				elog(ERROR, "fixme: only int4 columns implemented");
			if (everyconst->constisnull)
				elog(ERROR, "NULL not allowed in EVERY"); // GPDB_12_MERGE_FIXME: improve message
			everyconsts = lappend(everyconsts, everyconst);
		}

		int start_int = DatumGetInt32(linitial_node(Const, startconsts)->constvalue);
		int end_int = DatumGetInt32(linitial_node(Const, endconsts)->constvalue);
		int every_int = DatumGetInt32(linitial_node(Const, everyconsts)->constvalue);

		for (int bound = start_int; bound < end_int; bound += every_int)
		{
			PartitionBoundSpec *boundspec;
			CreateStmt *childstmt;
			char	   *partname;

			boundspec = makeNode(PartitionBoundSpec);
			boundspec->strategy = PARTITION_STRATEGY_RANGE;
			boundspec->is_default = false;
			boundspec->lowerdatums = list_make1(makeConst(INT4OID,
														  -1,
														  InvalidOid,
														  sizeof(int32),
														  Int32GetDatum(bound),
														  false,
														  true));
			boundspec->upperdatums = list_make1(makeConst(INT4OID,
														  -1,
														  InvalidOid,
														  sizeof(int32),
														  Int32GetDatum(bound + every_int),
														  false,
														  true));
			boundspec->location = -1;

			/* GPDB_12_MERGE_FIXME: this way of choosing the partition rel name
			 * is different from what it used to be.
			 */
			partname = psprintf("%s_%d", RelationGetRelationName(parentrel), bound);

			childstmt = makePartitionCreateStmt(parentrel, partname, boundspec);

			result = lappend(result, childstmt);
		}
	}
	else
	{
		/* Without EVERY, create just one partition that covers the whole range */
		PartitionBoundSpec *boundspec;
		CreateStmt *childstmt;
		char	   *partname;

		int start_int = DatumGetInt32(linitial_node(Const, startconsts)->constvalue);
		int end_int = DatumGetInt32(linitial_node(Const, endconsts)->constvalue);

		boundspec = makeNode(PartitionBoundSpec);
		boundspec->strategy = PARTITION_STRATEGY_RANGE;
		boundspec->is_default = false;
		boundspec->lowerdatums = list_make1(makeConst(INT4OID,
													  -1,
													  InvalidOid,
													  sizeof(int32),
													  Int32GetDatum(start_int),
													  false,
													  true));
		boundspec->upperdatums = list_make1(makeConst(INT4OID,
													  -1,
													  InvalidOid,
													  sizeof(int32),
													  Int32GetDatum(end_int),
													  false,
													  true));
		boundspec->location = -1;

		/* GPDB_12_MERGE_FIXME: this way of choosing the partition rel name
		 * is different from what it used to be.
		 */
		partname = psprintf("%s_%d", RelationGetRelationName(parentrel), start_int);

		childstmt = makePartitionCreateStmt(parentrel, partname, boundspec);

		result = lappend(result, childstmt);
	}

	return result;
}

static List *
generateListPartition(ParseState *pstate, CreateStmt *cstmt,
					  Relation parentrel,
					  GpPartitionElem *elem,
					  const char *queryString)
{
	GpPartitionValuesSpec *gpvaluesspec;
	PartitionBoundSpec *boundspec;
	CreateStmt *childstmt;
	char	   *partname;
	PartitionKey partkey;
	ListCell   *lc;
	List	  *listdatums;

	gpvaluesspec = (GpPartitionValuesSpec *) elem->boundSpec;
	Assert(gpvaluesspec);

	partkey = RelationGetPartitionKey(parentrel);

	boundspec = makeNode(PartitionBoundSpec);
	boundspec->strategy = PARTITION_STRATEGY_LIST;
	boundspec->is_default = false;
	/* GPDB_12_MERGE_FIXME: The syntax is a list in a list, to support
	 * list-partitioned tables with multiple partitioning keys. But
	 * PostgreSQL doesn't support that. Not sure what to do about that.
	 * Add support for it to PostgreSQL? Simplify the grammar to not
	 * allow that?
	 */
	listdatums = NIL;
	foreach (lc, gpvaluesspec->partValues)
	{
		List	   *thisvalue = lfirst(lc);

		if (list_length(thisvalue) != 1)
			elog(ERROR, "VALUES specification with more than one column not allowed");

		listdatums = lappend(listdatums, linitial(thisvalue));
	}

	boundspec->listdatums = listdatums;
	boundspec->location = -1;

	/*
	 * GPDB_12_MERGE_FIXME: this way of choosing the partition rel name
	 * is different from what it used to be.
	 */
	if (!elem->partName)
		elog(ERROR, "list partition must have a name");
	partname = psprintf("%s_prt_%s", RelationGetRelationName(parentrel), elem->partName);

	boundspec = transformPartitionBound(pstate, parentrel, boundspec);

	childstmt = makePartitionCreateStmt(parentrel, partname, boundspec);

	return list_make1(childstmt);
}

static List *
generateDefaultPartition(ParseState *pstate, CreateStmt *cstmt,
						 Relation parentrel,
						 GpPartitionElem *elem,
						 const char *queryString)
{
	PartitionBoundSpec *boundspec;
	CreateStmt *childstmt;
	char	   *partname;

	boundspec = makeNode(PartitionBoundSpec);
	boundspec->is_default = true;
	boundspec->location = -1;

	/* GPDB_12_MERGE_FIXME: this way of choosing the partition rel name
	 * is different from what it used to be.
	 */
	partname = psprintf("%s_prt_default", RelationGetRelationName(parentrel));

	childstmt = makePartitionCreateStmt(parentrel, partname, boundspec);

	return list_make1(childstmt);
}

static CreateStmt *
makePartitionCreateStmt(Relation parentrel, char *partname, PartitionBoundSpec *boundspec)
{
	CreateStmt *childstmt;
	RangeVar   *parentrv;
	char	   *schemaname;

	schemaname = get_namespace_name(parentrel->rd_rel->relnamespace);
	parentrv = makeRangeVar(schemaname, RelationGetRelationName(parentrel), -1);

	childstmt = makeNode(CreateStmt);
	childstmt->relation = makeRangeVar(schemaname, partname, -1);
	childstmt->tableElts = NIL;
	childstmt->inhRelations = list_make1(parentrv);
	childstmt->partbound = boundspec;
	childstmt->partspec = NULL;
	childstmt->ofTypename = NULL;
	childstmt->constraints = NIL;
	childstmt->options = NIL; // FIXME: copy from parent stmt?
	childstmt->oncommit = ONCOMMIT_NOOP;  // FIXME: copy from parent stmt?
	childstmt->tablespacename = NULL;   // FIXME: copy from parent stmt?
	childstmt->accessMethod = get_am_name(parentrel->rd_rel->relam);
	childstmt->if_not_exists = false;
	childstmt->distributedBy = make_distributedby_for_rel(parentrel);
	childstmt->partitionBy = NULL;
	childstmt->relKind = 0;
	childstmt->ownerid = parentrel->rd_rel->relowner;

	return childstmt;
}
