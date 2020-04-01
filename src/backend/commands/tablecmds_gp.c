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

#include "commands/tablecmds.h"
#include "nodes/parsenodes.h"
#include "partitioning/partdesc.h"
#include "utils/rel.h"


static Oid
find_target_partition(Relation parent, GpAlterPartitionId *partid)
{
	Oid			target_relid = InvalidOid;

	switch (partid->idtype)
	{
		case AT_AP_IDDefault:
			/* Find default partition */
			target_relid =
				get_default_oid_from_partdesc(RelationGetPartitionDesc(parent));
			if (!OidIsValid(target_relid))
				ereport(ERROR,
						(errcode(ERRCODE_UNDEFINED_OBJECT),
						 errmsg("DEFAULT partition of relation \"%s\" does not exist",
								RelationGetRelationName(parent))));
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

	target_relid = find_target_partition(parent, castNode(GpAlterPartitionId, cmd->partid));

	obj.classId = RelationRelationId;
	obj.objectId = target_relid;
	obj.objectSubId = 0;

	performDeletion(&obj, cmd->behavior, 0);
}
