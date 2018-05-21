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

override CPPFLAGS := -I. -I$(srcdir) $(CPPFLAGS)

OBJS = logtape.o sortsupport.o tuplesort.o tuplestore.o tuplestorenew.o tuplesort_mk.o tuplesort_mkheap.o tuplesort_mkqsort.o

include $(top_srcdir)/src/backend/common.mk

maintainer-clean:
	rm -f qsort_tuple.c
