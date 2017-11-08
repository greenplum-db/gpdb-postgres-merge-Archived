#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for optimizer/util
#
# IDENTIFICATION
#    $PostgreSQL: pgsql/src/backend/optimizer/util/Makefile,v 1.19 2008/10/21 20:42:53 tgl Exp $
#
#-------------------------------------------------------------------------

subdir = src/backend/optimizer/util
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

OBJS = clauses.o joininfo.o pathnode.o placeholder.o plancat.o predtest.o \
<<<<<<< HEAD
       relnode.o restrictinfo.o tlist.o var.o walkers.o
=======
       relnode.o restrictinfo.o tlist.o var.o
>>>>>>> 38e9348282e

include $(top_srcdir)/src/backend/common.mk
