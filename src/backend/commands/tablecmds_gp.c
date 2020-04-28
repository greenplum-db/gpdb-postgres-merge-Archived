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
#include "catalog/pg_attribute.h"
#include "commands/defrem.h"
#include "commands/tablecmds.h"
#include "executor/execPartition.h"
#include "nodes/parsenodes.h"
#include "nodes/primnodes.h"
#include "nodes/makefuncs.h"
#include "parser/parse_utilcmd.h"
#include "partitioning/partdesc.h"
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
		isnull[i] = false;
		i++;
	}

	for (; i < partkey->partnatts; i++)
	{
		values[i] = 0;
		isnull[i] = true;
	}
}

Oid
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

			snprintf(partsubstring, NAMEDATALEN, "prt_%s",
					 strVal(partid->partiddef));
			/* GPDB_12_MERGE_FIXME: pass correct level */
			partname = makeObjectName(RelationGetRelationName(parent),
									  "1", partsubstring);

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

				if (!partdesc->is_leaf[partidx])
					elog(ERROR, "not implemented yet");

				if (partdesc->oids[partidx] ==
					get_default_oid_from_partdesc(RelationGetPartitionDesc(parent)))
				{
					ereport(ERROR,
							(errcode(ERRCODE_WRONG_OBJECT_TYPE),
							 errmsg("FOR expression matches DEFAULT partition for specified value of %s",
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
 * ALTER TABLE DROP PARTITION
 */
void
ATExecPartDrop(Relation parent, GpDropPartitionCmd *cmd)
{
	Oid			target_relid;
	ObjectAddress obj;

	target_relid = GpFindTargetPartition(parent,
										 castNode(GpAlterPartitionId, cmd->partid),
										 cmd->missing_ok);

	if (!OidIsValid(target_relid))
		return;

	obj.classId = RelationRelationId;
	obj.objectId = target_relid;
	obj.objectSubId = 0;

	performDeletion(&obj, cmd->behavior, 0);
}
