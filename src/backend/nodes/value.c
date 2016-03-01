/*-------------------------------------------------------------------------
 *
 * value.c
 *	  implementation of Value nodes
 *
 *
<<<<<<< HEAD
 * Copyright (c) 2003-2009, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/nodes/value.c,v 1.6 2009/01/01 17:23:43 momjian Exp $
=======
 * Copyright (c) 2003-2008, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/nodes/value.c,v 1.5 2008/01/01 19:45:50 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "nodes/parsenodes.h"

/*
 *	makeInteger
 */
Value *
makeInteger(long i)
{
	Value	   *v = makeNode(Value);

	v->type = T_Integer;
	v->val.ival = i;
	return v;
}

/*
 *	makeFloat
 *
 * Caller is responsible for passing a palloc'd string.
 */
Value *
makeFloat(char *numericStr)
{
	Value	   *v = makeNode(Value);

	v->type = T_Float;
	v->val.str = numericStr;
	return v;
}

/*
 *	makeString
 *
 * Caller is responsible for passing a palloc'd string.
 */
Value *
makeString(char *str)
{
	Value	   *v = makeNode(Value);

	v->type = T_String;
	v->val.str = str;
	return v;
}

/*
 *	makeBitString
 *
 * Caller is responsible for passing a palloc'd string.
 */
Value *
makeBitString(char *str)
{
	Value	   *v = makeNode(Value);

	v->type = T_BitString;
	v->val.str = str;
	return v;
}
