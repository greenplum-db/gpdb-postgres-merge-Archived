/*-------------------------------------------------------------------------
 *
 * tuplesort.h
 *	  Generalized tuple sorting routines.
 *
 * This module handles sorting of heap tuples, index tuples, or single
 * Datums (and could easily support other kinds of sortable objects,
 * if necessary).  It works efficiently for both small and large amounts
 * of data.  Small amounts are sorted in-memory using qsort().  Large
 * amounts are sorted using temporary files and a standard external sort
 * algorithm.  Parallel sorts use a variant of this external sort
 * algorithm, and are typically only used for large amounts of data.
 *
 * GPDB has two implementations of sorting. One is inherited from PostgreSQL,
 * and lives in tuplesort.c. The other one lives in tuplesort_mk.c. Both
 * provide the same API, and have the same set of functions, just with a
 * different suffix.
 *
 * The rest of the system uses functions named tuplesort_*(), to perform
 * sorting. But there are actually three versions of each of these functions:
 *
 * tuplesort_*_pg			- the Postgres implementation
 * tuplesort_*_mk			- the Multi-Key implementation
 * switcheroo_tuplesort_*	- wrapper that calls one of the above.
 *
 * This system is fairly robust, even if new functions are added in the
 * upstream, and we merge that code in. If you don't add the #defines
 * for the new functions, you will get compiler warnings from any callers
 * of them, as the callers will pass a switcheroo_Tuplesortstate struct
 * as the argument, instead of Tuplesortstate_pk.
 *
 * Portions Copyright (c) 2007-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/tuplesort.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TUPLESORT_H
#define TUPLESORT_H

/*
 * First, declare the prototypes for the tuplesort_*_pg functions. In order to
 * keeping this file unchanged from the upstream, the extern declarations
 * use just tuplesort_*(), but we map them with #define to tuplesort_*_pg().
 *
 * The tuplesort_*_mk() functions are declare normally, without #defines,
 * in tuplesort_mk.h
 */

/* these are in tuplesort.h */
#define Tuplesortstate Tuplesortstate_pg

#define tuplesort_begin_heap tuplesort_begin_heap_pg
#define tuplesort_begin_cluster tuplesort_begin_cluster_pg
#define tuplesort_begin_index_btree tuplesort_begin_index_btree_pg
#define tuplesort_begin_index_hash tuplesort_begin_index_hash_pg
#define tuplesort_begin_datum tuplesort_begin_datum_pg
#define tuplesort_set_bound tuplesort_set_bound_pg
#define tuplesort_puttupleslot tuplesort_puttupleslot_pg
#define tuplesort_putheaptuple tuplesort_putheaptuple_pg
#define tuplesort_putindextuplevalues tuplesort_putindextuplevalues_pg
#define tuplesort_putdatum tuplesort_putdatum_pg
#define tuplesort_performsort tuplesort_performsort_pg
#define tuplesort_gettupleslot tuplesort_gettupleslot_pg
#define tuplesort_getheaptuple tuplesort_getheaptuple_pg
#define tuplesort_getindextuple tuplesort_getindextuple_pg
#define tuplesort_getdatum tuplesort_getdatum_pg
#define tuplesort_skiptuples tuplesort_skiptuples_pg
#define tuplesort_end tuplesort_end_pg
/* tuplesort_merge_order not switched */
#define tuplesort_rescan tuplesort_rescan_pg
#define tuplesort_markpos tuplesort_markpos_pg
#define tuplesort_restorepos tuplesort_restorepos_pg

/* these are in tuplesort_gp.h */
#define tuplesort_begin_heap_file_readerwriter tuplesort_begin_heap_file_readerwriter_pg
#define cdb_tuplesort_init cdb_tuplesort_init_pg
#define tuplesort_begin_pos tuplesort_begin_pos_pg
#define tuplesort_gettupleslot_pos tuplesort_gettupleslot_pos_pg
#define tuplesort_flush tuplesort_flush_pg
#define tuplesort_finalize_stats tuplesort_finalize_stats_pg
#define tuplesort_rescan_pos tuplesort_rescan_pos_pg
#define tuplesort_markpos_pos tuplesort_markpos_pos_pg
#define tuplesort_restorepos_pos tuplesort_restorepos_pos_pg
#define tuplesort_set_instrument tuplesort_set_instrument_pg
#define tuplesort_set_gpmon tuplesort_set_gpmon_pg

#include "access/itup.h"
#include "executor/tuptable.h"
#include "fmgr.h"
#include "storage/dsm.h"
#include "utils/relcache.h"

#include "utils/tuplesort_gp.h"

/*
 * Tuplesortstate and Sharedsort are opaque types whose details are not
 * known outside tuplesort.c.
 */
typedef struct Tuplesortstate Tuplesortstate;
typedef struct Sharedsort Sharedsort;

/*
 * Tuplesort parallel coordination state, allocated by each participant in
 * local memory.  Participant caller initializes everything.  See usage notes
 * below.
 */
typedef struct SortCoordinateData
{
	/* Worker process?  If not, must be leader. */
	bool		isWorker;

	/*
	 * Leader-process-passed number of participants known launched (workers
	 * set this to -1).  Includes state within leader needed for it to
	 * participate as a worker, if any.
	 */
	int			nParticipants;

	/* Private opaque state (points to shared memory) */
	Sharedsort *sharedsort;
}			SortCoordinateData;

typedef struct SortCoordinateData *SortCoordinate;

/*
 * Data structures for reporting sort statistics.  Note that
 * TuplesortInstrumentation can't contain any pointers because we
 * sometimes put it in shared memory.
 */
typedef enum
{
	SORT_TYPE_STILL_IN_PROGRESS = 0,
	SORT_TYPE_TOP_N_HEAPSORT,
	SORT_TYPE_QUICKSORT,
	SORT_TYPE_EXTERNAL_SORT,
	SORT_TYPE_EXTERNAL_MERGE
} TuplesortMethod;

typedef enum
{
	SORT_SPACE_TYPE_DISK,
	SORT_SPACE_TYPE_MEMORY
} TuplesortSpaceType;

typedef struct TuplesortInstrumentation
{
	TuplesortMethod sortMethod; /* sort algorithm used */
	TuplesortSpaceType spaceType;	/* type of space spaceUsed represents */
	long		spaceUsed;		/* space consumption, in kB */
} TuplesortInstrumentation;


/*
 * We provide multiple interfaces to what is essentially the same code,
 * since different callers have different data to be sorted and want to
 * specify the sort key information differently.  There are two APIs for
 * sorting HeapTuples and two more for sorting IndexTuples.  Yet another
 * API supports sorting bare Datums.
 *
 * Serial sort callers should pass NULL for their coordinate argument.
 *
 * The "heap" API actually stores/sorts MinimalTuples, which means it doesn't
 * preserve the system columns (tuple identity and transaction visibility
 * info).  The sort keys are specified by column numbers within the tuples
 * and sort operator OIDs.  We save some cycles by passing and returning the
 * tuples in TupleTableSlots, rather than forming actual HeapTuples (which'd
 * have to be converted to MinimalTuples).  This API works well for sorts
 * executed as parts of plan trees.
 *
 * The "cluster" API stores/sorts full HeapTuples including all visibility
 * info. The sort keys are specified by reference to a btree index that is
 * defined on the relation to be sorted.  Note that putheaptuple/getheaptuple
 * go with this API, not the "begin_heap" one!
 *
 * The "index_btree" API stores/sorts IndexTuples (preserving all their
 * header fields).  The sort keys are specified by a btree index definition.
 *
 * The "index_hash" API is similar to index_btree, but the tuples are
 * actually sorted by their hash codes not the raw data.
 *
 * Parallel sort callers are required to coordinate multiple tuplesort states
 * in a leader process and one or more worker processes.  The leader process
 * must launch workers, and have each perform an independent "partial"
 * tuplesort, typically fed by the parallel heap interface.  The leader later
 * produces the final output (internally, it merges runs output by workers).
 *
 * Callers must do the following to perform a sort in parallel using multiple
 * worker processes:
 *
 * 1. Request tuplesort-private shared memory for n workers.  Use
 *    tuplesort_estimate_shared() to get the required size.
 * 2. Have leader process initialize allocated shared memory using
 *    tuplesort_initialize_shared().  Launch workers.
 * 3. Initialize a coordinate argument within both the leader process, and
 *    for each worker process.  This has a pointer to the shared
 *    tuplesort-private structure, as well as some caller-initialized fields.
 *    Leader's coordinate argument reliably indicates number of workers
 *    launched (this is unused by workers).
 * 4. Begin a tuplesort using some appropriate tuplesort_begin* routine,
 *    (passing the coordinate argument) within each worker.  The workMem
 *    arguments need not be identical.  All other arguments should match
 *    exactly, though.
 * 5. tuplesort_attach_shared() should be called by all workers.  Feed tuples
 *    to each worker, and call tuplesort_performsort() within each when input
 *    is exhausted.
 * 6. Call tuplesort_end() in each worker process.  Worker processes can shut
 *    down once tuplesort_end() returns.
 * 7. Begin a tuplesort in the leader using the same tuplesort_begin*
 *    routine, passing a leader-appropriate coordinate argument (this can
 *    happen as early as during step 3, actually, since we only need to know
 *    the number of workers successfully launched).  The leader must now wait
 *    for workers to finish.  Caller must use own mechanism for ensuring that
 *    next step isn't reached until all workers have called and returned from
 *    tuplesort_performsort().  (Note that it's okay if workers have already
 *    also called tuplesort_end() by then.)
 * 8. Call tuplesort_performsort() in leader.  Consume output using the
 *    appropriate tuplesort_get* routine.  Leader can skip this step if
 *    tuplesort turns out to be unnecessary.
 * 9. Call tuplesort_end() in leader.
 *
 * This division of labor assumes nothing about how input tuples are produced,
 * but does require that caller combine the state of multiple tuplesorts for
 * any purpose other than producing the final output.  For example, callers
 * must consider that tuplesort_get_stats() reports on only one worker's role
 * in a sort (or the leader's role), and not statistics for the sort as a
 * whole.
 *
 * Note that callers may use the leader process to sort runs as if it was an
 * independent worker process (prior to the process performing a leader sort
 * to produce the final sorted output).  Doing so only requires a second
 * "partial" tuplesort within the leader process, initialized like that of a
 * worker process.  The steps above don't touch on this directly.  The only
 * difference is that the tuplesort_attach_shared() call is never needed within
 * leader process, because the backend as a whole holds the shared fileset
 * reference.  A worker Tuplesortstate in leader is expected to do exactly the
 * same amount of total initial processing work as a worker process
 * Tuplesortstate, since the leader process has nothing else to do before
 * workers finish.
 *
 * Note that only a very small amount of memory will be allocated prior to
 * the leader state first consuming input, and that workers will free the
 * vast majority of their memory upon returning from tuplesort_performsort().
 * Callers can rely on this to arrange for memory to be used in a way that
 * respects a workMem-style budget across an entire parallel sort operation.
 *
 * Callers are responsible for parallel safety in general.  However, they
 * can at least rely on there being no parallel safety hazards within
 * tuplesort, because tuplesort thinks of the sort as several independent
 * sorts whose results are combined.  Since, in general, the behavior of
 * sort operators is immutable, caller need only worry about the parallel
 * safety of whatever the process is through which input tuples are
 * generated (typically, caller uses a parallel heap scan).
 */
struct ScanState;

<<<<<<< HEAD
extern Tuplesortstate *tuplesort_begin_heap(struct ScanState *ss, TupleDesc tupDesc,
					 int nkeys, AttrNumber *attNums,
					 Oid *sortOperators, Oid *sortCollations,
					 bool *nullsFirstFlags,
					 int workMem, bool randomAccess);
=======
extern Tuplesortstate *tuplesort_begin_heap(TupleDesc tupDesc,
											int nkeys, AttrNumber *attNums,
											Oid *sortOperators, Oid *sortCollations,
											bool *nullsFirstFlags,
											int workMem, SortCoordinate coordinate,
											bool randomAccess);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern Tuplesortstate *tuplesort_begin_cluster(TupleDesc tupDesc,
											   Relation indexRel, int workMem,
											   SortCoordinate coordinate, bool randomAccess);
extern Tuplesortstate *tuplesort_begin_index_btree(Relation heapRel,
												   Relation indexRel,
												   bool enforceUnique,
												   int workMem, SortCoordinate coordinate,
												   bool randomAccess);
extern Tuplesortstate *tuplesort_begin_index_hash(Relation heapRel,
<<<<<<< HEAD
						   Relation indexRel,
						   uint32 hash_mask,
						   int workMem, bool randomAccess);
extern Tuplesortstate *tuplesort_begin_datum(struct ScanState *ss, Oid datumType,
					  Oid sortOperator, Oid sortCollation,
					  bool nullsFirstFlag,
					  int workMem, bool randomAccess);
=======
												  Relation indexRel,
												  uint32 high_mask,
												  uint32 low_mask,
												  uint32 max_buckets,
												  int workMem, SortCoordinate coordinate,
												  bool randomAccess);
extern Tuplesortstate *tuplesort_begin_datum(Oid datumType,
											 Oid sortOperator, Oid sortCollation,
											 bool nullsFirstFlag,
											 int workMem, SortCoordinate coordinate,
											 bool randomAccess);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

extern void tuplesort_set_bound(Tuplesortstate *state, int64 bound);

extern void tuplesort_puttupleslot(Tuplesortstate *state,
								   TupleTableSlot *slot);
extern void tuplesort_putheaptuple(Tuplesortstate *state, HeapTuple tup);
extern void tuplesort_putindextuplevalues(Tuplesortstate *state,
										  Relation rel, ItemPointer self,
										  Datum *values, bool *isnull);
extern void tuplesort_putdatum(Tuplesortstate *state, Datum val,
							   bool isNull);

extern void tuplesort_performsort(Tuplesortstate *state);

extern bool tuplesort_gettupleslot(Tuplesortstate *state, bool forward,
								   bool copy, TupleTableSlot *slot, Datum *abbrev);
extern HeapTuple tuplesort_getheaptuple(Tuplesortstate *state, bool forward);
extern IndexTuple tuplesort_getindextuple(Tuplesortstate *state, bool forward);
extern bool tuplesort_getdatum(Tuplesortstate *state, bool forward,
							   Datum *val, bool *isNull, Datum *abbrev);

extern bool tuplesort_skiptuples(Tuplesortstate *state, int64 ntuples,
								 bool forward);

extern void tuplesort_end(Tuplesortstate *state);

extern void tuplesort_get_stats(Tuplesortstate *state,
								TuplesortInstrumentation *stats);
extern const char *tuplesort_method_name(TuplesortMethod m);
extern const char *tuplesort_space_type_name(TuplesortSpaceType t);

extern int	tuplesort_merge_order(int64 allowedMem);

extern Size tuplesort_estimate_shared(int nworkers);
extern void tuplesort_initialize_shared(Sharedsort *shared, int nWorkers,
										dsm_segment *seg);
extern void tuplesort_attach_shared(Sharedsort *shared, dsm_segment *seg);

/*
 * These routines may only be called if randomAccess was specified 'true'.
 * Likewise, backwards scan in gettuple/getdatum is only allowed if
 * randomAccess was specified.  Note that parallel sorts do not support
 * randomAccess.
 */

extern void tuplesort_rescan(Tuplesortstate *state);
extern void tuplesort_markpos(Tuplesortstate *state);
extern void tuplesort_restorepos(Tuplesortstate *state);


/*
 * We have now declared the protoypes for all the *_pg functions. If we're
 * compiling tuplesort.c itself, continue compilation with #define's in
 * place, so that the functions in tuplesort.c get defined as *_pg.
 *
 * If we're compiling anything else, define switcheroo_* functions that check
 * which implementation should be used, and redefine tuplesort_* to point to
 * the switcheroo-functions. This way, all callers of tuplesort_* get either
 * the Postgres or MK-implementation, depending on the gp_enable_mk_sort
 * GUC.
 */
#ifndef COMPILING_TUPLESORT_C

#undef tuplesort_begin_heap
#undef tuplesort_begin_cluster
#undef tuplesort_begin_index_btree
#undef tuplesort_begin_index_hash
#undef tuplesort_begin_datum
#undef tuplesort_set_bound
#undef tuplesort_puttupleslot
#undef tuplesort_putheaptuple
#undef tuplesort_putindextuplevalues
#undef tuplesort_putdatum
#undef tuplesort_performsort
#undef tuplesort_gettupleslot
#undef tuplesort_getheaptuple
#undef tuplesort_getindextuple
#undef tuplesort_getdatum
#undef tuplesort_skiptuples
#undef tuplesort_end
#undef tuplesort_rescan
#undef tuplesort_markpos
#undef tuplesort_restorepos
#undef tuplesort_begin_heap_file_readerwriter
#undef cdb_tuplesort_init
#undef tuplesort_begin_pos
#undef tuplesort_gettupleslot_pos
#undef tuplesort_flush
#undef tuplesort_finalize_stats
#undef tuplesort_rescan_pos
#undef tuplesort_markpos_pos
#undef tuplesort_restorepos_pos
#undef tuplesort_set_instrument
#undef tuplesort_set_gpmon

#include "tuplesort_mk.h"

extern bool gp_enable_mk_sort;

/*
 * Common header between the regular and MK tuplesort state. We store
 * a flag indicating which one this is, at the beginning, so that
 * the functions that operate on an existing tuplesort are redirected
 * correctly, even if gp_enable_mk_sort changes on the fly.
 */
struct switcheroo_Tuplesortstate
{
	bool		is_mk_tuplesortstate;
};

typedef struct switcheroo_Tuplesortstate switcheroo_Tuplesortstate;

static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_heap(struct ScanState *ss, TupleDesc tupDesc,
					 int nkeys, AttrNumber *attNums,
					 Oid *sortOperators, Oid *sortCollations,
					 bool *nullsFirstFlags,
					 int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	if (gp_enable_mk_sort)
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_heap_mk(ss, tupDesc, nkeys, attNums,
									sortOperators, sortCollations,
									nullsFirstFlags,
									workMem, randomAccess);
	}
	else
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_heap_pg(ss, tupDesc, nkeys, attNums,
									sortOperators,sortCollations,
									nullsFirstFlags,
									workMem, randomAccess);
	}
	state->is_mk_tuplesortstate = gp_enable_mk_sort;
	return state;
}

static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_cluster(TupleDesc tupDesc,
						Relation indexRel,
						int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	/* GPDB_91_MERGE_FIXME: The new 'cluster' variant hasn't been implemented
	 * in mksort yet.
	 */
#if 0
	if (gp_enable_mk_sort)
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_cluster_mk(tupDesc, indexRel,
									   workMem, randomAccess);
	}
	else
#endif
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_cluster_pg(tupDesc, indexRel,
									   workMem, randomAccess);
	}
	state->is_mk_tuplesortstate = gp_enable_mk_sort;
	state->is_mk_tuplesortstate = false;
	return state;
}

static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_index_btree(Relation heapRel,
							Relation indexRel,
							bool enforceUnique,
							int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	if (gp_enable_mk_sort)
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_index_mk(heapRel, indexRel, enforceUnique, workMem, randomAccess);
	}
	else
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_index_btree_pg(heapRel, indexRel, enforceUnique, workMem, randomAccess);
	}
	state->is_mk_tuplesortstate = gp_enable_mk_sort;
	return state;
}
static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_index_hash(Relation heapRel,
							Relation indexRel,
							uint32 hash_mask,
							int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	/* There is no MK variant of this */
	state = (switcheroo_Tuplesortstate *)
		tuplesort_begin_index_hash_pg(heapRel, indexRel, hash_mask, workMem, randomAccess);
	state->is_mk_tuplesortstate = false;
	return state;
}

static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_datum(struct ScanState *ss,
								 Oid datumType, Oid sortOperator, Oid sortCollation,
								 bool nullsFirstFlag,
								 int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	if (gp_enable_mk_sort)
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_datum_mk(ss, datumType, sortOperator, sortCollation,
									 nullsFirstFlag, workMem, randomAccess);
	}
	else
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_datum_pg(ss, datumType, sortOperator, sortCollation,
									 nullsFirstFlag, workMem, randomAccess);
	}
	state->is_mk_tuplesortstate = gp_enable_mk_sort;
	return state;
}

static inline void
switcheroo_tuplesort_set_bound(switcheroo_Tuplesortstate *state, int64 bound)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_set_bound_mk((Tuplesortstate_mk *) state, bound);
	else
		tuplesort_set_bound_pg((Tuplesortstate_pg *) state, bound);
}

static inline void
switcheroo_tuplesort_puttupleslot(switcheroo_Tuplesortstate *state, TupleTableSlot *slot)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_puttupleslot_mk((Tuplesortstate_mk *) state, slot);
	else
		tuplesort_puttupleslot_pg((Tuplesortstate_pg *) state, slot);
}

static inline void
switcheroo_tuplesort_putheaptuple(switcheroo_Tuplesortstate *state, HeapTuple tup)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_putheaptuple_mk((Tuplesortstate_mk *) state, tup);
	else
		tuplesort_putheaptuple_pg((Tuplesortstate_pg *) state, tup);
}

static inline void
switcheroo_tuplesort_putindextuplevalues(switcheroo_Tuplesortstate *state,
										 Relation rel, ItemPointer self,
										 Datum *values, bool *isnull)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_putindextuplevalues_mk((Tuplesortstate_mk *) state, rel,
										 self, values, isnull);
	else
		tuplesort_putindextuplevalues_pg((Tuplesortstate_pg *) state, rel,
										 self, values, isnull);
}

static inline void
switcheroo_tuplesort_putdatum(switcheroo_Tuplesortstate *state, Datum val, bool isNull)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_putdatum_mk((Tuplesortstate_mk *) state, val, isNull);
	else
		tuplesort_putdatum_pg((Tuplesortstate_pg *) state, val, isNull);
}

static inline void
switcheroo_tuplesort_performsort(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_performsort_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_performsort_pg((Tuplesortstate_pg *) state);
}

static inline bool
switcheroo_tuplesort_gettupleslot(switcheroo_Tuplesortstate *state, bool forward,
								   TupleTableSlot *slot, Datum *abbrev)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_gettupleslot_mk((Tuplesortstate_mk *) state, forward, slot, abbrev);
	else
		return tuplesort_gettupleslot_pg((Tuplesortstate_pg *) state, forward, slot, abbrev);
}

static inline HeapTuple
switcheroo_tuplesort_getheaptuple(switcheroo_Tuplesortstate *state, bool forward, bool *should_free)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_getheaptuple_mk((Tuplesortstate_mk *) state, forward, should_free);
	else
		return tuplesort_getheaptuple_pg((Tuplesortstate_pg *) state, forward, should_free);
}

static inline IndexTuple
switcheroo_tuplesort_getindextuple(switcheroo_Tuplesortstate *state, bool forward, bool *should_free)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_getindextuple_mk((Tuplesortstate_mk *) state, forward, should_free);
	else
		return tuplesort_getindextuple_pg((Tuplesortstate_pg *) state, forward, should_free);
}

static inline bool
switcheroo_tuplesort_getdatum(switcheroo_Tuplesortstate *state, bool forward, Datum *val, bool *isNull, Datum *abbrev)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_getdatum_mk((Tuplesortstate_mk *) state, forward, val, isNull, abbrev);
	else
		return tuplesort_getdatum_pg((Tuplesortstate_pg *) state, forward, val, isNull, abbrev);
}

static inline bool
switcheroo_tuplesort_skiptuples(switcheroo_Tuplesortstate *state, int64 ntuples, bool forward)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_skiptuples_mk((Tuplesortstate_mk *) state, ntuples, forward);
	else
		return tuplesort_skiptuples_pg((Tuplesortstate_pg *) state, ntuples, forward);
}

static inline void
switcheroo_tuplesort_end(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_end_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_end_pg((Tuplesortstate_pg *) state);
}

static inline void
switcheroo_tuplesort_rescan(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_rescan_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_rescan_pg((Tuplesortstate_pg *) state);
}

static inline void
switcheroo_tuplesort_markpos(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_markpos_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_markpos_pg((Tuplesortstate_pg *) state);
}


static inline void
switcheroo_tuplesort_restorepos(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_restorepos_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_restorepos_pg((Tuplesortstate_pg *) state);
}

static inline switcheroo_Tuplesortstate *
switcheroo_tuplesort_begin_heap_file_readerwriter(
		struct ScanState * ss,
		const char* rwfile_prefix, bool isWriter,
		TupleDesc tupDesc,
		int nkeys, AttrNumber *attNums,
		Oid *sortOperators,
		Oid *sortCollations,
		bool *nullsFirstFlags,
		int workMem, bool randomAccess)
{
	switcheroo_Tuplesortstate *state;

	if (gp_enable_mk_sort)
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_heap_file_readerwriter_mk(ss, rwfile_prefix, isWriter,
													  tupDesc, nkeys, attNums,
													  sortOperators, sortCollations,
													  nullsFirstFlags,
													  workMem, randomAccess);
	}
	else
	{
		state = (switcheroo_Tuplesortstate *)
			tuplesort_begin_heap_file_readerwriter_pg(ss, rwfile_prefix, isWriter,
													  tupDesc, nkeys, attNums,
													  sortOperators, sortCollations,
													  nullsFirstFlags,
													  workMem, randomAccess);
	}
	state->is_mk_tuplesortstate = gp_enable_mk_sort;
	return state;
}

static inline void
switcheroo_cdb_tuplesort_init(switcheroo_Tuplesortstate *state, int unique,
							  int sort_flags,
							  int64 maxdistinct)
{
	if (state->is_mk_tuplesortstate)
		cdb_tuplesort_init_mk((Tuplesortstate_mk *) state, unique, sort_flags, maxdistinct);
	else
		cdb_tuplesort_init_pg((Tuplesortstate_pg *) state, unique, sort_flags, maxdistinct);
}

static inline void
switcheroo_tuplesort_begin_pos(switcheroo_Tuplesortstate *state, TuplesortPos **pos)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_begin_pos_mk((Tuplesortstate_mk *) state, (TuplesortPos_mk **) pos);
	else
		tuplesort_begin_pos_pg((Tuplesortstate_pg *) state, pos);
}

static inline bool
switcheroo_tuplesort_gettupleslot_pos(switcheroo_Tuplesortstate *state, TuplesortPos *pos,
                          bool forward, TupleTableSlot *slot, Datum *abbrev, MemoryContext mcontext)
{
	if (state->is_mk_tuplesortstate)
		return tuplesort_gettupleslot_pos_mk((Tuplesortstate_mk *) state, (TuplesortPos_mk *) pos, forward, slot, abbrev, mcontext);
	else
		return tuplesort_gettupleslot_pos_pg((Tuplesortstate_pg *) state, pos, forward, slot, abbrev, mcontext);
}

static inline void
switcheroo_tuplesort_flush(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_flush_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_flush_pg((Tuplesortstate_pg *) state);
}

static inline void
switcheroo_tuplesort_finalize_stats(switcheroo_Tuplesortstate *state)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_finalize_stats_mk((Tuplesortstate_mk *) state);
	else
		tuplesort_finalize_stats_pg((Tuplesortstate_pg *) state);
}

static inline void
switcheroo_tuplesort_rescan_pos(switcheroo_Tuplesortstate *state, TuplesortPos *pos)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_rescan_pos_mk((Tuplesortstate_mk *) state, (TuplesortPos_mk *) pos);
	else
		tuplesort_rescan_pos_pg((Tuplesortstate_pg *) state, pos);
}

static inline void
switcheroo_tuplesort_markpos_pos(switcheroo_Tuplesortstate *state, TuplesortPos *pos)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_markpos_pos_mk((Tuplesortstate_mk *) state, (TuplesortPos_mk *) pos);
	else
		tuplesort_markpos_pos_pg((Tuplesortstate_pg *) state, pos);
}

static inline void
switcheroo_tuplesort_restorepos_pos(switcheroo_Tuplesortstate *state, TuplesortPos *pos)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_restorepos_pos_mk((Tuplesortstate_mk *) state, (TuplesortPos_mk *) pos);
	else
		tuplesort_restorepos_pos_pg((Tuplesortstate_pg *) state, pos);
}

static inline void
switcheroo_tuplesort_set_instrument(switcheroo_Tuplesortstate *state,
									struct Instrumentation *instrument,
									struct StringInfoData *explainbuf)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_set_instrument_mk((Tuplesortstate_mk *) state, instrument, explainbuf);
	else
		tuplesort_set_instrument_pg((Tuplesortstate_pg *) state, instrument, explainbuf);
}

static inline void
switcheroo_tuplesort_set_gpmon(switcheroo_Tuplesortstate *state,
							   gpmon_packet_t *gpmon_pkt,
							   int *gpmon_tick)
{
	if (state->is_mk_tuplesortstate)
		tuplesort_set_gpmon_mk((Tuplesortstate_mk *) state, gpmon_pkt, gpmon_tick);
	else
		tuplesort_set_gpmon_pg((Tuplesortstate_pg *) state, gpmon_pkt, gpmon_tick);
}

/* these are in tuplesort.h */
#undef Tuplesortstate
#define Tuplesortstate switcheroo_Tuplesortstate

#define tuplesort_begin_heap switcheroo_tuplesort_begin_heap
#define tuplesort_begin_cluster switcheroo_tuplesort_begin_cluster
#define tuplesort_begin_index_btree switcheroo_tuplesort_begin_index_btree
#define tuplesort_begin_index_hash switcheroo_tuplesort_begin_index_hash
#define tuplesort_begin_datum switcheroo_tuplesort_begin_datum
#define tuplesort_set_bound switcheroo_tuplesort_set_bound
#define tuplesort_puttupleslot switcheroo_tuplesort_puttupleslot
#define tuplesort_putheaptuple switcheroo_tuplesort_putheaptuple
#define tuplesort_putindextuplevalues switcheroo_tuplesort_putindextuplevalues
#define tuplesort_putdatum switcheroo_tuplesort_putdatum
#define tuplesort_performsort switcheroo_tuplesort_performsort
#define tuplesort_gettupleslot switcheroo_tuplesort_gettupleslot
#define tuplesort_getheaptuple switcheroo_tuplesort_getheaptuple
#define tuplesort_getindextuple switcheroo_tuplesort_getindextuple
#define tuplesort_getdatum switcheroo_tuplesort_getdatum
#define tuplesort_skiptuples switcheroo_tuplesort_skiptuples
#define tuplesort_end switcheroo_tuplesort_end
#define tuplesort_rescan switcheroo_tuplesort_rescan
#define tuplesort_markpos switcheroo_tuplesort_markpos
#define tuplesort_restorepos switcheroo_tuplesort_restorepos

/* these are in tuplesort_gp.h */
#define tuplesort_begin_heap_file_readerwriter switcheroo_tuplesort_begin_heap_file_readerwriter
#define cdb_tuplesort_init switcheroo_cdb_tuplesort_init
#define tuplesort_begin_pos switcheroo_tuplesort_begin_pos
#define tuplesort_gettupleslot_pos switcheroo_tuplesort_gettupleslot_pos
#define tuplesort_flush switcheroo_tuplesort_flush
#define tuplesort_finalize_stats switcheroo_tuplesort_finalize_stats
#define tuplesort_rescan_pos switcheroo_tuplesort_rescan_pos
#define tuplesort_markpos_pos switcheroo_tuplesort_markpos_pos
#define tuplesort_restorepos_pos switcheroo_tuplesort_restorepos_pos
#define tuplesort_set_instrument switcheroo_tuplesort_set_instrument
#define tuplesort_set_gpmon switcheroo_tuplesort_set_gpmon

#endif


#endif							/* TUPLESORT_H */
