/*-------------------------------------------------------------------------
 *
 * keywords.c
 *	  lexical token lookup for key words in PostgreSQL
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * NB: This file is also used by pg_dump.
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/parser/keywords.c,v 1.195 2008/03/27 03:57:33 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */

/* Use c.h so that this file can be built in either frontend or backend */
#include "c.h"

<<<<<<< HEAD
#include "nodes/nodes.h"
#include "nodes/parsenodes.h"
=======
#include <ctype.h>

/*
 * This macro definition overrides the YYSTYPE union definition in parse.h.
 * We don't need that struct in this file, and including the real definition
 * would require sucking in some backend-only include files.
 */
#define YYSTYPE int

>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
#include "parser/keywords.h"
#include "parser/gram.h"

<<<<<<< HEAD
#define PG_KEYWORD(a,b,c) {a,b,c},
=======
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238


const ScanKeyword ScanKeywords[] = {
#include "parser/kwlist.h"
};

/* End of ScanKeywords, for use in kwlookup.c and elsewhere */
const ScanKeyword *LastScanKeyword = endof(ScanKeywords);
