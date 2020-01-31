/*-------------------------------------------------------------------------
 *
 * nodeAppend.h
 *
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeAppend.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEAPPEND_H
#define NODEAPPEND_H

#include "access/parallel.h"
#include "nodes/execnodes.h"

extern AppendState *ExecInitAppend(Append *node, EState *estate, int eflags);
extern void ExecEndAppend(AppendState *node);
extern void ExecReScanAppend(AppendState *node);
<<<<<<< HEAD
extern void ExecSquelchAppend(AppendState *node);
=======
extern void ExecAppendEstimate(AppendState *node, ParallelContext *pcxt);
extern void ExecAppendInitializeDSM(AppendState *node, ParallelContext *pcxt);
extern void ExecAppendReInitializeDSM(AppendState *node, ParallelContext *pcxt);
extern void ExecAppendInitializeWorker(AppendState *node, ParallelWorkerContext *pwcxt);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#endif							/* NODEAPPEND_H */
