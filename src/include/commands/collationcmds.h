/*-------------------------------------------------------------------------
 *
 * collationcmds.h
 *	  prototypes for collationcmds.c.
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/collationcmds.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef COLLATIONCMDS_H
#define COLLATIONCMDS_H

#include "catalog/objectaddress.h"
#include "nodes/parsenodes.h"

<<<<<<< HEAD
extern ObjectAddress DefineCollation(List *names, List *parameters, bool if_not_exists);
=======
extern ObjectAddress DefineCollation(ParseState *pstate, List *names, List *parameters, bool if_not_exists);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern void IsThereCollationInNamespace(const char *collname, Oid nspOid);
extern ObjectAddress AlterCollation(AlterCollationStmt *stmt);

#endif							/* COLLATIONCMDS_H */
