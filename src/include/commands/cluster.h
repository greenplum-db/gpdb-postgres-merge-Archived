/*-------------------------------------------------------------------------
 *
 * cluster.h
 *	  header file for postgres cluster command stuff
 *
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/cluster.h,v 1.41 2010/02/26 02:01:24 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef CLUSTER_H
#define CLUSTER_H

#include "nodes/parsenodes.h"
#include "utils/relcache.h"


extern void cluster(ClusterStmt *stmt, bool isTopLevel);
extern void cluster_rel(Oid tableOid, Oid indexOid, bool recheck,
			bool verbose, int freeze_min_age, int freeze_table_age);
extern void check_index_is_clusterable(Relation OldHeap, Oid indexOid,
						   bool recheck);
extern void mark_index_clustered(Relation rel, Oid indexOid);
<<<<<<< HEAD
extern Oid make_new_heap(Oid OIDOldHeap, const char *NewName,
			  Oid NewTableSpace,
			  bool createAoBlockDirectory);
extern void swap_relation_files(Oid r1, Oid r2, TransactionId frozenXid,
								bool swap_stats);
=======

extern Oid	make_new_heap(Oid OIDOldHeap, Oid NewTableSpace);
extern void finish_heap_swap(Oid OIDOldHeap, Oid OIDNewHeap,
				 bool is_system_catalog,
				 bool swap_toast_by_content,
				 TransactionId frozenXid);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

#endif   /* CLUSTER_H */
