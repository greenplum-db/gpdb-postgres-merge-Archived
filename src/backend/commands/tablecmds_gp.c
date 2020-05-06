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
#include "catalog/pg_collation.h"
#include "catalog/pg_inherits.h"
#include "catalog/pg_partitioned_table.h"
#include "catalog/pg_opclass.h"
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
#include "utils/syscache.h"

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

/*
 * Generate partition key specification of a partitioned table
 */
static PartitionSpec *
generatePartitionSpec(Relation rel)
{
	HeapTuple   tuple;
	Form_pg_partitioned_table form;
	AttrNumber *attrs;
	oidvector  *opclass;
	oidvector  *collation;
	Datum		datum;
	bool		isnull;
	Oid 		relid = RelationGetRelid(rel);
	PartitionSpec* subpart = makeNode(PartitionSpec);

	/*
	 * We cannot get the opclass oids of the partition keys directly from
	 * rel->rd_partkey because it only stores opcfamily oids and opcintype oids,
	 * and there are no syscache entries exists to lookup pg_opclass with those
	 * two values. Therefore we need to lookup pg_partitioned_table to fetch
	 * the opclass names. Since we have just opened the relation and built the
	 * partition descriptor, changes are high we hit the cache.
	 *
	 * We choose to as well use Form_pg_partitioned_table to fetch the partition
	 * attributes, their collations and expressions, even though we do have
	 * access to the same from rel->rd_partkey.
	 */
	tuple = SearchSysCache1(PARTRELID, ObjectIdGetDatum(relid));

	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "missing partition key information for oid %d", relid);
	/* Fixed-length attributes */
	form = (Form_pg_partitioned_table) GETSTRUCT(tuple);

	switch (form->partstrat)
	{
		case PARTITION_STRATEGY_RANGE:
			subpart->strategy = psprintf("range");
			break;
		case PARTITION_STRATEGY_LIST:
			subpart->strategy = psprintf("list");
			break;
		case PARTITION_STRATEGY_HASH:
			subpart->strategy = psprintf("hash");
			break;
	}

	subpart->location = -1;

	/*
	 * We can rely on the first variable-length attribute being mapped to the
	 * relevant field of the catalog's C struct, because all previous
	 * attributes are non-nullable and fixed-length.
	 */
	attrs = form->partattrs.values;

	/* But use the hard way to retrieve further variable-length attributes */
	/* Operator class */
	datum = SysCacheGetAttr(PARTRELID, tuple,
							Anum_pg_partitioned_table_partclass, &isnull);
	Assert(!isnull);
	opclass = (oidvector *) DatumGetPointer(datum);

	/* Collation */
	datum = SysCacheGetAttr(PARTRELID, tuple,
							Anum_pg_partitioned_table_partcollation, &isnull);
	Assert(!isnull);
	collation = (oidvector *) DatumGetPointer(datum);

	/* Expression */
	SysCacheGetAttr(PARTRELID, tuple,
							Anum_pg_partitioned_table_partexprs, &isnull);
	if (!isnull)
	{
		elog(ERROR, "Subpartition key contains expressions, GPDB add partition "
					"syntax does not support");
	}

	subpart->partParams = NIL;
	for (int i = 0; i < form->partnatts; i++)
	{
		PartitionElem *elem = makeNode(PartitionElem);
		AttrNumber  attno = attrs[i];
		Form_pg_opclass opclassform;
		Form_pg_collation collationform;
		char *opclassname;
		char *collationname = NULL;
		HeapTuple tmptuple;

		Assert(attno > 0);
		Assert(attno <= RelationGetDescr(rel)->natts);
		Form_pg_attribute attr = TupleDescAttr(RelationGetDescr(rel), attno - 1);
		Assert(attr != NULL);
		Assert(NameStr(attr->attname) != NULL);
		elem->name = pstrdup(NameStr(attr->attname));

		/* Collect opfamily information */
		tmptuple = SearchSysCache1(CLAOID,
								   ObjectIdGetDatum(opclass->values[i]));
		if (!HeapTupleIsValid(tmptuple))
			elog(ERROR, "cache lookup failed for opclass %u", opclass->values[i]);
		opclassform = (Form_pg_opclass) GETSTRUCT(tmptuple);
		opclassname = psprintf("%s", NameStr(opclassform->opcname));
		elem->opclass = list_make1(makeString(opclassname));
		ReleaseSysCache(tmptuple);

		/* Collect collation information */
		if (collation->values[i])
		{
			tmptuple = SearchSysCache1(COLLOID, ObjectIdGetDatum(collation->values[i]));
			if (!HeapTupleIsValid(tmptuple))
				elog(ERROR, "collation with OID %u does not exist", collation->values[i]);
			collationform = (Form_pg_collation) GETSTRUCT(tmptuple);
			collationname = psprintf("%s", NameStr(collationform->collname));
			elem->collation = list_make1(makeString(collationname));
			ReleaseSysCache(tmptuple);
		}

		subpart->partParams = lappend(subpart->partParams, elem);
	}

	ReleaseSysCache(tuple);

	return subpart;
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
			GpAlterPartitionCmd		*add_cmd = castNode(GpAlterPartitionCmd, cmd->def);
			GpPartitionDefinition	*gpPartDef = makeNode(GpPartitionDefinition);
			GpPartDefElem			*elem = castNode(GpPartDefElem, add_cmd->arg);
			PartitionSpec			*subpart = NULL;
			Relation 				temprel = rel;
			PartitionSpec 			*tempsubpart = NULL;
			ListCell 				*l;

			gpPartDef->partDefElems = list_make1(elem);

			/*
			 * Populate PARTITION BY spec for each level of the parents
			 * in the partitioning hierarchy. The PartitionSpec or a chain of
			 * PartitionSpecs if subpartitioning exists, are generated based on
			 * the first existing partition of each partition depth.
			 */
			do
			{
				PartitionDesc	partdesc;
				PartitionSpec *temptempsubpart = NULL;
				Oid firstchildoid;

				Assert(temprel->rd_rel->relkind == RELKIND_PARTITIONED_TABLE);
				partdesc = RelationGetPartitionDesc(temprel);

				if (partdesc->nparts == 0)
					elog(ERROR, "GPDB add partition syntax needs at least one sibling to exist");

				if (partdesc->is_leaf[0])
				{
					if (temprel != rel)
						table_close(temprel, AccessShareLock);
					break;
				}

				firstchildoid = partdesc->oids[0];
				if (temprel != rel)
					table_close(temprel, AccessShareLock);

				temprel = table_open(firstchildoid, AccessShareLock);

				temptempsubpart = generatePartitionSpec(temprel);
				if (tempsubpart == NULL)
					subpart = tempsubpart = temptempsubpart;
				else
				{
					tempsubpart->subPartSpec = temptempsubpart;
					tempsubpart = tempsubpart->subPartSpec;
				}
			} while (1);

			List *cstmts = generatePartitions(RelationGetRelid(rel),
											  gpPartDef, subpart, NULL,
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
