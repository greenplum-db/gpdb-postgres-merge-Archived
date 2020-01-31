/*-------------------------------------------------------------------------
 *
 * fastpath.h
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/tcop/fastpath.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef FASTPATH_H
#define FASTPATH_H

#include "lib/stringinfo.h"

extern int	GetOldFunctionMessage(StringInfo buf);
<<<<<<< HEAD
extern void	HandleFunctionRequest(StringInfo msgBuf);
=======
extern void HandleFunctionRequest(StringInfo msgBuf);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#endif							/* FASTPATH_H */
