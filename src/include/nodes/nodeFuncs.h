/*-------------------------------------------------------------------------
 *
 * nodeFuncs.h
 *		Various general-purpose manipulations of Node trees
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 *
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/nodes/nodeFuncs.h,v 1.26 2008/01/01 19:45:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEFUNCS_H
#define NODEFUNCS_H

#include "nodes/primnodes.h"
#include "nodes/parsenodes.h"

extern int	exprLocation(Node *expr);

extern bool single_node(Node *node);
extern bool var_is_outer(Var *var);
extern bool var_is_rel(Var *var);

#endif   /* NODEFUNCS_H */
