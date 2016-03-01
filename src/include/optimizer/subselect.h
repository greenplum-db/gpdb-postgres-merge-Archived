/*-------------------------------------------------------------------------
 *
 * subselect.h
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2006-2008, Greenplum inc
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/optimizer/subselect.h,v 1.30 2008/01/01 19:45:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef SUBSELECT_H
#define SUBSELECT_H

#include "nodes/plannodes.h"
#include "nodes/relation.h"

extern Node *convert_testexpr(PlannerInfo *root, Node *testexpr, int rtindex, List **righthandIds);
extern Node *SS_replace_correlation_vars(PlannerInfo *root, Node *expr);
extern Node *SS_process_sublinks(PlannerInfo *root, Node *expr, bool isQual);
extern void SS_finalize_plan(PlannerInfo *root, List *rtable, Plan *plan,
							 bool attach_initplans);
extern Param *SS_make_initplan_from_plan(PlannerInfo *root, Plan *plan,
						   Oid resulttype, int32 resulttypmod);

extern bool IsSubqueryCorrelated(Query *sq);
extern bool IsSubqueryMultiLevelCorrelated(Query *sq);
#endif   /* SUBSELECT_H */
