/*-------------------------------------------------------------------------
 *
 * catalog.h
 *	  prototypes for functions in backend/catalog/catalog.c
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/catalog.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CATALOG_H
#define CATALOG_H

#include "catalog/pg_class.h"
#include "storage/relfilenode.h"
#include "utils/relcache.h"

<<<<<<< HEAD
#define OIDCHARS		10		/* max chars printed by %u */
/*
 * In PostgreSQL, this is called just TABLESPACE_VERSION_DIRECTORY..
 * This constant has been renamed so that we catch and know to modify all
 * upstream uses of TABLESPACE_VERSION_DIRECTORY.
 */
#define GP_TABLESPACE_VERSION_DIRECTORY	"GPDB_" GP_MAJORVERSION "_" \
									CppAsString2(CATALOG_VERSION_NO)

/*
 * This file is used to store internal configuration information specific to a
 * server that's not same between primary and mirror pair. For example it
 * stores gp_dbid, which is different for primary and mirror pair, even if
 * contentid is same for them. This file is not copied over during
 * pg_basebackup and pg_rewind to mirror from primary.
 */
#define GP_INTERNAL_AUTO_CONF_FILE_NAME "internal.auto.conf"
=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

extern bool IsSystemRelation(Relation relation);
extern bool IsToastRelation(Relation relation);
extern bool IsCatalogRelation(Relation relation);

extern bool IsSystemClass(Oid relid, Form_pg_class reltuple);
extern bool IsToastClass(Form_pg_class reltuple);

extern bool IsCatalogRelationOid(Oid relid);

extern bool IsCatalogNamespace(Oid namespaceId);
extern bool IsToastNamespace(Oid namespaceId);
extern bool IsAoSegmentNamespace(Oid namespaceId);

extern bool IsReservedName(const char *name);
extern char* GetReservedPrefix(const char *name);

extern bool IsSharedRelation(Oid relationId);

<<<<<<< HEAD
extern Oid GetNewOid(Relation relation);
extern Oid GetNewOidWithIndex(Relation relation, Oid indexId,
				   AttrNumber oidcolumn);
extern Oid GetNewRelFileNode(Oid reltablespace, Relation pg_class,
				  char relpersistence);

extern void reldir_and_filename(RelFileNode rnode, BackendId backend, ForkNumber forknum,
					char **dir, char **filename);
extern char *aorelpathbackend(RelFileNode node, BackendId backend, int32 segno);

#endif   /* CATALOG_H */
=======
extern Oid	GetNewOidWithIndex(Relation relation, Oid indexId,
							   AttrNumber oidcolumn);
extern Oid	GetNewRelFileNode(Oid reltablespace, Relation pg_class,
							  char relpersistence);

#endif							/* CATALOG_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
