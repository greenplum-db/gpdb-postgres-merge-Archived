/*-------------------------------------------------------------------------
 *
 * libpq-events.h
 *	  This file contains definitions that are useful to applications
 *	  that invoke the libpq "events" API, but are not interesting to
 *	  ordinary users of libpq.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2010, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/interfaces/libpq/libpq-events.h,v 1.5 2010/01/02 16:58:12 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/interfaces/libpq/libpq-events.h,v 1.2 2008/09/19 20:06:13 tgl Exp $
>>>>>>> 38e9348282e
 *
 *-------------------------------------------------------------------------
 */

#ifndef LIBPQ_EVENTS_H
#define LIBPQ_EVENTS_H

#include "libpq-fe.h"

#ifdef __cplusplus
extern		"C"
{
#endif

/* Callback Event Ids */
<<<<<<< HEAD
			typedef enum
=======
typedef enum
>>>>>>> 38e9348282e
{
	PGEVT_REGISTER,
	PGEVT_CONNRESET,
	PGEVT_CONNDESTROY,
	PGEVT_RESULTCREATE,
	PGEVT_RESULTCOPY,
	PGEVT_RESULTDESTROY
} PGEventId;

typedef struct
{
<<<<<<< HEAD
	PGconn	   *conn;
=======
	PGconn *conn;
>>>>>>> 38e9348282e
} PGEventRegister;

typedef struct
{
<<<<<<< HEAD
	PGconn	   *conn;
=======
	PGconn *conn;
>>>>>>> 38e9348282e
} PGEventConnReset;

typedef struct
{
<<<<<<< HEAD
	PGconn	   *conn;
=======
	PGconn *conn;
>>>>>>> 38e9348282e
} PGEventConnDestroy;

typedef struct
{
<<<<<<< HEAD
	PGconn	   *conn;
	PGresult   *result;
=======
	PGconn *conn;
	PGresult *result;
>>>>>>> 38e9348282e
} PGEventResultCreate;

typedef struct
{
	const PGresult *src;
<<<<<<< HEAD
	PGresult   *dest;
=======
	PGresult *dest;
>>>>>>> 38e9348282e
} PGEventResultCopy;

typedef struct
{
<<<<<<< HEAD
	PGresult   *result;
=======
	PGresult *result;
>>>>>>> 38e9348282e
} PGEventResultDestroy;

typedef int (*PGEventProc) (PGEventId evtId, void *evtInfo, void *passThrough);

/* Registers an event proc with the given PGconn. */
<<<<<<< HEAD
extern int PQregisterEventProc(PGconn *conn, PGEventProc proc,
					const char *name, void *passThrough);
=======
extern int	PQregisterEventProc(PGconn *conn, PGEventProc proc,
								const char *name, void *passThrough);
>>>>>>> 38e9348282e

/* Sets the PGconn instance data for the provided proc to data. */
extern int	PQsetInstanceData(PGconn *conn, PGEventProc proc, void *data);

/* Gets the PGconn instance data for the provided proc. */
extern void *PQinstanceData(const PGconn *conn, PGEventProc proc);

/* Sets the PGresult instance data for the provided proc to data. */
extern int	PQresultSetInstanceData(PGresult *result, PGEventProc proc, void *data);

/* Gets the PGresult instance data for the provided proc. */
extern void *PQresultInstanceData(const PGresult *result, PGEventProc proc);

/* Fires RESULTCREATE events for an application-created PGresult. */
extern int	PQfireResultCreateEvents(PGconn *conn, PGresult *res);

#ifdef __cplusplus
}
#endif

<<<<<<< HEAD
#endif   /* LIBPQ_EVENTS_H */
=======
#endif /* LIBPQ_EVENTS_H */
>>>>>>> 38e9348282e
