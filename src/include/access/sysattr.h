/*-------------------------------------------------------------------------
 *
 * sysattr.h
 *	  POSTGRES system attribute definitions.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/sysattr.h,v 1.2 2009/01/01 17:23:56 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/access/sysattr.h,v 1.1 2008/05/12 00:00:53 alvherre Exp $
>>>>>>> 49f001d81e
 *
 *-------------------------------------------------------------------------
 */
#ifndef SYSATTR_H
#define SYSATTR_H


/*
 * Attribute numbers for the system-defined attributes
 */
#define SelfItemPointerAttributeNumber			(-1)
#define ObjectIdAttributeNumber					(-2)
#define MinTransactionIdAttributeNumber			(-3)
#define MinCommandIdAttributeNumber				(-4)
#define MaxTransactionIdAttributeNumber			(-5)
#define MaxCommandIdAttributeNumber				(-6)
#define TableOidAttributeNumber					(-7)
<<<<<<< HEAD
#define GpSegmentIdAttributeNumber			    (-8)    /*CDB*/
#define FirstLowInvalidHeapAttributeNumber		(-9)
=======
#define FirstLowInvalidHeapAttributeNumber		(-8)
>>>>>>> 49f001d81e


#endif   /* SYSATTR_H */
