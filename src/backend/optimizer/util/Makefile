#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for optimizer/util
#
# IDENTIFICATION
<<<<<<< HEAD
#    $PostgreSQL: pgsql/src/backend/optimizer/util/Makefile,v 1.19 2008/10/21 20:42:53 tgl Exp $
=======
#    $PostgreSQL: pgsql/src/backend/optimizer/util/Makefile,v 1.18 2008/02/19 10:30:07 petere Exp $
>>>>>>> 0f855d621b
#
#-------------------------------------------------------------------------

subdir = src/backend/optimizer/util
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$../../gp_libpq_fe $(CPPFLAGS)

OBJS = clauses.o joininfo.o pathnode.o plancat.o predtest.o \
       relnode.o restrictinfo.o tlist.o var.o walkers.o

include $(top_srcdir)/src/backend/common.mk
