/*-------------------------------------------------------------------------
 *
 * parse_utilcmd.h
 *		parse analysis for utility commands
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parse_utilcmd.h,v 1.3 2008/01/01 19:45:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_UTILCMD_H
#define PARSE_UTILCMD_H

<<<<<<< HEAD
#include "nodes/parsenodes.h"
#include "parser/analyze.h"
#include "parser/parse_node.h"

extern void transformIndexConstraints(ParseState *pstate, CreateStmtContext *cxt, bool mayDefer);
extern void transformInhRelation(ParseState *pstate, CreateStmtContext *cxt,
								 InhRelation *inhRelation, bool forceBareCol);
=======
#include "parser/parse_node.h"


extern List *transformCreateStmt(CreateStmt *stmt, const char *queryString);
extern List *transformAlterTableStmt(AlterTableStmt *stmt,
						const char *queryString);
extern IndexStmt *transformIndexStmt(IndexStmt *stmt, const char *queryString);
extern void transformRuleStmt(RuleStmt *stmt, const char *queryString,
				  List **actions, Node **whereClause);
extern List *transformCreateSchemaStmt(CreateSchemaStmt *stmt);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

#endif   /* PARSE_UTILCMD_H */
