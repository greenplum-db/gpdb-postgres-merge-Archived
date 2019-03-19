/*
 * rmgrdesc.c
 *
 * pg_xlogdump resource managers definition
 *
 * src/bin/pg_xlogdump/rmgrdesc.c
 */
#define FRONTEND 1
#include "postgres.h"

#include "access/brin_xlog.h"
#include "access/clog.h"
#include "access/commit_ts.h"
#include "access/gin.h"
#include "access/gist_private.h"
#include "access/hash.h"
#include "access/heapam_xlog.h"
#include "access/multixact.h"
#include "access/nbtree.h"
#include "access/rmgr.h"
#include "access/spgist.h"
#include "access/xact.h"
#include "access/xlog_internal.h"
#include "catalog/storage_xlog.h"
#include "commands/dbcommands_xlog.h"
#include "commands/sequence.h"
#include "commands/tablespace.h"
#include "rmgrdesc.h"
#include "storage/standby.h"
#include "utils/relmapper.h"

<<<<<<< HEAD:contrib/pg_xlogdump/rmgrdesc.c
#include "access/bitmap.h"
#include "access/distributedlog.h"
#include "cdb/cdbappendonlyxlog.h"

#define PG_RMGR(symname,name,redo,desc,startup,cleanup) \
	{ name, desc, },
=======
#define PG_RMGR(symname,name,redo,desc,identify,startup,cleanup) \
	{ name, desc, identify},
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8:src/bin/pg_xlogdump/rmgrdesc.c

const RmgrDescData RmgrDescTable[RM_MAX_ID + 1] = {
#include "access/rmgrlist.h"
};
