/*-------------------------------------------------------------------------
 *
 * catalog.h
 *	  prototypes for functions in backend/catalog/catalog.c
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/catalog.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CATALOG_H
#define CATALOG_H

#include "catalog/catversion.h"
#include "catalog/pg_class.h"
#include "storage/relfilenode.h"
#include "utils/relcache.h"

#include "catalog/oid_dispatch.h"
#include "cdb/cdbvars.h"

#define OIDCHARS		10		/* max chars printed by %u */
/*
 * In PostgreSQL, this is called just TABLESPACE_VERSION_DIRECTORY. But in 
 * GPDB, you should use tablespace_version_directory() function instead.
 * This constant has been renamed so that we catch and know to modify all
 * upstream uses of TABLESPACE_VERSION_DIRECTORY.
 */
#define GP_TABLESPACE_VERSION_DIRECTORY	"GPDB_" GP_MAJORVERSION "_" \
									CppAsString2(CATALOG_VERSION_NO)

extern const char *forkNames[];
extern ForkNumber forkname_to_number(char *forkName);
extern int	forkname_chars(const char *str, ForkNumber *);

extern char *relpathbackend(RelFileNode rnode, BackendId backend,
			   ForkNumber forknum);
extern char *GetDatabasePath(Oid dbNode, Oid spcNode);

extern void reldir_and_filename(RelFileNode rnode, BackendId backend, ForkNumber forknum,
					char **dir, char **filename);

/* First argument is a RelFileNodeBackend */
#define relpath(rnode, forknum) \
		relpathbackend((rnode).node, (rnode).backend, (forknum))

/* First argument is a RelFileNode */
#define relpathperm(rnode, forknum) \
		relpathbackend((rnode), InvalidBackendId, (forknum))

/*
 * In PostgreSQL, the backend's backend ID is used as part of the filenames
 * of temporary tables. However, in GPDB, temporary tables are shared across
 * backends, if you have a query with multiple QE reader processes. Because
 * of that, they are kept in the shared buffer cache, but it also means that
 * we cannot use the "current backend ID" in the filename, because each
 * QE process has a different backend ID. Use the current "session id"
 * instead.
 *
 * MyTempSessionId() macro should be used in place of MyBackendId, wherever
 * we deal with RelFileNodes. That includes at leastRelFileNodeBackend.backend
 * and RelationData.rd_backend fields.
 */
#define MyTempSessionId() \
	((Gp_role == GP_ROLE_DISPATCH || Gp_role == GP_ROLE_EXECUTE) ? gp_session_id : MyBackendId)

extern bool IsSystemRelation(Relation relation);
extern bool IsToastRelation(Relation relation);

extern bool IsSystemClass(Form_pg_class reltuple);
extern bool IsToastClass(Form_pg_class reltuple);

extern bool IsSystemNamespace(Oid namespaceId);
extern bool IsToastNamespace(Oid namespaceId);
extern bool IsAoSegmentNamespace(Oid namespaceId);

extern bool IsReservedName(const char *name);
extern char* GetReservedPrefix(const char *name);

extern bool IsSharedRelation(Oid relationId);

extern Oid GetNewOid(Relation relation);
extern Oid GetNewOidWithIndex(Relation relation, Oid indexId,
				   AttrNumber oidcolumn);
extern Oid GetNewSequenceRelationOid(Relation relation);
extern Oid GetNewRelFileNode(Oid reltablespace, Relation pg_class,
				  char relpersistence);

const char *tablespace_version_directory(void);

#endif   /* CATALOG_H */
