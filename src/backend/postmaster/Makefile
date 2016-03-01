#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for src/backend/postmaster
#
# IDENTIFICATION
<<<<<<< HEAD
#    $PostgreSQL: pgsql/src/backend/postmaster/Makefile,v 1.24 2008/02/19 10:30:07 petere Exp $
=======
#    $PostgreSQL: pgsql/src/backend/postmaster/Makefile,v 1.23 2007/07/24 04:54:09 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

subdir = src/backend/postmaster
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(top_srcdir)/src/backend/gp_libpq_fe $(CPPFLAGS)

<<<<<<< HEAD
OBJS = bgwriter.o autovacuum.o checkpoint.o seqserver.o pgarch.o pgstat.o \
	postmaster.o primary_mirror_mode.o primary_mirror_transition_client.o syslogger.o \
	fork_process.o perfmon.o backoff.o perfmon_segmentinfo.o \
	sendalert.o alertseverity.o autostats.o
=======
OBJS = autovacuum.o bgwriter.o fork_process.o pgarch.o pgstat.o postmaster.o \
	syslogger.o walwriter.o
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

include $(top_srcdir)/src/backend/common.mk
