/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * src/bin/psql/input.h
=======
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/input.h,v 1.30.2.1 2009/08/24 16:18:25 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef INPUT_H
#define INPUT_H

/*
 * If some other file needs to have access to readline/history, include this
 * file and save yourself all this work.
 *
 * USE_READLINE is the definite pointers regarding existence or not.
 */
#ifdef HAVE_LIBREADLINE
#define USE_READLINE 1

#if defined(HAVE_READLINE_READLINE_H)
#include <readline/readline.h>
#if defined(HAVE_READLINE_HISTORY_H)
#include <readline/history.h>
#endif
<<<<<<< HEAD
=======

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#elif defined(HAVE_EDITLINE_READLINE_H)
#include <editline/readline.h>
#if defined(HAVE_EDITLINE_HISTORY_H)
#include <editline/history.h>
#endif
<<<<<<< HEAD
=======

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#elif defined(HAVE_READLINE_H)
#include <readline.h>
#if defined(HAVE_HISTORY_H)
#include <history.h>
#endif
<<<<<<< HEAD
#endif   /* HAVE_READLINE_READLINE_H, etc */
#endif   /* HAVE_LIBREADLINE */
=======

#endif /* HAVE_READLINE_READLINE_H, etc */
#endif /* HAVE_LIBREADLINE */
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

#include "pqexpbuffer.h"


char	   *gets_interactive(const char *prompt);
char	   *gets_fromFile(FILE *source);

void		initializeInput(int flags);
bool		saveHistory(char *fname, int max_lines, bool appendFlag, bool encodeFlag);

void		pg_append_history(const char *s, PQExpBuffer history_buf);
void		pg_send_history(PQExpBuffer history_buf);

#endif   /* INPUT_H */
