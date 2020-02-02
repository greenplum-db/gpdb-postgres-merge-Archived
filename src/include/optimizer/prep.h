/*-------------------------------------------------------------------------
 *
 * prep.h
 *	  prototypes for files in optimizer/prep/
 *
 *
 * Portions Copyright (c) 2006-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/optimizer/prep.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PREP_H
#define PREP_H

#include "nodes/pathnodes.h"
#include "nodes/plannodes.h"


/*
 * prototypes for prepjointree.c
 */
extern void replace_empty_jointree(Query *parse);
extern void pull_up_sublinks(PlannerInfo *root);
extern void inline_set_returning_functions(PlannerInfo *root);
extern void pull_up_subqueries(PlannerInfo *root);
extern void flatten_simple_union_all(PlannerInfo *root);
extern void reduce_outer_joins(PlannerInfo *root);
extern void remove_useless_result_rtes(PlannerInfo *root);
extern Relids get_relids_in_jointree(Node *jtnode, bool include_joins);
<<<<<<< HEAD
extern Relids get_relids_for_join(PlannerInfo *root, int joinrelid);

extern List *init_list_cteplaninfo(int numCtes);

/*
 * prototypes for prepqual.c
 */
extern Node *negate_clause(Node *node);
extern Expr *canonicalize_qual(Expr *qual);
extern Expr *canonicalize_qual_ext(Expr *qual, bool is_check);

/*
 * prototypes for prepsecurity.c
 */
extern void expand_security_quals(PlannerInfo *root, List *tlist);
=======
extern Relids get_relids_for_join(Query *query, int joinrelid);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/*
 * prototypes for preptlist.c
 */
<<<<<<< HEAD
extern List *preprocess_targetlist(PlannerInfo *root, List *tlist);

extern List *preprocess_onconflict_targetlist(PlannerInfo *root, List *tlist,
								 int result_relation, List *range_table);
=======
extern List *preprocess_targetlist(PlannerInfo *root);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

extern PlanRowMark *get_plan_rowmark(List *rowmarks, Index rtindex);

/*
 * prototypes for prepunion.c
 */
extern RelOptInfo *plan_set_operations(PlannerInfo *root);

#endif							/* PREP_H */
