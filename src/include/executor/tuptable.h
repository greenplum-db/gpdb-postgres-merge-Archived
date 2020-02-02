/*-------------------------------------------------------------------------
 *
 * tuptable.h
 *	  tuple table support stuff
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/executor/tuptable.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TUPTABLE_H
#define TUPTABLE_H

#include "access/htup.h"
#include "access/sysattr.h"
#include "access/tupdesc.h"
#include "access/htup_details.h"
#include "storage/buf.h"

#include "access/memtup.h"

/*----------
 * The executor stores tuples in a "tuple table" which is a List of
 * independent TupleTableSlots.
 *
 * There's various different types of tuple table slots, each being able to
 * store different types of tuples. Additional types of slots can be added
 * without modifying core code. The type of a slot is determined by the
 * TupleTableSlotOps* passed to the slot creation routine. The builtin types
 * of slots are
 *
 * 1. physical tuple in a disk buffer page (TTSOpsBufferHeapTuple)
 * 2. physical tuple constructed in palloc'ed memory (TTSOpsHeapTuple)
 * 3. "minimal" physical tuple constructed in palloc'ed memory
 *    (TTSOpsMinimalTuple)
 * 4. "virtual" tuple consisting of Datum/isnull arrays (TTSOpsVirtual)
 *
 *
 * The first two cases are similar in that they both deal with "materialized"
 * tuples, but resource management is different.  For a tuple in a disk page
 * we need to hold a pin on the buffer until the TupleTableSlot's reference
 * to the tuple is dropped; while for a palloc'd tuple we usually want the
 * tuple pfree'd when the TupleTableSlot's reference is dropped.
 *
 * A "minimal" tuple is handled similarly to a palloc'd regular tuple.
 * At present, minimal tuples never are stored in buffers, so there is no
 * parallel to case 1.  Note that a minimal tuple has no "system columns".
 * (Actually, it could have an OID, but we have no need to access the OID.)
 *
 * A "virtual" tuple is an optimization used to minimize physical data copying
 * in a nest of plan nodes.  Until materialized pass-by-reference Datums in
 * the slot point to storage that is not directly associated with the
 * TupleTableSlot; generally they will point to part of a tuple stored in a
 * lower plan node's output TupleTableSlot, or to a function result
 * constructed in a plan node's per-tuple econtext.  It is the responsibility
 * of the generating plan node to be sure these resources are not released for
 * as long as the virtual tuple needs to be valid or is materialized.  Note
 * also that a virtual tuple does not have any "system columns".
 *
 * The Datum/isnull arrays of a TupleTableSlot serve double duty.  For virtual
 * slots they are the authoritative data.  For the other builtin slots,
 * the arrays contain data extracted from the tuple.  (In this state, any
 * pass-by-reference Datums point into the physical tuple.)  The extracted
 * information is built "lazily", ie, only as needed.  This serves to avoid
 * repeated extraction of data from the physical tuple.
 *
 * A TupleTableSlot can also be "empty", indicated by flag TTS_FLAG_EMPTY set
 * in tts_flags, holding no valid data.  This is the only valid state for a
 * freshly-created slot that has not yet had a tuple descriptor assigned to
 * it.  In this state, TTS_SHOULDFREE should not be set in tts_flags, tts_tuple
 * must be NULL and tts_nvalid zero.
 *
 * The tupleDescriptor is simply referenced, not copied, by the TupleTableSlot
 * code.  The caller of ExecSetSlotDescriptor() is responsible for providing
 * a descriptor that will live as long as the slot does.  (Typically, both
 * slots and descriptors are in per-query memory and are freed by memory
 * context deallocation at query end; so it's not worth providing any extra
 * mechanism to do more.  However, the slot will increment the tupdesc
 * reference count if a reference-counted tupdesc is supplied.)
 *
 * When TTS_SHOULDFREE is set in tts_flags, the physical tuple is "owned" by
 * the slot and should be freed when the slot's reference to the tuple is
 * dropped.
 *
 * tts_values/tts_isnull are allocated either when the slot is created (when
 * the descriptor is provided), or when a descriptor is assigned to the slot;
 * they are of length equal to the descriptor's natts.
 *
 * The TTS_FLAG_SLOW flag is saved state for
 * slot_deform_heap_tuple, and should not be touched by any other code.
 *----------
 */

<<<<<<< HEAD
/* tts_flags */
#define         TTS_ISEMPTY     1
#define         TTS_SHOULDFREE 	2
#define         TTS_SHOULDFREE_MEM 	4	/* should pfree tts_memtuple? */
#define         TTS_VIRTUAL     8

typedef struct TupleTableSlot
{
	NodeTag		type;
	int         PRIVATE_tts_flags;      /* TTS_xxx flags */

	/* Heap tuple stuff */
	HeapTuple PRIVATE_tts_heaptuple;
	void    *PRIVATE_tts_htup_buf;
	uint32   PRIVATE_tts_htup_buf_len;

	/* Mem tuple stuff */
	MemTuple PRIVATE_tts_memtuple;
	void 	*PRIVATE_tts_mtup_buf;
	uint32  PRIVATE_tts_mtup_buf_len;
	ItemPointerData PRIVATE_tts_synthetic_ctid;	/* needed if memtuple is stored on disk */
	
	/* Virtual tuple stuff */
	int 	PRIVATE_tts_nvalid;		/* number of valid virtual tup entries */
	long    PRIVATE_tts_off; 		/* hack to remember state of last decoding. */
	bool    PRIVATE_tts_slow; 		/* hack to remember state of last decoding. */
	Datum 	*PRIVATE_tts_values;		/* virtual tuple values */
	bool 	*PRIVATE_tts_isnull;		/* virtual tuple nulls */

	TupleDesc	tts_tupleDescriptor;	/* slot's tuple descriptor */
	MemTupleBinding *tts_mt_bind;		/* mem tuple's binding */ 
	MemoryContext 	tts_mcxt;		/* slot itself is in this context */
	Buffer		tts_buffer;		/* tuple's buffer, or InvalidBuffer */

    /* System attributes */
    Oid         tts_tableOid;
} TupleTableSlot;

#ifndef FRONTEND

static inline bool TupIsNull(TupleTableSlot *slot)
{
	return (slot == NULL || (slot->PRIVATE_tts_flags & TTS_ISEMPTY) != 0);
}
static inline void TupClearIsEmpty(TupleTableSlot *slot)
{
	slot->PRIVATE_tts_flags &= (~TTS_ISEMPTY);
}
static inline bool TupShouldFree(TupleTableSlot *slot)
{
	Assert(slot);
	return ((slot->PRIVATE_tts_flags & TTS_SHOULDFREE) != 0); 
}
static inline void TupSetShouldFree(TupleTableSlot *slot)
{
	slot->PRIVATE_tts_flags |= TTS_SHOULDFREE; 
}
static inline void TupClearShouldFree(TupleTableSlot *slot)
{
	slot->PRIVATE_tts_flags &= (~TTS_SHOULDFREE);
}
static inline bool TupHasHeapTuple(TupleTableSlot *slot)
{
	Assert(slot);
	return slot->PRIVATE_tts_heaptuple != NULL;
}
static inline bool TupHasMemTuple(TupleTableSlot *slot)
{
	Assert(slot);
	return slot->PRIVATE_tts_memtuple != NULL;
}
static inline bool TupHasVirtualTuple(TupleTableSlot *slot)
{
	Assert(slot);
    return (slot->PRIVATE_tts_flags & TTS_VIRTUAL) ? true : false;
}
static inline HeapTuple TupGetHeapTuple(TupleTableSlot *slot)
{
	Assert(TupHasHeapTuple(slot));
	return slot->PRIVATE_tts_heaptuple; 
}
static inline MemTuple TupGetMemTuple(TupleTableSlot *slot)
{
	Assert(TupHasMemTuple(slot));
	return slot->PRIVATE_tts_memtuple;
}

static inline void free_heaptuple_memtuple(TupleTableSlot *slot)
{
	if(TupShouldFree(slot))
	{
		if(slot->PRIVATE_tts_heaptuple && slot->PRIVATE_tts_heaptuple != slot->PRIVATE_tts_htup_buf)
			pfree(slot->PRIVATE_tts_heaptuple);
		
		if(slot->PRIVATE_tts_memtuple && slot->PRIVATE_tts_memtuple != slot->PRIVATE_tts_mtup_buf)
			pfree(slot->PRIVATE_tts_memtuple);
	}

	slot->PRIVATE_tts_heaptuple = NULL;
	slot->PRIVATE_tts_memtuple = NULL;
}

static inline void TupSetVirtualTuple(TupleTableSlot *slot)
{
	Assert(slot);
	slot->PRIVATE_tts_flags |= TTS_VIRTUAL;
}

static inline void TupSetVirtualTupleNValid(TupleTableSlot *slot, int nvalid)
{
        free_heaptuple_memtuple(slot);
        slot->PRIVATE_tts_flags = TTS_VIRTUAL;
        slot->PRIVATE_tts_nvalid = nvalid;
}

static inline Datum *slot_get_values(TupleTableSlot *slot)
{
	return slot->PRIVATE_tts_values;
}

static inline bool *slot_get_isnull(TupleTableSlot *slot)
{
	return slot->PRIVATE_tts_isnull;
}

extern void _slot_getsomeattrs(TupleTableSlot *slot, int attnum);
static inline void slot_getsomeattrs(TupleTableSlot *slot, int attnum)
{

	if(TupHasVirtualTuple(slot))
	{
		if(slot->PRIVATE_tts_nvalid >= attnum)
			return;
	}

	if(TupHasMemTuple(slot))
	{
		int i; 
		for(i=0; i<attnum; ++i)
			slot->PRIVATE_tts_values[i] = memtuple_getattr(
					slot->PRIVATE_tts_memtuple, slot->tts_mt_bind, 
					i+1, &(slot->PRIVATE_tts_isnull[i]));

		TupSetVirtualTuple(slot);
		slot->PRIVATE_tts_nvalid = attnum;
		return;
	}

	_slot_getsomeattrs(slot, attnum); 
	TupSetVirtualTuple(slot);
}

static inline void
slot_getallattrs(TupleTableSlot *slot)
{
	slot_getsomeattrs(slot, slot->tts_tupleDescriptor->natts);
}

extern bool slot_getsysattr(TupleTableSlot *slot, int attnum,
				Datum *value, bool *isnull);

/*
 * Set the synthetic ctid to a given ctid value.
 *
 * If the slot contains a memtuple or a virtual tuple, the PRIVATE_tts_synthetic_ctid
 * is set to the given ctid value. If the slot contains a heap tuple, then t_self
 * in the heap tuple is set to the given ctid value.
 *
 * This function will set both PRIVATE_tts_synthetic_ctid and heaptuple->t_self when
 * both heap tuple and memtuple or virtual tuple are presented.
 */
static inline void slot_set_ctid(TupleTableSlot *slot, ItemPointer new_ctid)
{
	Assert(slot);

	if (TupHasHeapTuple(slot))
	{
		HeapTuple tuple = TupGetHeapTuple(slot);
		tuple->t_self = *new_ctid;
	}
	
	if (TupHasMemTuple(slot) || TupHasVirtualTuple(slot))
		slot->PRIVATE_tts_synthetic_ctid = *new_ctid;
}

extern void slot_set_ctid_from_fake(TupleTableSlot *slot, ItemPointerData *fake_ctid);

/*
 * Retrieve the synthetic ctid value from the slot.
 *
 * This function assumes that the PRIVATE_tts_synthetic_ctid and heaptuple->t_self
 * are in sync if both of them are presented in the slot.
 */
static inline ItemPointer slot_get_ctid(TupleTableSlot *slot)
{
	if (TupHasHeapTuple(slot))
	{
		Assert(ItemPointerIsValid(&(slot->PRIVATE_tts_heaptuple->t_self)));
		return &(slot->PRIVATE_tts_heaptuple->t_self);
	}
	
	return &(slot->PRIVATE_tts_synthetic_ctid);
}

/* GPDB_94_MERGE_FIXME: We really should move some large functions out of header files. */
=======
/* true = slot is empty */
#define			TTS_FLAG_EMPTY			(1 << 1)
#define TTS_EMPTY(slot)	(((slot)->tts_flags & TTS_FLAG_EMPTY) != 0)

/* should pfree tuple "owned" by the slot? */
#define			TTS_FLAG_SHOULDFREE		(1 << 2)
#define TTS_SHOULDFREE(slot) (((slot)->tts_flags & TTS_FLAG_SHOULDFREE) != 0)

/* saved state for slot_deform_heap_tuple */
#define			TTS_FLAG_SLOW		(1 << 3)
#define TTS_SLOW(slot) (((slot)->tts_flags & TTS_FLAG_SLOW) != 0)

/* fixed tuple descriptor */
#define			TTS_FLAG_FIXED		(1 << 4)
#define TTS_FIXED(slot) (((slot)->tts_flags & TTS_FLAG_FIXED) != 0)

struct TupleTableSlotOps;
typedef struct TupleTableSlotOps TupleTableSlotOps;

/* base tuple table slot type */
typedef struct TupleTableSlot
{
	NodeTag		type;
#define FIELDNO_TUPLETABLESLOT_FLAGS 1
	uint16		tts_flags;		/* Boolean states */
#define FIELDNO_TUPLETABLESLOT_NVALID 2
	AttrNumber	tts_nvalid;		/* # of valid values in tts_values */
	const TupleTableSlotOps *const tts_ops; /* implementation of slot */
#define FIELDNO_TUPLETABLESLOT_TUPLEDESCRIPTOR 4
	TupleDesc	tts_tupleDescriptor;	/* slot's tuple descriptor */
#define FIELDNO_TUPLETABLESLOT_VALUES 5
	Datum	   *tts_values;		/* current per-attribute values */
#define FIELDNO_TUPLETABLESLOT_ISNULL 6
	bool	   *tts_isnull;		/* current per-attribute isnull flags */
	MemoryContext tts_mcxt;		/* slot itself is in this context */
	ItemPointerData tts_tid;	/* stored tuple's tid */
	Oid			tts_tableOid;	/* table oid of tuple */
} TupleTableSlot;

/* routines for a TupleTableSlot implementation */
struct TupleTableSlotOps
{
	/* Minimum size of the slot */
	size_t		base_slot_size;

	/* Initialization. */
	void		(*init) (TupleTableSlot *slot);

	/* Destruction. */
	void		(*release) (TupleTableSlot *slot);

	/*
	 * Clear the contents of the slot. Only the contents are expected to be
	 * cleared and not the tuple descriptor. Typically an implementation of
	 * this callback should free the memory allocated for the tuple contained
	 * in the slot.
	 */
	void		(*clear) (TupleTableSlot *slot);

	/*
	 * Fill up first natts entries of tts_values and tts_isnull arrays with
	 * values from the tuple contained in the slot. The function may be called
	 * with natts more than the number of attributes available in the tuple,
	 * in which case it should set tts_nvalid to the number of returned
	 * columns.
	 */
	void		(*getsomeattrs) (TupleTableSlot *slot, int natts);

	/*
	 * Returns value of the given system attribute as a datum and sets isnull
	 * to false, if it's not NULL. Throws an error if the slot type does not
	 * support system attributes.
	 */
	Datum		(*getsysattr) (TupleTableSlot *slot, int attnum, bool *isnull);

	/*
	 * Make the contents of the slot solely depend on the slot, and not on
	 * underlying resources (like another memory context, buffers, etc).
	 */
	void		(*materialize) (TupleTableSlot *slot);

	/*
	 * Copy the contents of the source slot into the destination slot's own
	 * context. Invoked using callback of the destination slot.
	 */
	void		(*copyslot) (TupleTableSlot *dstslot, TupleTableSlot *srcslot);

	/*
	 * Return a heap tuple "owned" by the slot. It is slot's responsibility to
	 * free the memory consumed by the heap tuple. If the slot can not "own" a
	 * heap tuple, it should not implement this callback and should set it as
	 * NULL.
	 */
	HeapTuple	(*get_heap_tuple) (TupleTableSlot *slot);

	/*
	 * Return a minimal tuple "owned" by the slot. It is slot's responsibility
	 * to free the memory consumed by the minimal tuple. If the slot can not
	 * "own" a minimal tuple, it should not implement this callback and should
	 * set it as NULL.
	 */
	MinimalTuple (*get_minimal_tuple) (TupleTableSlot *slot);

	/*
	 * Return a copy of heap tuple representing the contents of the slot. The
	 * copy needs to be palloc'd in the current memory context. The slot
	 * itself is expected to remain unaffected. It is *not* expected to have
	 * meaningful "system columns" in the copy. The copy is not be "owned" by
	 * the slot i.e. the caller has to take responsibility to free memory
	 * consumed by the slot.
	 */
	HeapTuple	(*copy_heap_tuple) (TupleTableSlot *slot);

	/*
	 * Return a copy of minimal tuple representing the contents of the slot.
	 * The copy needs to be palloc'd in the current memory context. The slot
	 * itself is expected to remain unaffected. It is *not* expected to have
	 * meaningful "system columns" in the copy. The copy is not be "owned" by
	 * the slot i.e. the caller has to take responsibility to free memory
	 * consumed by the slot.
	 */
	MinimalTuple (*copy_minimal_tuple) (TupleTableSlot *slot);
};

/*
 * Predefined TupleTableSlotOps for various types of TupleTableSlotOps. The
 * same are used to identify the type of a given slot.
 */
extern PGDLLIMPORT const TupleTableSlotOps TTSOpsVirtual;
extern PGDLLIMPORT const TupleTableSlotOps TTSOpsHeapTuple;
extern PGDLLIMPORT const TupleTableSlotOps TTSOpsMinimalTuple;
extern PGDLLIMPORT const TupleTableSlotOps TTSOpsBufferHeapTuple;

#define TTS_IS_VIRTUAL(slot) ((slot)->tts_ops == &TTSOpsVirtual)
#define TTS_IS_HEAPTUPLE(slot) ((slot)->tts_ops == &TTSOpsHeapTuple)
#define TTS_IS_MINIMALTUPLE(slot) ((slot)->tts_ops == &TTSOpsMinimalTuple)
#define TTS_IS_BUFFERTUPLE(slot) ((slot)->tts_ops == &TTSOpsBufferHeapTuple)


/*
 * Tuple table slot implementations.
 */

typedef struct VirtualTupleTableSlot
{
	TupleTableSlot base;

	char	   *data;			/* data for materialized slots */
} VirtualTupleTableSlot;

typedef struct HeapTupleTableSlot
{
	TupleTableSlot base;

#define FIELDNO_HEAPTUPLETABLESLOT_TUPLE 1
	HeapTuple	tuple;			/* physical tuple */
#define FIELDNO_HEAPTUPLETABLESLOT_OFF 2
	uint32		off;			/* saved state for slot_deform_heap_tuple */
	HeapTupleData tupdata;		/* optional workspace for storing tuple */
} HeapTupleTableSlot;

/* heap tuple residing in a buffer */
typedef struct BufferHeapTupleTableSlot
{
	HeapTupleTableSlot base;

	/*
	 * If buffer is not InvalidBuffer, then the slot is holding a pin on the
	 * indicated buffer page; drop the pin when we release the slot's
	 * reference to that buffer.  (TTS_FLAG_SHOULDFREE should not be set be
	 * false in such a case, since presumably tts_tuple is pointing at the
	 * buffer page.)
	 */
	Buffer		buffer;			/* tuple's buffer, or InvalidBuffer */
} BufferHeapTupleTableSlot;

typedef struct MinimalTupleTableSlot
{
	TupleTableSlot base;

	/*
	 * In a minimal slot tuple points at minhdr and the fields of that struct
	 * are set correctly for access to the minimal tuple; in particular,
	 * minhdr.t_data points MINIMAL_TUPLE_OFFSET bytes before mintuple.  This
	 * allows column extraction to treat the case identically to regular
	 * physical tuples.
	 */
#define FIELDNO_MINIMALTUPLETABLESLOT_TUPLE 1
	HeapTuple	tuple;			/* tuple wrapper */
	MinimalTuple mintuple;		/* minimal tuple, or NULL if none */
	HeapTupleData minhdr;		/* workspace for minimal-tuple-only case */
#define FIELDNO_MINIMALTUPLETABLESLOT_OFF 4
	uint32		off;			/* saved state for slot_deform_heap_tuple */
} MinimalTupleTableSlot;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/*
 * Get an attribute from the tuple table slot.
 */
<<<<<<< HEAD
static inline Datum slot_getattr(TupleTableSlot *slot, int attnum, bool *isnull)
{
	Datum value;

	Assert(!TupIsNull(slot));
	Assert(attnum <= slot->tts_tupleDescriptor->natts);

	/* System attribute */
	if(attnum <= 0)
	{
		if (slot_getsysattr(slot, attnum, &value, isnull) == false)
			elog(ERROR, "Fail to get system attribute with attnum: %d", attnum);
		return value;
	}

	/* fast path for virtual tuple */
	if(TupHasVirtualTuple(slot) && slot->PRIVATE_tts_nvalid >= attnum)
	{
		*isnull = slot->PRIVATE_tts_isnull[attnum-1];
		return slot->PRIVATE_tts_values[attnum-1];
	}

	/* Mem tuple: We do not even populate virtual tuple */
	if(TupHasMemTuple(slot))
	{
		Assert(slot->tts_mt_bind);
		return memtuple_getattr(slot->PRIVATE_tts_memtuple, slot->tts_mt_bind, attnum, isnull);
	}

	/* Slow: heap tuple */
	Assert(TupHasHeapTuple(slot));

	_slot_getsomeattrs(slot, attnum);
	Assert(TupHasVirtualTuple(slot) && slot->PRIVATE_tts_nvalid >= attnum);
	*isnull = slot->PRIVATE_tts_isnull[attnum-1];
	return slot->PRIVATE_tts_values[attnum-1];
}

static inline bool slot_attisnull(TupleTableSlot *slot, int attnum)
{
	if(attnum <= 0)
		return false;

	if(TupHasHeapTuple(slot))
		return heap_attisnull_normalattr(slot->PRIVATE_tts_heaptuple, attnum);
		
	if(TupHasVirtualTuple(slot) && slot->PRIVATE_tts_nvalid >= attnum)
		return slot->PRIVATE_tts_isnull[attnum-1];

	Assert(TupHasMemTuple(slot));
	return memtuple_attisnull(slot->PRIVATE_tts_memtuple, slot->tts_mt_bind, attnum);
}

/* in executor/execTuples.c */
extern void init_slot(TupleTableSlot *slot, TupleDesc tupdesc);

extern TupleTableSlot *MakeTupleTableSlot(void);
extern TupleTableSlot *ExecAllocTableSlot(List **tupleTable);
=======
#define TupIsNull(slot) \
	((slot) == NULL || TTS_EMPTY(slot))

/* in executor/execTuples.c */
extern TupleTableSlot *MakeTupleTableSlot(TupleDesc tupleDesc,
										  const TupleTableSlotOps *tts_ops);
extern TupleTableSlot *ExecAllocTableSlot(List **tupleTable, TupleDesc desc,
										  const TupleTableSlotOps *tts_ops);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern void ExecResetTupleTable(List *tupleTable, bool shouldFree);
extern TupleTableSlot *MakeSingleTupleTableSlot(TupleDesc tupdesc,
												const TupleTableSlotOps *tts_ops);
extern void ExecDropSingleTupleTableSlot(TupleTableSlot *slot);
<<<<<<< HEAD
extern void ExecSetSlotDescriptor(TupleTableSlot *slot, TupleDesc tupdesc); 

extern TupleTableSlot *ExecStoreHeapTuple(HeapTuple tuple,
			   TupleTableSlot *slot,
			   Buffer buffer,
			   bool shouldFree);
extern TupleTableSlot *ExecStoreMinimalTuple(MemTuple mtup,
					  TupleTableSlot *slot,
					  bool shouldFree);

extern TupleTableSlot *ExecClearTuple(TupleTableSlot *slot);
extern TupleTableSlot *ExecStoreVirtualTuple(TupleTableSlot *slot);
extern TupleTableSlot *ExecStoreAllNullTuple(TupleTableSlot *slot);

extern HeapTuple ExecCopySlotHeapTuple(TupleTableSlot *slot);
extern MemTuple ExecCopySlotMemTuple(TupleTableSlot *slot);
extern MemTuple ExecCopySlotMemTupleTo(TupleTableSlot *slot, MemoryContext pctxt, char *dest, unsigned int *len);

extern HeapTuple ExecFetchSlotHeapTuple(TupleTableSlot *slot);
extern MemTuple ExecFetchSlotMemTuple(TupleTableSlot *slot);
extern Datum ExecFetchSlotTupleDatum(TupleTableSlot *slot);

static inline GenericTuple
ExecFetchSlotGenericTuple(TupleTableSlot *slot)
{
	Assert(!TupIsNull(slot));
	if (slot->PRIVATE_tts_memtuple == NULL && slot->PRIVATE_tts_heaptuple != NULL)
		return (GenericTuple) slot->PRIVATE_tts_heaptuple;

	return (GenericTuple) ExecFetchSlotMemTuple(slot);
}

static inline TupleTableSlot *
ExecStoreGenericTuple(GenericTuple tup, TupleTableSlot *slot, bool shouldFree)
{
	if (is_memtuple(tup))
		return ExecStoreMinimalTuple((MemTuple) tup, slot, shouldFree);

	return ExecStoreHeapTuple((HeapTuple) tup, slot, InvalidBuffer, shouldFree);
}

static inline GenericTuple
ExecCopyGenericTuple(TupleTableSlot *slot)
{
	Assert(!TupIsNull(slot));
	if(slot->PRIVATE_tts_heaptuple != NULL && slot->PRIVATE_tts_memtuple == NULL)
		return (GenericTuple) ExecCopySlotHeapTuple(slot);
	return (GenericTuple) ExecCopySlotMemTuple(slot);
}

extern HeapTuple ExecMaterializeSlot(TupleTableSlot *slot);
extern TupleTableSlot *ExecCopySlot(TupleTableSlot *dstslot,
			 TupleTableSlot *srcslot);

extern void ExecModifyMemTuple(TupleTableSlot *slot, Datum *values, bool *isnull, bool *doRepl);

#endif /* !FRONTEND */

#endif   /* TUPTABLE_H */
=======
extern void ExecSetSlotDescriptor(TupleTableSlot *slot, TupleDesc tupdesc);
extern TupleTableSlot *ExecStoreHeapTuple(HeapTuple tuple,
										  TupleTableSlot *slot,
										  bool shouldFree);
extern void ExecForceStoreHeapTuple(HeapTuple tuple,
									TupleTableSlot *slot,
									bool shouldFree);
extern TupleTableSlot *ExecStoreBufferHeapTuple(HeapTuple tuple,
												TupleTableSlot *slot,
												Buffer buffer);
extern TupleTableSlot *ExecStorePinnedBufferHeapTuple(HeapTuple tuple,
													  TupleTableSlot *slot,
													  Buffer buffer);
extern TupleTableSlot *ExecStoreMinimalTuple(MinimalTuple mtup,
											 TupleTableSlot *slot,
											 bool shouldFree);
extern void ExecForceStoreMinimalTuple(MinimalTuple mtup, TupleTableSlot *slot,
									   bool shouldFree);
extern TupleTableSlot *ExecStoreVirtualTuple(TupleTableSlot *slot);
extern TupleTableSlot *ExecStoreAllNullTuple(TupleTableSlot *slot);
extern void ExecStoreHeapTupleDatum(Datum data, TupleTableSlot *slot);
extern HeapTuple ExecFetchSlotHeapTuple(TupleTableSlot *slot, bool materialize, bool *shouldFree);
extern MinimalTuple ExecFetchSlotMinimalTuple(TupleTableSlot *slot,
											  bool *shouldFree);
extern Datum ExecFetchSlotHeapTupleDatum(TupleTableSlot *slot);
extern void slot_getmissingattrs(TupleTableSlot *slot, int startAttNum,
								 int lastAttNum);
extern void slot_getsomeattrs_int(TupleTableSlot *slot, int attnum);


#ifndef FRONTEND

/*
 * This function forces the entries of the slot's Datum/isnull arrays to be
 * valid at least up through the attnum'th entry.
 */
static inline void
slot_getsomeattrs(TupleTableSlot *slot, int attnum)
{
	if (slot->tts_nvalid < attnum)
		slot_getsomeattrs_int(slot, attnum);
}

/*
 * slot_getallattrs
 *		This function forces all the entries of the slot's Datum/isnull
 *		arrays to be valid.  The caller may then extract data directly
 *		from those arrays instead of using slot_getattr.
 */
static inline void
slot_getallattrs(TupleTableSlot *slot)
{
	slot_getsomeattrs(slot, slot->tts_tupleDescriptor->natts);
}


/*
 * slot_attisnull
 *
 * Detect whether an attribute of the slot is null, without actually fetching
 * it.
 */
static inline bool
slot_attisnull(TupleTableSlot *slot, int attnum)
{
	AssertArg(attnum > 0);

	if (attnum > slot->tts_nvalid)
		slot_getsomeattrs(slot, attnum);

	return slot->tts_isnull[attnum - 1];
}

/*
 * slot_getattr - fetch one attribute of the slot's contents.
 */
static inline Datum
slot_getattr(TupleTableSlot *slot, int attnum,
			 bool *isnull)
{
	AssertArg(attnum > 0);

	if (attnum > slot->tts_nvalid)
		slot_getsomeattrs(slot, attnum);

	*isnull = slot->tts_isnull[attnum - 1];

	return slot->tts_values[attnum - 1];
}

/*
 * slot_getsysattr - fetch a system attribute of the slot's current tuple.
 *
 *  If the slot type does not contain system attributes, this will throw an
 *  error.  Hence before calling this function, callers should make sure that
 *  the slot type is the one that supports system attributes.
 */
static inline Datum
slot_getsysattr(TupleTableSlot *slot, int attnum, bool *isnull)
{
	AssertArg(attnum < 0);		/* caller error */

	if (attnum == TableOidAttributeNumber)
	{
		*isnull = false;
		return ObjectIdGetDatum(slot->tts_tableOid);
	}
	else if (attnum == SelfItemPointerAttributeNumber)
	{
		*isnull = false;
		return PointerGetDatum(&slot->tts_tid);
	}

	/* Fetch the system attribute from the underlying tuple. */
	return slot->tts_ops->getsysattr(slot, attnum, isnull);
}

/*
 * ExecClearTuple - clear the slot's contents
 */
static inline TupleTableSlot *
ExecClearTuple(TupleTableSlot *slot)
{
	slot->tts_ops->clear(slot);

	return slot;
}

/* ExecMaterializeSlot - force a slot into the "materialized" state.
 *
 * This causes the slot's tuple to be a local copy not dependent on any
 * external storage (i.e. pointing into a Buffer, or having allocations in
 * another memory context).
 *
 * A typical use for this operation is to prepare a computed tuple for being
 * stored on disk.  The original data may or may not be virtual, but in any
 * case we need a private copy for heap_insert to scribble on.
 */
static inline void
ExecMaterializeSlot(TupleTableSlot *slot)
{
	slot->tts_ops->materialize(slot);
}

/*
 * ExecCopySlotHeapTuple - return HeapTuple allocated in caller's context
 */
static inline HeapTuple
ExecCopySlotHeapTuple(TupleTableSlot *slot)
{
	Assert(!TTS_EMPTY(slot));

	return slot->tts_ops->copy_heap_tuple(slot);
}

/*
 * ExecCopySlotMinimalTuple - return MinimalTuple allocated in caller's context
 */
static inline MinimalTuple
ExecCopySlotMinimalTuple(TupleTableSlot *slot)
{
	return slot->tts_ops->copy_minimal_tuple(slot);
}

/*
 * ExecCopySlot - copy one slot's contents into another.
 *
 * If a source's system attributes are supposed to be accessed in the target
 * slot, the target slot and source slot types need to match.
 */
static inline TupleTableSlot *
ExecCopySlot(TupleTableSlot *dstslot, TupleTableSlot *srcslot)
{
	Assert(!TTS_EMPTY(srcslot));

	dstslot->tts_ops->copyslot(dstslot, srcslot);

	return dstslot;
}

#endif							/* FRONTEND */

#endif							/* TUPTABLE_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
