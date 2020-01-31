/*-------------------------------------------------------------------------
 *
 * nodeBitmapHeapscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeBitmapHeapscan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEBITMAPHEAPSCAN_H
#define NODEBITMAPHEAPSCAN_H

#include "nodes/execnodes.h"
#include "access/parallel.h"

extern BitmapHeapScanState *ExecInitBitmapHeapScan(BitmapHeapScan *node, EState *estate, int eflags);
<<<<<<< HEAD
extern BitmapHeapScanState *ExecInitBitmapHeapScanForPartition(BitmapHeapScan *node, EState *estate, int eflags, Relation currentRelation);
extern TupleTableSlot *ExecBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecEndBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecReScanBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecSquelchBitmapHeapScan(BitmapHeapScanState *node);
=======
extern void ExecEndBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecReScanBitmapHeapScan(BitmapHeapScanState *node);
extern void ExecBitmapHeapEstimate(BitmapHeapScanState *node,
								   ParallelContext *pcxt);
extern void ExecBitmapHeapInitializeDSM(BitmapHeapScanState *node,
										ParallelContext *pcxt);
extern void ExecBitmapHeapReInitializeDSM(BitmapHeapScanState *node,
										  ParallelContext *pcxt);
extern void ExecBitmapHeapInitializeWorker(BitmapHeapScanState *node,
										   ParallelWorkerContext *pwcxt);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#endif							/* NODEBITMAPHEAPSCAN_H */
