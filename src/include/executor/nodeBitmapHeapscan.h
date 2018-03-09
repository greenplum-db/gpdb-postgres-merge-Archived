/*-------------------------------------------------------------------------
 *
 * nodeBitmapHeapscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeBitmapHeapscan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEBITMAPHEAPSCAN_H
#define NODEBITMAPHEAPSCAN_H

#include "nodes/execnodes.h"

extern BitmapHeapScanState *ExecInitBitmapHeapScan(BitmapHeapScan *node, EState *estate, int eflags);
extern TupleTableSlot *ExecBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecEndBitmapHeapScan(BitmapHeapScanState *node);
<<<<<<< HEAD
extern void ExecBitmapHeapReScan(BitmapHeapScanState *node, ExprContext *exprCtxt);
extern void ExecEagerFreeBitmapHeapScan(BitmapHeapScanState *node);

extern void bitgetpage(HeapScanDesc scan, TBMIterateResult *tbmres);
=======
extern void ExecReScanBitmapHeapScan(BitmapHeapScanState *node);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* NODEBITMAPHEAPSCAN_H */
