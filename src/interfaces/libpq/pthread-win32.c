/*-------------------------------------------------------------------------
*
* pthread-win32.c
*	 partial pthread implementation for win32
*
<<<<<<< HEAD
* Copyright (c) 2004-2012, PostgreSQL Global Development Group
=======
* Copyright (c) 2004-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
* IDENTIFICATION
*	src/interfaces/libpq/pthread-win32.c
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
