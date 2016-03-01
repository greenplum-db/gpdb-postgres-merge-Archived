/*-------------------------------------------------------------------------
 *
 * version.c
 *	 Returns the PostgreSQL version string
 *
<<<<<<< HEAD
 * Copyright (c) 1998-2009, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *
 * $PostgreSQL: pgsql/src/backend/utils/adt/version.c,v 1.18 2009/01/01 17:23:50 momjian Exp $
=======
 * Copyright (c) 1998-2008, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *
 * $PostgreSQL: pgsql/src/backend/utils/adt/version.c,v 1.16 2008/01/01 19:45:53 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "utils/builtins.h"


Datum
pgsql_version(PG_FUNCTION_ARGS __attribute__((unused)) )
{
	char version[512];

<<<<<<< HEAD
	strcpy(version, PG_VERSION_STR " compiled on " __DATE__ " " __TIME__);
	
#ifdef USE_ASSERT_CHECKING
	strcat(version, " (with assert checking)");
#endif 
=======
	SET_VARSIZE(ret, n + VARHDRSZ);
	memcpy(VARDATA(ret), PG_VERSION_STR, n);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	PG_RETURN_TEXT_P(cstring_to_text(version));
}
