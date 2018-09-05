/*-------------------------------------------------------------------------
 *
 * collationcmds.h
 *	  prototypes for collationcmds.c.
 *
 *
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/collationcmds.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef COLLATIONCMDS_H
#define COLLATIONCMDS_H

#include "nodes/parsenodes.h"

<<<<<<< HEAD
extern void DefineCollation(List *names, List *parameters, bool if_not_exists);
extern void RenameCollation(List *name, const char *newname);
extern void AlterCollationOwner(List *name, Oid newOwnerId);
extern void AlterCollationOwner_oid(Oid collationOid, Oid newOwnerId);
extern void AlterCollationNamespace(List *name, const char *newschema);
extern Oid	AlterCollationNamespace_oid(Oid collOid, Oid newNspOid);
extern Datum pg_import_system_collations(PG_FUNCTION_ARGS);
=======
extern Oid	DefineCollation(List *names, List *parameters);
extern void IsThereCollationInNamespace(const char *collname, Oid nspOid);
>>>>>>> e472b921406407794bab911c64655b8b82375196

#endif   /* COLLATIONCMDS_H */
