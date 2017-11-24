/*-------------------------------------------------------------------------
 *
 * parse_cte.h
 *	  handle CTEs (common table expressions) in parser
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 * Portions Copyright (c) 2011 - present, EMC Greenplum.
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 *
<<<<<<< HEAD
 * src/include/parser/parse_cte.h
=======
 * $PostgreSQL: pgsql/src/include/parser/parse_cte.h,v 1.2 2009/01/01 17:24:00 momjian Exp $
>>>>>>> b0a6ad70a12b6949fdebffa8ca1650162bf0254a
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_CTE_H
#define PARSE_CTE_H

#include "parser/parse_node.h"

extern List *transformWithClause(ParseState *pstate, WithClause *withClause);
extern CommonTableExpr *GetCTEForRTE(ParseState *pstate, RangeTblEntry *rte, int rtelevelsup);

#endif   /* PARSE_CTE_H */
