/*-------------------------------------------------------------------------
 *
 * pgarch.h
 *	  Exports from postmaster/pgarch.c.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/postmaster/pgarch.h,v 1.7 2009/01/01 17:24:01 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/postmaster/pgarch.h,v 1.6 2008/01/01 19:45:58 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef _PGARCH_H
#define _PGARCH_H

/* ----------
 * Functions called from postmaster
 * ----------
 */
extern int	pgarch_start(void);

#ifdef EXEC_BACKEND
extern void PgArchiverMain(int argc, char *argv[]);
#endif

#endif   /* _PGARCH_H */
