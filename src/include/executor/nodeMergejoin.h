/*-------------------------------------------------------------------------
 *
 * nodeMergejoin.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeMergejoin.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEMERGEJOIN_H
#define NODEMERGEJOIN_H

#include "nodes/execnodes.h"

extern MergeJoinState *ExecInitMergeJoin(MergeJoin *node, EState *estate, int eflags);
extern TupleTableSlot *ExecMergeJoin(MergeJoinState *node);
extern void ExecEndMergeJoin(MergeJoinState *node);
<<<<<<< HEAD
extern void ExecReScanMergeJoin(MergeJoinState *node, ExprContext *exprCtxt);
extern void ExecEagerFreeMergeJoin(MergeJoinState *node);
=======
extern void ExecReScanMergeJoin(MergeJoinState *node);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* NODEMERGEJOIN_H */
