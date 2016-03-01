/*-------------------------------------------------------------------------
 *
 * view.h
 *
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/view.h,v 1.26 2008/01/01 19:45:57 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef VIEW_H
#define VIEW_H

#include "nodes/parsenodes.h"

<<<<<<< HEAD
extern void DefineView(ViewStmt *stmt);
=======
extern void DefineView(ViewStmt *stmt, const char *queryString);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
extern void RemoveView(const RangeVar *view, DropBehavior behavior);

#endif   /* VIEW_H */
