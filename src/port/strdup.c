/*-------------------------------------------------------------------------
 *
 * strdup.c
 *	  copies a null-terminated string.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
<<<<<<< HEAD
 *	  $PostgreSQL: pgsql/src/port/strdup.c,v 1.15 2009/01/01 17:24:04 momjian Exp $
=======
 *	  $PostgreSQL: pgsql/src/port/strdup.c,v 1.14 2008/01/01 19:46:00 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"


char *
strdup(const char *string)
{
	char	   *nstr;

	nstr = (char *) malloc(strlen(string) + 1);
	if (nstr)
		strcpy(nstr, string);
	return nstr;
}
