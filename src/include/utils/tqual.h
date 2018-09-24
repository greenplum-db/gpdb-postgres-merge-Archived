/*-------------------------------------------------------------------------
 *
 * tqual.h
 *	  POSTGRES "time qualification" definitions, ie, tuple visibility rules.
 *
 *	  Should be moved/renamed...    - vadim 07/28/98
 *
 * Portions Copyright (c) 1996-2014, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/tqual.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TQUAL_H
#define TQUAL_H

#include "utils/rel.h"	/* Relation */
#include "utils/snapshot.h"


/* Static variables representing various special snapshot semantics */
extern PGDLLIMPORT SnapshotData SnapshotSelfData;
extern PGDLLIMPORT SnapshotData SnapshotAnyData;
extern PGDLLIMPORT SnapshotData SnapshotToastData;
extern PGDLLIMPORT SnapshotData CatalogSnapshotData;

#define SnapshotSelf		(&SnapshotSelfData)
#define SnapshotAny			(&SnapshotAnyData)
#define SnapshotToast		(&SnapshotToastData)

/*
 * We don't provide a static SnapshotDirty variable because it would be
 * non-reentrant.  Instead, users of that snapshot type should declare a
 * local variable of type SnapshotData, and initialize it with this macro.
 */
#define InitDirtySnapshot(snapshotdata)  \
	((snapshotdata).satisfies = HeapTupleSatisfiesDirty)

/* This macro encodes the knowledge of which snapshots are MVCC-safe */
#define IsMVCCSnapshot(snapshot)  \
	((snapshot)->satisfies == HeapTupleSatisfiesMVCC || \
	 (snapshot)->satisfies == HeapTupleSatisfiesHistoricMVCC)

/*
 * HeapTupleSatisfiesVisibility
 *		True iff heap tuple satisfies a time qual.
 *
 * Notes:
 *	Assumes heap tuple is valid.
 *	Beware of multiple evaluations of snapshot argument.
 *	Hint bits in the HeapTuple's t_infomask may be updated as a side effect;
 *	if so, the indicated buffer is marked dirty.
 *
 *   GP: The added relation parameter helps us decide if we are going to set tuple hint
 *   bits.  If it is null, we ignore the gp_disable_tuple_hints GUC.
 */
<<<<<<< HEAD
#define HeapTupleSatisfiesVisibility(rel, tuple, snapshot, buffer)	\
	((*(snapshot)->satisfies) (rel, (tuple)->t_data, snapshot, buffer))
=======
#define HeapTupleSatisfiesVisibility(tuple, snapshot, buffer) \
	((*(snapshot)->satisfies) (tuple, snapshot, buffer))
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237

/* Result codes for HeapTupleSatisfiesVacuum */
typedef enum
{
	HEAPTUPLE_DEAD,				/* tuple is dead and deletable */
	HEAPTUPLE_LIVE,				/* tuple is live (committed, no deleter) */
	HEAPTUPLE_RECENTLY_DEAD,	/* tuple is dead, but not deletable yet */
	HEAPTUPLE_INSERT_IN_PROGRESS,		/* inserting xact is still in progress */
	HEAPTUPLE_DELETE_IN_PROGRESS	/* deleting xact is still in progress */
} HTSV_Result;

/* These are the "satisfies" test routines for the various snapshot types */
<<<<<<< HEAD
extern bool HeapTupleSatisfiesMVCC(Relation relation, HeapTupleHeader tuple,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesNow(Relation relation, HeapTupleHeader tuple,
					  Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesSelf(Relation relation, HeapTupleHeader tuple,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesAny(Relation relation, HeapTupleHeader tuple,
					  Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesToast(Relation relation, HeapTupleHeader tuple,
						Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesDirty(Relation relation, HeapTupleHeader tuple,
=======
extern bool HeapTupleSatisfiesMVCC(HeapTuple htup,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesSelf(HeapTuple htup,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesAny(HeapTuple htup,
					  Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesToast(HeapTuple htup,
						Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesDirty(HeapTuple htup,
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
						Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesHistoricMVCC(HeapTuple htup,
							   Snapshot snapshot, Buffer buffer);

/* Special "satisfies" routines with different APIs */
<<<<<<< HEAD
extern HTSU_Result HeapTupleSatisfiesUpdate(Relation relation, HeapTupleHeader tuple,
						 CommandId curcid, Buffer buffer);
extern HTSV_Result HeapTupleSatisfiesVacuum(Relation relation, HeapTupleHeader tuple,
=======
extern HTSU_Result HeapTupleSatisfiesUpdate(HeapTuple htup,
						 CommandId curcid, Buffer buffer);
extern HTSV_Result HeapTupleSatisfiesVacuum(HeapTuple htup,
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
						 TransactionId OldestXmin, Buffer buffer);
extern bool HeapTupleIsSurelyDead(HeapTuple htup,
					  TransactionId OldestXmin);

extern void HeapTupleSetHintBits(HeapTupleHeader tuple, Buffer buffer, Relation rel,
					 uint16 infomask, TransactionId xid);
extern bool HeapTupleHeaderIsOnlyLocked(HeapTupleHeader tuple);

/*
 * To avoid leaking to much knowledge about reorderbuffer implementation
 * details this is implemented in reorderbuffer.c not tqual.c.
 */
extern bool ResolveCminCmaxDuringDecoding(struct HTAB *tuplecid_data,
							  Snapshot snapshot,
							  HeapTuple htup,
							  Buffer buffer,
							  CommandId *cmin, CommandId *cmax);
#endif   /* TQUAL_H */
