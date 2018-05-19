/*-------------------------------------------------------------------------
 *
 * print.h
 *	  definitions for nodes/print.c
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2007-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/nodes/print.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PRINT_H
#define PRINT_H

<<<<<<< HEAD
#include "nodes/parsenodes.h"
#include "nodes/plannodes.h"
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
#include "executor/tuptable.h"

#define nodeDisplay(x)		pprint(x)

<<<<<<< HEAD
extern char *plannode_type(Plan *p);
extern void print(void *obj);
extern void pprint(void *obj);
=======
extern void print(const void *obj);
extern void pprint(const void *obj);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
extern void elog_node_display(int lev, const char *title,
				  const void *obj, bool pretty);
extern char *format_node_dump(const char *dump);
extern char *pretty_format_node_dump(const char *dump);
extern void print_rt(const List *rtable);
extern void print_expr(const Node *expr, const List *rtable);
extern void print_pathkeys(const List *pathkeys, const List *rtable);
extern void print_tl(const List *tlist, const List *rtable);
extern void print_slot(TupleTableSlot *slot);
extern void print_plan(Plan *p, Query *parsetree);

#endif   /* PRINT_H */
