#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for utils/sort
#
# IDENTIFICATION
#    src/backend/utils/sort/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/utils/sort
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global

<<<<<<< HEAD
OBJS = logtape.o tuplesort.o tuplestore.o tuplestorenew.o tuplesort_mk.o tuplesort_mkheap.o tuplesort_mkqsort.o
=======
override CPPFLAGS := -I. -I$(srcdir) $(CPPFLAGS)

OBJS = logtape.o sortsupport.o tuplesort.o tuplestore.o

tuplesort.o: qsort_tuple.c

qsort_tuple.c: gen_qsort_tuple.pl
	$(PERL) $(srcdir)/gen_qsort_tuple.pl $< > $@
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

include $(top_srcdir)/src/backend/common.mk

maintainer-clean:
	rm -f qsort_tuple.c
