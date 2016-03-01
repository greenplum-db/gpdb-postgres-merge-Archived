/*-------------------------------------------------------------------------
 *
 * tqual.h
 *	  POSTGRES "time qualification" definitions, ie, tuple visibility rules.
 *
 *	  Should be moved/renamed...	- vadim 07/28/98
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/utils/tqual.h,v 1.71 2008/01/01 19:45:59 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef TQUAL_H
#define TQUAL_H

#include "access/htup.h"
#include "storage/buf.h"
#include "utils/timestamp.h"  /* TimestampTz */
#include "access/xact.h"   /* MaxGpSavePoints */
#include "utils/combocid.h" /*MaxComboCids*/
#include "utils/rel.h"	/* Relation */


#include "cdb/cdbdistributedsnapshot.h"  /* DistributedSnapshotWithLocalMapping */

#define MAX_XIDBUF_SIZE (1024 * 1024)
#define MAX_XIDBUF_XIDS (MAX_XIDBUF_SIZE/sizeof(uint32))
#define MAX_XIDBUF_INIT_PAGES 16

/*
 * We use SnapshotData structures to represent both "regular" (MVCC)
 * snapshots and "special" snapshots that have non-MVCC semantics.
 * The specific semantics of a snapshot are encoded by the "satisfies"
 * function.
 */
typedef struct SnapshotData *Snapshot;

typedef bool (*SnapshotSatisfiesFunc) (HeapTupleHeader tuple,
										   Snapshot snapshot, Buffer buffer);

typedef struct SnapshotData
{
	SnapshotSatisfiesFunc satisfies;	/* tuple test function */

	/*
	 * The remaining fields are used only for MVCC snapshots, and are normally
	 * just zeroes in special snapshots.  (But xmin and xmax are used
	 * specially by HeapTupleSatisfiesDirty.)
	 *
	 * An MVCC snapshot can never see the effects of XIDs >= xmax. It can see
	 * the effects of all older XIDs except those listed in the snapshot. xmin
	 * is stored as an optimization to avoid needing to search the XID arrays
	 * for most tuples.
	 */
	TransactionId xmin;			/* all XID < xmin are visible to me */
	TransactionId xmax;			/* all XID >= xmax are invisible to me */
	uint32		xcnt;			/* # of xact ids in xip[] */
	TransactionId *xip;			/* array of xact IDs in progress */
	/* note: all ids in xip[] satisfy xmin <= xip[i] < xmax */
	int32		subxcnt;		/* # of xact ids in subxip[], -1 if overflow */
	TransactionId *subxip;		/* array of subxact IDs in progress */
	/*
	 * note: all ids in subxip[] are >= xmin, but we don't bother filtering
	 * out any that are >= xmax
	 */
	CommandId	curcid;			/* in my xact, CID < curcid are visible */

	bool haveDistribSnapshot;
						/* True if this snapshot is distributed. */
								
	DistributedSnapshotWithLocalMapping	distribSnapshotWithLocalMapping;
								/*
								 * GP: Global information about which
								 * transactions are visible for a distributed
								 * transaction, with cached local xids
								 */
} SnapshotData;

#define InvalidSnapshot		((Snapshot) NULL)

/* Static variables representing various special snapshot semantics */
extern PGDLLIMPORT SnapshotData SnapshotNowData;
extern PGDLLIMPORT SnapshotData SnapshotSelfData;
extern PGDLLIMPORT SnapshotData SnapshotAnyData;
extern PGDLLIMPORT SnapshotData SnapshotToastData;

<<<<<<< HEAD
extern PGDLLIMPORT Snapshot SnapshotDirty;
=======
#define SnapshotNow			(&SnapshotNowData)
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
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

/* This macro encodes the knowledge of which snapshots are MVCC-safe */
#define IsMVCCSnapshot(snapshot)  \
	((snapshot)->satisfies == HeapTupleSatisfiesMVCC)


extern PGDLLIMPORT Snapshot SerializableSnapshot;
extern PGDLLIMPORT Snapshot LatestSnapshot;
extern PGDLLIMPORT Snapshot ActiveSnapshot;

extern TransactionId TransactionXmin;
extern TransactionId RecentXmin;
extern TransactionId RecentGlobalXmin;

<<<<<<< HEAD
/* MPP Shared Snapshot */
typedef struct SharedSnapshotSlot
{
	int4			slotindex;  /* where in the array this one is. */
	int4	 		slotid;
	pid_t	 		pid; /* pid of writer seg */
	TransactionId	xid;
	CommandId       cid;
	TimestampTz		startTimestamp;
	volatile TransactionId   QDxid;
	volatile CommandId		QDcid;
	volatile bool			ready;
	volatile uint32			segmateSync;
	uint32		total_subcnt;	/* Total # of subxids */
	uint32		inmemory_subcnt;    /* subxids in memory */
	TransactionId   subxids[MaxGpSavePoints];
	uint32			combocidcnt;
	ComboCidKeyData combocids[MaxComboCids];
	SnapshotData	snapshot;

} SharedSnapshotSlot;

extern volatile SharedSnapshotSlot *SharedLocalSnapshotSlot;

=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
/*
 * HeapTupleSatisfiesVisibility
 *		True iff heap tuple satisfies a time qual.
 *
 * Notes:
 *	Assumes heap tuple is valid.
 *	Beware of multiple evaluations of snapshot argument.
<<<<<<< HEAD
 *	Hint bits in the HeapTuple's t_infomask may be updated as a side effect.
 *
 *   GP: The added relation parameter helps us decide if we are going to set tuple hint
 *   bits.  If it is null, we ignore the gp_disable_tuple_hints GUC.
 *
 */
#define HeapTupleSatisfiesVisibility(relation, tuple, snapshot, buffer) \
((snapshot) == SnapshotNow ? \
	HeapTupleSatisfiesNow(relation, (tuple)->t_data, buffer) \
: \
	((snapshot) == SnapshotSelf ? \
		HeapTupleSatisfiesItself(relation, (tuple)->t_data, buffer) \
	: \
		((snapshot) == SnapshotAny ? \
			true \
		: \
			((snapshot) == SnapshotToast ? \
				HeapTupleSatisfiesToast(relation, (tuple)->t_data, buffer) \
			: \
				((snapshot) == SnapshotDirty ? \
					HeapTupleSatisfiesDirty(relation, (tuple)->t_data, buffer) \
				: \
					HeapTupleSatisfiesSnapshot(relation, (tuple)->t_data, snapshot, buffer) \
				) \
			) \
		) \
	) \
)
=======
 *	Hint bits in the HeapTuple's t_infomask may be updated as a side effect;
 *	if so, the indicated buffer is marked dirty.
 */
#define HeapTupleSatisfiesVisibility(tuple, snapshot, buffer) \
	((*(snapshot)->satisfies) ((tuple)->t_data, snapshot, buffer))
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

/* Result codes for HeapTupleSatisfiesUpdate */
typedef enum
{
	HeapTupleMayBeUpdated,
	HeapTupleInvisible,
	HeapTupleSelfUpdated,
	HeapTupleUpdated,
	HeapTupleBeingUpdated
} HTSU_Result;

/* Result codes for HeapTupleSatisfiesVacuum */
typedef enum
{
	HEAPTUPLE_DEAD,				/* tuple is dead and deletable */
	HEAPTUPLE_LIVE,				/* tuple is live (committed, no deleter) */
	HEAPTUPLE_RECENTLY_DEAD,	/* tuple is dead, but not deletable yet */
	HEAPTUPLE_INSERT_IN_PROGRESS,		/* inserting xact is still in progress */
	HEAPTUPLE_DELETE_IN_PROGRESS	/* deleting xact is still in progress */
} HTSV_Result;

<<<<<<< HEAD
#ifdef WATCH_VISIBILITY_IN_ACTION

/* Watch visibility flag bits */
typedef enum
{
    WATCH_VISIBILITY_XMIN_NOT_HINT_COMMITTED = 0,
	WATCH_VISIBILITY_XMIN_ABORTED,
	WATCH_VISIBILITY_XMIN_MOVED_AWAY_BY_VACUUM,
	WATCH_VISIBILITY_VACUUM_XID_CURRENT,
	WATCH_VISIBILITY_SET_XMIN_VACUUM_MOVED_INVALID,
	WATCH_VISIBILITY_SET_XMIN_COMMITTED,
	WATCH_VISIBILITY_SET_XMIN_ABORTED,
	WATCH_VISIBILITY_XMIN_MOVED_IN_BY_VACUUM,
	WATCH_VISIBILITY_VACUUM_XID_NOT_CURRENT,
	WATCH_VISIBILITY_VACUUM_XID_IN_PROGRESS,
	WATCH_VISIBILITY_XMIN_CURRENT,
	WATCH_VISIBILITY_XMIN_NOT_CURRENT,
	WATCH_VISIBILITY_XMIN_INSERTED_AFTER_SCAN_STARTED,
	WATCH_VISIBILITY_XMAX_INVALID,
	WATCH_VISIBILITY_LOCKED,
	WATCH_VISIBILITY_SET_XMAX_ABORTED,
	WATCH_VISIBILITY_DELETED_AFTER_SCAN_STARTED,
	WATCH_VISIBILITY_DELETED_BEFORE_SCAN_STARTED,
	WATCH_VISIBILITY_XMIN_IN_PROGRESS,
	WATCH_VISIBILITY_SNAPSHOT_SAYS_XMIN_IN_PROGRESS,
	WATCH_VISIBILITY_XMAX_INVALID_OR_ABORTED,
	WATCH_VISIBILITY_XMAX_MULTIXACT,
    WATCH_VISIBILITY_XMAX_NOT_HINT_COMMITTED,
    WATCH_VISIBILITY_XMAX_HINT_COMMITTED,
	WATCH_VISIBILITY_XMAX_CURRENT,
	WATCH_VISIBILITY_XMAX_IN_PROGRESS,
	WATCH_VISIBILITY_SET_XMAX_COMMITTED,
	WATCH_VISIBILITY_SNAPSHOT_SAYS_XMAX_IN_PROGRESS,
	WATCH_VISIBILITY_SNAPSHOT_SAYS_XMAX_VISIBLE,
	WATCH_VISIBILITY_USE_DISTRIBUTED_SNAPSHOT_FOR_XMIN,
	WATCH_VISIBILITY_USE_DISTRIBUTED_SNAPSHOT_FOR_XMAX,
	WATCH_VISIBILITY_IGNORE_DISTRIBUTED_SNAPSHOT_FOR_XMIN,
	WATCH_VISIBILITY_IGNORE_DISTRIBUTED_SNAPSHOT_FOR_XMAX,
	WATCH_VISIBILITY_NO_DISTRIBUTED_SNAPSHOT_FOR_XMIN,
	WATCH_VISIBILITY_NO_DISTRIBUTED_SNAPSHOT_FOR_XMAX,
	WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_FOUND_BY_LOCAL,
	WATCH_VISIBILITY_XMAX_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_FOUND_BY_LOCAL,
	WATCH_VISIBILITY_XMIN_LOCAL_DISTRIBUTED_CACHE_RETURNED_LOCAL,
	WATCH_VISIBILITY_XMAX_LOCAL_DISTRIBUTED_CACHE_RETURNED_LOCAL,
	WATCH_VISIBILITY_XMIN_LOCAL_DISTRIBUTED_CACHE_RETURNED_DISTRIB,
	WATCH_VISIBILITY_XMAX_LOCAL_DISTRIBUTED_CACHE_RETURNED_DISTRIB,
	WATCH_VISIBILITY_XMIN_NOT_KNOWN_BY_LOCAL_DISTRIBUTED_XACT,
	WATCH_VISIBILITY_XMAX_NOT_KNOWN_BY_LOCAL_DISTRIBUTED_XACT,
	WATCH_VISIBILITY_XMIN_DIFF_DTM_START_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMAX_DIFF_DTM_START_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMIN_FOUND_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMAX_FOUND_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMIN_KNOWN_LOCAL_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMAX_KNOWN_LOCAL_IN_DISTRIBUTED_LOG,
	WATCH_VISIBILITY_XMIN_KNOWN_BY_LOCAL_DISTRIBUTED_XACT,
	WATCH_VISIBILITY_XMAX_KNOWN_BY_LOCAL_DISTRIBUTED_XACT,
	WATCH_VISIBILITY_XMIN_LESS_THAN_ALL_CURRENT_DISTRIBUTED,
	WATCH_VISIBILITY_XMAX_LESS_THAN_ALL_CURRENT_DISTRIBUTED,
	WATCH_VISIBILITY_XMIN_LESS_THAN_DISTRIBUTED_SNAPSHOT_XMIN,
	WATCH_VISIBILITY_XMAX_LESS_THAN_DISTRIBUTED_SNAPSHOT_XMIN,
	WATCH_VISIBILITY_XMIN_GREATER_THAN_EQUAL_DISTRIBUTED_SNAPSHOT_XMAX,
	WATCH_VISIBILITY_XMAX_GREATER_THAN_EQUAL_DISTRIBUTED_SNAPSHOT_XMAX,
	WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_BY_DISTRIB,
	WATCH_VISIBILITY_XMAX_DISTRIBUTED_SNAPSHOT_IN_PROGRESS_BY_DISTRIB,
	WATCH_VISIBILITY_XMIN_DISTRIBUTED_SNAPSHOT_NOT_IN_PROGRESS,
	WATCH_VISIBILITY_XMAX_DISTRIBUTED_SNAPSHOT_NOT_IN_PROGRESS,
	WATCH_VISIBILITY_XMIN_LESS_THAN_SNAPSHOT_XMIN,
	WATCH_VISIBILITY_XMAX_LESS_THAN_SNAPSHOT_XMIN,
	WATCH_VISIBILITY_XMIN_GREATER_THAN_EQUAL_SNAPSHOT_XMAX,
	WATCH_VISIBILITY_XMAX_GREATER_THAN_EQUAL_SNAPSHOT_XMAX,
	WATCH_VISIBILITY_XMIN_SNAPSHOT_SUBTRANSACTION,
	WATCH_VISIBILITY_XMAX_SNAPSHOT_SUBTRANSACTION,
	WATCH_VISIBILITY_XMIN_MAPPED_SUBTRANSACTION,
	WATCH_VISIBILITY_XMAX_MAPPED_SUBTRANSACTION,
	WATCH_VISIBILITY_XMIN_LESS_THAN_SNAPSHOT_XMIN_2,
	WATCH_VISIBILITY_XMAX_LESS_THAN_SNAPSHOT_XMIN_2,
	WATCH_VISIBILITY_XMIN_SNAPSHOT_IN_PROGRESS,
	WATCH_VISIBILITY_XMAX_SNAPSHOT_IN_PROGRESS,
	WATCH_VISIBILITY_XMIN_SNAPSHOT_NOT_IN_PROGRESS,
	WATCH_VISIBILITY_XMAX_SNAPSHOT_NOT_IN_PROGRESS,
	WATCH_VISIBILITY_SET_XMIN_DISTRIBUTED_SNAPSHOT_IGNORE,
	WATCH_VISIBILITY_SET_XMAX_DISTRIBUTED_SNAPSHOT_IGNORE,
	WATCH_VISIBILITY_GUC_OFF_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_GUC_OFF_MARK_BUFFFER_DIRTY_FOR_XMAX,
	WATCH_VISIBILITY_NO_REL_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_NO_REL_MARK_BUFFFER_DIRTY_FOR_XMAX,
	WATCH_VISIBILITY_SYS_CAT_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_SYS_CAT_MARK_BUFFFER_DIRTY_FOR_XMAX,
	WATCH_VISIBILITY_NO_XID_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_NO_XID_MARK_BUFFFER_DIRTY_FOR_XMAX,
	WATCH_VISIBILITY_TOO_OLD_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_TOO_OLD_MARK_BUFFFER_DIRTY_FOR_XMAX,
	WATCH_VISIBILITY_YOUNG_DO_NOT_MARK_BUFFFER_DIRTY_FOR_XMIN,
	WATCH_VISIBILITY_YOUNG_DO_NOT_MARK_BUFFFER_DIRTY_FOR_XMAX,
	MAX_WATCH_VISIBILITY	// Must be last.
} WATCH_VISIBILITY;

#define WATCH_VISIBILITY_BYTE_LEN 12

extern uint8 WatchVisibilityFlags[WATCH_VISIBILITY_BYTE_LEN];

#define WATCH_VISIBILITY_CLEAR() \
		memset(WatchVisibilityFlags,0,WATCH_VISIBILITY_BYTE_LEN); \
		WatchVisibilityXminCurrent[0] = '\0'; \
		WatchVisibilityXmaxCurrent[0] = '\0';

#define WATCH_VISIBILITY_ADD(nth) \
{ \
	int evalNth = (nth); \
	int nthByte = evalNth >> 3; \
	char nthBit  = 1 << (evalNth & 7); \
	WatchVisibilityFlags[nthByte] |= nthBit; \
}

#define WATCH_VISIBILITY_ADDPAIR(nth,is_xmax) \
{ \
	int evalNth = (is_xmax ? nth + 1 : nth); \
	int nthByte = evalNth >> 3; \
	char nthBit  = 1 << (evalNth & 7); \
	WatchVisibilityFlags[nthByte] |= nthBit; \
}

extern char* WatchVisibilityInActionString(
		BlockNumber 	page,
		OffsetNumber 	lineoff,
		HeapTuple		tuple,
		Snapshot		snapshot);

extern bool WatchVisibilityAllZeros(void);

#else
#define WATCH_VISIBILITY_CLEAR()
#define WATCH_VISIBILITY_ADD(nth)
#define WATCH_VISIBILITY_ADDPAIR(nth,is_xmax)
#endif

/* Result codes for TupleTransactionStatus */
typedef enum TupleTransactionStatus
{
	TupleTransactionStatus_None,
	TupleTransactionStatus_Frozen,
	TupleTransactionStatus_HintCommitted,
	TupleTransactionStatus_HintAborted,
	TupleTransactionStatus_CLogInProgress,
	TupleTransactionStatus_CLogCommitted,
	TupleTransactionStatus_CLogAborted,
	TupleTransactionStatus_CLogSubCommitted,
} TupleTransactionStatus;

/* Result codes for TupleVisibilityStatus */
typedef enum TupleVisibilityStatus
{
	TupleVisibilityStatus_Unknown,
	TupleVisibilityStatus_InProgress,
	TupleVisibilityStatus_Aborted,
	TupleVisibilityStatus_Past,
	TupleVisibilityStatus_Now,
} TupleVisibilityStatus;

typedef struct TupleVisibilitySummary
{
	ItemPointerData				tid;
	int16						infomask;
	int16						infomask2;
	ItemPointerData				updateTid;
	TransactionId				xmin;
	TupleTransactionStatus      xminStatus;
	TransactionId				xmax;
	TupleTransactionStatus      xmaxStatus;
	CommandId					cid;			/* inserting or deleting command ID, or both */
	TupleVisibilityStatus		visibilityStatus;
} TupleVisibilitySummary;


/* MPP Shared Snapshot */
extern Size SharedSnapshotShmemSize(void);
extern void CreateSharedSnapshotArray(void);
extern char *SharedSnapshotDump(void);

extern void SharedSnapshotRemove(volatile SharedSnapshotSlot *slot, char *creatorDescription);
extern void addSharedSnapshot(char *creatorDescription, int id);
extern void lookupSharedSnapshot(char *lookerDescription, char *creatorDescription, int id);

extern void dumpSharedLocalSnapshot_forCursor(void);
extern void readSharedLocalSnapshot_forCursor(Snapshot snapshot);

extern bool HeapTupleSatisfiesItself(Relation relation, HeapTupleHeader tuple, Buffer buffer);
extern bool HeapTupleSatisfiesNow(Relation relation, HeapTupleHeader tuple, Buffer buffer);
extern bool HeapTupleSatisfiesDirty(Relation relation, HeapTupleHeader tuple, Buffer buffer);
extern bool HeapTupleSatisfiesToast(Relation relation, HeapTupleHeader tuple, Buffer buffer);
extern bool HeapTupleSatisfiesSnapshot(Relation relation, HeapTupleHeader tuple,
						   Snapshot snapshot, Buffer buffer);
extern HTSU_Result HeapTupleSatisfiesUpdate(Relation relation, HeapTupleHeader tuple,
=======
/* These are the "satisfies" test routines for the various snapshot types */
extern bool HeapTupleSatisfiesMVCC(HeapTupleHeader tuple,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesNow(HeapTupleHeader tuple,
					  Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesSelf(HeapTupleHeader tuple,
					   Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesAny(HeapTupleHeader tuple,
					  Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesToast(HeapTupleHeader tuple,
						Snapshot snapshot, Buffer buffer);
extern bool HeapTupleSatisfiesDirty(HeapTupleHeader tuple,
						Snapshot snapshot, Buffer buffer);

/* Special "satisfies" routines with different APIs */
extern HTSU_Result HeapTupleSatisfiesUpdate(HeapTupleHeader tuple,
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
						 CommandId curcid, Buffer buffer);
extern HTSV_Result HeapTupleSatisfiesVacuum(HeapTupleHeader tuple,
						 TransactionId OldestXmin, Buffer buffer);

extern void HeapTupleSetHintBits(HeapTupleHeader tuple, Buffer buffer,
					 uint16 infomask, TransactionId xid);

extern Snapshot GetTransactionSnapshot(void);
extern Snapshot GetLatestSnapshot(void);
extern Snapshot CopySnapshot(Snapshot snapshot);
extern void FreeSnapshot(Snapshot snapshot);
extern void FreeXactSnapshot(void);
extern void LogDistributedSnapshotInfo(Snapshot snapshot, const char *prefix);
extern void GetTupleVisibilitySummary(
	HeapTuple				tuple,
	TupleVisibilitySummary	*tupleVisibilitySummary);


// 0  gp_tid                    			TIDOID
// 1  gp_xmin                  			INT4OID
// 2  gp_xmin_status        			TEXTOID
// 3  gp_xmin_commit_distrib_id		TEXTOID
// 4  gp_xmax		          		INT4OID
// 5  gp_xmax_status       			TEXTOID
// 6  gp_xmax_distrib_id    			TEXTOID
// 7  gp_command_id	    			INT4OID
// 8  gp_infomask    	   			TEXTOID
// 9  gp_update_tid         			TIDOID
// 10 gp_visibility             			TEXTOID

extern void GetTupleVisibilitySummaryDatums(
	Datum		*values,
	bool		*nulls,
	TupleVisibilitySummary	*tupleVisibilitySummary);

extern char *GetTupleVisibilitySummaryString(
	TupleVisibilitySummary	*tupleVisibilitySummary);

#endif   /* TQUAL_H */
