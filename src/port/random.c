/*-------------------------------------------------------------------------
 *
 * random.c
 *	  random() wrapper
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
<<<<<<< HEAD
 *	  $PostgreSQL: pgsql/src/port/random.c,v 1.11 2010/01/02 16:58:13 momjian Exp $
=======
 *	  $PostgreSQL: pgsql/src/port/random.c,v 1.9 2008/01/01 19:46:00 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */

#include "c.h"

#include <math.h>


long
random()
{
	return lrand48();
}
