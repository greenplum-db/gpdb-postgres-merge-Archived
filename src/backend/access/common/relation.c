/*-------------------------------------------------------------------------
 *
 * relation.c
 *	  Generic relation related routines.
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  src/backend/access/common/relation.c
 *
 * NOTES
 *	  This file contains relation_ routines that implement access to relations
 *	  (tables, indexes, etc). Support that's specific to subtypes of relations
 *	  should go into their respective files, not here.
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

#include "access/relation.h"
#include "access/xact.h"
#include "catalog/namespace.h"
#include "miscadmin.h"
#include "pgstat.h"
#include "storage/lmgr.h"
#include "utils/inval.h"
#include "utils/syscache.h"


/* ----------------
 *		relation_open - open any relation by relation OID
 *
 *		If lockmode is not "NoLock", the specified kind of lock is
 *		obtained on the relation.  (Generally, NoLock should only be
 *		used if the caller knows it has some appropriate lock on the
 *		relation already.)
 *
 *		An error is raised if the relation does not exist.
 *
 *		NB: a "relation" is anything with a pg_class entry.  The caller is
 *		expected to check whether the relkind is something it can handle.
 * ----------------
 */
Relation
relation_open(Oid relationId, LOCKMODE lockmode)
{
	Relation	r;

	Assert(lockmode >= NoLock && lockmode < MAX_LOCKMODES);

	/* Get the lock before trying to open the relcache entry */
	if (lockmode != NoLock)
		LockRelationOid(relationId, lockmode);

	/* The relcache does all the real work... */
	r = RelationIdGetRelation(relationId);

	/* GPDB_12_MERGE_FIXME: We had added the errdetail in GPDB. Is it still valid? */
	if (!RelationIsValid(r))
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_TABLE),
				 errmsg("could not open relation with OID %u", relationId),
				 errdetail("This can be validly caused by a concurrent delete operation on this object.")));

	/*
	 * If we didn't get the lock ourselves, assert that caller holds one,
	 * except in bootstrap mode where no locks are used.
	 */
	Assert(lockmode != NoLock ||
		   IsBootstrapProcessingMode() ||
		   CheckRelationLockedByMe(r, AccessShareLock, true));

#if 0 /* Upstream code not applicable to GPDB */
	/* Make note that we've accessed a temporary relation */
	if (RelationUsesLocalBuffers(r))
		MyXactFlags |= XACT_FLAGS_ACCESSEDTEMPNAMESPACE;
#endif

	pgstat_initstats(r);

	return r;
}

/* ----------------
 *		try_relation_open - open any relation by relation OID
 *
 *		Same as relation_open, except return NULL instead of failing
 *		if the relation does not exist.
 * ----------------
 */
Relation
try_relation_open(Oid relationId, LOCKMODE lockmode, bool noWait)
{
	Relation	r;

	Assert(lockmode >= NoLock && lockmode < MAX_LOCKMODES);

	/* Get the lock first */
	if (lockmode != NoLock)
	{
		if (!noWait)
			LockRelationOid(relationId, lockmode);
		else
		{
			/*
			 * noWait is a Greenplum addition to the open_relation code
			 * basically to support INSERT ... FOR UPDATE NOWAIT.  Our NoWait
			 * handling needs to be more tolerant of failed locks than standard
			 * postgres largely due to the fact that we have to promote certain
			 * update locks in order to handle distributed updates.
			 */
			if (!ConditionalLockRelationOid(relationId, lockmode))
				return NULL;
		}
	}

	/*
	 * Now that we have the lock, probe to see if the relation really exists
	 * or not.
	 */
	if (!SearchSysCacheExists1(RELOID, ObjectIdGetDatum(relationId)))
	{
		/* Release useless lock */
		if (lockmode != NoLock)
			UnlockRelationOid(relationId, lockmode);

		return NULL;
	}

	/* Should be safe to do a relcache load */
	r = RelationIdGetRelation(relationId);

	if (!RelationIsValid(r))
	{
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_TABLE),
				 errmsg("could not open relation with OID %u", relationId),
				 errdetail("This can be validly caused by a concurrent delete operation on this object.")));
	}

	/* If we didn't get the lock ourselves, assert that caller holds one */
	Assert(lockmode != NoLock ||
		   CheckRelationLockedByMe(r, AccessShareLock, true));

#if 0 /* Upstream code not applicable to GPDB */
	/* Make note that we've accessed a temporary relation */
	if (RelationUsesLocalBuffers(r))
		MyXactFlags |= XACT_FLAGS_ACCESSEDTEMPNAMESPACE;
#endif

	pgstat_initstats(r);

	return r;
}

/*
 * CdbTryOpenRelation -- Opens a relation with a specified lock mode.
 *
 * CDB: Like try_relation_open, except that it will upgrade the lock when needed
 * for distributed tables.
 */
Relation
CdbTryOpenRelation(Oid relid, LOCKMODE reqmode, bool noWait, bool *lockUpgraded)
{
    LOCKMODE    lockmode = reqmode;
	Relation    rel;

	if (lockUpgraded != NULL)
		*lockUpgraded = false;

    /*
	 * Since we have introduced GDD(global deadlock detector), for heap table
	 * we do not need to upgrade the requested lock. For ao table, because of
	 * the design of ao table's visibilitymap, we have to upgrade the lock
	 * (More details please refer https://groups.google.com/a/greenplum.org/forum/#!topic/gpdb-dev/iDj8WkLus4g)
	 *
	 * And we select for update statement's lock is upgraded at addRangeTableEntry.
	 *
	 * Note: This code could be improved substantially after we redesign ao table
	 * and select for update.
	 */
	if (lockmode == RowExclusiveLock)
	{
		if (Gp_role == GP_ROLE_DISPATCH &&
			CondUpgradeRelLock(relid, noWait))
		{
			lockmode = ExclusiveLock;
			if (lockUpgraded != NULL)
				*lockUpgraded = true;
		}
    }

	rel = try_relation_open(relid, lockmode, noWait);
	if (!RelationIsValid(rel))
		return NULL;

	/* 
	 * There is a slim chance that ALTER TABLE SET DISTRIBUTED BY may
	 * have altered the distribution policy between the time that we
	 * decided to upgrade the lock and the time we opened the relation
	 * with the lock.  Double check that our chosen lock mode is still
	 * okay.
	 */
	if (lockmode == RowExclusiveLock &&
		Gp_role == GP_ROLE_DISPATCH && RelationIsAppendOptimized(rel))
	{
		elog(ERROR, "relation \"%s\" concurrently updated", 
			 RelationGetRelationName(rel));
	}

	/* inject fault after holding the lock */
	SIMPLE_FAULT_INJECTOR("upgrade_row_lock");

	return rel;
}                                       /* CdbOpenRelation */

/*
 * CdbOpenRelation -- Opens a relation with a specified lock mode.
 *
 * CDB: Like CdbTryOpenRelation, except that it guarantees either
 * an error or a valid opened relation returned.
 */
Relation
CdbOpenRelation(Oid relid, LOCKMODE reqmode, bool noWait, bool *lockUpgraded)
{
	Relation rel;

	rel = CdbTryOpenRelation(relid, reqmode, noWait, lockUpgraded);

	if (!RelationIsValid(rel))
	{
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_TABLE),
				 errmsg("relation not found (OID %u)", relid),
				 errdetail("This can be validly caused by a concurrent delete operation on this object.")));
	}

	return rel;

}                                       /* CdbOpenRelation */

/*
 * CdbOpenRelationRv -- Opens a relation with a specified lock mode.
 *
 * CDB: Like CdbTryOpenRelation, except that it guarantees either
 * an error or a valid opened relation returned.
 */
Relation
CdbOpenRelationRv(const RangeVar *relation, LOCKMODE reqmode, bool noWait, 
				  bool *lockUpgraded)
{
	Oid			relid;
	Relation	rel;

	/* Look up the appropriate relation using namespace search */
	relid = RangeVarGetRelid(relation, NoLock, false);
	rel = CdbTryOpenRelation(relid, reqmode, noWait, lockUpgraded);

	if (!RelationIsValid(rel))
	{
		if (relation->schemaname)
		{
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_TABLE),
					 errmsg("relation \"%s.%s\" does not exist",
							relation->schemaname, relation->relname)));
		}
		else
		{
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_TABLE),
					 errmsg("relation \"%s\" does not exist",
							relation->relname)));
		}
	}

	return rel;

}                                       /* CdbOpenRelation */

/* ----------------
 *		relation_openrv - open any relation specified by a RangeVar
 *
 *		Same as relation_open, but the relation is specified by a RangeVar.
 * ----------------
 */
Relation
relation_openrv(const RangeVar *relation, LOCKMODE lockmode)
{
	Oid			relOid;

	/*
	 * Check for shared-cache-inval messages before trying to open the
	 * relation.  This is needed even if we already hold a lock on the
	 * relation, because GRANT/REVOKE are executed without taking any lock on
	 * the target relation, and we want to be sure we see current ACL
	 * information.  We can skip this if asked for NoLock, on the assumption
	 * that such a call is not the first one in the current command, and so we
	 * should be reasonably up-to-date already.  (XXX this all could stand to
	 * be redesigned, but for the moment we'll keep doing this like it's been
	 * done historically.)
	 */
	if (lockmode != NoLock)
		AcceptInvalidationMessages();

	/* Look up and lock the appropriate relation using namespace search */
	relOid = RangeVarGetRelid(relation, lockmode, false);

	/* 
	 * use try_relation_open instead of relation_open so that we can
	 * throw a more graceful error message if the relation was dropped
	 * between the RangeVarGetRelid and when we try to open the relation.
	 */
	rel = try_relation_open(relOid, NoLock, false);

	if (!RelationIsValid(rel))
	{
		if (relation->schemaname)
		{
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_TABLE),
					 errmsg("relation \"%s.%s\" does not exist",
							relation->schemaname, relation->relname)));
		}
		else
		{
			ereport(ERROR,
					(errcode(ERRCODE_UNDEFINED_TABLE),
					 errmsg("relation \"%s\" does not exist",
							relation->relname)));
		}
	}

	return rel;
}

/* ----------------
 *		relation_openrv_extended - open any relation specified by a RangeVar
 *
 *		Same as relation_openrv, but with an additional missing_ok argument
 *		allowing a NULL return rather than an error if the relation is not
 *		found.  (Note that some other causes, such as permissions problems,
 *		will still result in an ereport.)
 * ----------------
 */
Relation
relation_openrv_extended(const RangeVar *relation, LOCKMODE lockmode,
						 bool missing_ok, bool noWait)
{
	Oid			relOid;

	/*
	 * Check for shared-cache-inval messages before trying to open the
	 * relation.  See comments in relation_openrv().
	 */
	if (lockmode != NoLock)
		AcceptInvalidationMessages();

	/* Look up and lock the appropriate relation using namespace search */
	relOid = RangeVarGetRelid(relation, lockmode, missing_ok);

	/* Return NULL on not-found */
	if (!OidIsValid(relOid))
		return NULL;

	/* GPDB_12_MERGE_FIXME: In PostgreSQL, we use NoLock here, becaue
	 * RangeVarGetRelid() locks the relation already. Is 'noWait' actually
	 * accomplishing anything here?
	 */
	/* Let try_relation_open do the rest */
	return try_relation_open(relOid, lockmode, noWait);
}

/* ----------------
 *		relation_close - close any relation
 *
 *		If lockmode is not "NoLock", we then release the specified lock.
 *
 *		Note that it is often sensible to hold a lock beyond relation_close;
 *		in that case, the lock is released automatically at xact end.
 * ----------------
 */
void
relation_close(Relation relation, LOCKMODE lockmode)
{
	LockRelId	relid = relation->rd_lockInfo.lockRelId;

	Assert(lockmode >= NoLock && lockmode < MAX_LOCKMODES);

	/* The relcache does the real work... */
	RelationClose(relation);

	if (lockmode != NoLock)
		UnlockRelationId(&relid, lockmode);
	else
	{
		LOCKTAG		tag;

		SET_LOCKTAG_RELATION(tag, relid.dbId, relid.relId);

		/*
		 * Closing with NoLock is a sufficient condition for a relation lock
		 * to be transaction-level(means the lock can only be released after
		 * the holding transaction is over).
		 * This is because the difference betwwen the ref counts in the
		 * relation and the lock tag can not be removed.
		 * So this is a good time to set the holdTillEndXact flag for the lock.
		 */
		LockSetHoldTillEndXact(&tag);
	}
}
