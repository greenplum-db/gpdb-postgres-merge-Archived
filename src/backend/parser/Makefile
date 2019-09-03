#-------------------------------------------------------------------------
#
# Makefile for parser
#
# src/backend/parser/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/parser
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

override CPPFLAGS := -I. -I$(srcdir) $(CPPFLAGS)

<<<<<<< HEAD
OBJS= analyze.o gram.o scan.o keywords.o kwlookup.o parser.o \
=======
OBJS= analyze.o gram.o scan.o parser.o \
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
      parse_agg.o parse_clause.o parse_coerce.o parse_collate.o parse_cte.o \
      parse_expr.o parse_func.o parse_node.o parse_oper.o parse_param.o \
      parse_relation.o parse_target.o parse_type.o parse_utilcmd.o scansup.o
OBJS+= parse_partition.o

include $(top_srcdir)/src/backend/common.mk


# Latest flex causes warnings in this file.
ifeq ($(GCC),yes)
scan.o: CFLAGS += -Wno-error
endif


# There is no correct way to write a rule that generates two files.
# Rules with two targets don't have that meaning, they are merely
# shorthand for two otherwise separate rules.  If we have an action
# that in fact generates two or more files, we must choose one of them
# as primary and show it as the action's output, then make all of the
# other output files dependent on the primary, like this.  Furthermore,
# the "touch" action is essential, because it ensures that gram.h is
# marked as newer than (or at least no older than) gram.c.  Without that,
# make is likely to try to rebuild gram.h in subsequent runs, which causes
# failures in VPATH builds from tarballs.

gram.h: gram.c
	touch $@

gram.c: BISONFLAGS += -d
gram.c: BISON_CHECK_CMD = $(PERL) $(srcdir)/check_keywords.pl $< $(top_srcdir)/src/include/parser/kwlist.h


scan.c: FLEXFLAGS = -CF -p -p
scan.c: FLEX_NO_BACKUP=yes


# Force these dependencies to be known even without dependency info built:
<<<<<<< HEAD
gram.o scan.o keywords.o parser.o: gram.h
=======
gram.o scan.o parser.o: gram.h
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365


# gram.c, gram.h, and scan.c are in the distribution tarball, so they
# are not cleaned here.
clean distclean maintainer-clean:
	rm -f lex.backup
