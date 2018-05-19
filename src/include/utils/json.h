/*-------------------------------------------------------------------------
 *
 * json.h
 *	  Declarations for JSON data type support.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/json.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef JSON_H
#define JSON_H

#include "fmgr.h"
#include "lib/stringinfo.h"

<<<<<<< HEAD
/* functions in json.c */
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
extern Datum json_in(PG_FUNCTION_ARGS);
extern Datum json_out(PG_FUNCTION_ARGS);
extern Datum json_recv(PG_FUNCTION_ARGS);
extern Datum json_send(PG_FUNCTION_ARGS);
extern Datum array_to_json(PG_FUNCTION_ARGS);
extern Datum array_to_json_pretty(PG_FUNCTION_ARGS);
extern Datum row_to_json(PG_FUNCTION_ARGS);
extern Datum row_to_json_pretty(PG_FUNCTION_ARGS);
<<<<<<< HEAD
extern Datum to_json(PG_FUNCTION_ARGS);

extern Datum json_agg_transfn(PG_FUNCTION_ARGS);
extern Datum json_agg_finalfn(PG_FUNCTION_ARGS);

extern void escape_json(StringInfo buf, const char *str);

/* functions in jsonfuncs.c */
extern Datum json_object_field(PG_FUNCTION_ARGS);
extern Datum json_object_field_text(PG_FUNCTION_ARGS);
extern Datum json_array_element(PG_FUNCTION_ARGS);
extern Datum json_array_element_text(PG_FUNCTION_ARGS);
extern Datum json_extract_path(PG_FUNCTION_ARGS);
extern Datum json_extract_path_text(PG_FUNCTION_ARGS);
extern Datum json_object_keys(PG_FUNCTION_ARGS);
extern Datum json_array_length(PG_FUNCTION_ARGS);
extern Datum json_each(PG_FUNCTION_ARGS);
extern Datum json_each_text(PG_FUNCTION_ARGS);
extern Datum json_array_elements(PG_FUNCTION_ARGS);
extern Datum json_populate_record(PG_FUNCTION_ARGS);
extern Datum json_populate_recordset(PG_FUNCTION_ARGS);

=======
extern void escape_json(StringInfo buf, const char *str);

>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
#endif   /* JSON_H */
