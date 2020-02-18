/*-------------------------------------------------------------------------
 *
 * pg_proc_fn.h
 *	 prototypes for functions in catalog/pg_proc.c
 *
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_proc_fn.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_PROC_FN_H
#define PG_PROC_FN_H

#include "catalog/objectaddress.h"
#include "nodes/pg_list.h"

extern bool function_parse_error_transpose(const char *prosrc);

extern List *oid_array_to_list(Datum datum);

#endif   /* PG_PROC_FN_H */
