// GPDB_12_MERGE_FIXME This file contains snippets of code that used to be in
// src/backend/commands/vacuum.c. AO vacuum needs to be re-implemented using
// the new table AM.

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
	Snapshot	appendOnlyMetaDataSnapshot;
	AppendOnlyVisimap visiMap;
	AppendOnlyBlockDirectory blockDirectory;
	AppendOnlyBlockDirectoryEntry blockDirectoryEntry;
} AppendOnlyIndexVacuumState;











/*
 *	appendonly_vacuum_rel() -- perform VACUUM for one appendonly relation
 *
 *		This routine vacuums a single AO table, cleans out its indexes, and
 *		updates its relpages and reltuples statistics.
 *
 *		At entry, we have already established a transaction and opened
 *		and locked the relation.
 */
void
appendonly_vacuum_rel(Relation onerel, VacuumParams *params,
					  BufferAccessStrategy bstrategy)
{
	LVRelStats *vacrelstats;
	Relation   *Irel;
	int			nindexes;
	PGRUsage	ru0;
	TimestampTz starttime = 0;
	long		secs;
	int			usecs;
	double		read_rate,
				write_rate;
	bool		aggressive;		/* should we scan all unfrozen pages? */
	bool		scanned_all_unfrozen;	/* actually scanned all such pages? */
	TransactionId xidFullScanLimit;
	MultiXactId mxactFullScanLimit;
	BlockNumber new_rel_pages;
	BlockNumber new_rel_allvisible;
	double		new_live_tuples;
	TransactionId new_frozen_xid;
	MultiXactId new_min_multi;

	Assert(params != NULL);
	Assert(params->index_cleanup != VACOPT_TERNARY_DEFAULT);
	Assert(params->truncate != VACOPT_TERNARY_DEFAULT);

	/* not every AM requires these to be valid, but heap does */
	Assert(TransactionIdIsNormal(onerel->rd_rel->relfrozenxid));
	Assert(MultiXactIdIsValid(onerel->rd_rel->relminmxid));

	/* measure elapsed time iff autovacuum logging requires it */
	if (IsAutoVacuumWorkerProcess() && params->log_min_duration >= 0)
	{
		pg_rusage_init(&ru0);
		starttime = GetCurrentTimestamp();
	}

	if (params->options & VACOPT_VERBOSE)
		elevel = INFO;
	else
		elevel = DEBUG2;

	if (Gp_role == GP_ROLE_DISPATCH)
		elevel = DEBUG2; /* vacuum and analyze messages aren't interesting from the QD */

#ifdef FAULT_INJECTOR
	if (ao_vacuum_phase_config != NULL &&
		ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
	{
			FaultInjector_InjectFaultIfSet(
				"compaction_before_segmentfile_drop",
				DDLNotSpecified,
				"",	// databaseName
				RelationGetRelationName(onerel)); // tableName
	}
	if (ao_vacuum_phase_config != NULL &&
		ao_vacuum_phase_config->appendonly_phase == AOVAC_CLEANUP)
	{
			FaultInjector_InjectFaultIfSet(
				"compaction_before_cleanup_phase",
				DDLNotSpecified,
				"",	// databaseName
				RelationGetRelationName(onerel)); // tableName
	}
#endif

	pgstat_progress_start_command(PROGRESS_COMMAND_VACUUM,
								  RelationGetRelid(onerel));

	vac_strategy = bstrategy;

	/*
	 * MPP-23647.  Update xid limits for heap as well as appendonly
	 * relations.  This allows setting relfrozenxid to correct value
	 * for an appendonly (AO/CO) table.
	 */

	vacuum_set_xid_limits(onerel,
						  params->freeze_min_age,
						  params->freeze_table_age,
						  params->multixact_freeze_min_age,
						  params->multixact_freeze_table_age,
						  &OldestXmin, &FreezeLimit, &xidFullScanLimit,
						  &MultiXactCutoff, &mxactFullScanLimit);

	/*
	 * We request an aggressive scan if the table's frozen Xid is now older
	 * than or equal to the requested Xid full-table scan limit; or if the
	 * table's minimum MultiXactId is older than or equal to the requested
	 * mxid full-table scan limit; or if DISABLE_PAGE_SKIPPING was specified.
	 */
	aggressive = TransactionIdPrecedesOrEquals(onerel->rd_rel->relfrozenxid,
											   xidFullScanLimit);
	aggressive |= MultiXactIdPrecedesOrEquals(onerel->rd_rel->relminmxid,
											  mxactFullScanLimit);
	if (params->options & VACOPT_DISABLE_PAGE_SKIPPING)
		aggressive = true;

	/*
	 * Normally the relfrozenxid for an anti-wraparound vacuum will be old
	 * enough to force an aggressive vacuum.  However, a concurrent vacuum
	 * might have already done this work that the relfrozenxid in relcache has
	 * been updated.  If that happens this vacuum is redundant, so skip it.
	 */
	if (params->is_wraparound && !aggressive)
	{
		ereport(DEBUG1,
				(errmsg("skipping redundant vacuum to prevent wraparound of table \"%s.%s.%s\"",
						get_database_name(MyDatabaseId),
						get_namespace_name(RelationGetNamespace(onerel)),
						RelationGetRelationName(onerel))));
		pgstat_progress_end_command();
		return;
	}

	vacrelstats = (LVRelStats *) palloc0(sizeof(LVRelStats));

	vacrelstats->old_rel_pages = onerel->rd_rel->relpages;
	vacrelstats->old_live_tuples = onerel->rd_rel->reltuples;
	vacrelstats->num_index_scans = 0;
	vacrelstats->pages_removed = 0;
	vacrelstats->lock_waiter_detected = false;

	/* Open all indexes of the relation */
	vac_open_indexes(onerel, RowExclusiveLock, &nindexes, &Irel);
	vacrelstats->useindex = (nindexes > 0 &&
							 params->index_cleanup == VACOPT_TERNARY_ENABLED);

	/* Do the vacuuming */
	lazy_scan_heap(onerel, params, vacrelstats, Irel, nindexes, aggressive);

	/* Done with indexes */
	vac_close_indexes(nindexes, Irel, NoLock);

	/*
	 * Compute whether we actually scanned the all unfrozen pages. If we did,
	 * we can adjust relfrozenxid and relminmxid.
	 *
	 * NB: We need to check this before truncating the relation, because that
	 * will change ->rel_pages.
	 */
	if ((vacrelstats->scanned_pages + vacrelstats->frozenskipped_pages)
		< vacrelstats->rel_pages)
	{
		Assert(!aggressive);
		scanned_all_unfrozen = false;
	}
	else
		scanned_all_unfrozen = true;

	/*
	 * Optionally truncate the relation.
	 */
	if (should_attempt_truncation(params, vacrelstats))
		lazy_truncate_heap(onerel, vacrelstats);

	/* Report that we are now doing final cleanup */
	pgstat_progress_update_param(PROGRESS_VACUUM_PHASE,
								 PROGRESS_VACUUM_PHASE_FINAL_CLEANUP);

	/*
	 * Update statistics in pg_class.
	 *
	 * A corner case here is that if we scanned no pages at all because every
	 * page is all-visible, we should not update relpages/reltuples, because
	 * we have no new information to contribute.  In particular this keeps us
	 * from replacing relpages=reltuples=0 (which means "unknown tuple
	 * density") with nonzero relpages and reltuples=0 (which means "zero
	 * tuple density") unless there's some actual evidence for the latter.
	 *
	 * It's important that we use tupcount_pages and not scanned_pages for the
	 * check described above; scanned_pages counts pages where we could not
	 * get cleanup lock, and which were processed only for frozenxid purposes.
	 *
	 * We do update relallvisible even in the corner case, since if the table
	 * is all-visible we'd definitely like to know that.  But clamp the value
	 * to be not more than what we're setting relpages to.
	 *
	 * Also, don't change relfrozenxid/relminmxid if we skipped any pages,
	 * since then we don't know for certain that all tuples have a newer xmin.
	 */
	new_rel_pages = vacrelstats->rel_pages;
	new_live_tuples = vacrelstats->new_live_tuples;
	if (vacrelstats->tupcount_pages == 0 && new_rel_pages > 0)
	{
		new_rel_pages = vacrelstats->old_rel_pages;
		new_live_tuples = vacrelstats->old_live_tuples;
	}

	visibilitymap_count(onerel, &new_rel_allvisible, NULL);
	if (new_rel_allvisible > new_rel_pages)
		new_rel_allvisible = new_rel_pages;

	new_frozen_xid = scanned_all_unfrozen ? FreezeLimit : InvalidTransactionId;
	new_min_multi = scanned_all_unfrozen ? MultiXactCutoff : InvalidMultiXactId;

	vac_update_relstats(onerel,
						new_rel_pages,
						new_live_tuples,
						new_rel_allvisible,
						nindexes > 0,
						new_frozen_xid,
						new_min_multi,
						false,
						true /* isvacuum */);

	/* report results to the stats collector, too */
	pgstat_report_vacuum(RelationGetRelid(onerel),
						 onerel->rd_rel->relisshared,
						 new_live_tuples,
						 vacrelstats->new_dead_tuples);
	pgstat_progress_end_command();

	if (gp_indexcheck_vacuum == INDEX_CHECK_ALL ||
		(gp_indexcheck_vacuum == INDEX_CHECK_SYSTEM &&
		 PG_CATALOG_NAMESPACE == RelationGetNamespace(onerel)))
	{
		int			i;

		for (i = 0; i < nindexes; i++)
		{
			if (Irel[i]->rd_rel->relam == BTREE_AM_OID)
				_bt_validate_vacuum(Irel[i], onerel, OldestXmin);
		}
	}

	/* and log the action if appropriate */
	if (IsAutoVacuumWorkerProcess() && params->log_min_duration >= 0)
	{
		TimestampTz endtime = GetCurrentTimestamp();

		if (params->log_min_duration == 0 ||
			TimestampDifferenceExceeds(starttime, endtime,
									   params->log_min_duration))
		{
			StringInfoData buf;
			char	   *msgfmt;

			TimestampDifference(starttime, endtime, &secs, &usecs);

			read_rate = 0;
			write_rate = 0;
			if ((secs > 0) || (usecs > 0))
			{
				read_rate = (double) BLCKSZ * VacuumPageMiss / (1024 * 1024) /
					(secs + usecs / 1000000.0);
				write_rate = (double) BLCKSZ * VacuumPageDirty / (1024 * 1024) /
					(secs + usecs / 1000000.0);
			}

			/*
			 * This is pretty messy, but we split it up so that we can skip
			 * emitting individual parts of the message when not applicable.
			 */
			initStringInfo(&buf);
			if (params->is_wraparound)
			{
				/* an anti-wraparound vacuum has to be aggressive */
				Assert(aggressive);
				msgfmt = _("automatic aggressive vacuum to prevent wraparound of table \"%s.%s.%s\": index scans: %d\n");
			}
			else
			{
				if (aggressive)
					msgfmt = _("automatic aggressive vacuum of table \"%s.%s.%s\": index scans: %d\n");
				else
					msgfmt = _("automatic vacuum of table \"%s.%s.%s\": index scans: %d\n");
			}
			appendStringInfo(&buf, msgfmt,
							 get_database_name(MyDatabaseId),
							 get_namespace_name(RelationGetNamespace(onerel)),
							 RelationGetRelationName(onerel),
							 vacrelstats->num_index_scans);
			appendStringInfo(&buf, _("pages: %u removed, %u remain, %u skipped due to pins, %u skipped frozen\n"),
							 vacrelstats->pages_removed,
							 vacrelstats->rel_pages,
							 vacrelstats->pinskipped_pages,
							 vacrelstats->frozenskipped_pages);
			appendStringInfo(&buf,
							 _("tuples: %.0f removed, %.0f remain, %.0f are dead but not yet removable, oldest xmin: %u\n"),
							 vacrelstats->tuples_deleted,
							 vacrelstats->new_rel_tuples,
							 vacrelstats->new_dead_tuples,
							 OldestXmin);
			appendStringInfo(&buf,
							 _("buffer usage: %d hits, %d misses, %d dirtied\n"),
							 VacuumPageHit,
							 VacuumPageMiss,
							 VacuumPageDirty);
			appendStringInfo(&buf, _("avg read rate: %.3f MB/s, avg write rate: %.3f MB/s\n"),
							 read_rate, write_rate);
			appendStringInfo(&buf, _("system usage: %s"), pg_rusage_show(&ru0));

			ereport(LOG,
					(errmsg_internal("%s", buf.data)));
			pfree(buf.data);
		}
	}
}

/*
 * lazy_vacuum_aorel -- perform LAZY VACUUM for one Append-only relation.
 */
static void
lazy_vacuum_aorel(Relation onerel, int options, AOVacuumPhaseConfig *ao_vacuum_phase_config)
{
	LVRelStats *vacrelstats;
	bool		update_relstats = true;

	vacrelstats = (LVRelStats *) palloc0(sizeof(LVRelStats));

	switch (ao_vacuum_phase_config->appendonly_phase)
	{
		case AOVAC_PREPARE:
			elogif(Debug_appendonly_print_compaction, LOG,
				   "Vacuum prepare phase %s", RelationGetRelationName(onerel));

			vacuum_appendonly_indexes(onerel, options);
			if (RelationIsAoRows(onerel))
				AppendOnlyTruncateToEOF(onerel);
			else
				AOCSTruncateToEOF(onerel);

			/*
			 * MPP-23647.  For empty tables, we skip compaction phase
			 * and cleanup phase.  Therefore, we update the stats
			 * (specifically, relfrozenxid) in prepare phase if the
			 * table is empty.  Otherwise, the stats will be updated in
			 * the cleanup phase, when we would have computed the
			 * correct values for stats.
			 */
			if (ao_vacuum_phase_config->appendonly_relation_empty)
			{
				update_relstats = true;
				/*
				 * For an empty relation, the only stats we care about
				 * is relfrozenxid and relhasindex.  We need to be
				 * mindful of correctly setting relhasindex here.
				 * relfrozenxid is already taken care of above by
				 * calling vacuum_set_xid_limits().
				 */
				vacrelstats->hasindex = onerel->rd_rel->relhasindex;
			}
			else
			{
				/*
				 * For a non-empty relation, follow the usual
				 * compaction phases and do not update stats in
				 * prepare phase.
				 */
				update_relstats = false;
			}
			break;

		case AOVAC_COMPACT:
		case AOVAC_DROP:
			vacuum_appendonly_rel(onerel, options, ao_vacuum_phase_config);
			update_relstats = false;
			break;

		case AOVAC_CLEANUP:
			elogif(Debug_appendonly_print_compaction, LOG,
				   "Vacuum cleanup phase %s", RelationGetRelationName(onerel));

			vacuum_appendonly_fill_stats(onerel, GetActiveSnapshot(),
										 &vacrelstats->rel_pages,
										 &vacrelstats->new_rel_tuples,
										 &vacrelstats->hasindex);
			/* reset the remaining LVRelStats values */
			vacrelstats->nonempty_pages = 0;
			vacrelstats->num_dead_tuples = 0;
			vacrelstats->max_dead_tuples = 0;
			vacrelstats->tuples_deleted = 0;
			vacrelstats->pages_removed = 0;
			break;

		default:
			elog(ERROR, "invalid AO vacuum phase %d", ao_vacuum_phase_config->appendonly_phase);
	}

	if (update_relstats)
	{
		/* Update statistics in pg_class */
		vac_update_relstats(onerel,
							vacrelstats->rel_pages,
							vacrelstats->new_rel_tuples,
							0, /* AO does not currently have an equivalent to
							      Heap's 'all visible pages' */
							vacrelstats->hasindex,
							FreezeLimit,
							MultiXactCutoff,
							false,
							true /* isvacuum */);

		/*
		 * Report results to the stats collector.
		 * Explicitly pass 0 as num_dead_tuples. AO tables use hidden
		 * tuples in a conceptually similar way that regular tables
		 * use dead tuples. However with regards to pgstat these are
		 * semantically distinct thus exposing them will be ambiguous.
		 */
		pgstat_report_vacuum(RelationGetRelid(onerel),
							 onerel->rd_rel->relisshared,
							 vacrelstats->new_rel_tuples,
							 0 /* num_dead_tuples */);
	}
}












/*
 * Assigns the compaction segment information.
 *
 * The segment to compact is returned in *compact_segno, and
 * the segment to move rows to, is returned in *insert_segno.
 */
static bool
vacuum_assign_compaction_segno(Relation onerel,
							   List *compactedSegmentFileList,
							   List *insertedSegmentFileList,
							   List **compactNowList,
							   int *insert_segno)
{
	List *new_compaction_list;
	bool is_drop;

	Assert(Gp_role != GP_ROLE_EXECUTE);
	Assert(RelationIsValid(onerel));
	Assert(RelationIsAppendOptimized(onerel));

	/*
	 * Assign a compaction segment num and insert segment num
	 * on master or on segment if in utility mode
	 */
	if (!gp_appendonly_compaction)
	{
		*insert_segno = -1;
		*compactNowList = NIL;
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
			*insert_segno = SetSegnoForCompactionInsert(onerel,
														new_compaction_list,
														compactedSegmentFileList);
		}
		else
		{
			/*
			 * If we continue an aborted drop phase, we do not assign a real
			 * insert segment file.
			 */
			*insert_segno = APPENDONLY_COMPACTION_SEGNO_INVALID;
		}
		*compactNowList = new_compaction_list;

		elogif(Debug_appendonly_print_compaction, LOG,
				"Schedule compaction on AO table: "
				"compact segno list length %d, insert segno %d",
				list_length(new_compaction_list), *insert_segno);
		return true;
	}
	else
	{
		elog(DEBUG3, "No valid compact segno for relation %s (%d)",
				RelationGetRelationName(onerel),
				RelationGetRelid(onerel));
		return false;
	}
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
 * For heap VACUUM we disable two-phase commit, because we do not actually make
 * any logical changes to the tables. Even if a VACUUM transaction fails on one
 * of the QE segments, it should not matter, because the data has not logically
 * changed on disk. VACUUM FULL and lazy vacuum are both completed in one
 * transaction.
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
vacuumStatement_Relation(Oid relid, List *relations, BufferAccessStrategy bstrategy,
						 bool isTopLevel, VacuumParams *params, int options, RangeVar *relation,
						 bool skip_twophase, AOVacuumPhaseConfig *ao_vacuum_phase_config)
{
	LOCKMODE			lmode = NoLock;
	Relation			onerel;
	LockRelId			onerelid;
	MemoryContext oldcontext;

	oldcontext = MemoryContextSwitchTo(vac_context);

	/* VACUUM, without ANALYZE */
	options &= ~VACOPT_ANALYZE;
	options |= VACOPT_VACUUM;

	MemoryContextSwitchTo(oldcontext);

	/*
	 * For each iteration we start/commit our own transactions,
	 * so that we can release resources such as locks and memories,
	 * and we can also safely perform non-transactional work
	 * along with transactional work. If we are a query executor and skipping
	 * a two phase commit, the expectation is that we will vacuum one relation
	 * per dispatch, so we can use the outer transaction for this instead.
	 */
	if (Gp_role != GP_ROLE_EXECUTE || !skip_twophase)
		StartTransactionCommand();

	/*
	 * Functions in indexes may want a snapshot set. Also, setting
	 * a snapshot ensures that RecentGlobalXmin is kept truly recent.
	 */
	PushActiveSnapshot(GetTransactionSnapshot());

	/*
	 * Determine the type of lock we want --- hard exclusive lock for a FULL
	 * vacuum, but just ShareUpdateExclusiveLock for concurrent vacuum. Either
	 * way, we can be sure that no other backend is vacuuming the same table.
	 * For analyze, we use ShareUpdateExclusiveLock.
	 */
	if (ao_vacuum_phase_config != NULL &&
		ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
	{
		Assert(Gp_role == GP_ROLE_EXECUTE);
		lmode = AccessExclusiveLock;
		SIMPLE_FAULT_INJECTOR("vacuum_relation_open_relation_during_drop_phase");
	}
	else if (!(options & VACOPT_VACUUM))
		lmode = ShareUpdateExclusiveLock;
	else
		lmode = (options & VACOPT_FULL) ? AccessExclusiveLock : ShareUpdateExclusiveLock;

	/*
	 * Open the relation and get the appropriate lock on it.
	 *
	 * There's a race condition here: the rel may have gone away since the
	 * last time we saw it.  If so, we don't need to vacuum it.
	 */
	onerel = try_relation_open(relid, lmode, false /* dontwait */);
	if (!onerel)
	{
		PopActiveSnapshot();
		CommitTransactionCommand();
		return;
	}


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
		PopActiveSnapshot();
		CommitTransactionCommand();
		return;
	}

	/*
	 * Check that it's a vacuumable relation; we used to do this in
	 * get_rel_oids() but seems safer to check after we've locked the
	 * relation.
	 */
	if ((onerel->rd_rel->relkind != RELKIND_RELATION &&
		 onerel->rd_rel->relkind != RELKIND_MATVIEW &&
		 onerel->rd_rel->relkind != RELKIND_TOASTVALUE &&
		 onerel->rd_rel->relkind != RELKIND_AOSEGMENTS &&
		 onerel->rd_rel->relkind != RELKIND_AOBLOCKDIR &&
		 onerel->rd_rel->relkind != RELKIND_AOVISIMAP)
		|| RelationIsForeign(onerel))
	{
		ereport(WARNING,
				(errmsg("skipping \"%s\" --- cannot vacuum non-tables, external tables, foreign tables or special system tables",
						RelationGetRelationName(onerel))));
		relation_close(onerel, lmode);
		PopActiveSnapshot();
		CommitTransactionCommand();
		return;
	}

	/*
	 * Silently ignore tables that are temp tables of other backends ---
	 * trying to vacuum these will lead to great unhappiness, since their
	 * contents are probably not up-to-date on disk.  (We don't throw a
	 * warning here; it would just lead to chatter during a database-wide
	 * VACUUM.)
	 */
	if (RELATION_IS_OTHER_TEMP(onerel))
	{
		relation_close(onerel, lmode);
		PopActiveSnapshot();
		CommitTransactionCommand();
		return;
	}

	/*
	 * Get a session-level lock too. This will protect our access to the
	 * relation across multiple transactions, so that we can vacuum the
	 * relation's TOAST table (if any) secure in the knowledge that no one is
	 * deleting the parent relation.
	 *
	 * NOTE: this cannot block, even if someone else is waiting for access,
	 * because the lock manager knows that both lock requests are from the
	 * same process.
	 */
	onerelid = onerel->rd_lockInfo.lockRelId;
	LockRelationIdForSession(&onerelid, lmode);

	/*
	 * Ensure that only one relation will be locked for vacuum, when the user
	 * issues a "VACUUM <db>" command, or a "VACUUM <parent_partition_table>"
	 * command.
	 */
	oldcontext = MemoryContextSwitchTo(vac_context);
	if (list_length(relations) > 1 || relation == NULL)
	{
		char *relname = get_rel_name(relid);
		char *nspname = get_namespace_name(get_rel_namespace(relid));

		if (relname == NULL)
			elog(ERROR, "relation name does not exist for relation with oid %d", relid);
		else if (nspname == NULL)
			elog(ERROR, "namespace does not exist for relation with oid %d", relid);

		relation = makeRangeVar(nspname, relname, -1);
	}
	MemoryContextSwitchTo(oldcontext);

	if (RelationIsHeap(onerel) || Gp_role == GP_ROLE_EXECUTE)
	{
		/* skip two-phase commit on heap table VACUUM */
		if (Gp_role == GP_ROLE_DISPATCH)
			skip_twophase = true;

		if (ao_vacuum_phase_config != NULL &&
			ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
		{
			SIMPLE_FAULT_INJECTOR("vacuum_relation_open_relation_during_drop_phase");
		}

		vacuum_rel(relid, relation, options, params, skip_twophase,
				   ao_vacuum_phase_config, onerel, lmode);

		/*
		 * Complete the transaction and free all temporary memory used.
		 */
		PopActiveSnapshot();
		/*
		 * Transaction commit is always executed on QD.
		 */
		if (Gp_role != GP_ROLE_EXECUTE)
			CommitTransactionCommand();

		onerel = NULL;
	}
	else
	{
		List	   *compactedSegmentFileList = NIL;
		List	   *insertedSegmentFileList = NIL;

		/*
		 * On QD, initialize the ao_vacuum_phase_config to start passing
		 * around through vacuum dispatches. For QE, the
		 * ao_vacuum_phase_config will be initialized from the vacuum dispatch
		 * statement.
		 */
		oldcontext = MemoryContextSwitchTo(vac_context);
		ao_vacuum_phase_config = palloc0(sizeof(AOVacuumPhaseConfig));
		ao_vacuum_phase_config->appendonly_compaction_segno = NIL;
		ao_vacuum_phase_config->appendonly_compaction_insert_segno = NIL;
		ao_vacuum_phase_config->appendonly_relation_empty = false;
		skip_twophase = false;
		MemoryContextSwitchTo(oldcontext);

		/*
		 * 1. Prepare phase
		 */
		vacuum_rel_ao_phase(relid, relation, options, params,
							skip_twophase, ao_vacuum_phase_config,
							onerel, lmode, NIL, NIL, AOVAC_PREPARE);

		PopActiveSnapshot();
		if (Gp_role != GP_ROLE_EXECUTE)
			CommitTransactionCommand();

		onerel = NULL;

		/*
		 * Loop between compaction and drop phases, until there is nothing more left
		 * to do for this relation.
		 */
		for (;;)
		{
			List	   *compactNowList = NIL;
			int			insertSegNo = -1;

			if (gp_appendonly_compaction)
			{
				/*
				 * 2. Compaction phase
				 */
				StartTransactionCommand();
				PushActiveSnapshot(GetTransactionSnapshot());
				onerel = try_relation_open(relid, lmode, false /* dontwait */);

				/* Chose a source and destination segfile for compaction. */
				if (!vacuum_assign_compaction_segno(onerel,
													compactedSegmentFileList,
													insertedSegmentFileList,
													&compactNowList,
													&insertSegNo))
				{
					/*
					 * There is nothing left to do for this relation. Proceed to
					 * the cleanup phase.
					 */
					break;
				}

				oldcontext = MemoryContextSwitchTo(vac_context);

				compactNowList = list_copy(compactNowList);

				compactedSegmentFileList =
					list_union_int(compactedSegmentFileList, compactNowList);
				insertedSegmentFileList =
					lappend_int(insertedSegmentFileList, insertSegNo);

				MemoryContextSwitchTo(oldcontext);

				vacuum_rel_ao_phase(relid, relation, options, params,
									skip_twophase, ao_vacuum_phase_config,
									onerel, lmode,
									list_make1_int(insertSegNo),
									compactNowList,
									AOVAC_COMPACT);
				PopActiveSnapshot();
				if (Gp_role != GP_ROLE_EXECUTE)
					CommitTransactionCommand();

				onerel = NULL;
			}

			/*
			 * 3. Drop phase
			 */

			StartTransactionCommand();
			PushActiveSnapshot(GetTransactionSnapshot());

			/*
			 * Upgrade to AccessExclusiveLock from SharedAccessExclusive here
			 * before doing the drops. We set the dontwait flag here to
			 * prevent deadlock scenarios such as a concurrent transaction
			 * holding AccessShareLock and then upgrading to ExclusiveLock to
			 * run DELETE/UPDATE while VACUUM is waiting here for
			 * AccessExclusiveLock.
			 *
			 * Skipping when we are not able to upgrade to AccessExclusivelock
			 * can be an issue though because it is possible to accumulate a
			 * large amount of segfiles marked AOSEG_STATE_AWAITING_DROP.
			 * However, we do not expect this to happen too frequently such
			 * that all segfiles are marked.
			 */
			SIMPLE_FAULT_INJECTOR("vacuum_relation_open_relation_during_drop_phase");
			onerel = try_relation_open(relid, AccessExclusiveLock, true /* dontwait */);

			if (!RelationIsValid(onerel))
			{
				/* Couldn't get AccessExclusiveLock. */
				PopActiveSnapshot();
				CommitTransactionCommand();

				/*
				 * Skip performing DROP and continue with other segfiles in
				 * case they have crossed threshold and need to be compacted
				 * or marked as AOSEG_STATE_AWAITING_DROP. To ensure that
				 * vacuum decreases the age for appendonly tables even if drop
				 * phase is getting skipped, perform cleanup phase when done
				 * iterating through all segfiles so that the relfrozenxid
				 * value is updated correctly in pg_class.
				 */
				continue;
			}

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

				DeregisterSegnoForCompactionDrop(relid, compactNowList);
				break;
			}

			elogif(Debug_appendonly_print_compaction, LOG,
				   "Dispatch drop transaction on append-only relation %s",
				   RelationGetRelationName(onerel));

			/* Perform the DROP phase */
			RegisterSegnoForCompactionDrop(relid, compactNowList);

			vacuum_rel_ao_phase(relid, relation, options, params,
								skip_twophase, ao_vacuum_phase_config,
								onerel, lmode,
								NIL,	/* insert segno */
								compactNowList,
								AOVAC_DROP);
			PopActiveSnapshot();
			if (Gp_role != GP_ROLE_EXECUTE)
				CommitTransactionCommand();

			onerel = NULL;

			if (!gp_appendonly_compaction)
				break;
		}

		/*
		 * 4. Cleanup phase.
		 *
		 * This vacuums all the auxiliary tables, like TOAST, AO segment tables etc.
		 *
		 * We can skip this, if we didn't compact anything. XXX: Really? Shouldn't we
		 * still process the aux tables?
		 */
		if (list_length(compactedSegmentFileList) > 0)
		{
			/* Provide the list of all compacted segment numbers with it */
			vacuum_rel_ao_phase(relid, relation, options, params,
								skip_twophase, ao_vacuum_phase_config,
								onerel, lmode,
								insertedSegmentFileList,
								compactedSegmentFileList,
								AOVAC_CLEANUP);
			PopActiveSnapshot();
			if (Gp_role != GP_ROLE_EXECUTE)
				CommitTransactionCommand();

			onerel = NULL;
		}

		pfree(ao_vacuum_phase_config);
	}

	if (lmode != NoLock)
	{
		UnlockRelationIdForSession(&onerelid, lmode);
	}

	if (Gp_role == GP_ROLE_DISPATCH)
	{
		/*
		 * We need some transaction to update the catalog.  We could do
		 * it on the outer vacuumStatement, but it is useful to track
		 * relation by relation.
		 */
		//if (!istemp) // FIXME
		{
			char *vsubtype = ""; /* NOFULL */
			bool		start_xact = false;

			if (!onerel)
			{
				StartTransactionCommand();
				start_xact = true;
			}

			if (IsAutoVacuumWorkerProcess())
				vsubtype = "AUTO";
			else
			{
				if ((options & VACOPT_FULL) &&
					(0 == params->freeze_min_age))
					vsubtype = "FULL FREEZE";
				else if ((options & VACOPT_FULL))
					vsubtype = "FULL";
				else if (0 == params->freeze_min_age)
					vsubtype = "FREEZE";
			}
			MetaTrackUpdObject(RelationRelationId,
							   relid,
							   GetUserId(),
							   "VACUUM",
							   vsubtype);
			if (start_xact)
				CommitTransactionCommand();
		}
	}

	if (onerel)
	{
		relation_close(onerel, NoLock);
		PopActiveSnapshot();
		CommitTransactionCommand();
	}
}


<<<<<<< HEAD
 * Dispatch a Vacuum command.
 */
static void
dispatchVacuum(int options, RangeVar *relation, bool skip_twophase,
			   AOVacuumPhaseConfig *ao_vacuum_phase_config, VacuumStatsContext *ctx)
{
	CdbPgResults cdb_pgresults;
	VacuumStmt *vacstmt = makeNode(VacuumStmt);
	int flags = DF_CANCEL_ON_ERROR | DF_WITH_SNAPSHOT;

	/* should these be marked volatile ? */

	Assert(Gp_role == GP_ROLE_DISPATCH);
	Assert(options & VACOPT_VACUUM);
	Assert(!(options & VACOPT_ANALYZE));

	vacstmt->options = options;
	vacstmt->relation = relation;
	vacstmt->va_cols = NIL;
	vacstmt->skip_twophase = skip_twophase;

	vacstmt->ao_vacuum_phase_config = makeNode(AOVacuumPhaseConfig);
	if (ao_vacuum_phase_config != NULL)
	{
		vacstmt->ao_vacuum_phase_config->appendonly_compaction_segno =
			ao_vacuum_phase_config->appendonly_compaction_segno;
		vacstmt->ao_vacuum_phase_config->appendonly_compaction_insert_segno =
			ao_vacuum_phase_config->appendonly_compaction_insert_segno;
		vacstmt->ao_vacuum_phase_config->appendonly_relation_empty =
			ao_vacuum_phase_config->appendonly_relation_empty;
		vacstmt->ao_vacuum_phase_config->appendonly_phase =
			ao_vacuum_phase_config->appendonly_phase;
	}

	if (!skip_twophase)
		flags |= DF_NEED_TWO_PHASE;

	/* XXX: Some kinds of VACUUM assign a new relfilenode. bitmap indexes maybe? */
	CdbDispatchUtilityStatement((Node *) vacstmt, flags,
								GetAssignedOidsForDispatch(),
								&cdb_pgresults);

	vacuum_combine_stats(ctx, &cdb_pgresults);

	cdbdisp_clearCdbPgResults(&cdb_pgresults);
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
		 * Process the stats for pg_class. We simply compute the maximum
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
				tmp_stats->relallvisible += pgclass_stats->relallvisible;
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


	
/*
 * Update relpages/reltuples of all the relations in the list.
 */
static void
vac_update_relstats_from_list(List *updated_stats)
{
	ListCell *lc;

	/*
	 * This function is only called in the context of the QD, so let's be
	 * explicit about that given the assumptions taken.
	 */
	Assert(Gp_role == GP_ROLE_DISPATCH);

	foreach (lc, updated_stats)
	{
		VPgClassStats *stats = (VPgClassStats *) lfirst(lc);
		Relation	rel;

		rel = relation_open(stats->relid, AccessShareLock);

		if (GpPolicyIsReplicated(rel->rd_cdbpolicy))
		{
			stats->rel_pages = stats->rel_pages / rel->rd_cdbpolicy->numsegments;
			stats->rel_tuples = stats->rel_tuples / rel->rd_cdbpolicy->numsegments;
			stats->relallvisible = stats->relallvisible / rel->rd_cdbpolicy->numsegments;
		}

		/*
		 * Pass 'false' for isvacuum, so that the stats are
		 * actually updated.
		 */
		vac_update_relstats(rel,
							stats->rel_pages, stats->rel_tuples,
							stats->relallvisible,
							rel->rd_rel->relhasindex,
							InvalidTransactionId,
							InvalidMultiXactId,
							false,
							false /* isvacuum */);
		relation_close(rel, AccessShareLock);
	}
}



static void
vacuum_rel_ao_phase(Oid relid, RangeVar *relation, int options, VacuumParams *params,
					bool skip_twophase, AOVacuumPhaseConfig *ao_vacuum_phase_config,
					Relation onerel, LOCKMODE lmode,
					List *compaction_insert_segno,
					List *compaction_segno,
					AOVacuumPhase phase)
{
	ao_vacuum_phase_config->appendonly_compaction_insert_segno = compaction_insert_segno;
	ao_vacuum_phase_config->appendonly_compaction_segno = compaction_segno;
	ao_vacuum_phase_config->appendonly_phase = phase;

	vacuum_rel(relid, relation, options, params, skip_twophase,
			   ao_vacuum_phase_config, onerel, lmode);
}




	 *
	 * Append-only relations don't support, nor need, a FULL vacuum, so perform
	 * a lazy vacuum instead, even if FULL was requested. Note that we have
	 * already locked the table, and if FULL was requested, we got an
	 * AccessExclusiveLock. Therefore, FULL isn't exactly the same as non-FULL
	 * on AO tables.



	
	/*
	 * If the relation has a secondary toast rel, vacuum that too while we
	 * still hold the session lock on the master table.  We do this in
	 * cleanup phase when it's AO table or in prepare phase if it's an
	 * empty AO table.
	 */
	if (toast_relid != InvalidOid)
		vacuum_rel(toast_relid, NULL, params);

	/*
	 * If an AO/CO table is empty on a segment,
	 * ao_vacuum_phase_config->appendonly_relation_empty will get set to true even in
	 * the compaction phase. In such a case, we end up updating the auxiliary
	 * tables and try to vacuum them all in the same transaction. This causes
	 * the auxiliary relation to not get vacuumed and it generates a notice to
	 * the user saying that transaction is already in progress. Hence we want
	 * to vacuum the auxliary relations only in cleanup phase or if we are in
	 * the prepare phase and the AO/CO table is empty.
	 */
	if (ao_vacuum_phase_config != NULL &&
		(ao_vacuum_phase_config->appendonly_phase == AOVAC_CLEANUP ||
		 (ao_vacuum_phase_config->appendonly_relation_empty &&
		  ao_vacuum_phase_config->appendonly_phase == AOVAC_PREPARE)))
	{
		/* do the same for an AO segments table, if any */
		if (aoseg_relid != InvalidOid && aoseg_rangevar != NULL)
			vacuum_rel(aoseg_relid, aoseg_rangevar, options, params,
					   skip_twophase, NULL, NULL, lmode);

		/* do the same for an AO block directory table, if any */
		if (aoblkdir_relid != InvalidOid && aoblkdir_rangevar != NULL)
			vacuum_rel(aoblkdir_relid, aoblkdir_rangevar, options, params,
					   skip_twophase, NULL, NULL, lmode);

		/* do the same for an AO visimap, if any */
		if (aovisimap_relid != InvalidOid && aovisimap_rangevar != NULL)
			vacuum_rel(aovisimap_relid, aovisimap_rangevar, options, params,
					   skip_twophase, NULL, NULL, lmode);
	}


	/*
	 * Update ao master tupcount the hard way after the compaction and
	 * after the drop.
	 */
	if (Gp_role == GP_ROLE_DISPATCH && !is_heap &&
		RelationIsAppendOptimized(onerel) &&
		ao_vacuum_phase_config->appendonly_compaction_segno)
	{
		Snapshot	appendOnlyMetaDataSnapshot = RegisterSnapshot(GetCatalogSnapshot(InvalidOid));

		if (ao_vacuum_phase_config->appendonly_phase == AOVAC_COMPACT)
		{
			/* In the compact phase, we need to update the information of the segment file we inserted into */
			if (list_length(ao_vacuum_phase_config->appendonly_compaction_insert_segno) == 1 &&
				linitial_int(ao_vacuum_phase_config->appendonly_compaction_insert_segno) == APPENDONLY_COMPACTION_SEGNO_INVALID)
			{
				/* this was a "pseudo" compaction phase. */
			}
			else
				UpdateMasterAosegTotalsFromSegments(onerel, appendOnlyMetaDataSnapshot, ao_vacuum_phase_config->appendonly_compaction_insert_segno, 0);
		}
		else if (ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
		{
			/* In the drop phase, we need to update the information of the compacted segment file(s) */
			UpdateMasterAosegTotalsFromSegments(onerel, appendOnlyMetaDataSnapshot, ao_vacuum_phase_config->appendonly_compaction_segno, 0);
		}

		UnregisterSnapshot(appendOnlyMetaDataSnapshot);
	}





/****************************************************************************
 *																			*
 *			Code for VACUUM FULL (only)										*
 *																			*
 ****************************************************************************
 */

static bool
vacuum_appendonly_index_should_vacuum(Relation aoRelation, int options,
									  AppendOnlyIndexVacuumState *vacuumIndexState,
									  double *rel_tuple_count)
{
	int64 hidden_tupcount;
	FileSegTotals *totals;

	Assert(RelationIsAppendOptimized(aoRelation));

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
		totals = GetSegFilesTotals(aoRelation, vacuumIndexState->appendOnlyMetaDataSnapshot);
	}
	else
	{
		Assert(RelationIsAoCols(aoRelation));
		totals = GetAOCSSSegFilesTotals(aoRelation, vacuumIndexState->appendOnlyMetaDataSnapshot);
	}
	hidden_tupcount = AppendOnlyVisimap_GetRelationHiddenTupleCount(&vacuumIndexState->visiMap);

	if(rel_tuple_count)
	{
		*rel_tuple_count = (double)(totals->totaltuples - hidden_tupcount);
		Assert((*rel_tuple_count) > -1.0);
	}

	pfree(totals);

	if(hidden_tupcount > 0 || (options & VACOPT_FULL))
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
vacuum_appendonly_indexes(Relation aoRelation, int options)
{
	int reindex_count = 1;
	int i;
	Relation   *Irel;
	int			nindexes;
	AppendOnlyIndexVacuumState vacuumIndexState;
	FileSegInfo **segmentFileInfo = NULL; /* Might be a casted AOCSFileSegInfo */
	int totalSegfiles;

	Assert(RelationIsAppendOptimized(aoRelation));

	memset(&vacuumIndexState, 0, sizeof(vacuumIndexState));

	elogif (Debug_appendonly_print_compaction, LOG,
			"Vacuum indexes for append-only relation %s",
			RelationGetRelationName(aoRelation));

	/* Now open all indexes of the relation */
	if ((options & VACOPT_FULL))
		vac_open_indexes(aoRelation, AccessExclusiveLock, &nindexes, &Irel);
	else
		vac_open_indexes(aoRelation, RowExclusiveLock, &nindexes, &Irel);

	vacuumIndexState.appendOnlyMetaDataSnapshot = GetActiveSnapshot();

	if (RelationIsAoRows(aoRelation))
	{
		segmentFileInfo = GetAllFileSegInfo(aoRelation,
											vacuumIndexState.appendOnlyMetaDataSnapshot,
											&totalSegfiles);
	}
	else
	{
		Assert(RelationIsAoCols(aoRelation));
		segmentFileInfo = (FileSegInfo **) GetAllAOCSFileSegInfo(aoRelation,
																vacuumIndexState.appendOnlyMetaDataSnapshot,
																&totalSegfiles);
	}

	AppendOnlyVisimap_Init(
			&vacuumIndexState.visiMap,
			aoRelation->rd_appendonly->visimaprelid,
			aoRelation->rd_appendonly->visimapidxid,
			AccessShareLock,
			vacuumIndexState.appendOnlyMetaDataSnapshot);

	AppendOnlyBlockDirectory_Init_forSearch(&vacuumIndexState.blockDirectory,
			vacuumIndexState.appendOnlyMetaDataSnapshot,
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
		if (options & VACOPT_VERBOSE)
			elevel = INFO;
		else
			elevel = DEBUG2;

		if (vacuum_appendonly_index_should_vacuum(aoRelation, options,
					&vacuumIndexState, &rel_tuple_count))
		{
			Assert(rel_tuple_count > -1.0);

			for (i = 0; i < nindexes; i++)
			{
				vacuum_appendonly_index(Irel[i], &vacuumIndexState,
										rel_tuple_count,
										elevel);
			}
			reindex_count++;
		}
		else
		{
			for (i = 0; i < nindexes; i++)
				scan_index(Irel[i], rel_tuple_count, true, elevel);
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
 *	scan_index() -- scan one index relation to update pg_class statistics.
 *
 * We use this when we have no deletions to do.
 */
static void
scan_index(Relation indrel, double num_tuples, bool check_stats, int elevel)
{
	IndexBulkDeleteResult *stats;
	IndexVacuumInfo ivinfo;
	PGRUsage	ru0;
	BlockNumber relallvisible;

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

	if (RelationIsAppendOptimized(indrel))
		relallvisible = 0;
	else
		visibilitymap_count(indrel, &relallvisible, NULL);

	/*
	 * Now update statistics in pg_class, but only if the index says the count
	 * is accurate.
	 */
	if (!stats->estimated_count)
		vac_update_relstats(indrel,
							stats->num_pages, stats->num_index_tuples,
							relallvisible,
							false,
							InvalidTransactionId,
							InvalidMultiXactId,
							false,
							true /* isvacuum */);

	ereport(elevel,
			(errmsg("index \"%s\" now contains %.0f row versions in %u pages",
					RelationGetRelationName(indrel),
					stats->num_index_tuples,
					stats->num_pages),
	errdetail("%u index pages have been deleted, %u are currently reusable.\n"
			  "%s.",
			  stats->pages_deleted, stats->pages_free,
			  pg_rusage_show(&ru0))));

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
						AppendOnlyIndexVacuumState *vacuumIndexState,
						double rel_tuple_count,
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

	/*
	 * Now update statistics in pg_class, but only if the index says the count
	 * is accurate.
	 */
	if (!stats->estimated_count)
		vac_update_relstats(indexRelation,
							stats->num_pages, stats->num_index_tuples,
							0, /* relallvisible */
							false,
							InvalidTransactionId,
							InvalidMultiXactId,
							false,
							true /* isvacuum */);

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
 * Collect a sample of rows from an AO or AOCS table.
 *
 * The block-sampling method used for heap tables doesn't work with
 * append-only tables. We could build a reasonably efficient block-sampling
 * method for AO tables, too, using the block directory, if it's available.
 * But for now, this scans the whole table.
 */
int
acquire_sample_rows_ao(Relation onerel, int elevel,
					   HeapTuple *rows, int targrows,
					   double *totalrows, double *totaldeadrows)
{
	AppendOnlyScanDesc aoScanDesc = NULL;
	AOCSScanDesc aocsScanDesc = NULL;
	Snapshot	appendOnlyMetaDataSnapshot;
	double		rstate;
	TupleTableSlot *slot;
	int			numrows = 0;	/* # rows now in reservoir */
	double		samplerows = 0; /* total # rows collected */
	double		rowstoskip = -1;	/* -1 means not set yet */

	/*
	 * the append-only meta data should never be fetched with
	 * SnapshotAny as bogus results are returned.
	 */
	appendOnlyMetaDataSnapshot = GetTransactionSnapshot();

	if (RelationIsAoRows(onerel))
		aoScanDesc = appendonly_beginscan(onerel,
										  SnapshotSelf,
										  appendOnlyMetaDataSnapshot,
										  0, NULL);
	else
	{
		int			natts = RelationGetNumberOfAttributes(onerel);
		bool	   *proj = (bool *) palloc(natts * sizeof(bool));
		int			i;

		for(i = 0; i < natts; i++)
			proj[i] = true;

		Assert(RelationIsAoCols(onerel));
		aocsScanDesc = aocs_beginscan(onerel,
									  SnapshotSelf,
									  appendOnlyMetaDataSnapshot,
									  RelationGetDescr(onerel), proj);
	}
	slot = MakeSingleTupleTableSlot(RelationGetDescr(onerel));

	/* Prepare for sampling rows */
	rstate = anl_init_selection_state(targrows);

	for (;;)
	{
		if (aoScanDesc)
			appendonly_getnext(aoScanDesc, ForwardScanDirection, slot);
		else
			aocs_getnext(aocsScanDesc, ForwardScanDirection, slot);

		if (TupIsNull(slot))
			break;

		if (rowstoskip < 0)
			rowstoskip = anl_get_next_S(samplerows, targrows,
										&rstate);

		/* XXX: this is copied from acquire_sample_rows_heap */

		/*
		 * The first targrows sample rows are simply copied into the
		 * reservoir. Then we start replacing tuples in the sample
		 * until we reach the end of the relation.	This algorithm is
		 * from Jeff Vitter's paper (see full citation below). It
		 * works by repeatedly computing the number of tuples to skip
		 * before selecting a tuple, which replaces a randomly chosen
		 * element of the reservoir (current set of tuples).  At all
		 * times the reservoir is a true random sample of the tuples
		 * we've passed over so far, so when we fall off the end of
		 * the relation we're done.
		 */
		if (numrows < targrows)
			rows[numrows++] = ExecCopySlotHeapTuple(slot);
		else
		{
			/*
			 * t in Vitter's paper is the number of records already
			 * processed.  If we need to compute a new S value, we
			 * must use the not-yet-incremented value of samplerows as
			 * t.
			 */
			if (rowstoskip < 0)
				rowstoskip = anl_get_next_S(samplerows, targrows,
											&rstate);

			if (rowstoskip <= 0)
			{
				/*
				 * Found a suitable tuple, so save it, replacing one
				 * old tuple at random
				 */
				int			k = (int) (targrows * anl_random_fract());

				Assert(k >= 0 && k < targrows);
				heap_freetuple(rows[k]);
				rows[k] = ExecCopySlotHeapTuple(slot);
			}

			rowstoskip -= 1;
		}

		samplerows += 1;
	}

	/* Get the total tuple count in the table */
	FileSegTotals *fstotal;
	int64		hidden_tupcount = 0;

	if (aoScanDesc)
	{
		fstotal = GetSegFilesTotals(onerel, SnapshotSelf);
		hidden_tupcount = AppendOnlyVisimap_GetRelationHiddenTupleCount(&aoScanDesc->visibilityMap);
	}
	else
	{
		fstotal = GetAOCSSSegFilesTotals(onerel, SnapshotSelf);
		hidden_tupcount = AppendOnlyVisimap_GetRelationHiddenTupleCount(&aocsScanDesc->visibilityMap);
	}
	*totalrows = (double) fstotal->totaltuples - hidden_tupcount;
	/*
	 * Currently, we always report 0 dead rows on an AO table. We could
	 * perhaps get a better estimate using the AO visibility map. But this
	 * will do for now.
	 */
	*totaldeadrows = 0;

	ExecDropSingleTupleTableSlot(slot);
	if (aoScanDesc)
		appendonly_endscan(aoScanDesc);
	if (aocsScanDesc)
		aocs_endscan(aocsScanDesc);

	return numrows;
}




#ifdef FAULT_INJECTOR
	if (ao_vacuum_phase_config != NULL &&
		ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
	{
			FaultInjector_InjectFaultIfSet(
				"compaction_before_segmentfile_drop",
				DDLNotSpecified,
				"",	// databaseName
				RelationGetRelationName(onerel)); // tableName
	}
	if (ao_vacuum_phase_config != NULL &&
		ao_vacuum_phase_config->appendonly_phase == AOVAC_CLEANUP)
	{
			FaultInjector_InjectFaultIfSet(
				"compaction_before_cleanup_phase",
				DDLNotSpecified,
				"",	// databaseName
				RelationGetRelationName(onerel)); // tableName
	}
#endif





/*
 * lazy_vacuum_aorel -- perform LAZY VACUUM for one Append-only relation.
 */
static void
lazy_vacuum_aorel(Relation onerel, int options, AOVacuumPhaseConfig *ao_vacuum_phase_config)
{
	LVRelStats *vacrelstats;
	bool		update_relstats = true;

	vacrelstats = (LVRelStats *) palloc0(sizeof(LVRelStats));

	switch (ao_vacuum_phase_config->appendonly_phase)
	{
		case AOVAC_PREPARE:
			elogif(Debug_appendonly_print_compaction, LOG,
				   "Vacuum prepare phase %s", RelationGetRelationName(onerel));

			vacuum_appendonly_indexes(onerel, options);
			if (RelationIsAoRows(onerel))
				AppendOnlyTruncateToEOF(onerel);
			else
				AOCSTruncateToEOF(onerel);

			/*
			 * MPP-23647.  For empty tables, we skip compaction phase
			 * and cleanup phase.  Therefore, we update the stats
			 * (specifically, relfrozenxid) in prepare phase if the
			 * table is empty.  Otherwise, the stats will be updated in
			 * the cleanup phase, when we would have computed the
			 * correct values for stats.
			 */
			if (ao_vacuum_phase_config->appendonly_relation_empty)
			{
				update_relstats = true;
				/*
				 * For an empty relation, the only stats we care about
				 * is relfrozenxid and relhasindex.  We need to be
				 * mindful of correctly setting relhasindex here.
				 * relfrozenxid is already taken care of above by
				 * calling vacuum_set_xid_limits().
				 */
				vacrelstats->hasindex = onerel->rd_rel->relhasindex;
			}
			else
			{
				/*
				 * For a non-empty relation, follow the usual
				 * compaction phases and do not update stats in
				 * prepare phase.
				 */
				update_relstats = false;
			}
			break;

		case AOVAC_COMPACT:
		case AOVAC_DROP:
			vacuum_appendonly_rel(onerel, options, ao_vacuum_phase_config);
			update_relstats = false;
			break;

		case AOVAC_CLEANUP:
			elogif(Debug_appendonly_print_compaction, LOG,
				   "Vacuum cleanup phase %s", RelationGetRelationName(onerel));

			vacuum_appendonly_fill_stats(onerel, GetActiveSnapshot(),
										 &vacrelstats->rel_pages,
										 &vacrelstats->new_rel_tuples,
										 &vacrelstats->hasindex);
			/* reset the remaining LVRelStats values */
			vacrelstats->nonempty_pages = 0;
			vacrelstats->num_dead_tuples = 0;
			vacrelstats->max_dead_tuples = 0;
			vacrelstats->tuples_deleted = 0;
			vacrelstats->pages_removed = 0;
			break;

		default:
			elog(ERROR, "invalid AO vacuum phase %d", ao_vacuum_phase_config->appendonly_phase);
	}

	if (update_relstats)
	{
		/* Update statistics in pg_class */
		vac_update_relstats(onerel,
							vacrelstats->rel_pages,
							vacrelstats->new_rel_tuples,
							0, /* AO does not currently have an equivalent to
							      Heap's 'all visible pages' */
							vacrelstats->hasindex,
							FreezeLimit,
							MultiXactCutoff,
							false,
							true /* isvacuum */);

		/*
		 * Report results to the stats collector.
		 * Explicitly pass 0 as num_dead_tuples. AO tables use hidden
		 * tuples in a conceptually similar way that regular tables
		 * use dead tuples. However with regards to pgstat these are
		 * semantically distinct thus exposing them will be ambiguous.
		 */
		pgstat_report_vacuum(RelationGetRelid(onerel),
							 onerel->rd_rel->relisshared,
							 vacrelstats->new_rel_tuples,
							 0 /* num_dead_tuples */);
	}
}





/*
 * Fills in the relation statistics for an append-only relation.
 *
 *	This information is used to update the reltuples and relpages information
 *	in pg_class. reltuples is the same as "pg_aoseg_<oid>:tupcount"
 *	column and we simulate relpages by subdividing the eof value
 *	("pg_aoseg_<oid>:eof") over the defined page size.
 *
 * Note: In QD, we don't track the file size across segments, so even
 * though the tuple count is returned correctly, the number of pages is
 * always 0.
 */
void
vacuum_appendonly_fill_stats(Relation aorel, Snapshot snapshot,
							 BlockNumber *rel_pages, double *rel_tuples,
							 bool *relhasindex)
{
	FileSegTotals *fstotal;
	BlockNumber nblocks;
	char	   *relname;
	double		num_tuples;
	double		totalbytes;
	double		eof;
	int64       hidden_tupcount;
	AppendOnlyVisimap visimap;

	Assert(RelationIsAppendOptimized(aorel));

	relname = RelationGetRelationName(aorel);

	/* get updated statistics from the pg_aoseg table */
	if (RelationIsAoRows(aorel))
	{
		fstotal = GetSegFilesTotals(aorel, snapshot);
	}
	else
	{
		Assert(RelationIsAoCols(aorel));
		fstotal = GetAOCSSSegFilesTotals(aorel, snapshot);
	}

	/* calculate the values we care about */
	eof = (double)fstotal->totalbytes;
	num_tuples = (double)fstotal->totaltuples;
	totalbytes = eof;
	nblocks = (uint32)RelationGuessNumberOfBlocks(totalbytes);

	AppendOnlyVisimap_Init(&visimap,
						   aorel->rd_appendonly->visimaprelid,
						   aorel->rd_appendonly->visimapidxid,
						   AccessShareLock,
						   snapshot);
	hidden_tupcount = AppendOnlyVisimap_GetRelationHiddenTupleCount(&visimap);
	num_tuples -= hidden_tupcount;
	Assert(num_tuples > -1.0);
	AppendOnlyVisimap_Finish(&visimap, AccessShareLock);

	elogif (Debug_appendonly_print_compaction, LOG,
			"Gather statistics after vacuum for append-only relation %s: "
			"page count %d, tuple count %f",
			relname,
			nblocks, num_tuples);

	*rel_pages = nblocks;
	*rel_tuples = num_tuples;
	*relhasindex = aorel->rd_rel->relhasindex;

	ereport(elevel,
			(errmsg("\"%s\": found %.0f rows in %u pages.",
					relname, num_tuples, nblocks)));
	pfree(fstotal);
}

/*
 *	vacuum_appendonly_rel() -- vaccum an append-only relation
 *
 *		This procedure will be what gets executed both for VACUUM
 *		and VACUUM FULL (and also ANALYZE or any other thing that
 *		needs the pg_class stats updated).
 *
 *		The function can compact append-only segment files or just
 *		truncating the segment file to its existing eof.
 *
 *		Afterwards, the reltuples and relpages information in pg_class
 *		are updated. reltuples is the same as "pg_aoseg_<oid>:tupcount"
 *		column and we simulate relpages by subdividing the eof value
 *		("pg_aoseg_<oid>:eof") over the defined page size.
 *
 *
 *		There are txn ids, hint bits, free space, dead tuples,
 *		etc. these are all irrelevant in the append only relation context.
 *
 */
void
vacuum_appendonly_rel(Relation aorel, int options, AOVacuumPhaseConfig *ao_vacuum_phase_config)
{
	char	   *relname;

	Assert(RelationIsAppendOptimized(aorel));

	relname = RelationGetRelationName(aorel);
	ereport(elevel,
			(errmsg("vacuuming \"%s.%s\"",
					get_namespace_name(RelationGetNamespace(aorel)),
					relname)));

	if (Gp_role == GP_ROLE_DISPATCH)
	{
		return;
	}

	if (ao_vacuum_phase_config->appendonly_phase == AOVAC_DROP)
	{
		Assert(!ao_vacuum_phase_config->appendonly_compaction_insert_segno);

		elogif(Debug_appendonly_print_compaction, LOG,
			"Vacuum drop phase %s", RelationGetRelationName(aorel));

		if (RelationIsAoRows(aorel))
		{
			AppendOnlyDrop(aorel, ao_vacuum_phase_config->appendonly_compaction_segno);
		}
		else
		{
			Assert(RelationIsAoCols(aorel));
			AOCSDrop(aorel, ao_vacuum_phase_config->appendonly_compaction_segno);
		}
	}
	else
	{
		Assert(ao_vacuum_phase_config->appendonly_phase == AOVAC_COMPACT);
		Assert(list_length(ao_vacuum_phase_config->appendonly_compaction_insert_segno) == 1);

		int insert_segno = linitial_int(ao_vacuum_phase_config->appendonly_compaction_insert_segno);

		if (insert_segno == APPENDONLY_COMPACTION_SEGNO_INVALID)
		{
			elogif(Debug_appendonly_print_compaction, LOG,
			"Vacuum pseudo-compaction phase %s", RelationGetRelationName(aorel));
		}
		else
		{
			elogif(Debug_appendonly_print_compaction, LOG,
				"Vacuum compaction phase %s", RelationGetRelationName(aorel));
			if (RelationIsAoRows(aorel))
			{
				AppendOnlyCompact(aorel,
								  ao_vacuum_phase_config->appendonly_compaction_segno,
								  insert_segno, (options & VACOPT_FULL));
			}
			else
			{
				Assert(RelationIsAoCols(aorel));
				AOCSCompact(aorel,
							ao_vacuum_phase_config->appendonly_compaction_segno,
							insert_segno, (options & VACOPT_FULL));
			}
		}
	}
}
