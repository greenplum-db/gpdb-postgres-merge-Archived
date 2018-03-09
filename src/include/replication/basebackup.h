/*-------------------------------------------------------------------------
 *
 * basebackup.h
 *	  Exports from replication/basebackup.c.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
 *
 * src/include/replication/basebackup.h
=======
 * Portions Copyright (c) 2010-2011, PostgreSQL Global Development Group
 *
 * src/include/replication/walsender.h
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 *-------------------------------------------------------------------------
 */
#ifndef _BASEBACKUP_H
#define _BASEBACKUP_H

<<<<<<< HEAD
#include "nodes/replnodes.h"
=======
#include "replication/replnodes.h"
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

extern void SendBaseBackup(BaseBackupCmd *cmd);

#endif   /* _BASEBACKUP_H */
