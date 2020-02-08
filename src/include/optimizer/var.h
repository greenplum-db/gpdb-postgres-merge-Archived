/*-------------------------------------------------------------------------
 *
 * var.h
 *	  prototypes for optimizer/util/var.c.
 *
 *
 * Portions Copyright (c) 2006-2009, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/optimizer/var.h
 *
 *-------------------------------------------------------------------------
 */
// GPDB_12_MERGE_FIXME: This file was removed in upstream
#ifndef VAR_H
#define VAR_H

typedef bool (*Cdb_walk_vars_callback_Aggref)(Aggref *aggref, void *context, int sublevelsup);
typedef bool (*Cdb_walk_vars_callback_Var)(Var *var, void *context, int sublevelsup);
typedef bool (*Cdb_walk_vars_callback_CurrentOf)(CurrentOfExpr *expr, void *context, int sublevelsup);
typedef bool (*Cdb_walk_vars_callback_placeholdervar)(PlaceHolderVar *expr, void *context, int sublevelsup);
bool        cdb_walk_vars(Node                         *node,
                          Cdb_walk_vars_callback_Var    callback_var,
                          Cdb_walk_vars_callback_Aggref callback_aggref,
                          Cdb_walk_vars_callback_CurrentOf callback_currentof,
						  Cdb_walk_vars_callback_placeholdervar callback_placeholdervar,
                          void                         *context,
                          int                           levelsup);

extern Relids pull_upper_varnos(Node *node);

extern bool contain_ctid_var_reference(Scan *scan);
bool contain_vars_of_level_or_above_cbPlaceHolderVar(PlaceHolderVar *placeholdervar, void *unused, int sublevelsup);

#endif   /* VAR_H */
