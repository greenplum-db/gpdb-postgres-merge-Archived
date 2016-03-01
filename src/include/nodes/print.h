/*-------------------------------------------------------------------------
 *
 * print.h
 *	  definitions for nodes/print.c
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2007-2008, Greenplum inc
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/nodes/print.h,v 1.28 2008/01/01 19:45:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PRINT_H
#define PRINT_H

#include "nodes/parsenodes.h"
#include "nodes/execnodes.h"

#define nodeDisplay(x)		pprint(x)

extern char *plannode_type(Plan *p);
extern void print(void *obj);
extern void pprint(void *obj);
extern void elog_node_display(int lev, const char *title,
				  void *obj, bool pretty);
extern char *format_node_dump(const char *dump);
extern char *pretty_format_node_dump(const char *dump);
extern void print_rt(List *rtable);
extern void print_expr(Node *expr, List *rtable);
extern void print_pathkeys(List *pathkeys, List *rtable);
extern void print_tl(List *tlist, List *rtable);
extern void print_slot(TupleTableSlot *slot);

#endif   /* PRINT_H */
