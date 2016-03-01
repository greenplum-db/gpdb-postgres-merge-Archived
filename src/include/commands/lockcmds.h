/*-------------------------------------------------------------------------
 *
 * lockcmds.h
 *	  prototypes for lockcmds.c.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/lockcmds.h,v 1.10 2009/01/01 17:23:58 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/lockcmds.h,v 1.9 2008/01/01 19:45:57 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#ifndef LOCKCMDS_H
#define LOCKCMDS_H

#include "nodes/parsenodes.h"

/*
 * LOCK
 */
extern void LockTableCommand(LockStmt *lockstmt);

#endif   /* LOCKCMDS_H */
