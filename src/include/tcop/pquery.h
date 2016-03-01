/*-------------------------------------------------------------------------
 *
 * pquery.h
 *	  prototypes for pquery.c.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/tcop/pquery.h,v 1.44 2008/01/01 19:45:59 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef PQUERY_H
#define PQUERY_H

#include "nodes/parsenodes.h"
#include "utils/portal.h"


extern PGDLLIMPORT Portal ActivePortal;


extern PortalStrategy ChoosePortalStrategy(List *stmts);

extern List *FetchPortalTargetList(Portal portal);

extern List *FetchStatementTargetList(Node *stmt);

extern void PortalStart(Portal portal, ParamListInfo params,
						Snapshot snapshot,
						const char *seqServerHost, int seqServerPort);

extern void PortalSetResultFormat(Portal portal, int nFormats,
					  int16 *formats);

<<<<<<< HEAD
extern bool PortalRun(Portal portal, int64 count, bool isTopLevel,
=======
extern bool PortalRun(Portal portal, long count, bool isTopLevel,
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
		  DestReceiver *dest, DestReceiver *altdest,
		  char *completionTag);

extern int64 PortalRunFetch(Portal portal,
			   FetchDirection fdirection,
			   int64 count,
			   DestReceiver *dest);

#endif   /* PQUERY_H */
