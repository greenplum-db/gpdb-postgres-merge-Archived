#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for backend/commands
#
# IDENTIFICATION
#    src/backend/commands/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/commands
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

explain.o: explain_gp.c

OBJS = aggregatecmds.o alter.o analyze.o async.o cluster.o comment.o  \
<<<<<<< HEAD
	constraint.o conversioncmds.o copy.o \
	dbcommands.o define.o discard.o explain.o extension.o foreigncmds.o functioncmds.o \
=======
	collationcmds.o constraint.o conversioncmds.o copy.o \
	dbcommands.o define.o discard.o explain.o extension.o \
	foreigncmds.o functioncmds.o \
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
	indexcmds.o lockcmds.o operatorcmds.o opclasscmds.o \
	portalcmds.o prepare.o proclang.o \
	schemacmds.o seclabel.o sequence.o tablecmds.o tablespace.o trigger.o \
	tsearchcmds.o typecmds.o user.o vacuum.o vacuumlazy.o \
	variable.o view.o

OBJS += analyzefuncs.o extprotocolcmds.o exttablecmds.o queue.o
OBJS += resgroupcmds.o

include $(top_srcdir)/src/backend/common.mk
