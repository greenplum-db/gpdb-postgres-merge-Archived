/*-------------------------------------------------------------------------
 *
 * pquery.h
 *	  prototypes for pquery.c.
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/tcop/pquery.h
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
<<<<<<< HEAD
			int eflags, Snapshot snapshot,
			QueryDispatchDesc *ddesc);
=======
						int eflags, Snapshot snapshot);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

extern void PortalSetResultFormat(Portal portal, int nFormats,
								  int16 *formats);

<<<<<<< HEAD
extern bool PortalRun(Portal portal, int64 count, bool isTopLevel,
		  DestReceiver *dest, DestReceiver *altdest,
		  char *completionTag);

extern uint64 PortalRunFetch(Portal portal,
			   FetchDirection fdirection,
			   int64 count,
			   DestReceiver *dest);
=======
extern bool PortalRun(Portal portal, long count, bool isTopLevel,
					  bool run_once, DestReceiver *dest, DestReceiver *altdest,
					  char *completionTag);

extern uint64 PortalRunFetch(Portal portal,
							 FetchDirection fdirection,
							 long count,
							 DestReceiver *dest);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

#endif							/* PQUERY_H */
