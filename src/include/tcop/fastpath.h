/*-------------------------------------------------------------------------
 *
 * fastpath.h
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/tcop/fastpath.h,v 1.22 2009/01/01 17:24:01 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/tcop/fastpath.h,v 1.21 2008/01/01 19:45:59 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef FASTPATH_H
#define FASTPATH_H

#include "lib/stringinfo.h"

extern int	HandleFunctionRequest(StringInfo msgBuf);

#endif   /* FASTPATH_H */
