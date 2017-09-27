/*-------------------------------------------------------------------------
 *
 * parser.h
 *		Definitions for the "raw" parser (lex and yacc phases only)
 *
 *
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
<<<<<<< HEAD
 * $PostgreSQL: pgsql/src/include/parser/parser.h,v 1.25 2009/04/19 21:50:08 tgl Exp $
=======
 * $PostgreSQL: pgsql/src/include/parser/parser.h,v 1.23 2008/05/12 00:00:53 alvherre Exp $
>>>>>>> 49f001d81e
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSER_H
#define PARSER_H

#include "nodes/pg_list.h"

extern List *raw_parser(const char *str);

#endif   /* PARSER_H */
