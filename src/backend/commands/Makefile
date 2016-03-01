#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for backend/commands
#
# IDENTIFICATION
<<<<<<< HEAD
#    $PostgreSQL: pgsql/src/backend/commands/Makefile,v 1.39 2008/12/19 16:25:17 petere Exp $
=======
#    $PostgreSQL: pgsql/src/backend/commands/Makefile,v 1.37 2007/08/21 01:11:14 tgl Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#
#-------------------------------------------------------------------------

subdir = src/backend/commands
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(top_srcdir)/src/backend/gp_libpq_fe $(CPPFLAGS)

OBJS = aggregatecmds.o alter.o analyze.o analyzefuncs.o async.o cluster.o comment.o  \
	conversioncmds.o copy.o \
<<<<<<< HEAD
	dbcommands.o define.o explain.o extprotocolcmds.o filespace.o functioncmds.o \
=======
	dbcommands.o define.o discard.o explain.o functioncmds.o \
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	indexcmds.o lockcmds.o operatorcmds.o opclasscmds.o \
	portalcmds.o prepare.o proclang.o queue.o \
	schemacmds.o sequence.o tablecmds.o tablespace.o trigger.o \
	tsearchcmds.o typecmds.o user.o vacuum.o vacuumlazy.o \
	variable.o view.o

include $(top_srcdir)/src/backend/common.mk
