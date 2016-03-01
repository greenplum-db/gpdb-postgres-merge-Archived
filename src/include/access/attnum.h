/*-------------------------------------------------------------------------
 *
 * attnum.h
 *	  POSTGRES attribute number definitions.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/attnum.h,v 1.26 2009/01/01 17:23:55 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/attnum.h,v 1.24.2.1 2008/04/05 01:58:28 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef ATTNUM_H
#define ATTNUM_H


/*
 * user defined attribute numbers start at 1.	-ay 2/95
 */
typedef int16 AttrNumber;

#define InvalidAttrNumber		0
#define MaxAttrNumber			32767

/* ----------------
 *		support macros
 * ----------------
 */
/*
 * AttributeNumberIsValid
 *		True iff the attribute number is valid.
 */
#define AttributeNumberIsValid(attributeNumber) \
	((bool) ((attributeNumber) != InvalidAttrNumber))

/*
 * AttrNumberIsForUserDefinedAttr
 *		True iff the attribute number corresponds to an user defined attribute.
 */
#define AttrNumberIsForUserDefinedAttr(attributeNumber) \
	((bool) ((attributeNumber) > 0))

/*
 * AttrNumberGetAttrOffset
 *		Returns the attribute offset for an attribute number.
 *
 * Note:
 *		Assumes the attribute number is for an user defined attribute.
 */
#define AttrNumberGetAttrOffset(attNum) \
( \
	AssertMacro(AttrNumberIsForUserDefinedAttr(attNum)), \
	((attNum) - 1) \
)

/*
 * AttributeOffsetGetAttributeNumber
 *		Returns the attribute number for an attribute offset.
 */
#define AttrOffsetGetAttrNumber(attributeOffset) \
	 ((AttrNumber) (1 + (attributeOffset)))

#endif   /* ATTNUM_H */
