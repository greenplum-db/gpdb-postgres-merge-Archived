/*-------------------------------------------------------------------------
 *
 * nodeFunctionscan.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeFunctionscan.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEFUNCTIONSCAN_H
#define NODEFUNCTIONSCAN_H

#include "executor/executor.h"

extern FunctionScanState *ExecInitFunctionScan(FunctionScan *node, EState *estate, int eflags);
extern TupleTableSlot *ExecFunctionScan(FunctionScanState *node);
extern void ExecEndFunctionScan(FunctionScanState *node);
<<<<<<< HEAD
extern void ExecFunctionReScan(FunctionScanState *node, ExprContext *exprCtxt);
extern void ExecEagerFreeFunctionScan(FunctionScanState *node);
=======
extern void ExecReScanFunctionScan(FunctionScanState *node);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* NODEFUNCTIONSCAN_H */
