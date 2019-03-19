/*-------------------------------------------------------------------------
 *
 * hashdesc.c
 *	  rmgr descriptor routines for access/hash/hash.c
 *
 * Portions Copyright (c) 1996-2015, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/rmgrdesc/hashdesc.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/hash.h"

void
<<<<<<< HEAD
hash_desc(StringInfo buf __attribute__((unused)), XLogRecord *record __attribute__((unused)))
=======
hash_desc(StringInfo buf, XLogReaderState *record)
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8
{
}

const char *
hash_identify(uint8 info)
{
	return NULL;
}
