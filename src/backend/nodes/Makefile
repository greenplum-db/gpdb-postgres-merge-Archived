#-------------------------------------------------------------------------
#
# Makefile--
#    Makefile for backend/nodes
#
# IDENTIFICATION
#    src/backend/nodes/Makefile
#
#-------------------------------------------------------------------------

subdir = src/backend/nodes
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global

OBJS = nodeFuncs.o nodes.o list.o bitmapset.o tidbitmap.o \
<<<<<<< HEAD
       copyfuncs.o equalfuncs.o makefuncs.o \
       outfuncs.o readfuncs.o print.o read.o params.o value.o \
	   outfast.o readfast.o

# readfast.c #includes readfuncs.c. Same with outfast.c and outfast.c
readfast.o: readfuncs.c
outfast.o: outfuncs.c
=======
       copyfuncs.o equalfuncs.o extensible.o makefuncs.o \
       outfuncs.o readfuncs.o print.o read.o params.o value.o
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365

include $(top_srcdir)/src/backend/common.mk
