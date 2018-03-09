/*-------------------------------------------------------------------------
 *
 * rewriteSupport.h
 *
 *
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/rewrite/rewriteSupport.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef REWRITESUPPORT_H
#define REWRITESUPPORT_H

/* The ON SELECT rule of a view is always named this: */
#define ViewSelectRuleName	"_RETURN"

extern void SetRelationRuleStatus(Oid relationId, bool relHasRules,
					  bool relIsBecomingView);

<<<<<<< HEAD
extern Oid  get_rewrite_oid(Oid relid, const char *rulename, bool missing_ok);
extern Oid  get_rewrite_oid_without_relid(const char *rulename, Oid *relid);
=======
extern Oid	get_rewrite_oid(Oid relid, const char *rulename, bool missing_ok);
extern Oid	get_rewrite_oid_without_relid(const char *rulename, Oid *relid);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

#endif   /* REWRITESUPPORT_H */
