/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2013, PostgreSQL Global Development Group
 *
 * src/bin/psql/stringutils.h
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
<<<<<<< HEAD
=======

>>>>>>> e472b921406407794bab911c64655b8b82375196
extern char *quote_if_needed(const char *source, const char *entails_quote,
				char quote, char escape, int encoding);

#endif   /* STRINGUTILS_H */
