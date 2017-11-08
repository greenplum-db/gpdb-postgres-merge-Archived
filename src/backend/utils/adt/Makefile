#
# Makefile for utils/adt
#
# $PostgreSQL: pgsql/src/backend/utils/adt/Makefile,v 1.70 2008/11/03 20:17:20 adunstan Exp $
#

subdir = src/backend/utils/adt
top_builddir = ../../../..
include $(top_builddir)/src/Makefile.global
override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)

# seems to be required for some date/time stuff 1999/07/22 bjm
ifeq "$(findstring alpha,$(host_cpu))" "alpha"
ifeq "$(GCC)" "yes"
override CFLAGS+= -mieee
endif
endif

<<<<<<< HEAD
OBJS = acl.o array_userfuncs.o arrayfuncs.o arrayutils.o ascii.o \
	bool.o cash.o char.o complex_type.o date.o datetime.o datum.o dbsize.o \
	domains.o encode.o enum.o float.o format_type.o formatting.o genfile.o \
	geo_ops.o geo_selfuncs.o gp_dump_oids.o gp_optimizer_functions.o \
	gp_partition_functions.o inet_cidr_ntop.o inet_net_pton.o int.o \
	int8.o interpolate.o like.o lockfuncs.o mac.o matrix.o misc.o nabstime.o name.o \
	network.o numeric.o numutils.o oid.o oracle_compat.o \
	percentile.o pg_locale.o pg_lzcompress.o pgstatfuncs.o pivot.o \
	pseudotypes.o quote.o regexp.o regproc.o ri_triggers.o rowtypes.o \
	ruleutils.o selfuncs.o tid.o timestamp.o varbit.o varchar.o varlena.o version.o \
=======
OBJS = acl.o arrayfuncs.o array_userfuncs.o arrayutils.o bool.o \
	cash.o char.o date.o datetime.o datum.o domains.o \
	enum.o float.o format_type.o \
	geo_ops.o geo_selfuncs.o int.o int8.o like.o lockfuncs.o \
	misc.o nabstime.o name.o numeric.o numutils.o \
	oid.o oracle_compat.o pseudotypes.o rowtypes.o \
	regexp.o regproc.o ruleutils.o selfuncs.o \
	tid.o timestamp.o varbit.o varchar.o varlena.o version.o xid.o \
	network.o mac.o inet_net_ntop.o inet_net_pton.o \
	ri_triggers.o pg_lzcompress.o pg_locale.o formatting.o \
	ascii.o quote.o pgstatfuncs.o encode.o dbsize.o genfile.o trigfuncs.o \
>>>>>>> 38e9348282e
	tsginidx.o tsgistidx.o tsquery.o tsquery_cleanup.o tsquery_gist.o \
	tsquery_op.o tsquery_rewrite.o tsquery_util.o tsrank.o \
	tsvector.o tsvector_op.o tsvector_parser.o \
	txid.o uuid.o xid.o xml.o json.o jsonfuncs.o

like.o: like.c like_match.c

include $(top_srcdir)/src/backend/common.mk
