/*-------------------------------------------------------------------------
 *
 * placeholder.h
 *	  prototypes for optimizer/util/placeholder.c.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2017, Pivotal Inc.
=======
>>>>>>> 38e9348282e
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/optimizer/placeholder.h,v 1.1 2008/10/21 20:42:53 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PLACEHOLDER_H
#define PLACEHOLDER_H

#include "nodes/relation.h"


extern PlaceHolderVar *make_placeholder_expr(PlannerInfo *root, Expr *expr,
											 Relids phrels);
extern PlaceHolderInfo *find_placeholder_info(PlannerInfo *root,
											  PlaceHolderVar *phv);
<<<<<<< HEAD
extern void find_placeholders_in_jointree(PlannerInfo *root);
extern void update_placeholder_eval_levels(PlannerInfo *root,
											  SpecialJoinInfo *new_sjinfo);
extern void fix_placeholder_input_needed_levels(PlannerInfo *root);
=======
extern void fix_placeholder_eval_levels(PlannerInfo *root);
>>>>>>> 38e9348282e
extern void add_placeholders_to_joinrel(PlannerInfo *root,
										RelOptInfo *joinrel);

#endif   /* PLACEHOLDER_H */
