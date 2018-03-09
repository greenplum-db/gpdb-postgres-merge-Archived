/*-------------------------------------------------------------------------
 *
 * unix_latch.c
 *	  Routines for inter-process latches
 *
<<<<<<< HEAD
=======
 * A latch is a boolean variable, with operations that let you to sleep
 * until it is set. A latch can be set from another process, or a signal
 * handler within the same process.
 *
 * The latch interface is a reliable replacement for the common pattern of
 * using pg_usleep() or select() to wait until a signal arrives, where the
 * signal handler sets a global variable. Because on some platforms, an
 * incoming signal doesn't interrupt sleep, and even on platforms where it
 * does there is a race condition if the signal arrives just before
 * entering the sleep, the common pattern must periodically wake up and
 * poll the global variable. pselect() system call was invented to solve
 * the problem, but it is not portable enough. Latches are designed to
 * overcome these limitations, allowing you to sleep without polling and
 * ensuring a quick response to signals from other processes.
 *
 * There are two kinds of latches: local and shared. A local latch is
 * initialized by InitLatch, and can only be set from the same process.
 * A local latch can be used to wait for a signal to arrive, by calling
 * SetLatch in the signal handler. A shared latch resides in shared memory,
 * and must be initialized at postmaster startup by InitSharedLatch. Before
 * a shared latch can be waited on, it must be associated with a process
 * with OwnLatch. Only the process owning the latch can wait on it, but any
 * process can set it.
 *
 * There are three basic operations on a latch:
 *
 * SetLatch		- Sets the latch
 * ResetLatch	- Clears the latch, allowing it to be set again
 * WaitLatch	- Waits for the latch to become set
 *
 * The correct pattern to wait for an event is:
 *
 * for (;;)
 * {
 *	   ResetLatch();
 *	   if (work to do)
 *		   Do Stuff();
 *
 *	   WaitLatch();
 * }
 *
 * It's important to reset the latch *before* checking if there's work to
 * do. Otherwise, if someone sets the latch between the check and the
 * ResetLatch call, you will miss it and Wait will block.
 *
 * To wake up the waiter, you must first set a global flag or something
 * else that the main loop tests in the "if (work to do)" part, and call
 * SetLatch *after* that. SetLatch is designed to return quickly if the
 * latch is already set.
 *
 *
 * Implementation
 * --------------
 *
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * The Unix implementation uses the so-called self-pipe trick to overcome
 * the race condition involved with select() and setting a global flag
 * in the signal handler. When a latch is set and the current process
 * is waiting for it, the signal handler wakes up the select() in
 * WaitLatch by writing a byte to a pipe. A signal by itself doesn't
 * interrupt select() on all platforms, and even on platforms where it
 * does, a signal that arrives just before the select() call does not
 * prevent the select() from entering sleep. An incoming byte on a pipe
<<<<<<< HEAD
 * however reliably interrupts the sleep, and causes select() to return
 * immediately even if the signal arrives before select() begins.
 *
 * (Actually, we prefer poll() over select() where available, but the
 * same comments apply to it.)
=======
 * however reliably interrupts the sleep, and makes select() to return
 * immediately if the signal arrives just before select() begins.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * When SetLatch is called from the same process that owns the latch,
 * SetLatch writes the byte directly to the pipe. If it's owned by another
 * process, SIGUSR1 is sent and the signal handler in the waiting process
 * writes the byte to the pipe on behalf of the signaling process.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/port/unix_latch.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
<<<<<<< HEAD
#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include "miscadmin.h"
<<<<<<< HEAD
#include "portability/instr_time.h"
#include "postmaster/postmaster.h"
#include "storage/latch.h"
#include "storage/pmsignal.h"
#include "storage/shmem.h"
#include "utils/guc.h"
=======
#include "storage/latch.h"
#include "storage/shmem.h"
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

/* Are we currently in WaitLatch? The signal handler would like to know. */
static volatile sig_atomic_t waiting = false;

<<<<<<< HEAD
/* Read and write ends of the self-pipe */
static int	selfpipe_readfd = -1;
static int	selfpipe_writefd = -1;

/* Private function prototypes */
static void sendSelfPipeByte(void);
static void drainSelfPipe(void);


/*
 * Initialize the process-local latch infrastructure.
 *
 * This must be called once during startup of any process that can wait on
 * latches, before it issues any InitLatch() or OwnLatch() calls.
 */
void
InitializeLatchSupport(void)
{
	int			pipefd[2];

	Assert(selfpipe_readfd == -1);

	/*
	 * Set up the self-pipe that allows a signal handler to wake up the
	 * select() in WaitLatch. Make the write-end non-blocking, so that
	 * SetLatch won't block if the event has already been set many times
	 * filling the kernel buffer. Make the read-end non-blocking too, so that
	 * we can easily clear the pipe by reading until EAGAIN or EWOULDBLOCK.
	 */
	if (pipe(pipefd) < 0)
		elog(FATAL, "pipe() failed: %m");
	if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) < 0)
		elog(FATAL, "fcntl() failed on read-end of self-pipe: %m");
	if (fcntl(pipefd[1], F_SETFL, O_NONBLOCK) < 0)
		elog(FATAL, "fcntl() failed on write-end of self-pipe: %m");

	selfpipe_readfd = pipefd[0];
	selfpipe_writefd = pipefd[1];
}

/*
=======
/* Read and write end of the self-pipe */
static int	selfpipe_readfd = -1;
static int	selfpipe_writefd = -1;

/* private function prototypes */
static void initSelfPipe(void);
static void drainSelfPipe(void);
static void sendSelfPipeByte(void);


/*
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * Initialize a backend-local latch.
 */
void
InitLatch(volatile Latch *latch)
{
<<<<<<< HEAD
	/* Assert InitializeLatchSupport has been called in this process */
	Assert(selfpipe_readfd >= 0);
=======
	/* Initialize the self pipe if this is our first latch in the process */
	if (selfpipe_readfd == -1)
		initSelfPipe();
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

	latch->is_set = false;
	latch->owner_pid = MyProcPid;
	latch->is_shared = false;
}

/*
 * Initialize a shared latch that can be set from other processes. The latch
<<<<<<< HEAD
 * is initially owned by no-one; use OwnLatch to associate it with the
=======
 * is initially owned by no-one, use OwnLatch to associate it with the
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * current process.
 *
 * InitSharedLatch needs to be called in postmaster before forking child
 * processes, usually right after allocating the shared memory block
<<<<<<< HEAD
 * containing the latch with ShmemInitStruct. (The Unix implementation
 * doesn't actually require that, but the Windows one does.) Because of
 * this restriction, we have no concurrency issues to worry about here.
=======
 * containing the latch with ShmemInitStruct. The Unix implementation
 * doesn't actually require that, but the Windows one does.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 */
void
InitSharedLatch(volatile Latch *latch)
{
	latch->is_set = false;
	latch->owner_pid = 0;
	latch->is_shared = true;
}

/*
 * Associate a shared latch with the current process, allowing it to
<<<<<<< HEAD
 * wait on the latch.
 *
 * Although there is a sanity check for latch-already-owned, we don't do
 * any sort of locking here, meaning that we could fail to detect the error
 * if two processes try to own the same latch at about the same time.  If
 * there is any risk of that, caller must provide an interlock to prevent it.
 *
 * In any process that calls OwnLatch(), make sure that
 * latch_sigusr1_handler() is called from the SIGUSR1 signal handler,
 * as shared latches use SIGUSR1 for inter-process communication.
=======
 * wait on it.
 *
 * Make sure that latch_sigusr1_handler() is called from the SIGUSR1 signal
 * handler, as shared latches use SIGUSR1 to for inter-process communication.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 */
void
OwnLatch(volatile Latch *latch)
{
<<<<<<< HEAD
	/* Assert InitializeLatchSupport has been called in this process */
	Assert(selfpipe_readfd >= 0);

	Assert(latch->is_shared);

	/* sanity check */
	if (latch->owner_pid != 0)
		elog(ERROR, "latch already owned");

=======
	Assert(latch->is_shared);

	/* Initialize the self pipe if this is our first latch in the process */
	if (selfpipe_readfd == -1)
		initSelfPipe();

	/* sanity check */
	if (latch->owner_pid != 0)
		elog(ERROR, "latch already owned");
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	latch->owner_pid = MyProcPid;
}

/*
 * Disown a shared latch currently owned by the current process.
 */
void
DisownLatch(volatile Latch *latch)
{
	Assert(latch->is_shared);
	Assert(latch->owner_pid == MyProcPid);
<<<<<<< HEAD

=======
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	latch->owner_pid = 0;
}

/*
<<<<<<< HEAD
 * Wait for a given latch to be set, or for postmaster death, or until timeout
 * is exceeded. 'wakeEvents' is a bitmask that specifies which of those events
 * to wait for. If the latch is already set (and WL_LATCH_SET is given), the
 * function returns immediately.
 *
 * The 'timeout' is given in milliseconds. It must be >= 0 if WL_TIMEOUT flag
 * is given.  Note that some extra overhead is incurred when WL_TIMEOUT is
 * given, so avoid using a timeout if possible.
=======
 * Wait for given latch to be set or until timeout is exceeded.
 * If the latch is already set, the function returns immediately.
 *
 * The 'timeout' is given in microseconds, and -1 means wait forever.
 * On some platforms, signals cause the timeout to be restarted, so beware
 * that the function can sleep for several times longer than the specified
 * timeout.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 *
 * The latch must be owned by the current process, ie. it must be a
 * backend-local latch initialized with InitLatch, or a shared latch
 * associated with the current process by calling OwnLatch.
 *
<<<<<<< HEAD
 * Returns bit mask indicating which condition(s) caused the wake-up. Note
 * that if multiple wake-up conditions are true, there is no guarantee that
 * we return all of them in one call, but we will return at least one.
 */
int
WaitLatch(volatile Latch *latch, int wakeEvents, long timeout)
{
	return WaitLatchOrSocket(latch, wakeEvents, PGINVALID_SOCKET, timeout);
}

/*
 * Like WaitLatch, but with an extra socket argument for WL_SOCKET_*
 * conditions.
 *
 * When waiting on a socket, WL_SOCKET_READABLE *must* be included in
 * 'wakeEvents'; WL_SOCKET_WRITEABLE is optional.  The reason for this is
 * that EOF and error conditions are reported only via WL_SOCKET_READABLE.
 */
int
WaitLatchOrSocket(volatile Latch *latch, int wakeEvents, pgsocket sock,
				  long timeout)
{
	int			result = 0;
	int			rc;
	instr_time	start_time,
				cur_time;
	long		cur_timeout;

#ifdef HAVE_POLL
	struct pollfd pfds[3];
	int			nfds;
#else
	struct timeval tv,
			   *tvp;
	fd_set		input_mask;
	fd_set		output_mask;
	int			hifd;
#endif

	/* Ignore WL_SOCKET_* events if no valid socket is given */
	if (sock == PGINVALID_SOCKET)
		wakeEvents &= ~(WL_SOCKET_READABLE | WL_SOCKET_WRITEABLE);

	Assert(wakeEvents != 0);	/* must have at least one wake event */
	/* Cannot specify WL_SOCKET_WRITEABLE without WL_SOCKET_READABLE */
	Assert((wakeEvents & (WL_SOCKET_READABLE | WL_SOCKET_WRITEABLE)) != WL_SOCKET_WRITEABLE);

	if ((wakeEvents & WL_LATCH_SET) && latch->owner_pid != MyProcPid)
		elog(ERROR, "cannot wait on a latch owned by another process");

	/*
	 * Initialize timeout if requested.  We must record the current time so
	 * that we can determine the remaining timeout if the poll() or select()
	 * is interrupted.	(On some platforms, select() will update the contents
	 * of "tv" for us, but unfortunately we can't rely on that.)
	 */
	if (wakeEvents & WL_TIMEOUT)
	{
		INSTR_TIME_SET_CURRENT(start_time);
		Assert(timeout >= 0);
		cur_timeout = timeout;

#ifndef HAVE_POLL
		tv.tv_sec = cur_timeout / 1000L;
		tv.tv_usec = (cur_timeout % 1000L) * 1000L;
		tvp = &tv;
#endif
	}
	else
	{
		cur_timeout = -1;

#ifndef HAVE_POLL
		tvp = NULL;
#endif
	}

	waiting = true;
	do
	{
		/*
		 * Clear the pipe, then check if the latch is set already. If someone
		 * sets the latch between this and the poll()/select() below, the
		 * setter will write a byte to the pipe (or signal us and the signal
		 * handler will do that), and the poll()/select() will return
		 * immediately.
		 *
		 * Note: we assume that the kernel calls involved in drainSelfPipe()
		 * and SetLatch() will provide adequate synchronization on machines
		 * with weak memory ordering, so that we cannot miss seeing is_set if
		 * the signal byte is already in the pipe when we drain it.
		 */
		drainSelfPipe();

		if ((wakeEvents & WL_LATCH_SET) && latch->is_set)
		{
			result |= WL_LATCH_SET;

			elogif(debug_latch, LOG,
					"latch wait -- Latch is set. No need to wait now.");

			/*
			 * Leave loop immediately, avoid blocking again. We don't attempt
			 * to report any other events that might also be satisfied.
			 */
			break;
		}

		/* Must wait ... we use poll(2) if available, otherwise select(2) */
#ifdef HAVE_POLL
		nfds = 0;
		if (wakeEvents & (WL_SOCKET_READABLE | WL_SOCKET_WRITEABLE))
		{
			/* socket, if used, is always in pfds[0] */
			pfds[0].fd = sock;
			pfds[0].events = 0;
			if (wakeEvents & WL_SOCKET_READABLE)
				pfds[0].events |= POLLIN;
			if (wakeEvents & WL_SOCKET_WRITEABLE)
				pfds[0].events |= POLLOUT;
			pfds[0].revents = 0;
			nfds++;
		}

		pfds[nfds].fd = selfpipe_readfd;
		pfds[nfds].events = POLLIN;
		pfds[nfds].revents = 0;
		nfds++;

		if (wakeEvents & WL_POSTMASTER_DEATH)
		{
			/* postmaster fd, if used, is always in pfds[nfds - 1] */
			pfds[nfds].fd = postmaster_alive_fds[POSTMASTER_FD_WATCH];
			pfds[nfds].events = POLLIN;
			pfds[nfds].revents = 0;
			nfds++;
		}

		elogif(debug_latch, LOG,
				"latch wait -- poll() timeout set to %d", (int)cur_timeout);

		/* Sleep */
		rc = poll(pfds, nfds, (int) cur_timeout);

		/* Check return code */
		if (rc < 0)
		{
			/* EINTR is okay, otherwise complain */
			if (errno != EINTR)
			{
				waiting = false;
				ereport(ERROR,
						(errcode_for_socket_access(),
						 errmsg("poll() failed: %m")));
			}

			elogif(debug_latch, LOG,
				"latch wait -- poll() rc = %d, this means wait on poll() was interrupted.",rc);
		}
		else if (rc == 0)
		{
			/* timeout exceeded */
			if (wakeEvents & WL_TIMEOUT)
			{
				elogif(debug_latch, LOG,
					"latch wait -- poll() rc = %d and wakeup event is timeout.",rc);

				result |= WL_TIMEOUT;
			}
		}
		else
		{
			/* at least one event occurred, so check revents values */
			if ((wakeEvents & WL_SOCKET_READABLE) &&
				(pfds[0].revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)))
			{
				/* data available in socket, or EOF/error condition */
				result |= WL_SOCKET_READABLE;

				elogif(debug_latch, LOG,
					"latch wait -- poll() rc = %d and wakeup event was data became "
					"available at socket or EOF/error condition.",rc);
			}
			if ((wakeEvents & WL_SOCKET_WRITEABLE) &&
				(pfds[0].revents & POLLOUT))
			{
				result |= WL_SOCKET_WRITEABLE;

				elogif(debug_latch, LOG,
						"latch wait -- poll() rc = %d and wakeup event was socket "
						"became writeable.",rc);
			}

			/*
			 * We expect a POLLHUP when the remote end is closed, but because
			 * we don't expect the pipe to become readable or to have any
			 * errors either, treat those cases as postmaster death, too.
			 */
			if ((wakeEvents & WL_POSTMASTER_DEATH) &&
				(pfds[nfds - 1].revents & (POLLHUP | POLLIN | POLLERR | POLLNVAL)))
			{
				/*
				 * According to the select(2) man page on Linux, select(2) may
				 * spuriously return and report a file descriptor as readable,
				 * when it's not; and presumably so can poll(2).  It's not
				 * clear that the relevant cases would ever apply to the
				 * postmaster pipe, but since the consequences of falsely
				 * returning WL_POSTMASTER_DEATH could be pretty unpleasant,
				 * we take the trouble to positively verify EOF with
				 * PostmasterIsAlive().
				 */
				if (!PostmasterIsAlive(false))
				{
					result |= WL_POSTMASTER_DEATH;

					elogif(debug_latch, LOG,
							"latch wait -- poll() rc = %d and wakeup event was death of the "
							"postmaster.",rc);
				}
			}
		}
#else							/* !HAVE_POLL */

		FD_ZERO(&input_mask);
		FD_ZERO(&output_mask);

		FD_SET(selfpipe_readfd, &input_mask);
		hifd = selfpipe_readfd;

		if (wakeEvents & WL_POSTMASTER_DEATH)
		{
			FD_SET(postmaster_alive_fds[POSTMASTER_FD_WATCH], &input_mask);
			if (postmaster_alive_fds[POSTMASTER_FD_WATCH] > hifd)
				hifd = postmaster_alive_fds[POSTMASTER_FD_WATCH];
		}

		if (wakeEvents & WL_SOCKET_READABLE)
=======
 * Returns 'true' if the latch was set, or 'false' if timeout was reached.
 */
bool
WaitLatch(volatile Latch *latch, long timeout)
{
	return WaitLatchOrSocket(latch, PGINVALID_SOCKET, false, false, timeout) > 0;
}

/*
 * Like WaitLatch, but will also return when there's data available in
 * 'sock' for reading or writing. Returns 0 if timeout was reached,
 * 1 if the latch was set, 2 if the socket became readable or writable.
 */
int
WaitLatchOrSocket(volatile Latch *latch, pgsocket sock, bool forRead,
				  bool forWrite, long timeout)
{
	struct timeval tv,
			   *tvp = NULL;
	fd_set		input_mask;
	fd_set		output_mask;
	int			rc;
	int			result = 0;

	if (latch->owner_pid != MyProcPid)
		elog(ERROR, "cannot wait on a latch owned by another process");

	/* Initialize timeout */
	if (timeout >= 0)
	{
		tv.tv_sec = timeout / 1000000L;
		tv.tv_usec = timeout % 1000000L;
		tvp = &tv;
	}

	waiting = true;
	for (;;)
	{
		int			hifd;

		/*
		 * Clear the pipe, and check if the latch is set already. If someone
		 * sets the latch between this and the select() below, the setter will
		 * write a byte to the pipe (or signal us and the signal handler will
		 * do that), and the select() will return immediately.
		 */
		drainSelfPipe();
		if (latch->is_set)
		{
			result = 1;
			break;
		}

		FD_ZERO(&input_mask);
		FD_SET(selfpipe_readfd, &input_mask);
		hifd = selfpipe_readfd;
		if (sock != PGINVALID_SOCKET && forRead)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		{
			FD_SET(sock, &input_mask);
			if (sock > hifd)
				hifd = sock;
		}

<<<<<<< HEAD
		if (wakeEvents & WL_SOCKET_WRITEABLE)
=======
		FD_ZERO(&output_mask);
		if (sock != PGINVALID_SOCKET && forWrite)
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		{
			FD_SET(sock, &output_mask);
			if (sock > hifd)
				hifd = sock;
		}

<<<<<<< HEAD
		/* Sleep */
		rc = select(hifd + 1, &input_mask, &output_mask, NULL, tvp);

		/* Check return code */
		if (rc < 0)
		{
			/* EINTR is okay, otherwise complain */
			if (errno != EINTR)
			{
				waiting = false;
				ereport(ERROR,
						(errcode_for_socket_access(),
						 errmsg("select() failed: %m")));
			}
		}
		else if (rc == 0)
		{
			/* timeout exceeded */
			if (wakeEvents & WL_TIMEOUT)
				result |= WL_TIMEOUT;
		}
		else
		{
			/* at least one event occurred, so check masks */
			if ((wakeEvents & WL_SOCKET_READABLE) && FD_ISSET(sock, &input_mask))
			{
				/* data available in socket, or EOF */
				result |= WL_SOCKET_READABLE;
			}
			if ((wakeEvents & WL_SOCKET_WRITEABLE) && FD_ISSET(sock, &output_mask))
			{
				result |= WL_SOCKET_WRITEABLE;
			}
			if ((wakeEvents & WL_POSTMASTER_DEATH) &&
			FD_ISSET(postmaster_alive_fds[POSTMASTER_FD_WATCH], &input_mask))
			{
				/*
				 * According to the select(2) man page on Linux, select(2) may
				 * spuriously return and report a file descriptor as readable,
				 * when it's not; and presumably so can poll(2).  It's not
				 * clear that the relevant cases would ever apply to the
				 * postmaster pipe, but since the consequences of falsely
				 * returning WL_POSTMASTER_DEATH could be pretty unpleasant,
				 * we take the trouble to positively verify EOF with
				 * PostmasterIsAlive().
				 */
				if (!PostmasterIsAlive())
					result |= WL_POSTMASTER_DEATH;
			}
		}
#endif   /* HAVE_POLL */

		/* If we're not done, update cur_timeout for next iteration */
		if (result == 0 && cur_timeout >= 0)
		{
			INSTR_TIME_SET_CURRENT(cur_time);
			INSTR_TIME_SUBTRACT(cur_time, start_time);
			cur_timeout = timeout - (long) INSTR_TIME_GET_MILLISEC(cur_time);
			if (cur_timeout < 0)
				cur_timeout = 0;

#ifndef HAVE_POLL
			tv.tv_sec = cur_timeout / 1000L;
			tv.tv_usec = (cur_timeout % 1000L) * 1000L;
#endif
		}
	} while (result == 0);
=======
		rc = select(hifd + 1, &input_mask, &output_mask, NULL, tvp);
		if (rc < 0)
		{
			if (errno == EINTR)
				continue;
			ereport(ERROR,
					(errcode_for_socket_access(),
					 errmsg("select() failed: %m")));
		}
		if (rc == 0)
		{
			/* timeout exceeded */
			result = 0;
			break;
		}
		if (sock != PGINVALID_SOCKET &&
			((forRead && FD_ISSET(sock, &input_mask)) ||
			 (forWrite && FD_ISSET(sock, &output_mask))))
		{
			result = 2;
			break;				/* data available in socket */
		}
	}
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	waiting = false;

	return result;
}

/*
<<<<<<< HEAD
 * Sets a latch and wakes up anyone waiting on it.
 *
 * This is cheap if the latch is already set, otherwise not so much.
 *
 * NB: when calling this in a signal handler, be sure to save and restore
 * errno around it.  (That's standard practice in most signal handlers, of
 * course, but we used to omit it in handlers that only set a flag.)
=======
 * Sets a latch and wakes up anyone waiting on it. Returns quickly if the
 * latch is already set.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 */
void
SetLatch(volatile Latch *latch)
{
	pid_t		owner_pid;

<<<<<<< HEAD
	/*
	 * XXX there really ought to be a memory barrier operation right here, to
	 * ensure that any flag variables we might have changed get flushed to
	 * main memory before we check/set is_set.	Without that, we have to
	 * require that callers provide their own synchronization for machines
	 * with weak memory ordering (see latch.h).
	 */

	/* Quick exit if already set */
	if (latch->is_set)
	{
		elogif(debug_latch, LOG, "latch set -- Latch for process (pid %u) is already set.",
									latch->owner_pid);
		return;
	}

	latch->is_set = true;
	elogif(debug_latch, LOG, "latch set -- Latch for process (pid %u) is now set.",
								latch->owner_pid);
=======
	/* Quick exit if already set */
	if (latch->is_set)
		return;

	latch->is_set = true;
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687

	/*
	 * See if anyone's waiting for the latch. It can be the current process if
	 * we're in a signal handler. We use the self-pipe to wake up the select()
	 * in that case. If it's another process, send a signal.
	 *
<<<<<<< HEAD
	 * Fetch owner_pid only once, in case the latch is concurrently getting
	 * owned or disowned. XXX: This assumes that pid_t is atomic, which isn't
	 * guaranteed to be true! In practice, the effective range of pid_t fits
	 * in a 32 bit integer, and so should be atomic. In the worst case, we
	 * might end up signaling the wrong process. Even then, you're very
	 * unlucky if a process with that bogus pid exists and belongs to
	 * Postgres; and PG database processes should handle excess SIGUSR1
	 * interrupts without a problem anyhow.
	 *
	 * Another sort of race condition that's possible here is for a new
	 * process to own the latch immediately after we look, so we don't signal
	 * it. This is okay so long as all callers of ResetLatch/WaitLatch follow
	 * the standard coding convention of waiting at the bottom of their loops,
	 * not the top, so that they'll correctly process latch-setting events
	 * that happen before they enter the loop.
	 */
	owner_pid = latch->owner_pid;
	if (owner_pid == 0)
	{
		elogif(debug_latch, LOG, "latch set -- Owner_pid of latch is 0. No process to signal.");
		return;
	}
	else if (owner_pid == MyProcPid)
	{
		if (waiting)
		{
			sendSelfPipeByte();
			elogif(debug_latch, LOG, "latch set -- Sent a byte to self pipe.");
		}
	}
	else
	{
		kill(owner_pid, SIGUSR1);
		elogif(debug_latch, LOG, "latch set -- Sent SIGUSR1 to process, pid %u.",owner_pid);
	}
=======
	 * Fetch owner_pid only once, in case the owner simultaneously disowns the
	 * latch and clears owner_pid. XXX: This assumes that pid_t is atomic,
	 * which isn't guaranteed to be true! In practice, the effective range of
	 * pid_t fits in a 32 bit integer, and so should be atomic. In the worst
	 * case, we might end up signaling wrong process if the right one disowns
	 * the latch just as we fetch owner_pid. Even then, you're very unlucky if
	 * a process with that bogus pid exists.
	 */
	owner_pid = latch->owner_pid;
	if (owner_pid == 0)
		return;
	else if (owner_pid == MyProcPid)
		sendSelfPipeByte();
	else
		kill(owner_pid, SIGUSR1);
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
}

/*
 * Clear the latch. Calling WaitLatch after this will sleep, unless
 * the latch is set again before the WaitLatch call.
 */
void
ResetLatch(volatile Latch *latch)
{
	/* Only the owner should reset the latch */
	Assert(latch->owner_pid == MyProcPid);

	latch->is_set = false;
<<<<<<< HEAD

	/*
	 * XXX there really ought to be a memory barrier operation right here, to
	 * ensure that the write to is_set gets flushed to main memory before we
	 * examine any flag variables.	Otherwise a concurrent SetLatch might
	 * falsely conclude that it needn't signal us, even though we have missed
	 * seeing some flag updates that SetLatch was supposed to inform us of.
	 * For the moment, callers must supply their own synchronization of flag
	 * variables (see latch.h).
	 */

	elogif(debug_latch, LOG, "latch reset -- Latch is now reset for process (pid %u).", latch->owner_pid);
}

/*
 * SetLatch uses SIGUSR1 to wake up the process waiting on the latch.
 *
 * Wake up WaitLatch, if we're waiting.  (We might not be, since SIGUSR1 is
 * overloaded for multiple purposes; or we might not have reached WaitLatch
 * yet, in which case we don't need to fill the pipe either.)
 *
 * NB: when calling this in a signal handler, be sure to save and restore
 * errno around it.
=======
}

/*
 * SetLatch uses SIGUSR1 to wake up the process waiting on the latch. Wake
 * up WaitLatch.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 */
void
latch_sigusr1_handler(void)
{
	if (waiting)
<<<<<<< HEAD
	{
		sendSelfPipeByte();
	}
=======
		sendSelfPipeByte();
}

/* initialize the self-pipe */
static void
initSelfPipe(void)
{
	int			pipefd[2];

	/*
	 * Set up the self-pipe that allows a signal handler to wake up the
	 * select() in WaitLatch. Make the write-end non-blocking, so that
	 * SetLatch won't block if the event has already been set many times
	 * filling the kernel buffer. Make the read-end non-blocking too, so that
	 * we can easily clear the pipe by reading until EAGAIN or EWOULDBLOCK.
	 */
	if (pipe(pipefd) < 0)
		elog(FATAL, "pipe() failed: %m");
	if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) < 0)
		elog(FATAL, "fcntl() failed on read-end of self-pipe: %m");
	if (fcntl(pipefd[1], F_SETFL, O_NONBLOCK) < 0)
		elog(FATAL, "fcntl() failed on write-end of self-pipe: %m");

	selfpipe_readfd = pipefd[0];
	selfpipe_writefd = pipefd[1];
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
}

/* Send one byte to the self-pipe, to wake up WaitLatch */
static void
sendSelfPipeByte(void)
{
	int			rc;
	char		dummy = 0;

retry:
	rc = write(selfpipe_writefd, &dummy, 1);
	if (rc < 0)
	{
		/* If interrupted by signal, just retry */
		if (errno == EINTR)
			goto retry;

		/*
		 * If the pipe is full, we don't need to retry, the data that's there
		 * already is enough to wake up WaitLatch.
		 */
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;

		/*
		 * Oops, the write() failed for some other reason. We might be in a
		 * signal handler, so it's not safe to elog(). We have no choice but
		 * silently ignore the error.
		 */
		return;
	}
}

<<<<<<< HEAD
/*
 * Read all available data from the self-pipe
 *
 * Note: this is only called when waiting = true.  If it fails and doesn't
 * return, it must reset that flag first (though ideally, this will never
 * happen).
 */
=======
/* Read all available data from the self-pipe */
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
static void
drainSelfPipe(void)
{
	/*
	 * There shouldn't normally be more than one byte in the pipe, or maybe a
<<<<<<< HEAD
	 * few bytes if multiple processes run SetLatch at the same instant.
=======
	 * few more if multiple processes run SetLatch at the same instant.
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	 */
	char		buf[16];
	int			rc;

	for (;;)
	{
		rc = read(selfpipe_readfd, buf, sizeof(buf));
		if (rc < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
<<<<<<< HEAD
			{
				elogif(debug_latch, LOG, "latch drainpipe -- pipe read() returned empty.");
				break;			/* the pipe is empty */
			}
			else if (errno == EINTR)
			{
				elogif(debug_latch, LOG, "latch drainpipe --pipe read() was interrupted.");
				continue;		/* retry */
			}
			else
			{
				waiting = false;
				elog(ERROR, "read() on self-pipe failed: %m");
			}
		}
		else if (rc == 0)
		{
			waiting = false;
			elog(ERROR, "unexpected EOF on self-pipe");
		}
		else if (rc < sizeof(buf))
		{
			/* we successfully drained the pipe; no need to read() again */
			elogif(debug_latch, LOG, "latch drainpipe -- pipe read() was successful.");
			break;
		}
		/* else buffer wasn't big enough, so read again */
=======
				break;			/* the pipe is empty */
			else if (errno == EINTR)
				continue;		/* retry */
			else
				elog(ERROR, "read() on self-pipe failed: %m");
		}
		else if (rc == 0)
			elog(ERROR, "unexpected EOF on self-pipe");
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	}
}
