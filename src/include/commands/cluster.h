/*-------------------------------------------------------------------------
 *
 * cluster.h
 *	  header file for postgres cluster command stuff
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 * src/include/commands/cluster.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CLUSTER_H
#define CLUSTER_H

#include "nodes/parsenodes.h"
#include "storage/lock.h"
#include "utils/relcache.h"


extern void cluster(ClusterStmt *stmt, bool isTopLevel);
<<<<<<< HEAD
extern bool cluster_rel(Oid tableOid, Oid indexOid, bool recheck,
			bool verbose, bool printError);
=======
extern void cluster_rel(Oid tableOid, Oid indexOid, int options);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern void check_index_is_clusterable(Relation OldHeap, Oid indexOid,
									   bool recheck, LOCKMODE lockmode);
extern void mark_index_clustered(Relation rel, Oid indexOid, bool is_internal);

<<<<<<< HEAD
extern Oid make_new_heap(Oid OIDOldHeap, Oid NewTableSpace, char relpersistence,
			  LOCKMODE lockmode,
			  bool createAoBlockDirectory,
			  bool makeCdbPolicy);
extern void finish_heap_swap(Oid OIDOldHeap, Oid OIDNewHeap,
				 bool is_system_catalog,
				 bool swap_toast_by_content,
				 bool swap_stats,
				 bool check_constraints,
				 bool is_internal,
				 TransactionId frozenXid,
				 MultiXactId minMulti,
				 char newrelpersistence);

extern void swap_relation_files(Oid r1, Oid r2, bool target_is_pg_class,
					bool swap_toast_by_content,
					bool swap_stats,
					bool is_internal,
					TransactionId frozenXid,
					MultiXactId frozenMulti,
					Oid *mapped_tables);

#endif   /* CLUSTER_H */
=======
extern Oid	make_new_heap(Oid OIDOldHeap, Oid NewTableSpace, char relpersistence,
						  LOCKMODE lockmode);
extern void finish_heap_swap(Oid OIDOldHeap, Oid OIDNewHeap,
							 bool is_system_catalog,
							 bool swap_toast_by_content,
							 bool check_constraints,
							 bool is_internal,
							 TransactionId frozenXid,
							 MultiXactId minMulti,
							 char newrelpersistence);

#endif							/* CLUSTER_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
