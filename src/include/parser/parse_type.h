/*-------------------------------------------------------------------------
 *
 * parse_type.h
 *		handle type operations for parser
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/parser/parse_type.h,v 1.39 2008/01/01 19:45:58 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PARSE_TYPE_H
#define PARSE_TYPE_H

#include "access/htup.h"
#include "parser/parse_node.h"


typedef HeapTuple Type;

<<<<<<< HEAD
#define ReleaseType(fmw) ReleaseSysCache((fmw))

extern Oid	LookupTypeName(ParseState *pstate, const TypeName *typname);
extern char *TypeNameToString(const TypeName *typname);
extern char *TypeNameListToString(List *typenames);
extern Oid	typenameTypeId(ParseState *pstate, const TypeName *typname);
extern Type typenameType(ParseState *pstate, const TypeName *typname);
extern int32 typenameTypeMod(ParseState *pstate, const TypeName *typname,
							 Oid typeId);
=======
extern Type LookupTypeName(ParseState *pstate, const TypeName *typename,
			   int32 *typmod_p);
extern Type typenameType(ParseState *pstate, const TypeName *typename,
			 int32 *typmod_p);
extern Oid typenameTypeId(ParseState *pstate, const TypeName *typename,
			   int32 *typmod_p);

extern char *TypeNameToString(const TypeName *typename);
extern char *TypeNameListToString(List *typenames);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

extern Type typeidType(Oid id);

extern Oid	typeTypeId(Type tp);
extern int16 typeLen(Type t);
extern bool typeByVal(Type t);
extern char *typeTypeName(Type t);
extern Oid	typeTypeRelid(Type typ);
extern Datum stringTypeDatum(Type tp, char *string, int32 atttypmod);

extern Oid	typeidTypeRelid(Oid type_id);

extern void parseTypeString(const char *str, Oid *type_id, int32 *typmod_p);

#define ISCOMPLEX(typid) (typeidTypeRelid(typid) != InvalidOid)

#endif   /* PARSE_TYPE_H */
