/*-------------------------------------------------------------------------
 *
 * nodeMergeAppend.h
 *
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeMergeAppend.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEMERGEAPPEND_H
#define NODEMERGEAPPEND_H

#include "nodes/execnodes.h"

extern MergeAppendState *ExecInitMergeAppend(MergeAppend *node, EState *estate, int eflags);
extern void ExecEndMergeAppend(MergeAppendState *node);
extern void ExecReScanMergeAppend(MergeAppendState *node);

<<<<<<< HEAD
extern void ExecSquelchMergeAppend(MergeAppendState *node);

#endif   /* NODEMERGEAPPEND_H */
=======
#endif							/* NODEMERGEAPPEND_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
