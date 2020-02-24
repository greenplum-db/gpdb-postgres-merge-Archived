/*-------------------------------------------------------------------------
 *
 * nodeDynamicSeqscan.h
 *
 * Portions Copyright (c) 2012 - present, EMC/Greenplum
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
 *
 * IDENTIFICATION
 *	    src/include/executor/nodeDynamicSeqscan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEDYNAMICSEQSCAN_H
#define NODEDYNAMICSEQSCAN_H

#include "nodes/execnodes.h"

/* GPDB_12_MERGE_FIXME */
#if 0
extern DynamicSeqScanState *ExecInitDynamicSeqScan(DynamicSeqScan *node, EState *estate, int eflags);
extern TupleTableSlot *ExecDynamicSeqScan(DynamicSeqScanState *node);
extern void ExecEndDynamicSeqScan(DynamicSeqScanState *node);
extern void ExecReScanDynamicSeqScan(DynamicSeqScanState *node);
#endif

#endif
