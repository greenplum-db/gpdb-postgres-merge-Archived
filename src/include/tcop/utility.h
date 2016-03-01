/*-------------------------------------------------------------------------
 *
 * utility.h
 *	  prototypes for utility.c.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/tcop/utility.h,v 1.34 2008/01/01 19:45:59 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef UTILITY_H
#define UTILITY_H

#include "tcop/tcopprot.h"


extern void ProcessUtility(Node *parsetree, const char *queryString,
			   ParamListInfo params, bool isTopLevel,
			   DestReceiver *dest, char *completionTag);

extern bool UtilityReturnsTuples(Node *parsetree);

extern TupleDesc UtilityTupleDescriptor(Node *parsetree);

extern const char *CreateCommandTag(Node *parsetree);

extern LogStmtLevel GetCommandLogLevel(Node *parsetree);

<<<<<<< HEAD
extern LogStmtLevel GetQueryLogLevel(Query *parsetree);

extern LogStmtLevel GetPlannedStmtLogLevel(PlannedStmt * stmt);

extern bool QueryReturnsTuples(Query *parsetree); /* Obsolete? */

extern bool QueryIsReadOnly(Query *parsetree);  /* Obsolete */

=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
extern bool CommandIsReadOnly(Node *parsetree);

extern void CheckRelationOwnership(RangeVar *rel, bool noCatalogs);
extern void DropErrorMsgNonExistent(const RangeVar *rel, char rightkind, bool missing_ok);

#endif   /* UTILITY_H */
