#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for src/backend/replication
#
# IDENTIFICATION
#    src/backend/replication/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/replication
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := -I$(srcdir) $(CPPFLAGS)
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

OBJS = walsender.o walreceiverfuncs.o walreceiver.o basebackup.o \
<<<<<<< HEAD
	repl_gram.o syncrep.o
OBJS += gp_libpqwalreceiver.o gp_replication.o
=======
	repl_gram.o slot.o slotfuncs.o syncrep.o

SUBDIRS = logical
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237

include $(top_srcdir)/src/backend/common.mk

# repl_scanner is compiled as part of repl_gram
repl_gram.o: repl_scanner.c

# repl_gram.c and repl_scanner.c are in the distribution tarball, so
# they are not cleaned here.
