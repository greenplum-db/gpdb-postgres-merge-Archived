/*-------------------------------------------------------------------------
 *
 * cluster.h
 *	  header file for postgres cluster command stuff
 *
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/cluster.h,v 1.34 2008/01/01 19:45:57 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef CLUSTER_H
#define CLUSTER_H

#include "nodes/parsenodes.h"
#include "utils/rel.h"


extern void cluster(ClusterStmt *stmt, bool isTopLevel);

extern void check_index_is_clusterable(Relation OldHeap, Oid indexOid,
						   bool recheck);
extern void mark_index_clustered(Relation rel, Oid indexOid);
<<<<<<< HEAD
extern Oid make_new_heap(Oid OIDOldHeap, const char *NewName, Oid NewTableSpace, 
		TableOidInfo * oidInfo,
        bool createAoBlockDirectory);
extern void swap_relation_files(Oid r1, Oid r2, bool swap_stats);

extern void populate_oidInfo(TableOidInfo *oidInfo, Oid TableSpace, 
							 bool relisshared, bool withTypes);
=======
extern Oid make_new_heap(Oid OIDOldHeap, const char *NewName,
			  Oid NewTableSpace);
extern void swap_relation_files(Oid r1, Oid r2, TransactionId frozenXid);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

#endif   /* CLUSTER_H */
