/*-------------------------------------------------------------------------
 *
 * nodePartitionSelector.h
 *	  implement the execution of PartitionSelector for selecting partition
 *	  Oids based on a given set of predicates. It works for both constant
 *	  partition elimination and join partition elimination
 *
 * Copyright (c) 2014-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/include/executor/nodePartitionSelector.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef NODEPARTITIONSELECTOR_H
#define NODEPARTITIONSELECTOR_H

#if 0
/* GPDB_12_MERGE_FIXME: re-implement this node with new postgres partition */
extern TupleTableSlot* ExecPartitionSelector(PartitionSelectorState *node);
extern PartitionSelectorState* ExecInitPartitionSelector(PartitionSelector *node, EState *estate, int eflags);
extern void ExecEndPartitionSelector(PartitionSelectorState *node);
extern void ExecReScanPartitionSelector(PartitionSelectorState *node);
#endif

#endif   /* NODEPARTITIONSELECTOR_H */

