/*-------------------------------------------------------------------------
 *
 * libpq-fs.h
 *	  definitions for using Inversion file system routines (ie, large objects)
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/libpq/libpq-fs.h,v 1.25 2010/01/02 16:58:04 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/libpq/libpq-fs.h,v 1.23 2008/01/01 19:45:58 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef LIBPQ_FS_H
#define LIBPQ_FS_H

/*
 *	Read/write mode flags for inversion (large object) calls
 */

#define INV_WRITE		0x00020000
#define INV_READ		0x00040000

#endif   /* LIBPQ_FS_H */
