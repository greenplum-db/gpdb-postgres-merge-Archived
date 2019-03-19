/*-------------------------------------------------------------------------
 *
 * fastpath.h
 *
 *
 * Portions Copyright (c) 1996-2015, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/tcop/fastpath.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef FASTPATH_H
#define FASTPATH_H

#include "lib/stringinfo.h"

<<<<<<< HEAD
extern int GetOldFunctionMessage(StringInfo buf);
extern void HandleFunctionRequest(StringInfo msgBuf);
=======
extern int	GetOldFunctionMessage(StringInfo buf);
extern int	HandleFunctionRequest(StringInfo msgBuf);
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

#endif   /* FASTPATH_H */
