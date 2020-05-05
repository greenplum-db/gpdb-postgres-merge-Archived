/*-------------------------------------------------------------------------
 *
 * tablecmds_gp.c
 *	  Greenplum extensions for ALTER TABLE.
 *
 * Portions Copyright (c) 2005-2010, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/commands/tablecmds_gp.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/table.h"
#include "catalog/partition.h"
#include "catalog/pg_attribute.h"
#include "cdb/cdbvars.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "executor/execPartition.h"
#include "nodes/parsenodes.h"
#include "nodes/primnodes.h"
#include "nodes/makefuncs.h"
#include "parser/parse_utilcmd.h"
#include "partitioning/partdesc.h"
#include "tcop/utility.h"
#include "utils/partcache.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/ruleutils.h"

/*
 * Build a datum according to tables partition key based on parse expr. Would
 * have been nice if FormPartitionKeyDatum() was generic and could have been
 * used instead.
 */
static void
FormPartitionKeyDatumFromExpr(Relation rel, Node *expr, Datum *values, bool *isnull)
{
	PartitionKey partkey;
	int 		num_expr;
	ListCell   *lc;
	int			i;

	Assert(rel);
	partkey = RelationGetPartitionKey(rel);
	Assert(partkey != NULL);

	Assert(IsA(expr, List));
	num_expr = list_length((List *) expr);

	if (num_expr > RelationGetDescr(rel)->natts)
		ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("too many columns in boundary specification (%d > %d)",
						num_expr, RelationGetDescr(rel)->natts)));

	if (num_expr > partkey->partnatts)
		ereport(ERROR,
				(errcode(ERRCODE_DATATYPE_MISMATCH),
				 errmsg("too many columns in boundary specification (%d > %d)",
						num_expr, partkey->partnatts)));

	i = 0;
	foreach(lc, (List *) expr)
	{
		Const      *result;
		char       *colname;
		Oid			coltype;
		int32		coltypmod;
		Oid			partcollation;
		Node	   *n1 = (Node *) lfirst(lc);

		/* Get column's name in case we need to output an error */
		if (partkey->partattrs[i] != 0)
			colname = get_attname(RelationGetRelid(rel),
								  partkey->partattrs[i], false);
		else
			colname = deparse_expression((Node *) linitial(partkey->partexprs),
										 deparse_context_for(RelationGetRelationName(rel),
															 RelationGetRelid(rel)),
										 false, false);

		coltype = get_partition_col_typid(partkey, i);
		coltypmod = get_partition_col_typmod(partkey, i);
		partcollation = get_partition_col_collation(partkey, i);
		result = transformPartitionBoundValue(make_parsestate(NULL), n1,
											  colname, coltype, coltypmod,
											  partcollation);

		values[i] = result->constvalue;
		isnull[i] = result->constisnull;
		i++;
	}

	for (; i < partkey->partnatts; i++)
	{
		values[i] = 0;
		isnull[i] = true;
	}
}

static Oid
GpFindTargetPartition(Relation parent, GpAlterPartitionId *partid,
					  bool missing_ok)
{
	Oid			target_relid = InvalidOid;

	switch (partid->idtype)
	{
		case AT_AP_IDDefault:
			/* Find default partition */
			target_relid =
				get_default_oid_from_partdesc(RelationGetPartitionDesc(parent));
			if (!OidIsValid(target_relid) && !missing_ok)
				ereport(ERROR,
						(errcode(ERRCODE_UNDEFINED_OBJECT),
						 errmsg("DEFAULT partition of relation \"%s\" does not exist",
								RelationGetRelationName(parent))));
			break;
		case AT_AP_IDName:
		{
			/* Find partition by name */
			RangeVar	*partrv;
			char		*schemaname;
			char		*partname;
			Relation	partRel;
			char partsubstring[NAMEDATALEN];
			List	   *ancestors = get_partition_ancestors(RelationGetRelid(parent));
			int			level = list_length(ancestors) + 1;
			char levelStr[NAMEDATALEN];

			snprintf(levelStr, NAMEDATALEN, "%d", level);
			snprintf(partsubstring, NAMEDATALEN, "prt_%s",
					 strVal(partid->partiddef));

			partname = makeObjectName(RelationGetRelationName(parent),
									  levelStr, partsubstring);

			schemaname   = get_namespace_name(parent->rd_rel->relnamespace);
			partrv       = makeRangeVar(schemaname, partname, -1);
			partRel      = table_openrv_extended(partrv, AccessShareLock, missing_ok);
			if (partRel)
			{
				target_relid = RelationGetRelid(partRel);
				table_close(partRel, AccessShareLock);
			}
			break;
		}

		case AT_AP_IDValue:
			{
				Datum		values[PARTITION_MAX_KEYS];
				bool		isnull[PARTITION_MAX_KEYS];
				PartitionDesc partdesc = RelationGetPartitionDesc(parent);
				int partidx;

				FormPartitionKeyDatumFromExpr(parent, partid->partiddef, values, isnull);
				partidx = get_partition_for_tuple(RelationGetPartitionKey(parent),
												  partdesc, values, isnull);

				if (partidx < 0)
				{
					if (missing_ok)
						break;
					ereport(ERROR,
							(errcode(ERRCODE_UNDEFINED_OBJECT),
							 errmsg("partition for specified value of %s does not exist",
									RelationGetRelationName(parent))));
				}

				if (partdesc->oids[partidx] ==
					get_default_oid_from_partdesc(RelationGetPartitionDesc(parent)))
				{
					ereport(ERROR,
							(errcode(ERRCODE_WRONG_OBJECT_TYPE),
							 errmsg("FOR expression matches DEFAULT partition for specified value of relation \"%s\"",
									RelationGetRelationName(parent)),
							 errhint("FOR expression may only specify a non-default partition in this context.")));
				}

				target_relid = partdesc->oids[partidx];
				break;
			}

		case AT_AP_IDNone:
			elog(ERROR, "not expected value");
			break;
	}

	return target_relid;
}

void
ATExecGPPartCmds(Relation origrel, AlterTableCmd *cmd)
{
	Relation rel = origrel;
	PlannedStmt *pstmt;
	List *stmts = NIL;
	ListCell *l;

	Assert(Gp_role != GP_ROLE_EXECUTE);
	if (Gp_role != GP_ROLE_DISPATCH)
		return;

	while (cmd->subtype == AT_PartAlter)
	{
		GpAlterPartitionCmd *pc;
		GpAlterPartitionId  *pid;
		Oid                 partrelid;

		pc  = castNode(GpAlterPartitionCmd, cmd->def);
		pid = (GpAlterPartitionId *) pc->partid;
		cmd = (AlterTableCmd *) pc->arg;

		if (rel->rd_rel->relkind != RELKIND_PARTITIONED_TABLE)
		{
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_OBJECT_DEFINITION),
					 errmsg("table \"%s\" is not partitioned",
							RelationGetRelationName(rel))));
		}

		partrelid = GpFindTargetPartition(rel, pid, false);
		Assert(OidIsValid(partrelid));

		if (rel != origrel)
			table_close(rel, AccessShareLock);
		rel = table_open(partrelid, AccessShareLock);
	}

	if (rel->rd_rel->relkind != RELKIND_PARTITIONED_TABLE)
	{
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_OBJECT_DEFINITION),
				 errmsg("table \"%s\" is not partitioned",
						RelationGetRelationName(rel))));
	}

	switch (cmd->subtype)
	{
		case AT_PartTruncate:
		{
			GpAlterPartitionCmd *pc = castNode(GpAlterPartitionCmd, cmd->def);
			GpAlterPartitionId *pid = (GpAlterPartitionId *) pc->partid;
			TruncateStmt *truncstmt = (TruncateStmt *) pc->arg;
			Oid partrelid;
			RangeVar *rv;
			Relation partrel;

			partrelid = GpFindTargetPartition(rel, pid, false);
			Assert(OidIsValid(partrelid));
			partrel = table_open(partrelid, AccessShareLock);
			rv = makeRangeVar(get_namespace_name(RelationGetNamespace(partrel)),
							  pstrdup(RelationGetRelationName(partrel)),
							  pc->location);
			truncstmt->relations = list_make1(rv);
			table_close(partrel, AccessShareLock);
			stmts = lappend(stmts, (Node *) truncstmt);
		}
		break;

		case AT_PartAdd:			/* Add */
		{
			ListCell *l;

			GpAlterPartitionCmd *add_cmd = castNode(GpAlterPartitionCmd, cmd->def);
			GpPartitionElem *pelem = castNode(GpPartitionElem, add_cmd->arg);
			GpPartitionSpec *gpPartSpec = makeNode(GpPartitionSpec);
			gpPartSpec->partElem = list_make1(pelem);
			List *cstmts = generatePartitions(RelationGetRelid(rel),
											  gpPartSpec, NULL, NULL,
											  NIL, NULL);
			foreach(l, cstmts)
			{
				Node *stmt = (Node *) lfirst(l);
				stmts = lappend(stmts, (Node *) stmt);
			}
		}
		break;

		case AT_PartDrop:			/* Drop */
		{
			GpDropPartitionCmd *pc = castNode(GpDropPartitionCmd, cmd->def);
			GpAlterPartitionId *pid = (GpAlterPartitionId *) pc->partid;
			DropStmt *dropstmt = makeNode(DropStmt);
			Oid partrelid;
			Relation partrel;

			partrelid = GpFindTargetPartition(rel, pid, pc->missing_ok);
			if (!OidIsValid(partrelid))
				break;
			partrel = table_open(partrelid, AccessShareLock);
			dropstmt->objects = list_make1(
				list_make2(makeString(
							   get_namespace_name(RelationGetNamespace(partrel))),
						   makeString(pstrdup(RelationGetRelationName(partrel)))));
			dropstmt->removeType = OBJECT_TABLE;
			dropstmt->behavior = pc->behavior;
			dropstmt->missing_ok = pc->missing_ok;
			table_close(partrel, AccessShareLock);
			stmts = lappend(stmts, (Node *)dropstmt);
		}
		break;

		default:
			elog(ERROR, "Not implemented");
			break;
	}

	if (rel != origrel)
		table_close(rel, AccessShareLock);

	foreach (l, stmts)
	{
		/* No planning needed, just make a wrapper PlannedStmt */
		Node *stmt = (Node *) lfirst(l);
		pstmt = makeNode(PlannedStmt);
		pstmt->commandType = CMD_UTILITY;
		pstmt->canSetTag = false;
		pstmt->utilityStmt = stmt;
		pstmt->stmt_location = -1;
		pstmt->stmt_len = 0;
		ProcessUtility(pstmt,
					   synthetic_sql,
					   PROCESS_UTILITY_SUBCOMMAND,
					   NULL,
					   NULL,
					   None_Receiver,
					   NULL);
	}
}
