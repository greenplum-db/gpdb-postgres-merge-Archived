#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for src/backend/postmaster
#
# IDENTIFICATION
#    src/backend/postmaster/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/postmaster
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

<<<<<<< HEAD
OBJS = autovacuum.o bgwriter.o checkpointer.o fork_process.o seqserver.o pgarch.o pgstat.o \
	postmaster.o syslogger.o \
	perfmon.o backoff.o perfmon_segmentinfo.o \
	sendalert.o alertseverity.o autostats.o walwriter.o
=======
OBJS = autovacuum.o bgwriter.o fork_process.o pgarch.o pgstat.o postmaster.o \
	startup.o syslogger.o walwriter.o checkpointer.o
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

include $(top_srcdir)/src/backend/common.mk
