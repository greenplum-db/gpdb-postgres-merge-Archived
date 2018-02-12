/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
<<<<<<< HEAD
 * src/bin/psql/stringutils.h
=======
 * $PostgreSQL: pgsql/src/bin/psql/stringutils.h,v 1.28 2010/01/02 16:58:00 momjian Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 */
#ifndef STRINGUTILS_H
#define STRINGUTILS_H

/* The cooler version of strtok() which knows about quotes and doesn't
 * overwrite your input */
extern char *strtokx(const char *s,
		const char *whitespace,
		const char *delim,
		const char *quote,
		char escape,
		bool e_strings,
		bool del_quotes,
		int encoding);

extern void strip_quotes(char *source, char quote, char escape, int encoding);

#endif   /* STRINGUTILS_H */
