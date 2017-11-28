/*-------------------------------------------------------------------------
*
* pthread-win32.c
*	 partial pthread implementation for win32
*
* Copyright (c) 2004-2012, PostgreSQL Global Development Group
* IDENTIFICATION
<<<<<<< HEAD
*	src/interfaces/libpq/pthread-win32.c
=======
*	$PostgreSQL: pgsql/src/interfaces/libpq/pthread-win32.c,v 1.19 2009/06/11 14:49:14 momjian Exp $
>>>>>>> 4d53a2f9699547bdc12831d2860c9d44c465e805
*
*-------------------------------------------------------------------------
*/

#include "postgres_fe.h"

#include <windows.h>
#include "pthread-win32.h"

DWORD
pthread_self(void)
{
	return GetCurrentThreadId();
}

void
pthread_setspecific(pthread_key_t key, void *val)
{
}

void *
pthread_getspecific(pthread_key_t key)
{
	return NULL;
}

int
pthread_mutex_init(pthread_mutex_t *mp, void *attr)
{
	*mp = (CRITICAL_SECTION *) malloc(sizeof(CRITICAL_SECTION));
	if (!*mp)
		return 1;
	InitializeCriticalSection(*mp);
	return 0;
}

int
pthread_mutex_lock(pthread_mutex_t *mp)
{
	if (!*mp)
		return 1;
	EnterCriticalSection(*mp);
	return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *mp)
{
	if (!*mp)
		return 1;
	LeaveCriticalSection(*mp);
	return 0;
}
