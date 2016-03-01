/*-------------------------------------------------------------------------
 *
 * pg_rusage.c
 *	  Resource usage measurement support routines.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
<<<<<<< HEAD
 *	  $PostgreSQL: pgsql/src/backend/utils/misc/pg_rusage.c,v 1.7 2009/01/01 17:23:53 momjian Exp $
=======
 *	  $PostgreSQL: pgsql/src/backend/utils/misc/pg_rusage.c,v 1.6 2008/01/01 19:45:54 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <unistd.h>

#include "utils/pg_rusage.h"


/*
 * Initialize usage snapshot.
 */
void
pg_rusage_init(PGRUsage *ru0)
{
	getrusage(RUSAGE_SELF, &ru0->ru);
	gettimeofday(&ru0->tv, NULL);
}

/*
 * Compute elapsed time since ru0 usage snapshot, and format into
 * a displayable string.  Result is in a static string, which is
 * tacky, but no one ever claimed that the Postgres backend is
 * threadable...
 */
const char *
pg_rusage_show(const PGRUsage *ru0)
{
	static char result[100];
	PGRUsage	ru1;

	pg_rusage_init(&ru1);

	if (ru1.tv.tv_usec < ru0->tv.tv_usec)
	{
		ru1.tv.tv_sec--;
		ru1.tv.tv_usec += 1000000;
	}
	if (ru1.ru.ru_stime.tv_usec < ru0->ru.ru_stime.tv_usec)
	{
		ru1.ru.ru_stime.tv_sec--;
		ru1.ru.ru_stime.tv_usec += 1000000;
	}
	if (ru1.ru.ru_utime.tv_usec < ru0->ru.ru_utime.tv_usec)
	{
		ru1.ru.ru_utime.tv_sec--;
		ru1.ru.ru_utime.tv_usec += 1000000;
	}

	snprintf(result, sizeof(result),
			 "CPU %d.%02ds/%d.%02du sec elapsed %d.%02d sec",
			 (int) (ru1.ru.ru_stime.tv_sec - ru0->ru.ru_stime.tv_sec),
		  (int) (ru1.ru.ru_stime.tv_usec - ru0->ru.ru_stime.tv_usec) / 10000,
			 (int) (ru1.ru.ru_utime.tv_sec - ru0->ru.ru_utime.tv_sec),
		  (int) (ru1.ru.ru_utime.tv_usec - ru0->ru.ru_utime.tv_usec) / 10000,
			 (int) (ru1.tv.tv_sec - ru0->tv.tv_sec),
			 (int) (ru1.tv.tv_usec - ru0->tv.tv_usec) / 10000);

	return result;
}
