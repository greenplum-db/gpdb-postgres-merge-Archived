/*-------------------------------------------------------------------------
 *
 * keywords.c
 *	  lexical token lookup for key words in PostgreSQL
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/parser/keywords.c,v 1.215 2010/01/02 16:57:49 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "parser/gramparse.h"

#define PG_KEYWORD(a,b,c) {a,b,c},


const ScanKeyword ScanKeywords[] = {
#include "parser/kwlist.h"
};

const int	NumScanKeywords = lengthof(ScanKeywords);
