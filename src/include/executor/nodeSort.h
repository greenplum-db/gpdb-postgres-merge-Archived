/*-------------------------------------------------------------------------
 *
 * nodeSort.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeSort.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODESORT_H
#define NODESORT_H

#include "nodes/execnodes.h"

extern SortState *ExecInitSort(Sort *node, EState *estate, int eflags);
extern struct TupleTableSlot *ExecSort(SortState *node);
extern void ExecEndSort(SortState *node);
extern void ExecSortMarkPos(SortState *node);
extern void ExecSortRestrPos(SortState *node);
<<<<<<< HEAD
extern void ExecReScanSort(SortState *node, ExprContext *exprCtxt);
extern void ExecEagerFreeSort(SortState *node);
=======
extern void ExecReScanSort(SortState *node);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* NODESORT_H */
