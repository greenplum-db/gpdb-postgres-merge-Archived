/*-------------------------------------------------------------------------
 *
 * subselect.h
 *	  Planning routines for subselects.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2006-2008, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/optimizer/subselect.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef SUBSELECT_H
#define SUBSELECT_H

#include "nodes/pathnodes.h"
#include "nodes/plannodes.h"

#if 0
/* Not used in GPDB */
extern void SS_process_ctes(PlannerInfo *root);
#endif
extern Node *convert_testexpr(PlannerInfo *root,
				 Node *testexpr,
				 List *subst_nodes);
extern JoinExpr *convert_ANY_sublink_to_join(PlannerInfo *root,
<<<<<<< HEAD
							SubLink *sublink,
							Relids available_rels);
extern Node *convert_EXISTS_sublink_to_join(PlannerInfo *root,
							   SubLink *sublink,
							   bool under_not,
							   Relids available_rels);
=======
											 SubLink *sublink,
											 Relids available_rels);
extern JoinExpr *convert_EXISTS_sublink_to_join(PlannerInfo *root,
												SubLink *sublink,
												bool under_not,
												Relids available_rels);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern Node *SS_replace_correlation_vars(PlannerInfo *root, Node *expr);
extern Node *SS_process_sublinks(PlannerInfo *root, Node *expr, bool isQual);
extern void SS_identify_outer_params(PlannerInfo *root);
extern void SS_charge_for_initplans(PlannerInfo *root, RelOptInfo *final_rel);
extern void SS_attach_initplans(PlannerInfo *root, Plan *plan);
extern void SS_finalize_plan(PlannerInfo *root, Plan *plan);
extern Param *SS_make_initplan_output_param(PlannerInfo *root,
											Oid resulttype, int32 resulttypmod,
											Oid resultcollation);
extern void SS_make_initplan_from_plan(PlannerInfo *root,
<<<<<<< HEAD
						   PlannerInfo *subroot, Plan *plan,
						   PlanSlice *subslice,
						   Param *prm);
extern Param *assign_nestloop_param_var(PlannerInfo *root, Var *var);
extern Param *assign_nestloop_param_placeholdervar(PlannerInfo *root,
									 PlaceHolderVar *phv);
extern int	SS_assign_special_param(PlannerInfo *root);


extern bool IsSubqueryCorrelated(Query *sq);
extern bool IsSubqueryMultiLevelCorrelated(Query *sq);

extern List *generate_subquery_vars(PlannerInfo *root, List *tlist,
					   Index varno);
extern bool QueryHasDistributedRelation(Query *q);

#endif   /* SUBSELECT_H */
=======
									   PlannerInfo *subroot, Plan *plan,
									   Param *prm);

#endif							/* SUBSELECT_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
