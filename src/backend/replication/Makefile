#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for src/backend/replication
#
# IDENTIFICATION
<<<<<<< HEAD
#    src/backend/replication/Makefile
=======
#    $PostgreSQL: pgsql/src/backend/replication/Makefile,v 1.2 2010/01/20 09:16:24 heikki Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
#
#-------------------------------------------------------------------------

subdir = src/backend/replication
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
override CPPFLAGS := -I$(srcdir) $(CPPFLAGS)
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

#OBJS = walsender.o walreceiverfuncs.o walreceiver.o basebackup.o \
#	repl_gram.o syncrep.o
OBJS = basebackup.o gp_libpqwalreceiver.o repl_gram.o walreceiver.o \
	   walreceiverfuncs.o walsender.o syncrep.o gp_replication.o

include $(top_srcdir)/src/backend/common.mk

# repl_scanner is compiled as part of repl_gram
repl_gram.o: repl_scanner.c

# See notes in src/backend/parser/Makefile about the following two rules

repl_gram.c: repl_gram.y
ifdef BISON
	$(BISON) -d $(BISONFLAGS) -o $@ $<
else
	@$(missing) bison $< $@
endif

repl_scanner.c: repl_scanner.l
ifdef FLEX
	$(FLEX) $(FLEXFLAGS) -o'$@' $<
else
	@$(missing) flex $< $@
endif

# repl_gram.c and repl_scanner.c are in the distribution tarball, so
# they are not cleaned here.
=======
OBJS = walsender.o walreceiverfuncs.o walreceiver.o

include $(top_srcdir)/src/backend/common.mk
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
