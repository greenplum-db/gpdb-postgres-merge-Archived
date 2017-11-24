/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * src/bin/psql/stringutils.h
=======
 * Copyright (c) 2000-2009, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/stringutils.h,v 1.27 2009/01/01 17:23:55 momjian Exp $
>>>>>>> b0a6ad70a12b6949fdebffa8ca1650162bf0254a
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
