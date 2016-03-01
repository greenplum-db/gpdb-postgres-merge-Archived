/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * src/bin/psql/describe.h
=======
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/describe.h,v 1.35 2008/01/01 19:45:56 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef DESCRIBE_H
#define DESCRIBE_H


/* \da */
extern bool describeAggregates(const char *pattern, bool verbose, bool showSystem);

/* \db */
extern bool describeTablespaces(const char *pattern, bool verbose);

/* \df, \dfa, \dfn, \dft, \dfw, etc. */
extern bool describeFunctions(const char *functypes, const char *pattern, bool verbose, bool showSystem);

/* \dT */
extern bool describeTypes(const char *pattern, bool verbose, bool showSystem);

/* \do */
extern bool describeOperators(const char *pattern, bool showSystem);

/* \du, \dg */
extern bool describeRoles(const char *pattern, bool verbose);

/* \drds */
extern bool listDbRoleSettings(const char *pattern1, const char *pattern2);

/* \z (or \dp) */
extern bool permissionsList(const char *pattern);

/* \ddp */
extern bool listDefaultACLs(const char *pattern);

/* \dd */
extern bool objectDescription(const char *pattern, bool showSystem);

/* \d foo */
extern bool describeTableDetails(const char *pattern, bool verbose, bool showSystem);

/* \dF */
extern bool listTSConfigs(const char *pattern, bool verbose);

/* \dFp */
extern bool listTSParsers(const char *pattern, bool verbose);

/* \dFd */
extern bool listTSDictionaries(const char *pattern, bool verbose);

/* \dFt */
extern bool listTSTemplates(const char *pattern, bool verbose);

/* \dF */
extern bool listTSConfigs(const char *pattern, bool verbose);

/* \dFp */
extern bool listTSParsers(const char *pattern, bool verbose);

/* \dFd */
extern bool listTSDictionaries(const char *pattern, bool verbose);

/* \dFt */
extern bool listTSTemplates(const char *pattern, bool verbose);

/* \l */
extern bool listAllDbs(bool verbose);

/* \dt, \di, \ds, \dS, etc. */
extern bool listTables(const char *tabtypes, const char *pattern, bool verbose, bool showSystem);

/* \dD */
extern bool listDomains(const char *pattern, bool showSystem);

/* \dc */
extern bool listConversions(const char *pattern, bool showSystem);

/* \dC */
extern bool listCasts(const char *pattern);

/* \dn */
extern bool listSchemas(const char *pattern, bool verbose);

#endif   /* DESCRIBE_H */
