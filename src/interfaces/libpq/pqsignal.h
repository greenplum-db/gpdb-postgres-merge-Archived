/*-------------------------------------------------------------------------
 *
 * pqsignal.h
 *	  prototypes for the reliable BSD-style signal(2) routine.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/interfaces/libpq/pqsignal.h
 *
 * NOTES
 *	  This shouldn't be in libpq, but the monitor and some other
 *	  things need it...
 *
 *-------------------------------------------------------------------------
 */
#ifndef PQSIGNAL_H
#define PQSIGNAL_H

typedef void (*pqsigfunc) (int);

extern pqsigfunc pqsignal(int signo, pqsigfunc func);

#endif   /* PQSIGNAL_H */
