/*-------------------------------------------------------------------------
 *
 * relcache.h
 *	  Relation descriptor cache definitions.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2005-2009, Greenplum inc.
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/utils/relcache.h,v 1.59 2007/03/29 00:15:39 tgl Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/utils/relcache.h,v 1.61 2008/01/01 19:45:59 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef RELCACHE_H
#define RELCACHE_H

#include "access/htup.h"
#include "access/skey.h"
#include "access/tupdesc.h"
#include "nodes/bitmapset.h"
#include "nodes/pg_list.h"
#include "catalog/pg_class.h"


struct SysScanDescData; /* defined in access/genam.h */

typedef struct RelationData *Relation;

/* ----------------
 *		RelationPtr is used in the executor to support index scans
 *		where we have to keep track of several index relations in an
 *		array.	-cim 9/10/89
 * ----------------
 */
typedef Relation *RelationPtr;

/*
 * Routines to open (lookup) and close a relcache entry
 */
extern Relation RelationIdGetRelation(Oid relationId);
extern void RelationClose(Relation relation);

/*
 * Routines to compute/retrieve additional cached information
 */
struct GpPolicy *RelationGetPartitioningKey(Relation relation);
extern List *RelationGetIndexList(Relation relation);
extern Oid	RelationGetOidIndex(Relation relation);
extern List *RelationGetIndexExpressions(Relation relation);
extern List *RelationGetIndexPredicate(Relation relation);
extern Bitmapset *RelationGetIndexAttrBitmap(Relation relation);

extern void RelationSetIndexList(Relation relation,
					 List *indexIds, Oid oidIndex);

extern void RelationInitIndexAccessInfo(Relation relation);

/*
 * Routines for backend startup
 */
extern void RelationCacheInitialize(void);
extern void RelationCacheInitializePhase2(void);
extern void RelationCacheInitializePhase3(void);
	
/*
 * Routine to create a relcache entry for an about-to-be-created relation
 */
extern Relation RelationBuildLocalRelation(const char *relname,
						   Oid relnamespace,
						   TupleDesc tupDesc,
						   Oid relid,
						   Oid reltablespace,
			               char relkind,            /*CDB*/
						   bool shared_relation);

/*
 * Routines for flushing/rebuilding relcache entries in various scenarios
 */
extern void RelationForgetRelation(Oid rid);

extern void RelationCacheInvalidateEntry(Oid relationId);

extern void RelationCacheInvalidate(void);

extern void AtEOXact_RelationCache(bool isCommit);
extern void AtEOSubXact_RelationCache(bool isCommit, SubTransactionId mySubid,
						  SubTransactionId parentSubid);

extern void RelationCacheMarkNewRelfilenode(Relation rel);

/*
 * Routines to help manage rebuilding of relcache init file
 */
extern bool RelationIdIsInInitFile(Oid relationId);
<<<<<<< HEAD
extern void RelationCacheInitFileInvalidate(bool beforeSend);
extern void RelationCacheInitFileRemove(void);

extern void IndexSupportInitialize(oidvector *indclass,
					   Oid *indexOperator,
					   RegProcedure *indexSupport,
					   Oid *opFamily,
					   Oid *opcInType,
					   StrategyNumber maxStrategyNumber,
					   StrategyNumber maxSupportNumber,
					   AttrNumber maxAttributeNumber);
=======
extern void RelationCacheInitFilePreInvalidate(void);
extern void RelationCacheInitFilePostInvalidate(void);
extern void RelationCacheInitFileRemove(const char *dbPath);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

/* should be used only by relcache.c and catcache.c */
extern bool criticalRelcachesBuilt;
extern bool criticalSharedRelcachesBuilt;

#endif   /* RELCACHE_H */
