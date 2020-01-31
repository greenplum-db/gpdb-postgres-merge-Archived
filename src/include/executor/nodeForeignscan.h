/*-------------------------------------------------------------------------
 *
 * nodeForeignscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeForeignscan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEFOREIGNSCAN_H
#define NODEFOREIGNSCAN_H

#include "access/parallel.h"
#include "nodes/execnodes.h"

extern ForeignScanState *ExecInitForeignScan(ForeignScan *node, EState *estate, int eflags);
extern void ExecEndForeignScan(ForeignScanState *node);
extern void ExecReScanForeignScan(ForeignScanState *node);

extern void ExecForeignScanEstimate(ForeignScanState *node,
									ParallelContext *pcxt);
extern void ExecForeignScanInitializeDSM(ForeignScanState *node,
										 ParallelContext *pcxt);
extern void ExecForeignScanReInitializeDSM(ForeignScanState *node,
										   ParallelContext *pcxt);
extern void ExecForeignScanInitializeWorker(ForeignScanState *node,
											ParallelWorkerContext *pwcxt);
extern void ExecShutdownForeignScan(ForeignScanState *node);

<<<<<<< HEAD
extern void ExecSquelchForeignScan(ForeignScanState *node);

#endif   /* NODEFOREIGNSCAN_H */
=======
#endif							/* NODEFOREIGNSCAN_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
