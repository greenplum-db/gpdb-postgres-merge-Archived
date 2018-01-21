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
 * $PostgreSQL: pgsql/src/include/parser/parse_cte.h,v 1.3 2009/09/09 03:32:52 tgl Exp $
>>>>>>> 78a09145e0
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_CTE_H
#define PARSE_CTE_H

#include "parser/parse_node.h"

extern List *transformWithClause(ParseState *pstate, WithClause *withClause);
extern CommonTableExpr *GetCTEForRTE(ParseState *pstate, RangeTblEntry *rte, int rtelevelsup);

extern void analyzeCTETargetList(ParseState *pstate, CommonTableExpr *cte,
								 List *tlist);

#endif   /* PARSE_CTE_H */
