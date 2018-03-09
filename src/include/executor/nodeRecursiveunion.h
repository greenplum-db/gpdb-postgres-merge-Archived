/*-------------------------------------------------------------------------
 *
 * nodeRecursiveunion.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeRecursiveunion.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODERECURSIVEUNION_H
#define NODERECURSIVEUNION_H

#include "nodes/execnodes.h"

extern RecursiveUnionState *ExecInitRecursiveUnion(RecursiveUnion *node, EState *estate, int eflags);
extern TupleTableSlot *ExecRecursiveUnion(RecursiveUnionState *node);
extern void ExecEndRecursiveUnion(RecursiveUnionState *node);
<<<<<<< HEAD
extern void ExecRecursiveUnionReScan(RecursiveUnionState *node, ExprContext *exprCtxt);
extern void ExecEagerFreeRecursiveUnion(RecursiveUnionState *node);
=======
extern void ExecReScanRecursiveUnion(RecursiveUnionState *node);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* NODERECURSIVEUNION_H */
