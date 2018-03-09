/*-------------------------------------------------------------------------
 *
 * nodeHash.h
 *	  prototypes for nodeHash.c
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2007-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/nodeHash.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef NODEHASH_H
#define NODEHASH_H

#include "nodes/execnodes.h"
#include "executor/hashjoin.h"  /* for HJTUPLE_OVERHEAD */
#include "access/memtup.h"

extern HashState *ExecInitHash(Hash *node, EState *estate, int eflags);
extern struct TupleTableSlot *ExecHash(HashState *node);
extern Node *MultiExecHash(HashState *node);
extern void ExecEndHash(HashState *node);
extern void ExecReScanHash(HashState *node);

<<<<<<< HEAD
extern HashJoinTable ExecHashTableCreate(HashState *hashState, HashJoinState *hjstate, List *hashOperators, uint64 operatorMemKB);
extern void ExecHashTableDestroy(HashState *hashState, HashJoinTable hashtable);
extern bool ExecHashTableInsert(HashState *hashState, HashJoinTable hashtable,
					struct TupleTableSlot *slot,
=======
extern HashJoinTable ExecHashTableCreate(Hash *node, List *hashOperators,
					bool keepNulls);
extern void ExecHashTableDestroy(HashJoinTable hashtable);
extern void ExecHashTableInsert(HashJoinTable hashtable,
					TupleTableSlot *slot,
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
					uint32 hashvalue);
extern bool ExecHashGetHashValue(HashState *hashState, HashJoinTable hashtable,
					 ExprContext *econtext,
					 List *hashkeys,
					 bool outer_tuple,
					 bool keep_nulls,
					 uint32 *hashvalue,
					 bool *hashkeys_null);
extern void ExecHashGetBucketAndBatch(HashJoinTable hashtable,
						  uint32 hashvalue,
						  int *bucketno,
						  int *batchno);
<<<<<<< HEAD
extern HashJoinTuple ExecScanHashBucket(HashState *hashState, HashJoinState *hjstate,
				   ExprContext *econtext);
extern void ExecHashTableReset(HashState *hashState, HashJoinTable hashtable);
=======
extern bool ExecScanHashBucket(HashJoinState *hjstate, ExprContext *econtext);
extern void ExecPrepHashTableForUnmatched(HashJoinState *hjstate);
extern bool ExecScanHashTableForUnmatched(HashJoinState *hjstate,
							  ExprContext *econtext);
extern void ExecHashTableReset(HashJoinTable hashtable);
extern void ExecHashTableResetMatchFlags(HashJoinTable hashtable);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
extern void ExecChooseHashTableSize(double ntuples, int tupwidth, bool useskew,
						uint64 operatorMemKB,
						int *numbuckets,
						int *numbatches,
						int *num_skew_mcvs);
extern int	ExecHashGetSkewBucket(HashJoinTable hashtable, uint32 hashvalue);

extern void ExecHashTableExplainInit(HashState *hashState, HashJoinState *hjstate,
                                     HashJoinTable  hashtable);
extern void ExecHashTableExplainBatchEnd(HashState *hashState, HashJoinTable hashtable);

static inline int
ExecHashRowSize(int tupwidth)
{
    return HJTUPLE_OVERHEAD +
		MAXALIGN(sizeof(MemTupleData)) +
		MAXALIGN(tupwidth);
}                               /* ExecHashRowSize */
#endif   /* NODEHASH_H */
