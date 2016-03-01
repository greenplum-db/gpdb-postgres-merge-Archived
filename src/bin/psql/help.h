/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/help.h,v 1.21 2010/01/02 16:57:59 momjian Exp $
=======
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/help.h,v 1.19 2008/01/01 19:45:56 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef HELP_H
#define HELP_H

void		usage(void);

void		slashUsage(unsigned short int pager);

void		helpSQL(const char *topic, unsigned short int pager);

void		print_copyright(void);

#endif
