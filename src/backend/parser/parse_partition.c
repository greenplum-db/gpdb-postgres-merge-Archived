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
#include "catalog/pg_collation.h"
#include "catalog/pg_type.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "nodes/makefuncs.h"
#include "nodes/nodeFuncs.h"
#include "nodes/parsenodes.h"
#include "optimizer/optimizer.h"
#include "parser/parse_expr.h"
#include "parser/parse_oper.h"
#include "parser/parse_utilcmd.h"
#include "utils/builtins.h"
#include "utils/datum.h"
#include "utils/lsyscache.h"
#include "utils/partcache.h"
#include "utils/rel.h"

static List *generateRangePartitions(ParseState *pstate,
									 Relation parentrel,
									 GpPartitionElem *elem,
									 PartitionSpec *subPart,
									 int *num_unnamed_parts_p);
static List *generateListPartition(ParseState *pstate,
								   Relation parentrel,
								   GpPartitionElem *elem,
								   PartitionSpec *subPart,
								   int *num_unnamed_parts_p);
static List *generateDefaultPartition(ParseState *pstate,
									  Relation parentrel,
									  GpPartitionElem *elem,
									  PartitionSpec *subPart);

static CreateStmt *makePartitionCreateStmt(Relation parentrel, char *partname,
										   PartitionBoundSpec *boundspec, PartitionSpec *subPart);

static char *ChoosePartitionName(Relation parentrel, const char *levelstr,
								 const char *partname, int partnum);

List *
generateSinglePartition(Relation parentrel, GpPartitionElem *elem, PartitionSpec *subPart,
						const char *queryString, int *num_unnamed_parts)
{
	List *new_parts;
	ParseState *pstate;

	pstate = make_parsestate(NULL);
	pstate->p_sourcetext = queryString;

	if (elem->isDefault)
		new_parts = generateDefaultPartition(pstate, parentrel, elem, subPart);
	else
	{
		PartitionKey key = RelationGetPartitionKey(parentrel);
		Assert(key != NULL);
		switch (key->strategy)
		{
			case PARTITION_STRATEGY_RANGE:
				new_parts = generateRangePartitions(pstate, parentrel, elem, subPart,
													num_unnamed_parts);
				break;

			case PARTITION_STRATEGY_LIST:
				new_parts = generateListPartition(pstate, parentrel, elem, subPart,
												  num_unnamed_parts);
				break;
			default:
				elog(ERROR, "Not supported partition strategy");
		}
	}

	return new_parts;
}

/*
 * Create a list of CreateStmts, to create partitions based on 'gpPartSpec'
 * specification.
 */
List *
generatePartitions(Oid parentrelid, GpPartitionSpec *gpPartSpec,
				   PartitionSpec *subPartSpec, const char *queryString)
{
	Relation	parentrel;
	List	   *result = NIL;
	ListCell   *lc;
	ParseState *pstate;
	int			num_unnamed_parts = 0;

	pstate = make_parsestate(NULL);
	pstate->p_sourcetext = queryString;

	parentrel = table_open(parentrelid, NoLock);

	foreach(lc, gpPartSpec->partElem)
	{
		Node	   *n = lfirst(lc);

		if (IsA(n, GpPartitionElem))
		{
			GpPartitionElem *elem = (GpPartitionElem *) n;
			List	   *new_parts;

			if (subPartSpec)
				subPartSpec->gpPartSpec = (GpPartitionSpec*) elem->subSpec;

			new_parts = generateSinglePartition(parentrel, elem, subPartSpec, queryString, &num_unnamed_parts);
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

/*
 * Functions for iterating through all the partition bounds based on
 * START/END/EVERY.
 */
typedef struct
{
	PartitionKeyData *partkey;
	Datum		endVal;

	ExprState   *plusexprstate;
	ParamListInfo plusexpr_params;
	EState	   *estate;

	Datum		currStart;
	Datum		currEnd;
	bool		called;
	bool		endReached;
} PartEveryIterator;

static PartEveryIterator *
initPartEveryIterator(ParseState *pstate, PartitionKeyData *partkey, const char *part_col_name,
					  Node *start, Node *end, Node *every)
{
	PartEveryIterator *iter;
	Datum		startVal;
	Datum		endVal;
	Datum		everyVal;
	Oid			plusop;
	Oid			part_col_typid;
	int32		part_col_typmod;
	Oid			part_col_collation;

	/* caller should've checked this already */
	Assert(partkey->partnatts == 1);

	part_col_typid = get_partition_col_typid(partkey, 0);
	part_col_typmod = get_partition_col_typmod(partkey, 0);
	part_col_collation = get_partition_col_collation(partkey, 0);

	/* Parse the START/END/EVERY clauses */
	if (start)
	{
		Const	   *startConst;

		startConst = transformPartitionBoundValue(pstate,
												  start,
												  part_col_name,
												  part_col_typid,
												  part_col_typmod,
												  part_col_collation);
		if (startConst->constisnull)
			elog(ERROR, "NULL not allowed in START"); // GPDB_12_MERGE_FIXME: improve message
		startVal = startConst->constvalue;
	}
	else
	{
		if (part_col_typid != INT4OID)
		{
			// GPDB_12_MERGE_FIXME: support other types, and do this properly
			elog(ERROR, "leaving out start value is only supported for integer column");
		}
		startVal = DatumGetInt32(INT_MAX);
	}

	if (end)
	{
		Const	   *endConst;

		endConst = transformPartitionBoundValue(pstate,
												end,
												part_col_name,
												part_col_typid,
												part_col_typmod,
												part_col_collation);
		if (endConst->constisnull)
			elog(ERROR, "NULL not allowed in END"); // GPDB_12_MERGE_FIXME: improve message
		endVal = endConst->constvalue;
	}
	else
	{
		if (part_col_typid != INT4OID)
		{
			// GPDB_12_MERGE_FIXME: support other types, and do this properly
			elog(ERROR, "leaving out end value is only supported for integer column");
		}
		endVal = DatumGetInt32(INT_MAX);
	}

	iter = palloc0(sizeof(PartEveryIterator));
	iter->partkey = partkey;
	iter->endVal = endVal;

	if (every)
	{
		Node	   *plusexpr;
		Param	   *param;

		/*
		 * NOTE: We don't use transformPartitionBoundValue() here. We don't want to cast
		 * the EVERY clause to that type; rather, we'll be passing it to the + operator.
		 * For example, if the partition column is a timestamp, the EVERY clause
		 * can be an interval, so don't try to cast it to timestamp.
		 */

		param = makeNode(Param);
		param->paramkind = PARAM_EXTERN;
		param->paramid = 1;
		param->paramtype = part_col_typid;
		param->paramtypmod = part_col_typmod;
		param->paramcollid = part_col_collation;
		param->location = -1;

		/* Look up + operator */
		plusexpr = (Node *) make_op(pstate,
									list_make2(makeString("pg_catalog"), makeString("+")),
									(Node *) param,
									(Node *) transformExpr(pstate, every, EXPR_KIND_PARTITION_BOUND),
									pstate->p_last_srf,
									-1);

		/*
		 * Check that the input expression's collation is compatible with one
		 * specified for the parent's partition key (partcollation).  Don't throw
		 * an error if it's the default collation which we'll replace with the
		 * parent's collation anyway.
		 */
		if (IsA(plusexpr, CollateExpr))
		{
			Oid			exprCollOid = exprCollation(plusexpr);

			if (OidIsValid(exprCollOid) &&
				exprCollOid != DEFAULT_COLLATION_OID &&
				exprCollOid != part_col_collation)
				ereport(ERROR,
						(errcode(ERRCODE_DATATYPE_MISMATCH),
						 errmsg("collation of partition bound value for column \"%s\" does not match partition key collation \"%s\"",
								part_col_name, get_collation_name(part_col_collation))));
		}

		plusexpr = coerce_to_target_type(pstate,
										 plusexpr, exprType(plusexpr),
										 part_col_typid,
										 part_col_typmod,
										 COERCION_ASSIGNMENT,
										 COERCE_IMPLICIT_CAST,
										 -1);
		if (plusexpr == NULL)
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
					 errmsg("specified value cannot be cast to type %s for column \"%s\"",
							format_type_be(part_col_typid), part_col_name)));

		iter->estate = CreateExecutorState();

		iter->plusexpr_params = makeParamList(1);
		iter->plusexpr_params->params[0].value = (Datum) 0;
		iter->plusexpr_params->params[0].isnull = true;
		iter->plusexpr_params->params[0].pflags = 0;
		iter->plusexpr_params->params[0].ptype = part_col_typid;

		iter->estate->es_param_list_info = iter->plusexpr_params;

		iter->plusexprstate = ExecInitExprWithParams((Expr *) plusexpr, iter->plusexpr_params);
	}
	else
	{
		everyVal = (Datum) 0;
		plusop = InvalidOid;
	}

	iter->currEnd = startVal;
	iter->currStart = (Datum) 0;
	iter->called = false;
	iter->endReached = false;

	return iter;
}

static void
freePartEveryIterator(PartEveryIterator *iter)
{
	if (iter->estate)
		FreeExecutorState(iter->estate);
}

/*
 * Return next partition bound in START/END/EVERY specification.
 */
static bool
nextPartBound(PartEveryIterator *iter)
{
	bool		firstcall;

	firstcall = !iter->called;
	iter->called = true;

	if (iter->plusexprstate)
	{
		/*
		 * Call (previous bound) + EVERY
		 */
		Datum		next;
		int32		cmpval;
		bool		isnull;

		/* If the previous partition reached END, we're done */
		if (iter->endReached)
			return false;

		iter->plusexpr_params->params[0].isnull = false;
		iter->plusexpr_params->params[0].value = iter->currEnd;

		next = ExecEvalExprSwitchContext(iter->plusexprstate,
										 GetPerTupleExprContext(iter->estate),
										 &isnull);
		if (isnull)
			elog(ERROR, "plus-operator returned NULL"); // GPDB_12_MERGE_FIXME: better message

		iter->currStart = iter->currEnd;

		/* Is the next bound greater than END? */
		cmpval = DatumGetInt32(FunctionCall2Coll(&iter->partkey->partsupfunc[0],
												 iter->partkey->partcollation[0],
												 next,
												 iter->endVal));
		if (cmpval >= 0)
		{
			iter->endReached = true;
			iter->currEnd = iter->endVal;
		}
		else
		{
			/*
			 * Sanity check that the next bound is > previous bound. This prevents us
			 * from getting into an infinite loop if the + operator is not behaving.
			 */
			cmpval = DatumGetInt32(FunctionCall2Coll(&iter->partkey->partsupfunc[0],
													 iter->partkey->partcollation[0],
													 iter->currEnd,
													 next));
			if (cmpval >= 0)
			{
				if (firstcall)
				{
					/*
					 * Second iteration: parameter hasn't increased the
					 * current end from the old end.
					 */
					ereport(ERROR,
							(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
							 errmsg("EVERY parameter too small")));
				}
				else
				{
					/*
					 * We got a smaller value but later than we
					 * thought so it must be an overflow.
					 */
					ereport(ERROR,
							(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
							 errmsg("END parameter not reached before type overflows")));
				}
			}

			iter->currEnd = next;
		}

		return true;
	}
	else
	{
		/* Without EVERY, create just one partition that covers the whole range */
		if (!firstcall)
			return false;

		iter->called = true;
		iter->currStart = iter->currEnd;
		iter->currEnd = iter->endVal;
		return true;
	}
}

/* Generate partitions for START (..) END (..) EVERY (..) */
static List *
generateRangePartitions(ParseState *pstate,
						Relation parentrel,
						GpPartitionElem *elem,
						PartitionSpec *subPart,
						int *num_unnamed_parts_p)
{
	GpPartitionBoundSpec *boundspec;
	List	   *result = NIL;
	PartitionKey partkey;
	char	   *partcolname;
	PartEveryIterator *boundIter;
	Node	   *start;
	Node	   *end = NULL;
	Node	   *every = NULL;

	if (elem->boundSpec == NULL)
		elog(ERROR, "missing boundary specification in partition%s of type RANGE",
			 elem->partName);

	if (!IsA(elem->boundSpec, GpPartitionBoundSpec))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
				 errmsg("invalid boundary specification for RANGE partition"),
				 parser_errposition(pstate, elem->location)));

	boundspec = (GpPartitionBoundSpec *) elem->boundSpec;
	partkey = RelationGetPartitionKey(parentrel);

	/*
	 * GPDB_12_MERGE_FIXME: Greenplum historically does not support multi column
	 * List partitions. Upstream Postgres allows it. Keep this restriction for
	 * now and most likely we will get the functionality for free from the merge
	 * and we should remove this restriction once we verifies that.
	 */
	if (partkey->partnatts != 1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
				 errmsg("too many columns for RANGE partition -- only one column is allowed")));

	/* not sure if the syntax allows this, but better safe than sorry */
	if (partkey->partattrs[0] == 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
				 errmsg("START/END/EVERY not supported when partition key is an expression")));
	partcolname = NameStr(TupleDescAttr(RelationGetDescr(parentrel), partkey->partattrs[0] - 1)->attname);

	if (list_length(boundspec->partStart) != partkey->partnatts)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
				 errmsg("invalid use of mixed named and unnamed RANGE boundary specifications"),
				 parser_errposition(pstate, boundspec->location)));
	start = linitial(boundspec->partStart);

	if (boundspec->partEnd)
	{
		if (list_length(boundspec->partEnd) != partkey->partnatts)
			elog(ERROR, "invalid number of end values"); // GPDB_12_MERGE_FIXME: improve message
		end = linitial(boundspec->partEnd);
	}

	if (boundspec->partEvery)
	{
		if (list_length(boundspec->partEvery) != partkey->partnatts)
			elog(ERROR, "invalid number of every values"); // GPDB_12_MERGE_FIXME: improve message
		every = linitial(boundspec->partEvery);
	}

	partkey = RelationGetPartitionKey(parentrel);

	boundIter = initPartEveryIterator(pstate, partkey, partcolname, start, end, every);

	while (nextPartBound(boundIter))
	{
		PartitionBoundSpec *boundspec;
		CreateStmt *childstmt;
		char	   *partname;

		boundspec = makeNode(PartitionBoundSpec);
		boundspec->strategy = PARTITION_STRATEGY_RANGE;
		boundspec->is_default = false;
		boundspec->lowerdatums = list_make1(makeConst(boundIter->partkey->parttypid[0],
													  boundIter->partkey->parttypmod[0],
													  boundIter->partkey->parttypcoll[0],
													  boundIter->partkey->parttyplen[0],
													  datumCopy(boundIter->currStart,
																boundIter->partkey->parttypbyval[0],
																boundIter->partkey->parttyplen[0]),
													  false,
													  boundIter->partkey->parttypbyval[0]));
		boundspec->upperdatums = list_make1(makeConst(boundIter->partkey->parttypid[0],
													  boundIter->partkey->parttypmod[0],
													  boundIter->partkey->parttypcoll[0],
													  boundIter->partkey->parttyplen[0],
													  datumCopy(boundIter->currEnd,
																boundIter->partkey->parttypbyval[0],
																boundIter->partkey->parttyplen[0]),
													  false,
													  boundIter->partkey->parttypbyval[0]));
		boundspec->location = -1;

		/* GPDB_12_MERGE_FIXME: pass correct level */
		partname = ChoosePartitionName(parentrel, "1", elem->partName, ++(*num_unnamed_parts_p));
		childstmt = makePartitionCreateStmt(parentrel, partname, boundspec, subPart);

		result = lappend(result, childstmt);
	}

	freePartEveryIterator(boundIter);

	return result;
}

static List *
generateListPartition(ParseState *pstate,
					  Relation parentrel,
					  GpPartitionElem *elem,
					  PartitionSpec *subPart,
					  int *num_unnamed_parts_p)
{
	GpPartitionValuesSpec *gpvaluesspec;
	PartitionBoundSpec *boundspec;
	CreateStmt *childstmt;
	char	   *partname;
	PartitionKey partkey;
	ListCell   *lc;
	List	  *listdatums;

	if (elem->boundSpec == NULL)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
					  errmsg("missing boundary specification in partition \"%s\" of type LIST",
							 elem->partName),
							 parser_errposition(pstate, elem->location)));

	if (!IsA(elem->boundSpec, GpPartitionValuesSpec))
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TABLE_DEFINITION),
					  errmsg("invalid boundary specification for LIST partition"),
					  parser_errposition(pstate, elem->location)));

	gpvaluesspec = (GpPartitionValuesSpec *) elem->boundSpec;

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

	/* GPDB_12_MERGE_FIXME: pass correct level */
	partname = ChoosePartitionName(parentrel, "1", elem->partName, ++(*num_unnamed_parts_p));
	boundspec = transformPartitionBound(pstate, parentrel, boundspec);

	childstmt = makePartitionCreateStmt(parentrel, partname, boundspec, subPart);

	return list_make1(childstmt);
}

static List *
generateDefaultPartition(ParseState *pstate,
						 Relation parentrel,
						 GpPartitionElem *elem,
						 PartitionSpec *subPart)
{
	PartitionBoundSpec *boundspec;
	CreateStmt *childstmt;
	char	   *partname;

	boundspec = makeNode(PartitionBoundSpec);
	boundspec->is_default = true;
	boundspec->location = -1;

	/* GPDB_12_MERGE_FIXME: pass correct level */
	partname = ChoosePartitionName(parentrel, "1", elem->partName, -1);
	childstmt = makePartitionCreateStmt(parentrel, partname, boundspec, subPart);

	return list_make1(childstmt);
}

static CreateStmt *
makePartitionCreateStmt(Relation parentrel, char *partname, PartitionBoundSpec *boundspec,
						PartitionSpec *subPart)
{
	CreateStmt *childstmt;
	RangeVar   *parentrv;
	char	   *schemaname;

	schemaname = get_namespace_name(parentrel->rd_rel->relnamespace);
	parentrv = makeRangeVar(schemaname, pstrdup(RelationGetRelationName(parentrel)), -1);

	childstmt = makeNode(CreateStmt);
	childstmt->relation = makeRangeVar(schemaname, partname, -1);
	childstmt->tableElts = NIL;
	childstmt->inhRelations = list_make1(parentrv);
	childstmt->partbound = boundspec;
	childstmt->partspec = subPart;
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

static char *
ChoosePartitionName(Relation parentrel, const char *levelstr,
					const char *partname, int partnum)
{
	char partsubstring[NAMEDATALEN];

	if (partname)
		snprintf(partsubstring, NAMEDATALEN, "prt_%s", partname);
	else if (partnum == -1)
		snprintf(partsubstring, NAMEDATALEN, "prt_default");
	else
		snprintf(partsubstring, NAMEDATALEN, "prt_%d", partnum);

	return ChooseRelationName(RelationGetRelationName(parentrel),
							  levelstr,
							  partsubstring,
							  RelationGetNamespace(parentrel),
							  false);
}
