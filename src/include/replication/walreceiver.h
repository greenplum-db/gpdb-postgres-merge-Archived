/*-------------------------------------------------------------------------
 *
 * walreceiver.h
 *	  Exports from replication/walreceiverfuncs.c.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2010-2012, PostgreSQL Global Development Group
 *
 * src/include/replication/walreceiver.h
=======
 * Portions Copyright (c) 2010-2010, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/include/replication/walreceiver.h,v 1.11 2010/07/06 19:19:00 momjian Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 *
 *-------------------------------------------------------------------------
 */
#ifndef _WALRECEIVER_H
#define _WALRECEIVER_H

<<<<<<< HEAD
#include "access/xlog.h"
#include "access/xlogdefs.h"
#include "storage/spin.h"
#include "utils/builtins.h"
#include "pgtime.h"

extern int	wal_receiver_status_interval;
extern bool hot_standby_feedback;
=======
#include "access/xlogdefs.h"
#include "storage/spin.h"
#include "pgtime.h"

extern bool am_walreceiver;
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

/*
 * MAXCONNINFO: maximum size of a connection string.
 *
 * XXX: Should this move to pg_config_manual.h?
 */
#define MAXCONNINFO		1024

/*
 * Values for WalRcv->walRcvState.
 */
typedef enum
{
	WALRCV_STOPPED,				/* stopped and mustn't start up again */
	WALRCV_STARTING,			/* launched, but the process hasn't
								 * initialized yet */
	WALRCV_RUNNING,				/* walreceiver is running */
	WALRCV_STOPPING				/* requested to stop, but still running */
} WalRcvState;

/* Shared memory area for management of walreceiver process */
typedef struct
{
	/*
	 * PID of currently active walreceiver process, its current state and
	 * start time (actually, the time at which it was requested to be
	 * started).
	 */
	pid_t		pid;
	WalRcvState walRcvState;
	pg_time_t	startTime;

	/*
<<<<<<< HEAD
	 * receiveStart is the first byte position that will be received. When
	 * startup process starts the walreceiver, it sets receiveStart to the
	 * point where it wants the streaming to begin.
	 */
	XLogRecPtr	receiveStart;

	/*
	 * receivedUpto-1 is the last byte position that has already been
	 * received.  At the first startup of walreceiver, receivedUpto is set to
	 * receiveStart. After that, walreceiver updates this whenever it flushes
	 * the received WAL to disk.
=======
	 * receivedUpto-1 is the last byte position that has already been
	 * received.  When startup process starts the walreceiver, it sets
	 * receivedUpto to the point where it wants the streaming to begin. After
	 * that, walreceiver updates this whenever it flushes the received WAL to
	 * disk.
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
	 */
	XLogRecPtr	receivedUpto;

	/*
	 * latestChunkStart is the starting byte position of the current "batch"
	 * of received WAL.  It's actually the same as the previous value of
	 * receivedUpto before the last flush to disk.	Startup process can use
	 * this to detect whether it's keeping up or not.
	 */
	XLogRecPtr	latestChunkStart;

	/*
<<<<<<< HEAD
	 * Time of send and receive of any message received.
	 */
	TimestampTz lastMsgSendTime;
	TimestampTz lastMsgReceiptTime;

	/*
	 * Latest reported end of WAL on the sender
	 */
	XLogRecPtr	latestWalEnd;
	TimestampTz latestWalEndTime;

	/*
=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
	 * connection string; is used for walreceiver to connect with the primary.
	 */
	char		conninfo[MAXCONNINFO];

	slock_t		mutex;			/* locks shared variables shown above */
} WalRcvData;

extern WalRcvData *WalRcv;

/* libpqwalreceiver hooks */
<<<<<<< HEAD
bool walrcv_connect(char *conninfo, XLogRecPtr startpoint);
bool walrcv_receive(int timeout, unsigned char *type,
					char **buffer, int *len);
void walrcv_send(const char *buffer, int nbytes);
void walrcv_disconnect(void);

/* prototypes for functions in walreceiver.c */
extern void WalReceiverMain(void);

/* prototypes for functions in walreceiverfuncs.c */
=======
typedef bool (*walrcv_connect_type) (char *conninfo, XLogRecPtr startpoint);
extern PGDLLIMPORT walrcv_connect_type walrcv_connect;

typedef bool (*walrcv_receive_type) (int timeout, unsigned char *type,
												 char **buffer, int *len);
extern PGDLLIMPORT walrcv_receive_type walrcv_receive;

typedef void (*walrcv_disconnect_type) (void);
extern PGDLLIMPORT walrcv_disconnect_type walrcv_disconnect;

extern void WalReceiverMain(void);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
extern Size WalRcvShmemSize(void);
extern void WalRcvShmemInit(void);
extern void ShutdownWalRcv(void);
extern bool WalRcvInProgress(void);
<<<<<<< HEAD
extern void RequestXLogStreaming(XLogRecPtr recptr, const char *conninfo);
extern XLogRecPtr GetWalRcvWriteRecPtr(XLogRecPtr *latestChunkStart);
extern int	GetReplicationApplyDelay(void);
extern int	GetReplicationTransferLatency(void);
extern const char *WalRcvGetStateString(WalRcvState state);
=======
extern XLogRecPtr WaitNextXLogAvailable(XLogRecPtr recptr, bool *finished);
extern void RequestXLogStreaming(XLogRecPtr recptr, const char *conninfo);
extern XLogRecPtr GetWalRcvWriteRecPtr(XLogRecPtr *latestChunkStart);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

#endif   /* _WALRECEIVER_H */
