/*-------------------------------------------------------------------------
 *
 * pg_exttable.h
 *	  definitions for system wide external relations
 *
 * Portions Copyright (c) 2007-2010, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/include/catalog/pg_exttable.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef PG_EXTTABLE_H
#define PG_EXTTABLE_H

#include "catalog/genbki.h"
#include "catalog/pg_exttable_d.h"
#include "nodes/pg_list.h"

/*
 * pg_exttable definition.
 */

/* ----------------
 *		pg_exttable definition.  cpp turns this into
 *		typedef struct FormData_pg_exttable
 * ----------------
 */
CATALOG(pg_exttable,6040,ExtTableRelationId)
{
	Oid		reloid;				/* refers to this relation's oid in pg_class  */
#ifdef CATALOG_VARLEN
	text	urilocation[1];		/* array of URI strings */
	text	execlocation[1];	/* array of ON locations */
	char	fmttype;			/* 't' (text) or 'c' (csv) */
	text	fmtopts;			/* the data format options */
	text	options[1];			/* the array of external table options */
	text	command;			/* the command string to EXECUTE */
	int32	rejectlimit;		/* error count reject limit per segment */
	char	rejectlimittype;	/* 'r' (rows) or 'p' (percent) */
	bool	logerrors;			/* 't' to log errors into file */
	int32	encoding;			/* character encoding of this external table */
	bool	writable;			/* 't' if writable, 'f' if readable */
#endif
} FormData_pg_exttable;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(reloid REFERENCES pg_class(oid));

/* ----------------
 *		Form_pg_exttable corresponds to a pointer to a tuple with
 *		the format of pg_exttable relation.
 * ----------------
 */
typedef FormData_pg_exttable *Form_pg_exttable;

/*
 * Descriptor of a single AO relation.
 * For now very similar to the catalog row itself but may change in time.
 */
typedef struct ExtTableEntry
{
	List*	urilocations;
	List*	execlocations;
	char	fmtcode;
	char*	fmtopts;
	List*	options;
	char*	command;
	int		rejectlimit;
	char	rejectlimittype;
	bool	logerrors;
    int		encoding;
    bool	iswritable;
    bool	isweb;		/* extra state, not cataloged */
} ExtTableEntry;

/* No initial contents. */

extern void InsertExtTableEntry(Oid 	tbloid,
					bool 	iswritable,
					bool	issreh,
					char	formattype,
					char	rejectlimittype,
					char*	commandString,
					int		rejectlimit,
					bool	logerrors,
					int		encoding,
					Datum	formatOptStr,
					Datum	optionsStr,
					Datum	locationExec,
					Datum	locationUris);

extern ExtTableEntry *GetExtTableEntry(Oid relid);
extern ExtTableEntry *GetExtTableEntryIfExists(Oid relid);

extern void RemoveExtTableEntry(Oid relid);

#define fmttype_is_custom(c) (c == 'b')
#define fmttype_is_text(c)   (c == 't')
#define fmttype_is_csv(c)    (c == 'c')

#endif /* PG_EXTTABLE_H */
