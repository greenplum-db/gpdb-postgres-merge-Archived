/*-------------------------------------------------------------------------
 *
 * nodeLimit.h
 *
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/nodeLimit.h,v 1.16 2009/01/01 17:23:59 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/executor/nodeLimit.h,v 1.15 2008/01/01 19:45:57 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODELIMIT_H
#define NODELIMIT_H

#include "nodes/execnodes.h"

extern int	ExecCountSlotsLimit(Limit *node);
extern LimitState *ExecInitLimit(Limit *node, EState *estate, int eflags);
extern TupleTableSlot *ExecLimit(LimitState *node);
extern void ExecEndLimit(LimitState *node);
extern void ExecReScanLimit(LimitState *node, ExprContext *exprCtxt);

enum {
	GPMON_LIMIT_TOTAL = GPMON_QEXEC_M_NODE_START, 
};

static inline gpmon_packet_t * GpmonPktFromLimitState(LimitState *node)
{
	return &node->ps.gpmon_pkt;
}

#endif   /* NODELIMIT_H */
