/*-------------------------------------------------------------------------
 *
 * nodeWindowAgg.h
 *	  prototypes for nodeWindowAgg.c
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeWindowAgg.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEWINDOWAGG_H
#define NODEWINDOWAGG_H

#include "nodes/execnodes.h"

extern WindowAggState *ExecInitWindowAgg(WindowAgg *node, EState *estate, int eflags);
extern void ExecEndWindowAgg(WindowAggState *node);
extern void ExecReScanWindowAgg(WindowAggState *node);

<<<<<<< HEAD
extern void ExecSquelchWindowAgg(WindowAggState *node);

#endif   /* NODEWINDOWAGG_H */
=======
#endif							/* NODEWINDOWAGG_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
