/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/copy.h,v 1.23 2010/01/02 16:57:59 momjian Exp $
=======
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/copy.h,v 1.21 2008/01/01 19:45:55 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef COPY_H
#define COPY_H

#include "libpq-fe.h"


/* handler for \copy */
bool		do_copy(const char *args);

/* lower level processors for copy in/out streams */

bool		handleCopyOut(PGconn *conn, FILE *copystream);
bool		handleCopyIn(PGconn *conn, FILE *copystream, bool isbinary);

#endif
