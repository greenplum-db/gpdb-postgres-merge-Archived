/*-------------------------------------------------------------------------
 *
 * dbcommands.h
 *		Database management commands (create/drop database).
 *
 *
 * Portions Copyright (c) 1996-2015, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/dbcommands.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef DBCOMMANDS_H
#define DBCOMMANDS_H

#include "access/xlogreader.h"
#include "catalog/objectaddress.h"
#include "lib/stringinfo.h"
#include "nodes/parsenodes.h"

<<<<<<< HEAD
/* XLOG stuff */
#define XLOG_DBASE_CREATE		0x00
#define XLOG_DBASE_DROP			0x10


typedef struct xl_dbase_create_rec_old
{
	/* Records copying of a single subdirectory incl. contents */
	Oid			db_id;
	char		src_path[1];	/* VARIABLE LENGTH STRING */
	/* dst_path follows src_path */
}	xl_dbase_create_rec_old;

typedef struct xl_dbase_drop_rec_old
{
	/* Records dropping of a single subdirectory incl. contents */
	Oid			db_id;
	char		dir_path[1];	/* VARIABLE LENGTH STRING */
}	xl_dbase_drop_rec_old;

typedef struct xl_dbase_create_rec
{
	/* Records copying of a single subdirectory incl. contents */
	Oid			db_id;
	Oid			tablespace_id;
	Oid			src_db_id;
	Oid			src_tablespace_id;
} xl_dbase_create_rec;

typedef struct xl_dbase_drop_rec
{
	/* Records dropping of a single subdirectory incl. contents */
	Oid			db_id;
	Oid			tablespace_id;
} xl_dbase_drop_rec;

=======
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
extern Oid	createdb(const CreatedbStmt *stmt);
extern void dropdb(const char *dbname, bool missing_ok);
extern ObjectAddress RenameDatabase(const char *oldname, const char *newname);
extern Oid	AlterDatabase(AlterDatabaseStmt *stmt, bool isTopLevel);
extern Oid	AlterDatabaseSet(AlterDatabaseSetStmt *stmt);
extern ObjectAddress AlterDatabaseOwner(const char *dbname, Oid newOwnerId);

extern Oid	get_database_oid(const char *dbname, bool missingok);
extern char *get_database_name(Oid dbid);

<<<<<<< HEAD
extern void dbase_redo(XLogRecPtr beginLoc  __attribute__((unused)), XLogRecPtr lsn  __attribute__((unused)), XLogRecord *rptr);
extern void dbase_desc(StringInfo buf, XLogRecord *record);

=======
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
extern void check_encoding_locale_matches(int encoding, const char *collate, const char *ctype);
extern void DropDatabaseDirectory(Oid db_id, Oid tblspcoid);

#endif   /* DBCOMMANDS_H */
