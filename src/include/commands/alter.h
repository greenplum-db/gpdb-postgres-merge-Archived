/*-------------------------------------------------------------------------
 *
 * alter.h
 *	  prototypes for commands/alter.c
 *
 *
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/alter.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef ALTER_H
#define ALTER_H

#include "catalog/dependency.h"
<<<<<<< HEAD
#include "utils/acl.h"
#include "utils/relcache.h"

extern void ExecRenameStmt(RenameStmt *stmt);
extern void ExecAlterObjectSchemaStmt(AlterObjectSchemaStmt *stmt);
extern Oid  AlterObjectNamespace_oid(Oid classId, Oid objid, Oid nspOid,
                         ObjectAddresses *objsMoved);
extern Oid AlterObjectNamespace(Relation rel, int oidCacheId, int nameCacheId,
					 Oid objid, Oid nspOid,
					 int Anum_name, int Anum_namespace, int Anum_owner,
					 AclObjectKind acl_kind);
extern void ExecAlterOwnerStmt(AlterOwnerStmt *stmt);
=======
#include "nodes/parsenodes.h"
#include "utils/relcache.h"

extern Oid	ExecRenameStmt(RenameStmt *stmt);

extern Oid	ExecAlterObjectSchemaStmt(AlterObjectSchemaStmt *stmt);
extern Oid AlterObjectNamespace_oid(Oid classId, Oid objid, Oid nspOid,
						 ObjectAddresses *objsMoved);

extern Oid	ExecAlterOwnerStmt(AlterOwnerStmt *stmt);
extern void AlterObjectOwner_internal(Relation catalog, Oid objectId,
						  Oid new_ownerId);
>>>>>>> e472b921406407794bab911c64655b8b82375196

#endif   /* ALTER_H */
