/*-------------------------------------------------------------------------
 *
 * vacuum.c
 *	  The postgres vacuum cleaner.
 *
 * This file now includes only control and dispatch code for VACUUM and
 * ANALYZE commands.  Regular VACUUM is implemented in vacuumlazy.c,
 * ANALYZE in analyze.c, and VACUUM FULL is a variant of CLUSTER, handled
 * in cluster.c.
 *
 *
 * Portions Copyright (c) 2005-2010, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/commands/vacuum.c,v 1.410 2010/02/26 02:00:40 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/clog.h"
#include "access/genam.h"
#include "access/heapam.h"
#include "access/appendonlywriter.h"
#include "access/appendonlytid.h"
#include "catalog/heap.h"
#include "access/transam.h"
#include "access/xact.h"
#include "access/appendonly_compaction.h"
#include "access/appendonly_visimap.h"
#include "access/aocs_compaction.h"
#include "catalog/catalog.h"
#include "catalog/namespace.h"
#include "catalog/pg_appendonly_fn.h"
#include "catalog/pg_database.h"
#include "catalog/pg_index.h"
#include "catalog/indexing.h"
#include "catalog/pg_namespace.h"
#include "commands/cluster.h"
#include "commands/tablecmds.h"
#include "commands/vacuum.h"
#include "cdb/cdbdisp_query.h"
#include "cdb/cdbpartition.h"
#include "cdb/cdbvars.h"
#include "cdb/cdbsrlz.h"
#include "cdb/cdbdispatchresult.h"      /* CdbDispatchResults */
#include "cdb/cdbappendonlyblockdirectory.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"             /* pq_beginmessage() etc. */
#include "miscadmin.h"
#include "pgstat.h"
#include "postmaster/autovacuum.h"
#include "storage/bufmgr.h"
#include "storage/lmgr.h"
#include "storage/proc.h"
#include "storage/procarray.h"
#include "utils/acl.h"
#include "utils/fmgroids.h"
#include "utils/guc.h"
#include "utils/memutils.h"
#include "utils/snapmgr.h"
#include "utils/syscache.h"
#include "utils/tqual.h"

#include "access/distributedlog.h"
#include "catalog/pg_inherits_fn.h"
#include "libpq-fe.h"
#include "libpq-int.h"
#include "nodes/makefuncs.h"     /* makeRangeVar */
#include "pgstat.h"
#include "utils/faultinjector.h"
#include "utils/lsyscache.h"
#include "utils/pg_rusage.h"


/*
 * GUC parameters
 */
int			vacuum_freeze_min_age;
int			vacuum_freeze_table_age;


typedef struct VacuumStatsContext
{
	List	   *updated_stats;
} VacuumStatsContext;

/*
 * State information used during the (full)
 * vacuum of indexes on append-only tables
 */
typedef struct AppendOnlyIndexVacuumState
{
	AppendOnlyVisimap visiMap;
	AppendOnlyBlockDirectory blockDirectory;
	AppendOnlyBlockDirectoryEntry blockDirectoryEntry;
} AppendOnlyIndexVacuumState;

/* A few variables that don't seem worth passing around as parameters */
static MemoryContext vac_context = NULL;

static BufferAccessStrategy vac_strategy;

/* non-export function prototypes */
static List *get_rel_oids(Oid relid, VacuumStmt *vacstmt,
			 const char *stmttype);
static void vac_truncate_clog(TransactionId frozenXID);
static void vacuum_rel(Relation onerel, VacuumStmt *vacstmt, LOCKMODE lmode, List *updated_stats,
		   bool for_wraparound);
static void scan_index(Relation indrel, double num_tuples, List *updated_stats,
					   bool check_stats, int elevel);
static bool appendonly_tid_reaped(ItemPointer itemptr, void *state);
static void dispatchVacuum(VacuumStmt *vacstmt, VacuumStatsContext *ctx);
static Relation open_relation_and_check_permission(VacuumStmt *vacstmt,
												   Oid relid,
												   char expected_relkind,
												   bool forceAccessExclusiveLock);
static void vacuumStatement_Relation(VacuumStmt *vacstmt, Oid relid,
						 List *relations, BufferAccessStrategy bstrategy,
						 bool do_toast,
						 bool for_wraparound, bool isTopLevel);

static void
vacuum_combine_stats(VacuumStatsContext *stats_context,
					CdbPgResults* cdb_pgresults);

static void vacuum_appendonly_index(Relation indexRelation,
		AppendOnlyIndexVacuumState *vacuumIndexState,
									List* updated_stats, double rel_tuple_count, bool isfull, int elevel);

/*
 * Primary entry point for VACUUM and ANALYZE commands.
 *
 * relid is normally InvalidOid; if it is not, then it provides the relation
 * OID to be processed, and vacstmt->relation is ignored.  (The non-invalid
 * case is currently only used by autovacuum.)
 *
 * do_toast is passed as FALSE by autovacuum, because it processes TOAST
 * tables separately.
 *
 * for_wraparound is used by autovacuum to let us know when it's forcing
 * a vacuum for wraparound, which should not be auto-cancelled.
 *
 * bstrategy is normally given as NULL, but in autovacuum it can be passed
 * in to use the same buffer strategy object across multiple vacuum() calls.
 *
 * isTopLevel should be passed down from ProcessUtility.
 *
 * It is the caller's responsibility that vacstmt and bstrategy
 * (if given) be allocated in a memory context that won't disappear
 * at transaction commit.
 */
void
vacuum(VacuumStmt *vacstmt, Oid relid, bool do_toast,
	   BufferAccessStrategy bstrategy, bool for_wraparound, bool isTopLevel)
{
	const char *stmttype;
	volatile bool all_rels,
				in_outer_xact,
				use_own_xacts;
	List	   *vacuum_relations;
	List	   *analyze_relations;

	if ((vacstmt->options & VACOPT_VACUUM) &&
		(vacstmt->options & VACOPT_ROOTONLY))
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("ROOTPARTITION option cannot be used together with VACUUM, try ANALYZE ROOTPARTITION")));

	/* sanity checks on options */
	Assert(vacstmt->options & (VACOPT_VACUUM | VACOPT_ANALYZE));
	Assert((vacstmt->options & VACOPT_VACUUM) ||
		   !(vacstmt->options & (VACOPT_FULL | VACOPT_FREEZE)));
	Assert((vacstmt->options & VACOPT_ANALYZE) || vacstmt->va_cols == NIL);

	stmttype = (vacstmt->options & VACOPT_VACUUM) ? "VACUUM" : "ANALYZE";

	/*
	 * We cannot run VACUUM inside a user transaction block; if we were inside
	 * a transaction, then our commit- and start-transaction-command calls
	 * would not have the intended effect!	There are numerous other subtle
	 * dependencies on this, too.
	 *
	 * ANALYZE (without VACUUM) can run either way.
	 */
	if (vacstmt->options & VACOPT_VACUUM)
	{
		if (Gp_role == GP_ROLE_DISPATCH)
			PreventTransactionChain(isTopLevel, stmttype);
		in_outer_xact = false;
	}
	else
		in_outer_xact = IsInTransactionChain(isTopLevel);

	/*
	 * Send info about dead objects to the statistics collector, unless we are
	 * in autovacuum --- autovacuum.c does this for itself.
	 */
	if ((vacstmt->options & VACOPT_VACUUM) && !IsAutoVacuumWorkerProcess())
		pgstat_vacuum_stat();

	/*
	 * Create special memory context for cross-transaction storage.
	 *
	 * Since it is a child of PortalContext, it will go away eventually even
	 * if we suffer an error; there's no need for special abort cleanup logic.
	 */
	vac_context = AllocSetContextCreate(PortalContext,
										"Vacuum",
										ALLOCSET_DEFAULT_MINSIZE,
										ALLOCSET_DEFAULT_INITSIZE,
										ALLOCSET_DEFAULT_MAXSIZE);

	/*
	 * If caller didn't give us a buffer strategy object, make one in the
	 * cross-transaction memory context.
	 */
	if (bstrategy == NULL)
	{
		MemoryContext old_context = MemoryContextSwitchTo(vac_context);

		bstrategy = GetAccessStrategy(BAS_VACUUM);
		MemoryContextSwitchTo(old_context);
	}
	vac_strategy = bstrategy;

	/* Remember whether we are processing everything in the DB */
	all_rels = (!OidIsValid(relid) && vacstmt->relation == NULL);

	/*
	 * Build list of relations to process, unless caller gave us one. (If we
	 * build one, we put it in vac_context for safekeeping.)
	 */

	/*
	 * Analyze on midlevel partition is not allowed directly so
	 * vacuum_relations and analyze_relations may be different.  In case of
	 * partitioned tables, vacuum_relation will contain all OIDs of the
	 * partitions of a partitioned table. However, analyze_relation will
	 * contain all the OIDs of partition of a partitioned table except midlevel
	 * partition unless GUC optimizer_analyze_midlevel_partition is set to on.
	 */
	if (vacstmt->options & VACOPT_VACUUM)
		vacuum_relations = get_rel_oids(relid, vacstmt, stmttype);
	if (vacstmt->options & VACOPT_ANALYZE)
		analyze_relations = get_rel_oids(relid, vacstmt, stmttype);

	/*
	 * Decide whether we need to start/commit our own transactions.
	 *
	 * For VACUUM (with or without ANALYZE): always do so, so that we can
	 * release locks as soon as possible.  (We could possibly use the outer
	 * transaction for a one-table VACUUM, but handling TOAST tables would be
	 * problematic.)
	 *
	 * For ANALYZE (no VACUUM): if inside a transaction block, we cannot
	 * start/commit our own transactions.  Also, there's no need to do so if
	 * only processing one relation.  For multiple relations when not within a
	 * transaction block, and also in an autovacuum worker, use own
	 * transactions so we can release locks sooner.
	 */
	if (vacstmt->options & VACOPT_VACUUM)
		use_own_xacts = true;
	else
	{
		Assert(vacstmt->options & VACOPT_ANALYZE);
		if (IsAutoVacuumWorkerProcess())
			use_own_xacts = true;
		else if (in_outer_xact)
			use_own_xacts = false;
		else if (list_length(analyze_relations) > 1)
			use_own_xacts = true;
		else
			use_own_xacts = false;
	}

	/*
	 * vacuum_rel expects to be entered with no transaction active; it will
	 * start and commit its own transaction.  But we are called by an SQL
	 * command, and so we are executing inside a transaction already. We
	 * commit the transaction started in PostgresMain() here, and start
	 * another one before exiting to match the commit waiting for us back in
	 * PostgresMain().
	 */
	if (use_own_xacts)
	{
		/* ActiveSnapshot is not set by autovacuum */
		if (ActiveSnapshotSet())
			PopActiveSnapshot();

		/* matches the StartTransaction in PostgresMain() */
		if (Gp_role != GP_ROLE_EXECUTE)
			CommitTransactionCommand();
	}

	/* Turn vacuum cost accounting on or off */
	PG_TRY();
	{
		ListCell   *cur;

		VacuumCostActive = (VacuumCostDelay > 0);
		VacuumCostBalance = 0;

		if (Gp_role == GP_ROLE_DISPATCH)
		{
			vacstmt->appendonly_compaction_segno = NIL;
			vacstmt->appendonly_compaction_insert_segno = NIL;
			vacstmt->appendonly_compaction_vacuum_cleanup = false;
			vacstmt->appendonly_relation_empty = false;
		}

		if (vacstmt->options & VACOPT_VACUUM)
		{
			/*
			 * Loop to process each selected relation which needs to be
			 * vacuumed.
			 */
			foreach(cur, vacuum_relations)
			{
				Oid			relid = lfirst_oid(cur);

				vacuumStatement_Relation(vacstmt, relid, vacuum_relations, bstrategy, do_toast, for_wraparound, isTopLevel);
			}
		}

		if (vacstmt->options & VACOPT_ANALYZE)
		{
			/*
			 * If there are no partition tables in the database and ANALYZE
			 * ROOTPARTITION ALL is executed, report a WARNING as no root
			 * partitions are there to be analyzed
			 */
			if ((vacstmt->options & VACOPT_ROOTONLY) && NIL == analyze_relations && !vacstmt->relation)
			{
				ereport(NOTICE,
						(errmsg("there are no partitioned tables in database to ANALYZE ROOTPARTITION")));
			}

			/*
			 * Loop to process each selected relation which needs to be analyzed.
			 */
			foreach(cur, analyze_relations)
			{
				Oid			relid = lfirst_oid(cur);

				/*
				 * If using separate xacts, start one for analyze. Otherwise,
				 * we can use the outer transaction.
				 */
				if (use_own_xacts)
				{
					StartTransactionCommand();
					/* functions in indexes may want a snapshot set */
					PushActiveSnapshot(GetTransactionSnapshot());
				}

				analyze_rel(relid, vacstmt, vac_strategy);

				if (use_own_xacts)
				{
					PopActiveSnapshot();
					CommitTransactionCommand();
				}
			}
		}
	}
	PG_CATCH();
	{
		/* Make sure cost accounting is turned off after error */
		VacuumCostActive = false;
		PG_RE_THROW();
	}
	PG_END_TRY();

	/* Turn off vacuum cost accounting */
	VacuumCostActive = false;

	/*
	 * Finish up processing.
	 */
	if (use_own_xacts)
	{
		/* here, we are not in a transaction */

		StartTransactionCommand();
	}

	if ((vacstmt->options & VACOPT_VACUUM) && !IsAutoVacuumWorkerProcess())
	{
		/*
		 * Update pg_database.datfrozenxid, and truncate pg_clog if possible.
		 * (autovacuum.c does this for itself.)
		 */
		vac_update_datfrozenxid();
	}

	/*
	 * Clean up working storage --- note we must do this after
	 * StartTransactionCommand, else we might be trying to delete the active
	 * context!
	 */
	MemoryContextDelete(vac_context);
	vac_context = NULL;
}

/*
 * Assigns the compaction segment information.
 *
 * The vacuum statement will be modified.
 *
 */
static bool vacuum_assign_compaction_segno(
		Relation onerel,
		List *compactedSegmentFileList,
		List *insertedSegmentFileList,
		VacuumStmt *vacstmt)
{
	List *new_compaction_list;
	List *insert_segno;
	bool is_drop;

	Assert(Gp_role != GP_ROLE_EXECUTE);
	Assert(vacstmt->appendonly_compaction_segno == NIL);
	Assert(vacstmt->appendonly_compaction_insert_segno == NIL);
	Assert (RelationIsValid(onerel));

	/*
	 * Assign a compaction segment num and insert segment num
	 * on master or on segment if in utility mode
	 */
	if (!(RelationIsAoRows(onerel) || RelationIsAoCols(onerel)) || !gp_appendonly_compaction)
	{
		return true;
	}

	if (HasSerializableBackends(false))
	{
		elog(LOG, "Skip compaction because of concurrent serializable transactions");
		return false;
	}

	new_compaction_list = SetSegnoForCompaction(onerel,
			compactedSegmentFileList, insertedSegmentFileList, &is_drop);
	if (new_compaction_list)
	{
		if (!is_drop)
		{
			insert_segno = lappend_int(NIL, SetSegnoForCompactionInsert(onerel,
				new_compaction_list, compactedSegmentFileList, insertedSegmentFileList));
		}
		else
		{
			/*
			 * If we continue an aborted drop phase, we do not assign a real
			 * insert segment file.
			 */
			insert_segno = list_make1_int(APPENDONLY_COMPACTION_SEGNO_INVALID);
		}

		elogif(Debug_appendonly_print_compaction, LOG,
				"Schedule compaction on AO table: "
				"compact segno list length %d, insert segno length %d",
				list_length(new_compaction_list), list_length(insert_segno));
	}

	if (!new_compaction_list)
	{
		elog(DEBUG3, "No valid compact segno for releation %s (%d)",
				RelationGetRelationName(onerel),
				RelationGetRelid(onerel));
		return false;
	}
	else
	{
		vacstmt->appendonly_compaction_insert_segno = insert_segno;
		vacstmt->appendonly_compaction_segno = new_compaction_list;
		return true;
	}
}

bool
vacuumStatement_IsTemporary(Relation onerel)
{
	bool bTemp = false;
	/* MPP-7576: don't track internal namespace tables */
	switch (RelationGetNamespace(onerel))
	{
		case PG_CATALOG_NAMESPACE:
			/* MPP-7773: don't track objects in system namespace
			 * if modifying system tables (eg during upgrade)
			 */
			if (allowSystemTableModsDDL)
				bTemp = true;
			break;

		case PG_TOAST_NAMESPACE:
		case PG_BITMAPINDEX_NAMESPACE:
		case PG_AOSEGMENT_NAMESPACE:
			bTemp = true;
			break;
		default:
			break;
	}

	/* MPP-7572: Don't track metadata if table in any
	 * temporary namespace
	 */
	if (!bTemp)
		bTemp = isAnyTempNamespace(RelationGetNamespace(onerel));
	return bTemp;
}

/*
 * Modify the Vacuum statement to vacuum an individual
 * relation. This ensures that only one relation will be
 * locked for vacuum, when the user issues a "vacuum <db>"
 * command, or a "vacuum <parent_partition_table>"
 * command.
 */
static void
vacuumStatement_AssignRelation(VacuumStmt *vacstmt, Oid relid, List *relations)
{
	if (list_length(relations) > 1 || vacstmt->relation == NULL)
	{
		char	*relname		= get_rel_name(relid);
		char	*namespace_name =
			get_namespace_name(get_rel_namespace(relid));

		if (relname == NULL)
		{
			elog(ERROR, "Relation name does not exist for relation with oid %d", relid);
			return;
		}

		if (namespace_name == NULL)
		{
			elog(ERROR, "Namespace does not exist for relation with oid %d", relid);
			return;
		}

		/* XXX: dispatch OID than name */
		vacstmt->relation = makeRangeVar(namespace_name, relname, -1);
	}
}

/*
 * Chose a source and destination segfile for compaction.  It assumes that we
 * are in the vacuum memory context, and executing in DISPATCH or UTILITY mode.
 * Return false if we are done with all segfiles.
 */
static bool
vacuumStatement_AssignAppendOnlyCompactionInfo(VacuumStmt *vacstmt,
		Relation onerel,
		List *compactedSegmentFileList,
		List *insertedSegmentFileList,
		bool *getnextrelation)
{
	Assert(Gp_role != GP_ROLE_EXECUTE);
	Assert(vacstmt);
	Assert(getnextrelation);
	Assert(RelationIsAoRows(onerel) || RelationIsAoCols(onerel));

	if (!vacuum_assign_compaction_segno(onerel,
				compactedSegmentFileList,
				insertedSegmentFileList,
				vacstmt))
	{
		/* There is nothing left to do for this relation */
		if (list_length(compactedSegmentFileList) > 0)
		{
			/*
			 * We now need to vacuum the auxility relations of the
			 * append-only relation
			 */
			vacstmt->appendonly_compaction_vacuum_cleanup = true;

			/* Provide the list of all compacted segment numbers with it */
			list_free(vacstmt->appendonly_compaction_segno);
			vacstmt->appendonly_compaction_segno = list_copy(compactedSegmentFileList);
			list_free(vacstmt->appendonly_compaction_insert_segno);
			vacstmt->appendonly_compaction_insert_segno = list_copy(insertedSegmentFileList);
		}
		else
		{
			return false;
		}
	}

	if (vacstmt->appendonly_compaction_segno &&
			vacstmt->appendonly_compaction_insert_segno &&
			!vacstmt->appendonly_compaction_vacuum_cleanup)
	{
		/*
		 * as long as there are real segno to compact, we
		 * keep processing this relation.
		 */
		*getnextrelation = false;
	}
	return true;
}

bool
vacuumStatement_IsInAppendOnlyDropPhase(VacuumStmt *vacstmt)
{
	Assert(vacstmt);
	return (vacstmt->appendonly_compaction_segno &&
			!vacstmt->appendonly_compaction_insert_segno &&
			!vacstmt->appendonly_compaction_vacuum_prepare &&
			!vacstmt->appendonly_compaction_vacuum_cleanup);
}

bool
vacuumStatement_IsInAppendOnlyCompactionPhase(VacuumStmt *vacstmt)
{
	Assert(vacstmt);
	return (vacstmt->appendonly_compaction_segno &&
			vacstmt->appendonly_compaction_insert_segno &&
			!vacstmt->appendonly_compaction_vacuum_prepare &&
			!vacstmt->appendonly_compaction_vacuum_cleanup);
}

bool
vacuumStatement_IsInAppendOnlyPseudoCompactionPhase(VacuumStmt *vacstmt)
{
	Assert(vacstmt);
	return (vacstmt->appendonly_compaction_segno &&
			vacstmt->appendonly_compaction_insert_segno &&
			linitial_int(vacstmt->appendonly_compaction_insert_segno)
				== APPENDONLY_COMPACTION_SEGNO_INVALID &&
			!vacstmt->appendonly_compaction_vacuum_prepare &&
			!vacstmt->appendonly_compaction_vacuum_cleanup);
}

bool
vacuumStatement_IsInAppendOnlyPreparePhase(VacuumStmt* vacstmt)
{
	Assert(vacstmt);
	return (vacstmt->appendonly_compaction_vacuum_prepare);
}

bool
vacummStatement_IsInAppendOnlyCleanupPhase(VacuumStmt *vacstmt)
{
	Assert(vacstmt);
	return (vacstmt->appendonly_compaction_vacuum_cleanup);
}

/*
 * Processing of the vacuumStatement for given relid.
 *
 * The function is called by vacuumStatement once for each relation to vacuum.
 * In order to connect QD and QE work for vacuum, we employ a little
 * complicated mechanism here; we separate one relation vacuum process
 * to a separate steps, depending on the type of storage (heap/AO),
 * and perform each step in separate transactions, so that QD can open
 * a distributed transaction and embrace QE work inside it.  As opposed to
 * old postgres code, where one transaction is opened and closed for each
 * auxiliary relation, here a transaction processes them as a set starting
 * from the base relation.  This is the entry point of one base relation,
 * and QD makes some decision what kind of stage we perform, and tells it
 * to QE with vacstmt fields through dispatch.
 *
 * For heap VACUUM FULL, we need two transactions.  One is to move tuples
 * from a page to another, to empty out last pages, which typically goes
 * into repair_frag.  We used to perform truncate operation there, but
 * it required to record transaction commit locally, which is not pleasant
 * if QD decides to cancel the whoe distributed transaction.  So the truncate
 * step is separated to a second transaction.  This two step operation is
 * performed on both base relation and toast relation at the same time.
 *
 * Lazy vacuum to heap is one step operation.
 *
 * AO compaction is rather complicated.  There are four phases.
 *   - prepare phase
 *   - compaction phase
 *   - drop phase
 *   - cleanup phase
 * Out of these, compaction and drop phase might repeat multiple times.
 * We go through the list of available segment files by looking up catalog,
 * and perform a compaction operation, which appends the whole segfile
 * to another available one, if the source segfile looks to be dirty enough.
 * If we find such one and perform compaction, the next step is drop. In
 * order to allow concurrent read it is required for the drop phase to
 * be a separate transaction.  We mark the segfile as an awaiting-drop
 * in the catalog, and the drop phase actually drops the segfile from the
 * disk.  There are some cases where we cannot drop the segfile immediately,
 * in which case we just skip it and leave the catalog to have awaiting-drop
 * state for this segfile.  Aside from the compaction and drop phases, the
 * rest is much simpler.  The prepare phase is to truncate unnecessary
 * blocks after the logical EOF, and the cleanup phase does normal heap
 * vacuum on auxiliary relations (toast, aoseg, block directory, visimap,)
 * as well as updating stats info in catalog.  Keep in mind that if the
 * vacuum is full, we need the same two steps as the heap base relation
 * case.  So cleanup phase in AO may consume two transactions.
 *
 * While executing these multiple transactions, we acquire a session
 * lock across transactions, in order to keep concurrent work on the
 * same relation away.  It doesn't look intuitive, though, if you look
 * at QE work, because from its perspective it is always one step, therefore
 * there is no session lock technically (we actually acquire and release
 * it as it's harmless.)  Session lock doesn't work here, because QE
 * is under a distributed transaction and we haven't supported session
 * lock recording in transaction prepare.  This should be ok as far as
 * we are dealing with user table, because other MPP work also tries
 * to take a relation lock, which would conflict with this vacuum work
 * on master.  Be careful with catalog tables, because we take locks on
 * them and release soon much before the end of transaction.  That means
 * QE still needs to deal with concurrent work well.
 */
static void
vacuumStatement_Relation(VacuumStmt *vacstmt, Oid relid,
						 List *relations, BufferAccessStrategy bstrategy,
						 bool do_toast, bool for_wraparound, bool isTopLevel)
{
	LOCKMODE			lmode = NoLock;
	Relation			onerel;
	LockRelId			onerelid;
	MemoryContext		oldctx;
	VacuumStatsContext stats_context;

	vacstmt = copyObject(vacstmt);
	/* VACUUM, without ANALYZE */
	vacstmt->options &= ~VACOPT_ANALYZE;
	vacstmt->options |= VACOPT_VACUUM;
	vacstmt->va_cols = NIL;		/* A plain VACUUM cannot list columns */

	/*
	 * We compact segment file by segment file.
	 * Therefore in some cases, we have multiple vacuum dispatches
	 * per relation.
	 */
	bool getnextrelation = false;

	/* Number of rounds performed on this relation */
	int relationRound = 0;

	List* compactedSegmentFileList = NIL;
	List* insertedSegmentFileList = NIL;

	bool dropPhase = false;
	bool truncatePhase = false;

	Assert(vacstmt);

	if (Gp_role != GP_ROLE_EXECUTE)
	{
		/* First call on a relation is the prepare phase */
		vacstmt->appendonly_compaction_vacuum_prepare = true;

		/*
		 * Reset truncate flag always as we may iterate more than one relation.
		 */
		vacstmt->heap_truncate = false;
	}

	while (!getnextrelation)
	{
		getnextrelation = true;

		/*
		 * The following block of code is relevant only for AO tables.  The
		 * only phases relevant are prepare phase, compaction phase and the
		 * first phase of cleanup for VACUUM FULL (truncatePhase set to true
		 * refers to this phase). 
		 */
		if (Gp_role != GP_ROLE_EXECUTE && (!dropPhase || truncatePhase))
		{
			 /* Reset the compaction segno if new relation or segment file is
			  * started
			  */
			list_free(vacstmt->appendonly_compaction_segno);
			list_free(vacstmt->appendonly_compaction_insert_segno);
			vacstmt->appendonly_compaction_segno = NIL;
			vacstmt->appendonly_compaction_insert_segno = NIL;
			/*
			 * We should not reset the cleanup flag for truncate phase because
			 * it is a sub part of the cleanup phase for VACUUM FULL case 
			 */ 
			if (!truncatePhase)
				vacstmt->appendonly_compaction_vacuum_cleanup = false;
		}

		/*
		 * For each iteration we start/commit our own transactions,
		 * so that we can release resources such as locks and memories,
		 * and we can also safely perform non-transactional work
		 * along with transactional work.
		 */
		StartTransactionCommand();

		/*
		 * Functions in indexes may want a snapshot set. Also, setting
		 * a snapshot ensures that RecentGlobalXmin is kept truly recent.
		 */
		PushActiveSnapshot(GetTransactionSnapshot());

		if (!(vacstmt->options & VACOPT_FULL))
		{
			/*
			 * PostgreSQL does this:
			 * During a lazy VACUUM we can set the PROC_IN_VACUUM flag, which lets other
			 * concurrent VACUUMs know that they can ignore this one while
			 * determining their OldestXmin.  (The reason we don't set it during a
			 * full VACUUM is exactly that we may have to run user- defined
			 * functions for functional indexes, and we want to make sure that if
			 * they use the snapshot set above, any tuples it requires can't get
			 * removed from other tables.  An index function that depends on the
			 * contents of other tables is arguably broken, but we won't break it
			 * here by violating transaction semantics.)
			 *
			 * GPDB doesn't use PROC_IN_VACUUM, as lazy vacuum for bitmap
			 * indexed tables performs reindex causing updates to pg_class
			 * tuples for index entries.
			 *
			 * We also set the VACUUM_FOR_WRAPAROUND flag, which is passed down
			 * by autovacuum; it's used to avoid cancelling a vacuum that was
			 * invoked in an emergency.
			 *
			 * Note: this flag remains set until CommitTransaction or
			 * AbortTransaction.  We don't want to clear it until we reset
			 * MyProc->xid/xmin, else OldestXmin might appear to go backwards,
			 * which is probably Not Good.
			 */
			LWLockAcquire(ProcArrayLock, LW_EXCLUSIVE);
#if 0 /* Upstream code not applicable to GPDB */
			MyProc->vacuumFlags |= PROC_IN_VACUUM;
#endif
			if (for_wraparound)
				MyProc->vacuumFlags |= PROC_VACUUM_FOR_WRAPAROUND;
			LWLockRelease(ProcArrayLock);
		}

		/*
		 * AO only: QE can tell drop phase here with dispatched vacstmt.
		 */
		if (Gp_role == GP_ROLE_EXECUTE)
			dropPhase = vacuumStatement_IsInAppendOnlyDropPhase(vacstmt);

		/*
		 * Open the relation with an appropriate lock, and check the permission.
		 */
		onerel = open_relation_and_check_permission(vacstmt, relid, RELKIND_RELATION, dropPhase);

		/*
		 * onerel can be NULL for the following cases:
		 * 1. If the user does not have the permissions to vacuum the table
		 * 2. For AO tables if the drop phase cannot be performed and should be skipped 
		 */
		if (onerel == NULL)
		{
			if ((Gp_role != GP_ROLE_EXECUTE) && dropPhase)
			{
				/*
				 * To ensure that vacuum decreases the age for appendonly
				 * tables even if drop phase is getting skipped, perform
				 * cleanup phase so that the relfrozenxid value is updated
				 * correctly in pg_class
				 */
				vacstmt->appendonly_compaction_vacuum_cleanup = true;
				dropPhase = false;
				/*
				 * Since the drop phase needs to be skipped, we need to
				 * deregister the segnos which were marked for drop in the
				 * compaction phase
				 */
				DeregisterSegnoForCompactionDrop(relid,
								vacstmt->appendonly_compaction_segno);
				onerel = open_relation_and_check_permission(vacstmt, relid, RELKIND_RELATION, false);
			}		

			if (onerel == NULL)
			{
				PopActiveSnapshot();
				CommitTransactionCommand();
				continue;
			}
		}

		vacuumStatement_AssignRelation(vacstmt, relid, relations);

		if (Gp_role != GP_ROLE_EXECUTE)
		{
			/*
			 * Keep things generated by this QD decision beyond a transaction.
			 */
			oldctx = MemoryContextSwitchTo(vac_context);
			if (RelationIsHeap(onerel))
			{
				/*
				 * We perform truncate in the second transaction, to avoid making
				 * it necessary to record transaction commit in the middle of
				 * vacuum operation in case we move tuples across pages.  It may
				 * not need to do so if the relation is clean, but the decision
				 * to perform truncate is segment-local and QD cannot tell if
				 * everyone can skip it.
				 */
				if (vacstmt->options & VACOPT_FULL)
				{
					Assert(relationRound == 0 || relationRound == 1);
					if (relationRound == 0)
						getnextrelation = false;
					else if (relationRound == 1)
						vacstmt->heap_truncate = true;
				}
			}
			else
			{
				/* the rest is about AO tables */
				if (vacstmt->appendonly_compaction_vacuum_prepare)
				{
					getnextrelation = false;
					dropPhase = false;
				}
				else if (dropPhase)
				{
					if (HasSerializableBackends(false))
					{
						/*
						 * Checking at this point is safe because
						 * any serializable transaction that could start afterwards
						 * will already see the state with AWAITING_DROP. We
						 * have only to deal with transactions that started before
						 * our transaction.
						 *
						 * We immediatelly get the next relation. There is no
						 * reason to stay in this relation. Actually, all
						 * other ao relation will skip the compaction step.
						 */
						elogif(Debug_appendonly_print_compaction, LOG,
								"Skipping freeing compacted append-only segment file "
								"because of concurrent serializable transaction");

						DeregisterSegnoForCompactionDrop(relid, vacstmt->appendonly_compaction_segno);
						vacstmt->appendonly_compaction_vacuum_cleanup = true;
						dropPhase = false;
						getnextrelation = false;
					}
					else
					{
						elogif(Debug_appendonly_print_compaction, LOG,
								"Dispatch drop transaction on append-only relation %s",
								RelationGetRelationName(onerel));

						RegisterSegnoForCompactionDrop(relid, vacstmt->appendonly_compaction_segno);
						list_free(vacstmt->appendonly_compaction_insert_segno);
						vacstmt->appendonly_compaction_insert_segno = NIL;
						dropPhase = false;
						getnextrelation = false;
					}
				}
				else
				{
					/* Either we are in cleanup or in compaction phase */
					if (!vacstmt->appendonly_compaction_vacuum_cleanup)
					{
						/* This block is only relevant for compaction */
						if (!vacuumStatement_AssignAppendOnlyCompactionInfo(vacstmt,
									onerel, compactedSegmentFileList,
									insertedSegmentFileList, &getnextrelation))
						{
							MemoryContextSwitchTo(oldctx);
							/* Nothing left to do for this relation */
							relation_close(onerel, NoLock);
							PopActiveSnapshot();
							CommitTransactionCommand();
							/* don't dispatch this iteration */
							continue;
						}

						compactedSegmentFileList =
							list_union_int(compactedSegmentFileList,
								vacstmt->appendonly_compaction_segno);
						insertedSegmentFileList =
							list_union_int(insertedSegmentFileList,
								vacstmt->appendonly_compaction_insert_segno);

						dropPhase = !getnextrelation;
					}
				}
				MemoryContextSwitchTo(oldctx);

				/*
				 * If VACUUM FULL and in cleanup phase, perform two-step for
				 * aux relations.
				 */
				if ((vacstmt->options & VACOPT_FULL) &&
					vacstmt->appendonly_compaction_vacuum_cleanup)
				{
					if (truncatePhase)
					{
						truncatePhase = false;
						vacstmt->heap_truncate = true;
					}
					else
					{
						truncatePhase = true;
						getnextrelation = false;
					}
				}
			}
		}

		/*
		 * If we are in the dispatch mode, dispatch this modified
		 * vacuum statement to QEs, and wait for them to finish.
		 */
		if (Gp_role == GP_ROLE_DISPATCH)
		{
			int 		i, nindexes;
			bool 		has_bitmap = false;
			Relation   *i_rel = NULL;

			stats_context.updated_stats = NIL;

			vac_open_indexes(onerel, AccessShareLock, &nindexes, &i_rel);
			if (i_rel != NULL)
			{
				for (i = 0; i < nindexes; i++)
				{
					if (RelationIsBitmapIndex(i_rel[i]))
					{
						has_bitmap = true;
						break;
					}
				}
			}
			vac_close_indexes(nindexes, i_rel, AccessShareLock);

			/*
			 * We have to acquire a ShareLock for the relation which has bitmap
			 * indexes, since reindex is used later. Otherwise, concurrent
			 * vacuum and inserts may cause deadlock. MPP-5960
			 */
			if (has_bitmap)
				LockRelation(onerel, ShareLock);

			dispatchVacuum(vacstmt, &stats_context);
		}

		if (vacstmt->options & VACOPT_FULL)
			lmode = AccessExclusiveLock;
		else if (RelationIsAoRows(onerel) || RelationIsAoCols(onerel))
			lmode = AccessShareLock;
		else
			lmode = ShareUpdateExclusiveLock;

		if (relationRound == 0)
		{
			onerelid = onerel->rd_lockInfo.lockRelId;

			/*
			 * Get a session-level lock too. This will protect our
			 * access to the relation across multiple transactions, so
			 * that we can vacuum the relation's TOAST table (if any)
			 * secure in the knowledge that no one is deleting the
			 * parent relation.
			 *
			 * NOTE: this cannot block, even if someone else is
			 * waiting for access, because the lock manager knows that
			 * both lock requests are from the same process.
			 */
			LockRelationIdForSession(&onerelid, lmode);
		}
		vacuum_rel(onerel, vacstmt, lmode, stats_context.updated_stats,
				   for_wraparound);

		if (Gp_role == GP_ROLE_DISPATCH)
		{
			list_free_deep(stats_context.updated_stats);
			stats_context.updated_stats = NIL;

			/*
			 * Update ao master tupcount the hard way after the compaction and
			 * after the drop.
			 */
			if (vacstmt->appendonly_compaction_segno)
			{
				Assert(RelationIsAoRows(onerel) || RelationIsAoCols(onerel));

				if (vacuumStatement_IsInAppendOnlyCompactionPhase(vacstmt) &&
						!vacuumStatement_IsInAppendOnlyPseudoCompactionPhase(vacstmt))
				{
					/* In the compact phase, we need to update the information of the segment file we inserted into */
					UpdateMasterAosegTotalsFromSegments(onerel, SnapshotNow, vacstmt->appendonly_compaction_insert_segno, 0);
				}
				else if (vacuumStatement_IsInAppendOnlyDropPhase(vacstmt))
				{
					/* In the drop phase, we need to update the information of the compacted segment file(s) */
					UpdateMasterAosegTotalsFromSegments(onerel, SnapshotNow, vacstmt->appendonly_compaction_segno, 0);
				}
			}

			/*
			 * We need some transaction to update the catalog.  We could do
			 * it on the outer vacuumStatement, but it is useful to track
			 * relation by relation.
			 */
			if (relationRound == 0 && !vacuumStatement_IsTemporary(onerel))
			{
				char *vsubtype = ""; /* NOFULL */

				if (IsAutoVacuumWorkerProcess())
					vsubtype = "AUTO";
				else
				{
					if ((vacstmt->options & VACOPT_FULL) &&
						(0 == vacstmt->freeze_min_age))
						vsubtype = "FULL FREEZE";
					else if ((vacstmt->options & VACOPT_FULL))
						vsubtype = "FULL";
					else if (0 == vacstmt->freeze_min_age)
						vsubtype = "FREEZE";
				}
				MetaTrackUpdObject(RelationRelationId,
								   relid,
								   GetUserId(),
								   "VACUUM",
								   vsubtype);
			}
		}

		/*
		 * Close source relation now, but keep lock so that no one
		 * deletes it before we commit.  (If someone did, they'd
		 * fail to clean up the entries we made in pg_statistic.
		 * Also, releasing the lock before commit would expose us
		 * to concurrent-update failures in update_attstats.)
		 */
		relation_close(onerel, NoLock);

		if (list_length(relations) > 1)
		{
			pfree(vacstmt->relation->schemaname);
			pfree(vacstmt->relation->relname);
			pfree(vacstmt->relation);
			vacstmt->relation = NULL;
		}
		vacstmt->appendonly_compaction_vacuum_prepare = false;

		PopActiveSnapshot();

		/*
		 * Transaction commit is always executed on QD.
		 */
		if (Gp_role != GP_ROLE_EXECUTE)
			CommitTransactionCommand();

		if (relationRound == 0)
		{
			SIMPLE_FAULT_INJECTOR(VacuumRelationEndOfFirstRound);
		}

		relationRound++;
	}

	if (lmode != NoLock)
	{
		UnlockRelationIdForSession(&onerelid, lmode);
	}

	if (compactedSegmentFileList)
	{
		list_free(compactedSegmentFileList);
		compactedSegmentFileList = NIL;
	}
	if (insertedSegmentFileList)
	{
		list_free(insertedSegmentFileList);
		insertedSegmentFileList = NIL;
	}
	if (vacstmt->appendonly_compaction_segno)
	{
		list_free(vacstmt->appendonly_compaction_segno);
		vacstmt->appendonly_compaction_segno = NIL;
	}
	if (vacstmt->appendonly_compaction_insert_segno)
	{
		list_free(vacstmt->appendonly_compaction_insert_segno);
		vacstmt->appendonly_compaction_insert_segno = NIL;
	}
}

/*
 * Build a list of Oids for each relation to be processed
 *
 * The list is built in vac_context so that it will survive across our
 * per-relation transactions.
 */
static List *
get_rel_oids(Oid relid, VacuumStmt *vacstmt, const char *stmttype)
{
	List	   *oid_list = NIL;
	MemoryContext oldcontext;

	/* OID supplied by VACUUM's caller? */
	if (OidIsValid(relid))
	{
		oldcontext = MemoryContextSwitchTo(vac_context);
		oid_list = lappend_oid(oid_list, relid);
		MemoryContextSwitchTo(oldcontext);
	}
	else if (vacstmt->relation)
	{
		if (strcmp(stmttype, "VACUUM") == 0)
		{
			/* Process a specific relation */
			Oid			relid;
			List	   *prels = NIL;

			relid = RangeVarGetRelid(vacstmt->relation, false);

			if (rel_is_partitioned(relid))
			{
				PartitionNode *pn;

				pn = get_parts(relid, 0, 0, false, true /*includesubparts*/);

				prels = all_partition_relids(pn);
			}
			else if (rel_is_child_partition(relid))
			{
				/* get my children */
				prels = find_all_inheritors(relid, NoLock, NULL);
			}

			/* Make a relation list entry for this relation */
			oldcontext = MemoryContextSwitchTo(vac_context);
			oid_list = lappend_oid(oid_list, relid);
			oid_list = list_concat_unique_oid(oid_list, prels);
			MemoryContextSwitchTo(oldcontext);
		}
		else
		{
			oldcontext = MemoryContextSwitchTo(vac_context);
			/**
			 * ANALYZE one relation (optionally, a list of columns).
			 */
			Oid relationOid = InvalidOid;

			relationOid = RangeVarGetRelid(vacstmt->relation, false);
			PartStatus ps = rel_part_status(relationOid);

			if (ps != PART_STATUS_ROOT && (vacstmt->options & VACOPT_ROOTONLY))
			{
				ereport(WARNING,
						(errmsg("skipping \"%s\" --- cannot analyze a non-root partition using ANALYZE ROOTPARTITION",
								get_rel_name(relationOid))));
			}
			else if (ps == PART_STATUS_ROOT)
			{
				PartitionNode *pn = get_parts(relationOid, 0 /*level*/ ,
											  0 /*parent*/, false /* inctemplate */, true /*includesubparts*/);
				Assert(pn);
				if (!(vacstmt->options & VACOPT_ROOTONLY))
				{
					oid_list = all_leaf_partition_relids(pn); /* all leaves */

					if (optimizer_analyze_midlevel_partition)
					{
						oid_list = list_concat(oid_list, all_interior_partition_relids(pn)); /* interior partitions */
					}
				}
				oid_list = lappend_oid(oid_list, relationOid); /* root partition */
			}
			else if (ps == PART_STATUS_INTERIOR) /* analyze an interior partition directly */
			{
				/* disable analyzing mid-level partitions directly since the users are encouraged
				 * to work with the root partition only. To gather stats on mid-level partitions
				 * (for Orca's use), the user should run ANALYZE or ANALYZE ROOTPARTITION on the
				 * root level with optimizer_analyze_midlevel_partition GUC set to ON.
				 * Planner uses the stats on leaf partitions, so its unnecesary to collect stats on
				 * midlevel partitions.
				 */
				ereport(WARNING,
						(errmsg("skipping \"%s\" --- cannot analyze a mid-level partition. "
								"Please run ANALYZE on the root partition table.",
								get_rel_name(relationOid))));
			}
			else
			{
				oid_list = list_make1_oid(relationOid);
			}
			MemoryContextSwitchTo(oldcontext);
		}
	}
	else
	{
		/* Process all plain relations listed in pg_class */
		Relation	pgclass;
		HeapScanDesc scan;
		HeapTuple	tuple;
		ScanKeyData key;
		Oid candidateOid;

		ScanKeyInit(&key,
					Anum_pg_class_relkind,
					BTEqualStrategyNumber, F_CHAREQ,
					CharGetDatum(RELKIND_RELATION));

		pgclass = heap_open(RelationRelationId, AccessShareLock);

		scan = heap_beginscan(pgclass, SnapshotNow, 1, &key);

		while ((tuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
		{
			Form_pg_class classForm = (Form_pg_class) GETSTRUCT(tuple);

			/*
			 * Don't include non-vacuum-able relations:
			 *   - External tables
			 *   - Foreign tables
			 *   - etc.
			 */
			if (classForm->relkind == RELKIND_RELATION && (
					classForm->relstorage == RELSTORAGE_EXTERNAL ||
					classForm->relstorage == RELSTORAGE_FOREIGN  ||
					classForm->relstorage == RELSTORAGE_VIRTUAL))
				continue;

			/* Make a relation list entry for this guy */
			candidateOid = HeapTupleGetOid(tuple);

			/* Skip non root partition tables if ANALYZE ROOTPARTITION ALL is executed */
			if ((vacstmt->options & VACOPT_ROOTONLY) && !rel_is_partitioned(candidateOid))
			{
				continue;
			}

			// skip mid-level partition tables if we have disabled collecting statistics for them
			PartStatus ps = rel_part_status(candidateOid);
			if (!optimizer_analyze_midlevel_partition && ps == PART_STATUS_INTERIOR)
			{
				continue;
			}

			oldcontext = MemoryContextSwitchTo(vac_context);
			oid_list = lappend_oid(oid_list, candidateOid);
			MemoryContextSwitchTo(oldcontext);
		}

		heap_endscan(scan);
		heap_close(pgclass, AccessShareLock);
	}

	return oid_list;
}

/*
 * vacuum_set_xid_limits() -- compute oldest-Xmin and freeze cutoff points
 */
void
vacuum_set_xid_limits(int freeze_min_age,
					  int freeze_table_age,
					  bool sharedRel,
					  TransactionId *oldestXmin,
					  TransactionId *freezeLimit,
					  TransactionId *freezeTableLimit)
{
	int			freezemin;
	TransactionId limit;
	TransactionId safeLimit;

	/*
	 * We can always ignore processes running lazy vacuum.	This is because we
	 * use these values only for deciding which tuples we must keep in the
	 * tables.	Since lazy vacuum doesn't write its XID anywhere, it's safe to
	 * ignore it.  In theory it could be problematic to ignore lazy vacuums in
	 * a full vacuum, but keep in mind that only one vacuum process can be
	 * working on a particular table at any time, and that each vacuum is
	 * always an independent transaction.
	 */
	*oldestXmin = GetOldestXmin(sharedRel, true);

	Assert(TransactionIdIsNormal(*oldestXmin));

	/*
	 * Determine the minimum freeze age to use: as specified by the caller, or
	 * vacuum_freeze_min_age, but in any case not more than half
	 * autovacuum_freeze_max_age, so that autovacuums to prevent XID
	 * wraparound won't occur too frequently.
	 */
	freezemin = freeze_min_age;
	if (freezemin < 0)
		freezemin = vacuum_freeze_min_age;
	freezemin = Min(freezemin, autovacuum_freeze_max_age / 2);
	Assert(freezemin >= 0);

	/*
	 * Compute the cutoff XID, being careful not to generate a "permanent" XID
	 */
	limit = *oldestXmin - freezemin;
	if (!TransactionIdIsNormal(limit))
		limit = FirstNormalTransactionId;

	/*
	 * If oldestXmin is very far back (in practice, more than
	 * autovacuum_freeze_max_age / 2 XIDs old), complain and force a minimum
	 * freeze age of zero.
	 */
	safeLimit = ReadNewTransactionId() - autovacuum_freeze_max_age;
	if (!TransactionIdIsNormal(safeLimit))
		safeLimit = FirstNormalTransactionId;

	if (TransactionIdPrecedes(limit, safeLimit))
	{
		ereport(WARNING,
				(errmsg("oldest xmin is far in the past"),
				 errhint("Close open transactions soon to avoid wraparound problems.")));
		limit = *oldestXmin;
	}

	*freezeLimit = limit;

	if (freezeTableLimit != NULL)
	{
		int			freezetable;

		/*
		 * Determine the table freeze age to use: as specified by the caller,
		 * or vacuum_freeze_table_age, but in any case not more than
		 * autovacuum_freeze_max_age * 0.95, so that if you have e.g nightly
		 * VACUUM schedule, the nightly VACUUM gets a chance to freeze tuples
		 * before anti-wraparound autovacuum is launched.
		 */
		freezetable = freeze_min_age;
		if (freezetable < 0)
			freezetable = vacuum_freeze_table_age;
		freezetable = Min(freezetable, autovacuum_freeze_max_age * 0.95);
		Assert(freezetable >= 0);

		/*
		 * Compute the cutoff XID, being careful not to generate a "permanent"
		 * XID.
		 */
		limit = ReadNewTransactionId() - freezetable;
		if (!TransactionIdIsNormal(limit))
			limit = FirstNormalTransactionId;

		*freezeTableLimit = limit;
	}
}


/*
 * vac_estimate_reltuples() -- estimate the new value for pg_class.reltuples
 *
 *		If we scanned the whole relation then we should just use the count of
 *		live tuples seen; but if we did not, we should not trust the count
 *		unreservedly, especially not in VACUUM, which may have scanned a quite
 *		nonrandom subset of the table.  When we have only partial information,
 *		we take the old value of pg_class.reltuples as a measurement of the
 *		tuple density in the unscanned pages.
 *
 *		This routine is shared by VACUUM and ANALYZE.
 */
double
vac_estimate_reltuples(Relation relation, bool is_analyze,
					   BlockNumber total_pages,
					   BlockNumber scanned_pages,
					   double scanned_tuples)
{
	BlockNumber	old_rel_pages = relation->rd_rel->relpages;
	double		old_rel_tuples = relation->rd_rel->reltuples;
	double		old_density;
	double		new_density;
	double		multiplier;
	double		updated_density;

	/* If we did scan the whole table, just use the count as-is */
	if (scanned_pages >= total_pages)
		return scanned_tuples;

	/*
	 * If scanned_pages is zero but total_pages isn't, keep the existing value
	 * of reltuples.  (Note: callers should avoid updating the pg_class
	 * statistics in this situation, since no new information has been
	 * provided.)
	 */
	if (scanned_pages == 0)
		return old_rel_tuples;

	/*
	 * If old value of relpages is zero, old density is indeterminate; we
	 * can't do much except scale up scanned_tuples to match total_pages.
	 */
	if (old_rel_pages == 0)
		return floor((scanned_tuples / scanned_pages) * total_pages + 0.5);

	/*
	 * Okay, we've covered the corner cases.  The normal calculation is to
	 * convert the old measurement to a density (tuples per page), then
	 * update the density using an exponential-moving-average approach,
	 * and finally compute reltuples as updated_density * total_pages.
	 *
	 * For ANALYZE, the moving average multiplier is just the fraction of
	 * the table's pages we scanned.  This is equivalent to assuming
	 * that the tuple density in the unscanned pages didn't change.  Of
	 * course, it probably did, if the new density measurement is different.
	 * But over repeated cycles, the value of reltuples will converge towards
	 * the correct value, if repeated measurements show the same new density.
	 *
	 * For VACUUM, the situation is a bit different: we have looked at a
	 * nonrandom sample of pages, but we know for certain that the pages we
	 * didn't look at are precisely the ones that haven't changed lately.
	 * Thus, there is a reasonable argument for doing exactly the same thing
	 * as for the ANALYZE case, that is use the old density measurement as
	 * the value for the unscanned pages.
	 *
	 * This logic could probably use further refinement.
	 */
	old_density = old_rel_tuples / old_rel_pages;
	new_density = scanned_tuples / scanned_pages;
	multiplier = (double) scanned_pages / (double) total_pages;
	updated_density = old_density + (new_density - old_density) * multiplier;
	return floor(updated_density * total_pages + 0.5);
}


void
vac_update_relstats_from_list(Relation rel,
							  BlockNumber num_pages, double num_tuples,
							  bool hasindex, TransactionId frozenxid,
							  List *updated_stats)
{
	/*
	 * If this is QD, use the stats collected in updated_stats instead of
	 * the one provided through 'num_pages' and 'num_tuples'.  It doesn't
	 * seem worth doing so for system tables, though (it'd better say
	 * "non-distributed" tables than system relations here, but for now
	 * it's effectively the same.)
	 */
	if (Gp_role == GP_ROLE_DISPATCH && !IsSystemRelation(rel))
	{
		ListCell *lc;
		num_pages = 0;
		num_tuples = 0.0;
		foreach (lc, updated_stats)
		{
			VPgClassStats *stats = (VPgClassStats *) lfirst(lc);
			if (stats->relid == RelationGetRelid(rel))
			{
				num_pages += stats->rel_pages;
				num_tuples += stats->rel_tuples;
				break;
			}
		}
	}

	vac_update_relstats(rel, num_pages, num_tuples,
						hasindex, frozenxid);
}

/*
 *	vac_update_relstats() -- update statistics for one relation
 *
 *		Update the whole-relation statistics that are kept in its pg_class
 *		row.  There are additional stats that will be updated if we are
 *		doing ANALYZE, but we always update these stats.  This routine works
 *		for both index and heap relation entries in pg_class.
 *
 *		We violate transaction semantics here by overwriting the rel's
 *		existing pg_class tuple with the new values.  This is reasonably
 *		safe since the new values are correct whether or not this transaction
 *		commits.  The reason for this is that if we updated these tuples in
 *		the usual way, vacuuming pg_class itself wouldn't work very well ---
 *		by the time we got done with a vacuum cycle, most of the tuples in
 *		pg_class would've been obsoleted.  Of course, this only works for
 *		fixed-size never-null columns, but these are.
 *
 *		Another reason for doing it this way is that when we are in a lazy
 *		VACUUM and have PROC_IN_VACUUM set, we mustn't do any updates ---
 *		somebody vacuuming pg_class might think they could delete a tuple
 *		marked with xmin = our xid.
 *
 *		Note another assumption: that two VACUUMs/ANALYZEs on a table can't
 *		run in parallel, nor can VACUUM/ANALYZE run in parallel with a
 *		schema alteration such as adding an index, rule, or trigger.  Otherwise
 *		our updates of relhasindex etc might overwrite uncommitted updates.
 *
 *		This routine is shared by VACUUM and stand-alone ANALYZE.
 */
void
vac_update_relstats(Relation relation,
					BlockNumber num_pages, double num_tuples,
					bool hasindex, TransactionId frozenxid)
{
	Oid			relid = RelationGetRelid(relation);
	Relation	rd;
	HeapTuple	ctup;
	Form_pg_class pgcform;
	bool		dirty;

	Assert(relid != InvalidOid);

	/*
	 * CDB: send the number of tuples and the number of pages in pg_class located
	 * at QEs through the dispatcher.
	 */
	if (Gp_role == GP_ROLE_EXECUTE)
	{
		/* cdbanalyze_get_relstats(rel, &num_pages, &num_tuples);*/
		StringInfoData buf;
		VPgClassStats stats;

		pq_beginmessage(&buf, 'y');
		pq_sendstring(&buf, "VACUUM");
		stats.relid = relid;
		stats.rel_pages = num_pages;
		stats.rel_tuples = num_tuples;
		stats.empty_end_pages = 0;
		pq_sendint(&buf, sizeof(VPgClassStats), sizeof(int));
		pq_sendbytes(&buf, (char *) &stats, sizeof(VPgClassStats));
		pq_endmessage(&buf);
	}

	/*
	 * We need a way to distinguish these 2 cases:
	 * a) ANALYZEd/VACUUMed table is empty
	 * b) Table has never been ANALYZEd/VACUUMed
	 * To do this, in case (a), we set relPages = 1. For case (b), relPages = 0.
	 */
	if (num_pages < 1.0)
	{
		Assert(num_tuples < 1.0);
		num_pages = 1.0;
	}

	/*
	 * update number of tuples and number of pages in pg_class
	 */
	rd = heap_open(RelationRelationId, RowExclusiveLock);

	/* Fetch a copy of the tuple to scribble on */
	ctup = SearchSysCacheCopy1(RELOID, ObjectIdGetDatum(relid));
	if (!HeapTupleIsValid(ctup))
		elog(ERROR, "pg_class entry for relid %u vanished during vacuuming",
			 relid);
	pgcform = (Form_pg_class) GETSTRUCT(ctup);

	/* Apply required updates, if any, to copied tuple */

	dirty = false;
	if (pgcform->relpages != (int32) num_pages)
	{
		pgcform->relpages = (int32) num_pages;
		dirty = true;
	}
	if (pgcform->reltuples != (float4) num_tuples)
	{
		pgcform->reltuples = (float4) num_tuples;
		dirty = true;
	}
	if (pgcform->relhasindex != hasindex)
	{
		pgcform->relhasindex = hasindex;
		dirty = true;
	}

	elog(DEBUG2, "Vacuum oid=%u pages=%d tuples=%f",
		 relid, pgcform->relpages, pgcform->reltuples);
	/*
	 * If we have discovered that there are no indexes, then there's no
	 * primary key either, nor any exclusion constraints.  This could be done
	 * more thoroughly...
	 */
	if (!hasindex)
	{
		if (pgcform->relhaspkey)
		{
			pgcform->relhaspkey = false;
			dirty = true;
		}
		if (pgcform->relhasexclusion && pgcform->relkind != RELKIND_INDEX)
		{
			pgcform->relhasexclusion = false;
			dirty = true;
		}
	}

	/* We also clear relhasrules and relhastriggers if needed */
	if (pgcform->relhasrules && relation->rd_rules == NULL)
	{
		pgcform->relhasrules = false;
		dirty = true;
	}
	if (pgcform->relhastriggers && relation->trigdesc == NULL)
	{
		pgcform->relhastriggers = false;
		dirty = true;
	}

	/*
	 * relfrozenxid should never go backward.  Caller can pass
	 * InvalidTransactionId if it has no new data.
	 */
	if (TransactionIdIsNormal(frozenxid) &&
		TransactionIdIsValid(pgcform->relfrozenxid) &&
		TransactionIdPrecedes(pgcform->relfrozenxid, frozenxid))
	{
		pgcform->relfrozenxid = frozenxid;
		dirty = true;
	}

	/* If anything changed, write out the tuple. */
	if (dirty)
		heap_inplace_update(rd, ctup);

	heap_close(rd, RowExclusiveLock);
}


/*
 *	vac_update_datfrozenxid() -- update pg_database.datfrozenxid for our DB
 *
 *		Update pg_database's datfrozenxid entry for our database to be the
 *		minimum of the pg_class.relfrozenxid values.  If we are able to
 *		advance pg_database.datfrozenxid, also try to truncate pg_clog.
 *
 *		We violate transaction semantics here by overwriting the database's
 *		existing pg_database tuple with the new value.	This is reasonably
 *		safe since the new value is correct whether or not this transaction
 *		commits.  As with vac_update_relstats, this avoids leaving dead tuples
 *		behind after a VACUUM.
 */
void
vac_update_datfrozenxid(void)
{
	HeapTuple	tuple;
	Form_pg_database dbform;
	Relation	relation;
	SysScanDesc scan;
	HeapTuple	classTup;
	TransactionId newFrozenXid;
	bool		dirty = false;

	/*
	 * Initialize the "min" calculation with GetOldestXmin, which is a
	 * reasonable approximation to the minimum relfrozenxid for not-yet-
	 * committed pg_class entries for new tables; see AddNewRelationTuple().
	 * Se we cannot produce a wrong minimum by starting with this.
	 */
	newFrozenXid = GetOldestXmin(true, true);

	/*
	 * We must seqscan pg_class to find the minimum Xid, because there is no
	 * index that can help us here.
	 */
	relation = heap_open(RelationRelationId, AccessShareLock);

	scan = systable_beginscan(relation, InvalidOid, false,
							  SnapshotNow, 0, NULL);

	while ((classTup = systable_getnext(scan)) != NULL)
	{
		Form_pg_class classForm = (Form_pg_class) GETSTRUCT(classTup);

		if (!should_have_valid_relfrozenxid(HeapTupleGetOid(classTup),
											classForm->relkind,
											classForm->relstorage))
		{
			Assert(!TransactionIdIsValid(classForm->relfrozenxid));
			continue;
		}

		Assert(TransactionIdIsNormal(classForm->relfrozenxid));

		if (TransactionIdPrecedes(classForm->relfrozenxid, newFrozenXid))
			newFrozenXid = classForm->relfrozenxid;
	}

	/* we're done with pg_class */
	systable_endscan(scan);
	heap_close(relation, AccessShareLock);

	Assert(TransactionIdIsNormal(newFrozenXid));

	/* Now fetch the pg_database tuple we need to update. */
	relation = heap_open(DatabaseRelationId, RowExclusiveLock);

	/* Fetch a copy of the tuple to scribble on */
	tuple = SearchSysCacheCopy1(DATABASEOID, ObjectIdGetDatum(MyDatabaseId));
	if (!HeapTupleIsValid(tuple))
		elog(ERROR, "could not find tuple for database %u", MyDatabaseId);
	dbform = (Form_pg_database) GETSTRUCT(tuple);

	/*
	 * Don't allow datfrozenxid to go backward (probably can't happen anyway);
	 * and detect the common case where it doesn't go forward either.
	 */
	if (TransactionIdPrecedes(dbform->datfrozenxid, newFrozenXid))
	{
		dbform->datfrozenxid = newFrozenXid;
		dirty = true;
	}

	if (dirty)
		heap_inplace_update(relation, tuple);

	heap_freetuple(tuple);
	heap_close(relation, RowExclusiveLock);

	/*
	 * If we were able to advance datfrozenxid, see if we can truncate
	 * pg_clog. Also do it if the shared XID-wrap-limit info is stale, since
	 * this action will update that too.
	 */
	if (dirty || ForceTransactionIdLimitUpdate())
		vac_truncate_clog(newFrozenXid);
}


/*
 *	vac_truncate_clog() -- attempt to truncate the commit log
 *
 *		Scan pg_database to determine the system-wide oldest datfrozenxid,
 *		and use it to truncate the transaction commit log (pg_clog).
 *		Also update the XID wrap limit info maintained by varsup.c.
 *
 *		The passed XID is simply the one I just wrote into my pg_database
 *		entry.	It's used to initialize the "min" calculation.
 *
 *		This routine is only invoked when we've managed to change our
 *		DB's datfrozenxid entry, or we found that the shared XID-wrap-limit
 *		info is stale.
 */
static void
vac_truncate_clog(TransactionId frozenXID)
{
	TransactionId myXID = GetCurrentTransactionId();
	Relation	relation;
	HeapScanDesc scan;
	HeapTuple	tuple;
	Oid			oldest_datoid;
	bool		frozenAlreadyWrapped = false;

	/* init oldest_datoid to sync with my frozenXID */
	oldest_datoid = MyDatabaseId;

	/*
	 * Scan pg_database to compute the minimum datfrozenxid
	 *
	 * Note: we need not worry about a race condition with new entries being
	 * inserted by CREATE DATABASE.  Any such entry will have a copy of some
	 * existing DB's datfrozenxid, and that source DB cannot be ours because
	 * of the interlock against copying a DB containing an active backend.
	 * Hence the new entry will not reduce the minimum.  Also, if two VACUUMs
	 * concurrently modify the datfrozenxid's of different databases, the
	 * worst possible outcome is that pg_clog is not truncated as aggressively
	 * as it could be.
	 */
	relation = heap_open(DatabaseRelationId, AccessShareLock);

	scan = heap_beginscan(relation, SnapshotNow, 0, NULL);

	while ((tuple = heap_getnext(scan, ForwardScanDirection)) != NULL)
	{
		Form_pg_database dbform = (Form_pg_database) GETSTRUCT(tuple);

		Assert(TransactionIdIsNormal(dbform->datfrozenxid));

		/*
		 * MPP-20053: Skip databases that cannot be connected to in computing
		 * the oldest database.
		 */
		if (dbform->datallowconn)
		{
			if (TransactionIdPrecedes(myXID, dbform->datfrozenxid))
				frozenAlreadyWrapped = true;
			else if (TransactionIdPrecedes(dbform->datfrozenxid, frozenXID))
			{
				frozenXID = dbform->datfrozenxid;
				oldest_datoid = HeapTupleGetOid(tuple);
			}
		}
	}

	heap_endscan(scan);

	heap_close(relation, AccessShareLock);

	/*
	 * Do not truncate CLOG if we seem to have suffered wraparound already;
	 * the computed minimum XID might be bogus.  This case should now be
	 * impossible due to the defenses in GetNewTransactionId, but we keep the
	 * test anyway.
	 */
	if (frozenAlreadyWrapped)
	{
		ereport(WARNING,
				(errmsg("some databases have not been vacuumed in over 2 billion transactions"),
				 errdetail("You might have already suffered transaction-wraparound data loss.")));
		return;
	}

	/* Truncate CLOG to the oldest frozenxid */
	TruncateCLOG(frozenXID);
	DistributedLog_Truncate(frozenXID);

	/*
	 * Update the wrap limit for GetNewTransactionId.  Note: this function
	 * will also signal the postmaster for an(other) autovac cycle if needed.
	 */
	SetTransactionIdLimit(frozenXID, oldest_datoid);
}


/*
 *	vacuum_rel() -- vacuum one heap relation
 *
 *		Doing one heap at a time incurs extra overhead, since we need to
 *		check that the heap exists again just before we vacuum it.	The
 *		reason that we do this is so that vacuuming can be spread across
 *		many small transactions.  Otherwise, two-phase locking would require
 *		us to lock the entire database during one pass of the vacuum cleaner.
 */
static void
vacuum_rel(Relation onerel, VacuumStmt *vacstmt, LOCKMODE lmode, List *updated_stats,
		   bool for_wraparound)
{
	Oid			toast_relid;
	Oid			aoseg_relid = InvalidOid;
	Oid         aoblkdir_relid = InvalidOid;
	Oid         aovisimap_relid = InvalidOid;
	Oid			save_userid;
	int			save_sec_context;
	int			save_nestlevel;

	/*
	 * Check for user-requested abort.	Note we want this to be inside a
	 * transaction, so xact.c doesn't issue useless WARNING.
	 */
	CHECK_FOR_INTERRUPTS();

	/*
	 * Remember the relation's TOAST and AO segments relations for later
	 */
	toast_relid = onerel->rd_rel->reltoastrelid;
	if (RelationIsAoRows(onerel) || RelationIsAoCols(onerel))
	{
		GetAppendOnlyEntryAuxOids(RelationGetRelid(onerel), SnapshotNow,
								  &aoseg_relid,
								  &aoblkdir_relid, NULL,
								  &aovisimap_relid, NULL);
		vacstmt->appendonly_relation_empty =
				AppendOnlyCompaction_IsRelationEmpty(onerel);
	}

	/*
	 * Switch to the table owner's userid, so that any index functions are run
	 * as that user.  Also lock down security-restricted operations and
	 * arrange to make GUC variable changes local to this command. (This is
	 * unnecessary, but harmless, for lazy VACUUM.)
	 */
	GetUserIdAndSecContext(&save_userid, &save_sec_context);
	SetUserIdAndSecContext(onerel->rd_rel->relowner,
						   save_sec_context | SECURITY_RESTRICTED_OPERATION);
	save_nestlevel = NewGUCNestLevel();

	/*
	 * Do the actual work --- either FULL or "lazy" vacuum
	 */
	if (vacstmt->options & VACOPT_FULL)
	{
		Oid			relid = RelationGetRelid(onerel);

		/* close relation before vacuuming, but hold lock until commit */
		relation_close(onerel, NoLock);
		onerel = NULL;

		/* VACUUM FULL is now a variant of CLUSTER; see cluster.c */
		cluster_rel(relid, InvalidOid, false, true /* printError */,
					(vacstmt->options & VACOPT_VERBOSE) != 0,
					vacstmt->freeze_min_age, vacstmt->freeze_table_age);
	}
	else
		lazy_vacuum_rel(onerel, vacstmt, vac_strategy, updated_stats);

	/* Roll back any GUC changes executed by index functions */
	AtEOXact_GUC(false, save_nestlevel);

	/* Restore userid and security context */
	SetUserIdAndSecContext(save_userid, save_sec_context);

	/* Don't close the relation in GPDB yet! The caller still needs it. */
#if 0
	/* all done with this class, but hold lock until commit */
	if (onerel)
		relation_close(onerel, NoLock);

	/*
	 * Complete the transaction and free all temporary memory used.
	 */
	PopActiveSnapshot();
	CommitTransactionCommand();
#endif

	/*
	 * If the relation has a secondary toast rel, vacuum that too while we
	 * still hold the session lock on the master table.  We do this in
	 * cleanup phase when it's AO table or in prepare phase if it's an
	 * empty AO table.
	 */
	if ((RelationIsHeap(onerel) && toast_relid != InvalidOid) ||
		(!RelationIsHeap(onerel) && (
				vacstmt->appendonly_compaction_vacuum_cleanup ||
				vacstmt->appendonly_relation_empty)))
	{
		Relation toast_rel = open_relation_and_check_permission(vacstmt, toast_relid,
																RELKIND_TOASTVALUE, false);
		if (toast_rel != NULL)
		{
			vacuum_rel(toast_rel, vacstmt, lmode, updated_stats, for_wraparound);

			/* all done with this class, but hold lock until commit */
			relation_close(toast_rel, NoLock);
		}
	}

	/*
	 * If an AO/CO table is empty on a segment,
	 * vacstmt->appendonly_relation_empty will get set to true even in the
	 * compaction phase. In such a case, we end up updating the auxiliary
	 * tables and try to vacuum them all in the same transaction. This causes
	 * the auxiliary relation to not get vacuumed and it generates a notice to
	 * the user saying that transaction is already in progress. Hence we want
	 * to vacuum the auxliary relations only in cleanup phase or if we are in
	 * the prepare phase and the AO/CO table is empty.
	 */
	if (vacstmt->appendonly_compaction_vacuum_cleanup ||
		(vacstmt->appendonly_relation_empty && vacstmt->appendonly_compaction_vacuum_prepare))
	{
		/* do the same for an AO segments table, if any */
		if (aoseg_relid != InvalidOid)
		{
			Relation aoseg_rel = open_relation_and_check_permission(vacstmt, aoseg_relid,
																	RELKIND_AOSEGMENTS, false);
			if (aoseg_rel != NULL)
			{
				vacuum_rel(aoseg_rel, vacstmt, lmode, updated_stats, for_wraparound);

				/* all done with this class, but hold lock until commit */
				relation_close(aoseg_rel, NoLock);
			}
		}

		/* do the same for an AO block directory table, if any */
		if (aoblkdir_relid != InvalidOid)
		{
			Relation aoblkdir_rel = open_relation_and_check_permission(vacstmt, aoblkdir_relid,
																	   RELKIND_AOBLOCKDIR, false);
			if (aoblkdir_rel != NULL)
			{
				vacuum_rel(aoblkdir_rel, vacstmt, lmode, updated_stats, for_wraparound);

				/* all done with this class, but hold lock until commit */
				relation_close(aoblkdir_rel, NoLock);
			}
		}

		/* do the same for an AO visimap, if any */
		if (aovisimap_relid != InvalidOid)
		{
			Relation aovisimap_rel = open_relation_and_check_permission(vacstmt, aovisimap_relid,
																	   RELKIND_AOVISIMAP, false);
			if (aovisimap_rel != NULL)
			{
				vacuum_rel(aovisimap_rel, vacstmt, lmode, updated_stats, for_wraparound);

				/* all done with this class, but hold lock until commit */
				relation_close(aovisimap_rel, NoLock);
			}
		}
	}
}


/****************************************************************************
 *																			*
 *			Code for VACUUM FULL (only)										*
 *																			*
 ****************************************************************************
 */

static bool vacuum_appendonly_index_should_vacuum(Relation aoRelation,
		VacuumStmt *vacstmt,
		AppendOnlyIndexVacuumState *vacuumIndexState, double *rel_tuple_count)
{
	int64 hidden_tupcount;
	FileSegTotals *totals;

	Assert(RelationIsAoRows(aoRelation) || RelationIsAoCols(aoRelation));

	if(Gp_role == GP_ROLE_DISPATCH)
	{
		if (rel_tuple_count)
		{
			*rel_tuple_count = 0.0;
		}
		return false;
	}

	if(RelationIsAoRows(aoRelation))
	{
		totals = GetSegFilesTotals(aoRelation, SnapshotNow);
	}
	else
	{
		Assert(RelationIsAoCols(aoRelation));
		totals = GetAOCSSSegFilesTotals(aoRelation, SnapshotNow);
	}
	hidden_tupcount = AppendOnlyVisimap_GetRelationHiddenTupleCount(&vacuumIndexState->visiMap);

	if(rel_tuple_count)
	{
		*rel_tuple_count = (double)(totals->totaltuples - hidden_tupcount);
		Assert((*rel_tuple_count) > -1.0);
	}

	pfree(totals);

	if(hidden_tupcount > 0 || (vacstmt->options & VACOPT_FULL))
	{
		return true;
	}
	return false;
}

/*
 * vacuum_appendonly_indexes()
 *
 * Perform a vacuum on all indexes of an append-only relation.
 *
 * The page and tuplecount information in vacrelstats are used, the
 * nindex value is set by this function.
 *
 * It returns the number of indexes on the relation.
 */
int
vacuum_appendonly_indexes(Relation aoRelation,
		VacuumStmt *vacstmt,
		List* updated_stats)
{
	int reindex_count = 1;
	int i;
	Relation   *Irel;
	int			nindexes;
	AppendOnlyIndexVacuumState vacuumIndexState;
	FileSegInfo **segmentFileInfo = NULL; /* Might be a casted AOCSFileSegInfo */
	int totalSegfiles;

	Assert(RelationIsAoRows(aoRelation) || RelationIsAoCols(aoRelation));
	Assert(vacstmt);

	memset(&vacuumIndexState, 0, sizeof(vacuumIndexState));

	elogif (Debug_appendonly_print_compaction, LOG,
			"Vacuum indexes for append-only relation %s",
			RelationGetRelationName(aoRelation));

	/* Now open all indexes of the relation */
	if ((vacstmt->options & VACOPT_FULL))
		vac_open_indexes(aoRelation, AccessExclusiveLock, &nindexes, &Irel);
	else
		vac_open_indexes(aoRelation, RowExclusiveLock, &nindexes, &Irel);

	if (RelationIsAoRows(aoRelation))
	{
		segmentFileInfo = GetAllFileSegInfo(aoRelation, SnapshotNow, &totalSegfiles);
	}
	else
	{
		Assert(RelationIsAoCols(aoRelation));
		segmentFileInfo = (FileSegInfo **)GetAllAOCSFileSegInfo(aoRelation, SnapshotNow, &totalSegfiles);
	}

	AppendOnlyVisimap_Init(
			&vacuumIndexState.visiMap,
			aoRelation->rd_appendonly->visimaprelid,
			aoRelation->rd_appendonly->visimapidxid,
			AccessShareLock,
			SnapshotNow);

	AppendOnlyBlockDirectory_Init_forSearch(&vacuumIndexState.blockDirectory,
			SnapshotNow,
			segmentFileInfo,
			totalSegfiles,
			aoRelation,
			1,
			RelationIsAoCols(aoRelation),
			NULL);

	/* Clean/scan index relation(s) */
	if (Irel != NULL)
	{
		double rel_tuple_count = 0.0;
		int			elevel;

		/* just scan indexes to update statistic */
		if (vacstmt->options & VACOPT_VERBOSE)
			elevel = INFO;
		else
			elevel = DEBUG2;

		if (vacuum_appendonly_index_should_vacuum(aoRelation, vacstmt,
					&vacuumIndexState, &rel_tuple_count))
		{
			Assert(rel_tuple_count > -1.0);

			for (i = 0; i < nindexes; i++)
			{
				vacuum_appendonly_index(Irel[i], &vacuumIndexState, updated_stats,
										rel_tuple_count, (vacstmt->options & VACOPT_FULL),
					elevel);
			}
			reindex_count++;
		}
		else
		{
			for (i = 0; i < nindexes; i++)
				scan_index(Irel[i], rel_tuple_count, updated_stats, true, elevel);
		}
	}

	AppendOnlyVisimap_Finish(&vacuumIndexState.visiMap, AccessShareLock);
	AppendOnlyBlockDirectory_End_forSearch(&vacuumIndexState.blockDirectory);

	if (segmentFileInfo)
	{
		if (RelationIsAoRows(aoRelation))
		{
			FreeAllSegFileInfo(segmentFileInfo, totalSegfiles);
		}
		else
		{
			FreeAllAOCSSegFileInfo((AOCSFileSegInfo **)segmentFileInfo, totalSegfiles);
		}
		pfree(segmentFileInfo);
	}

	vac_close_indexes(nindexes, Irel, NoLock);
	return nindexes;
}


/*
 * Is an index partial (ie, could it contain fewer tuples than the heap?)
 */
static bool
vac_is_partial_index(Relation indrel)
{
	/*
	 * If the index's AM doesn't support nulls, it's partial for our purposes
	 */
	if (!indrel->rd_am->amindexnulls)
		return true;

	/* Otherwise, look to see if there's a partial-index predicate */
	if (!heap_attisnull(indrel->rd_indextuple, Anum_pg_index_indpred))
		return true;

	return false;
}

/*
 *	scan_index() -- scan one index relation to update pg_class statistics.
 *
 * We use this when we have no deletions to do.
 */
static void
scan_index(Relation indrel, double num_tuples, List *updated_stats, bool check_stats, int elevel)
{
	IndexBulkDeleteResult *stats;
	IndexVacuumInfo ivinfo;
	PGRUsage	ru0;

	pg_rusage_init(&ru0);

	ivinfo.index = indrel;
	ivinfo.analyze_only = false;
	ivinfo.estimated_count = false;
	ivinfo.message_level = elevel;
	ivinfo.num_heap_tuples = num_tuples;
	ivinfo.strategy = vac_strategy;

	stats = index_vacuum_cleanup(&ivinfo, NULL);

	if (!stats)
		return;

	/*
	 * Now update statistics in pg_class, but only if the index says the count
	 * is accurate.
	 */
	if (!stats->estimated_count)
		vac_update_relstats_from_list(indrel,
							stats->num_pages, stats->num_index_tuples,
							false, InvalidTransactionId, updated_stats);

	ereport(elevel,
			(errmsg("index \"%s\" now contains %.0f row versions in %u pages",
					RelationGetRelationName(indrel),
					stats->num_index_tuples,
					stats->num_pages),
	errdetail("%u index pages have been deleted, %u are currently reusable.\n"
			  "%s.",
			  stats->pages_deleted, stats->pages_free,
			  pg_rusage_show(&ru0))));

	/*
	 * Check for tuple count mismatch.	If the index is partial, then it's OK
	 * for it to have fewer tuples than the heap; else we got trouble.
	 */
	if (check_stats &&
		!stats->estimated_count &&
		stats->num_index_tuples != num_tuples)
	{
		if (stats->num_index_tuples > num_tuples ||
			!vac_is_partial_index(indrel))
			ereport(WARNING,
					(errmsg("index \"%s\" contains %.0f row versions, but table contains %.0f row versions",
							RelationGetRelationName(indrel),
							stats->num_index_tuples, num_tuples),
					 errhint("Rebuild the index with REINDEX.")));
	}

	pfree(stats);
}

/*
 * Vacuums an index on an append-only table.
 *
 * This is called after an append-only segment file compaction to move
 * all tuples from the compacted segment files.
 * The segmentFileList is an
 */
static void
vacuum_appendonly_index(Relation indexRelation,
		AppendOnlyIndexVacuumState* vacuumIndexState,
		List *updated_stats,
		double rel_tuple_count,
						bool isfull,
	int elevel)
{
	Assert(RelationIsValid(indexRelation));
	Assert(vacuumIndexState);

	IndexBulkDeleteResult *stats;
	IndexVacuumInfo ivinfo;
	PGRUsage	ru0;

	pg_rusage_init(&ru0);

	ivinfo.index = indexRelation;
	ivinfo.message_level = elevel;
	ivinfo.num_heap_tuples = rel_tuple_count;
	ivinfo.strategy = vac_strategy;

	/* Do bulk deletion */
	stats = index_bulk_delete(&ivinfo, NULL, appendonly_tid_reaped,
			(void *) vacuumIndexState);

	/* Do post-VACUUM cleanup */
	stats = index_vacuum_cleanup(&ivinfo, stats);

	if (!stats)
		return;

	/* now update statistics in pg_class */
	vac_update_relstats_from_list(indexRelation,
						stats->num_pages, stats->num_index_tuples,
						false, InvalidTransactionId, updated_stats);

	ereport(elevel,
			(errmsg("index \"%s\" now contains %.0f row versions in %u pages",
					RelationGetRelationName(indexRelation),
					stats->num_index_tuples,
					stats->num_pages),
			 errdetail("%.0f index row versions were removed.\n"
			 "%u index pages have been deleted, %u are currently reusable.\n"
					   "%s.",
					   stats->tuples_removed,
					   stats->pages_deleted, stats->pages_free,
					   pg_rusage_show(&ru0))));

	pfree(stats);

}

static bool
appendonly_tid_reapded_check_block_directory(AppendOnlyIndexVacuumState* vacuumState,
		AOTupleId* aoTupleId)
{
	if (vacuumState->blockDirectory.currentSegmentFileNum ==
			AOTupleIdGet_segmentFileNum(aoTupleId) &&
			AppendOnlyBlockDirectoryEntry_RangeHasRow(&vacuumState->blockDirectoryEntry,
				AOTupleIdGet_rowNum(aoTupleId)))
	{
		return true;
	}

	if (!AppendOnlyBlockDirectory_GetEntry(&vacuumState->blockDirectory,
		aoTupleId,
		0,
		&vacuumState->blockDirectoryEntry))
	{
		return false;
	}
	return (vacuumState->blockDirectory.currentSegmentFileNum ==
			AOTupleIdGet_segmentFileNum(aoTupleId) &&
			AppendOnlyBlockDirectoryEntry_RangeHasRow(&vacuumState->blockDirectoryEntry,
				AOTupleIdGet_rowNum(aoTupleId)));
}

/*
 * appendonly_tid_reaped()
 *
 * Is a particular tid for an appendonly reaped?
 * state should contain an integer list of all compacted
 * segment files.
 *
 * This has the right signature to be an IndexBulkDeleteCallback.
 */
static bool
appendonly_tid_reaped(ItemPointer itemptr, void *state)
{
	AOTupleId* aoTupleId;
	AppendOnlyIndexVacuumState* vacuumState;
	bool reaped;

	Assert(itemptr);
	Assert(state);

	aoTupleId = (AOTupleId *)itemptr;
	vacuumState = (AppendOnlyIndexVacuumState *)state;

	reaped = !appendonly_tid_reapded_check_block_directory(vacuumState,
			aoTupleId);
	if (!reaped)
	{
		/* Also check visi map */
		reaped = !AppendOnlyVisimap_IsVisible(&vacuumState->visiMap,
		aoTupleId);
	}

	elogif(Debug_appendonly_print_compaction, DEBUG3,
			"Index vacuum %s %d",
			AOTupleIdToString(aoTupleId), reaped);
	return reaped;
}

/*
 * Open all the vacuumable indexes of the given relation, obtaining the
 * specified kind of lock on each.	Return an array of Relation pointers for
 * the indexes into *Irel, and the number of indexes into *nindexes.
 *
 * We consider an index vacuumable if it is marked insertable (IndexIsReady).
 * If it isn't, probably a CREATE INDEX CONCURRENTLY command failed early in
 * execution, and what we have is too corrupt to be processable.  We will
 * vacuum even if the index isn't indisvalid; this is important because in a
 * unique index, uniqueness checks will be performed anyway and had better not
 * hit dangling index pointers.
 */
void
vac_open_indexes(Relation relation, LOCKMODE lockmode,
				 int *nindexes, Relation **Irel)
{
	List	   *indexoidlist;
	ListCell   *indexoidscan;
	int			i;

	Assert(lockmode != NoLock);

	indexoidlist = RelationGetIndexList(relation);

	/* allocate enough memory for all indexes */
	i = list_length(indexoidlist);

	if (i > 0)
		*Irel = (Relation *) palloc(i * sizeof(Relation));
	else
		*Irel = NULL;

	/* collect just the ready indexes */
	i = 0;
	foreach(indexoidscan, indexoidlist)
	{
		Oid			indexoid = lfirst_oid(indexoidscan);
		Relation	indrel;

		indrel = index_open(indexoid, lockmode);
		if (IndexIsReady(indrel->rd_index))
			(*Irel)[i++] = indrel;
		else
			index_close(indrel, lockmode);
	}

	*nindexes = i;

	list_free(indexoidlist);
}

/*
 * Release the resources acquired by vac_open_indexes.	Optionally release
 * the locks (say NoLock to keep 'em).
 */
void
vac_close_indexes(int nindexes, Relation *Irel, LOCKMODE lockmode)
{
	if (Irel == NULL)
		return;

	while (nindexes--)
	{
		Relation	ind = Irel[nindexes];

		index_close(ind, lockmode);
	}
	pfree(Irel);
}

/*
 * vacuum_delay_point --- check for interrupts and cost-based delay.
 *
 * This should be called in each major loop of VACUUM processing,
 * typically once per page processed.
 */
void
vacuum_delay_point(void)
{
	/* Always check for interrupts */
	CHECK_FOR_INTERRUPTS();

	/* Nap if appropriate */
	if (VacuumCostActive && !InterruptPending &&
		VacuumCostBalance >= VacuumCostLimit)
	{
		int			msec;

		msec = VacuumCostDelay * VacuumCostBalance / VacuumCostLimit;
		if (msec > VacuumCostDelay * 4)
			msec = VacuumCostDelay * 4;

		pg_usleep(msec * 1000L);

		VacuumCostBalance = 0;

		/* update balance values for workers */
		AutoVacuumUpdateDelay();

		/* Might have gotten an interrupt while sleeping */
		CHECK_FOR_INTERRUPTS();
	}
}

/*
 * Dispatch a Vacuum command.
 */
static void
dispatchVacuum(VacuumStmt *vacstmt, VacuumStatsContext *ctx)
{
	CdbPgResults cdb_pgresults;

	/* should these be marked volatile ? */

	Assert(Gp_role == GP_ROLE_DISPATCH);
	Assert(vacstmt);
	Assert(vacstmt->options & VACOPT_VACUUM);
	Assert(!(vacstmt->options & VACOPT_ANALYZE));

	/* XXX: Some kinds of VACUUM assign a new relfilenode. bitmap indexes maybe? */
	CdbDispatchUtilityStatement((Node *) vacstmt,
								DF_CANCEL_ON_ERROR|
								DF_WITH_SNAPSHOT|
								DF_NEED_TWO_PHASE,
								GetAssignedOidsForDispatch(),
								&cdb_pgresults);

	vacuum_combine_stats(ctx, &cdb_pgresults);

	cdbdisp_clearCdbPgResults(&cdb_pgresults);
}

/*
 * open_relation_and_check_permission -- open the relation with an appropriate
 * lock based on the vacuum statement, and check for the permissions on this
 * relation.
 */
static Relation
open_relation_and_check_permission(VacuumStmt *vacstmt,
								   Oid relid,
								   char expected_relkind,
								   bool isDropTransaction)
{
	Relation onerel;
	LOCKMODE lmode;
	bool dontWait = false;

	/*
	 * If this is a drop transaction and there is another parallel drop transaction
	 * (on any relation) active. We drop out there. The other drop transaction
	 * might be on the same relation and that would be upgrade deadlock.
	 *
	 * Note: By the time we would have reached try_relation_open the other
	 * drop transaction might already be completed, but we don't take that
	 * risk here.
	 *
	 * My marking the drop transaction as busy before checking, the worst
	 * thing that can happen is that both transaction see each other and
	 * both cancel the drop.
	 *
	 * The upgrade deadlock is not applicable to vacuum full because
	 * it begins with an AccessExclusive lock and doesn't need to
	 * upgrade it.
	 */

	if (isDropTransaction && !(vacstmt->options & VACOPT_FULL))
	{
		MyProc->inDropTransaction = true;
		SIMPLE_FAULT_INJECTOR(VacuumRelationOpenRelationDuringDropPhase);
		if (HasDropTransaction(false))
		{
			elogif(Debug_appendonly_print_compaction, LOG,
					"Skip drop because of concurrent drop transaction");

			return NULL;
		}
	}

	/*
	 * Determine the type of lock we want --- hard exclusive lock for a FULL
	 * vacuum, but just ShareUpdateExclusiveLock for concurrent vacuum. Either
	 * way, we can be sure that no other backend is vacuuming the same table.
	 * For analyze, we use ShareUpdateExclusiveLock.
	 */
	if (isDropTransaction)
	{
		lmode = AccessExclusiveLock;
		dontWait = true;
	}
	else if (!(vacstmt->options & VACOPT_VACUUM))
		lmode = ShareUpdateExclusiveLock;
	else
		lmode = (vacstmt->options & VACOPT_FULL) ? AccessExclusiveLock : ShareUpdateExclusiveLock;

	/*
	 * Open the relation and get the appropriate lock on it.
	 *
	 * There's a race condition here: the rel may have gone away since the
	 * last time we saw it.  If so, we don't need to vacuum it.
	 */
	onerel = try_relation_open(relid, lmode, dontWait);

	if (!RelationIsValid(onerel))
		return NULL;

	/*
	 * Check permissions.
	 *
	 * We allow the user to vacuum a table if he is superuser, the table
	 * owner, or the database owner (but in the latter case, only if it's not
	 * a shared relation).	pg_class_ownercheck includes the superuser case.
	 *
	 * Note we choose to treat permissions failure as a WARNING and keep
	 * trying to vacuum the rest of the DB --- is this appropriate?
	 */
	if (!(pg_class_ownercheck(RelationGetRelid(onerel), GetUserId()) ||
		  (pg_database_ownercheck(MyDatabaseId, GetUserId()) && !onerel->rd_rel->relisshared)))
	{
		if (Gp_role != GP_ROLE_EXECUTE)
		{
			if (onerel->rd_rel->relisshared)
				ereport(WARNING,
						(errmsg("skipping \"%s\" --- only superuser can vacuum it",
								RelationGetRelationName(onerel))));
			else if (onerel->rd_rel->relnamespace == PG_CATALOG_NAMESPACE)
				ereport(WARNING,
						(errmsg("skipping \"%s\" --- only superuser or database owner can vacuum it",
								RelationGetRelationName(onerel))));
			else
				ereport(WARNING,
						(errmsg("skipping \"%s\" --- only table or database owner can vacuum it",
								RelationGetRelationName(onerel))));
		}
		relation_close(onerel, lmode);
		return NULL;
	}

	/*
	 * Check that it's a plain table; we used to do this in get_rel_oids() but
	 * seems safer to check after we've locked the relation.
	 */
	if (onerel->rd_rel->relkind != expected_relkind ||
		RelationIsExternal(onerel))
	{
		ereport(WARNING,
				(errmsg("skipping \"%s\" --- cannot vacuum indexes, views or external tables",
						RelationGetRelationName(onerel))));
		relation_close(onerel, lmode);
		return NULL;
	}

	/*
	 * Silently ignore tables that are temp tables of other backends ---
	 * trying to vacuum these will lead to great unhappiness, since their
	 * contents are probably not up-to-date on disk.  (We don't throw a
	 * warning here; it would just lead to chatter during a database-wide
	 * VACUUM.)
	 */
	if (isOtherTempNamespace(RelationGetNamespace(onerel)))
	{
		relation_close(onerel, lmode);
		return NULL;
	}

	/*
	 * We can ANALYZE any table except pg_statistic. See update_attstats
	 */
	if ((vacstmt->options & VACOPT_ANALYZE) && RelationGetRelid(onerel) == StatisticRelationId)
	{
		relation_close(onerel, ShareUpdateExclusiveLock);
		return NULL;
	}

	return onerel;
}

/*
 * vacuum_combine_stats
 * This function combine the stats information sent by QEs to generate
 * the final stats for QD relations.
 *
 * Note that the mirrorResults is ignored by this function.
 */
static void
vacuum_combine_stats(VacuumStatsContext *stats_context, CdbPgResults* cdb_pgresults)
{
	int result_no;

	Assert(Gp_role == GP_ROLE_DISPATCH);

	if (cdb_pgresults == NULL || cdb_pgresults->numResults <= 0)
		return;

	/*
	 * Process the dispatch results from the primary. Note that the QE
	 * processes also send back the new stats info, such as stats on
	 * pg_class, for the relevant table and its
	 * indexes. We parse this information, and compute the final stats
	 * for the QD.
	 *
	 * For pg_class stats, we compute the maximum number of tuples and
	 * maximum number of pages after processing the stats from each QE.
	 *
	 */
	for(result_no = 0; result_no < cdb_pgresults->numResults; result_no++)
	{

		VPgClassStats *pgclass_stats = NULL;
		ListCell *lc = NULL;
		struct pg_result *pgresult = cdb_pgresults->pg_results[result_no];

		if (pgresult->extras == NULL)
			continue;

		Assert(pgresult->extraslen > sizeof(int));

		/*
		 * Process the stats for pg_class. We simple compute the maximum
		 * number of rel_tuples and rel_pages.
		 */
		pgclass_stats = (VPgClassStats *) pgresult->extras;
		foreach (lc, stats_context->updated_stats)
		{
			VPgClassStats *tmp_stats = (VPgClassStats *) lfirst(lc);

			if (tmp_stats->relid == pgclass_stats->relid)
			{
				tmp_stats->rel_pages += pgclass_stats->rel_pages;
				tmp_stats->rel_tuples += pgclass_stats->rel_tuples;
				break;
			}
		}

		if (lc == NULL)
		{
			Assert(pgresult->extraslen == sizeof(VPgClassStats));

			pgclass_stats = palloc(sizeof(VPgClassStats));
			memcpy(pgclass_stats, pgresult->extras, pgresult->extraslen);

			stats_context->updated_stats =
					lappend(stats_context->updated_stats, pgclass_stats);
		}
	}
}
