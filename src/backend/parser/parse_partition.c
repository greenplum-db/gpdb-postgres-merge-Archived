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
#include "nodes/makefuncs.h"
#include "nodes/parsenodes.h"
#include "parser/parse_utilcmd.h"
#include "utils/lsyscache.h"
#include "utils/partcache.h"
#include "utils/rel.h"

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
	RangeVar   *parentrv;
	ListCell   *lc;
	PartitionKey partkey;
	ParseState *pstate;

	pstate = make_parsestate(NULL);
	pstate->p_sourcetext = queryString;

	parentrel = table_open(parentrelid, NoLock);
	partkey = RelationGetPartitionKey(parentrel);

	schemaname = get_namespace_name(parentrel->rd_rel->relnamespace);

	parentrv = makeRangeVar(schemaname, RelationGetRelationName(parentrel), -1);

	foreach(lc, gpPartSpec->partElem)
	{
		GpPartitionElem *elem = lfirst_node(GpPartitionElem, lc);
		GpPartitionBoundSpec *boundspec;
		List	   *startconsts = NIL;
		List	   *endconsts = NIL;
		List	   *everyconsts = NIL;

		if (!IsA(elem->boundSpec, GpPartitionBoundSpec))
			elog(ERROR, "only range START/END/EVERY implemented");
		boundspec = (GpPartitionBoundSpec *) elem->boundSpec;
		Assert(boundspec);

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
		}
		else
		{
			everyconsts = list_make1(makeConst(INT4OID, -1, InvalidOid, sizeof(int32),
											   Int32GetDatum(1), false, true));
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
			childstmt->accessMethod = cstmt->accessMethod;
			childstmt->if_not_exists = false;
			childstmt->distributedBy = NULL; // FIXME: copy from parent stmt?
			childstmt->partitionBy = NULL;
			childstmt->relKind = 0;
			childstmt->ownerid = cstmt->ownerid;

			result = lappend(result, childstmt);
		}
	}

	free_parsestate(pstate);

	table_close(parentrel, NoLock);

	return result;
}
