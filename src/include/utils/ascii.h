/*-----------------------------------------------------------------------
 * ascii.h
 *
<<<<<<< HEAD
 *	 Portions Copyright (c) 1999-2009, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/include/utils/ascii.h,v 1.17 2009/01/01 17:24:02 momjian Exp $
=======
 *	 Portions Copyright (c) 1999-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/include/utils/ascii.h,v 1.16 2008/01/01 19:45:59 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-----------------------------------------------------------------------
 */

#ifndef _ASCII_H_
#define _ASCII_H_

#include "fmgr.h"

extern Datum to_ascii_encname(PG_FUNCTION_ARGS);
extern Datum to_ascii_enc(PG_FUNCTION_ARGS);
extern Datum to_ascii_default(PG_FUNCTION_ARGS);

#endif   /* _ASCII_H_ */
