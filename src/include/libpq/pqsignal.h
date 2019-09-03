/*-------------------------------------------------------------------------
 *
 * pqsignal.h
 *	  Backend signal(2) support (see also src/port/pqsignal.c)
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/libpq/pqsignal.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef PQSIGNAL_H
#define PQSIGNAL_H

#include <signal.h>

<<<<<<< HEAD
#ifdef HAVE_SIGPROCMASK
extern sigset_t UnBlockSig,
			BlockSig,
			StartupBlockSig;

/* 
 * Use pthread_sigmask() instead of sigprocmask() as the latter has undefined
 * behaviour in multithreaded processes.
 */
#define PG_SETMASK(mask)	pthread_sigmask(SIG_SETMASK, mask, NULL)
#else							/* not HAVE_SIGPROCMASK */
extern int	UnBlockSig,
			BlockSig,
			StartupBlockSig;

=======
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
#ifndef WIN32
#define PG_SETMASK(mask)	sigprocmask(SIG_SETMASK, mask, NULL)
#else
/* Emulate POSIX sigset_t APIs on Windows */
typedef int sigset_t;

extern int	pqsigsetmask(int mask);

#define PG_SETMASK(mask)		pqsigsetmask(*(mask))
#define sigemptyset(set)		(*(set) = 0)
#define sigfillset(set)			(*(set) = ~0)
#define sigaddset(set, signum)	(*(set) |= (sigmask(signum)))
#define sigdelset(set, signum)	(*(set) &= ~(sigmask(signum)))
#endif   /* WIN32 */

extern sigset_t UnBlockSig,
			BlockSig,
			StartupBlockSig;

extern void pqinitmask(void);

#endif   /* PQSIGNAL_H */
