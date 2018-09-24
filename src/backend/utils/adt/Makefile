#
# Makefile for utils/adt
#
# src/backend/utils/adt/Makefile
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

# keep this list arranged alphabetically or it gets to be a mess
OBJS = acl.o arrayfuncs.o array_selfuncs.o array_typanalyze.o \
<<<<<<< HEAD
	array_userfuncs.o arrayutils.o bool.o \
	cash.o char.o complex_type.o gp_dump_oids.o gp_optimizer_functions.o \
	gp_partition_functions.o interpolate.o matrix.o orderedsetaggs.o \
	pivot.o date.o datetime.o datum.o domains.o \
	enum.o float.o format_type.o \
	geo_ops.o geo_selfuncs.o int.o int8.o json.o jsonfuncs.o like.o \
	lockfuncs.o misc.o nabstime.o name.o numeric.o numutils.o \
	oid.o oracle_compat.o pseudotypes.o rangetypes.o rangetypes_gist.o \
	rowtypes.o regexp.o regproc.o ruleutils.o selfuncs.o \
	tid.o timestamp.o varbit.o varchar.o varlena.o version.o xid.o \
	network.o mac.o inet_cidr_ntop.o inet_net_pton.o \
	ri_triggers.o pg_lzcompress.o pg_locale.o formatting.o \
	ascii.o quote.o pgstatfuncs.o encode.o dbsize.o genfile.o trigfuncs.o \
=======
	array_userfuncs.o arrayutils.o ascii.o bool.o \
	cash.o char.o date.o datetime.o datum.o dbsize.o domains.o \
	encode.o enum.o float.o format_type.o formatting.o genfile.o \
	geo_ops.o geo_selfuncs.o inet_cidr_ntop.o inet_net_pton.o int.o \
	int8.o json.o jsonb.o jsonb_gin.o jsonb_op.o jsonb_util.o \
	jsonfuncs.o like.o lockfuncs.o mac.o misc.o nabstime.o name.o \
	network.o network_gist.o network_selfuncs.o \
	numeric.o numutils.o oid.o oracle_compat.o \
	orderedsetaggs.o pg_lzcompress.o pg_locale.o pg_lsn.o \
	pgstatfuncs.o pseudotypes.o quote.o rangetypes.o rangetypes_gist.o \
	rangetypes_selfuncs.o rangetypes_spgist.o rangetypes_typanalyze.o \
	regexp.o regproc.o ri_triggers.o rowtypes.o ruleutils.o \
	selfuncs.o tid.o timestamp.o trigfuncs.o \
>>>>>>> ab76208e3df6841b3770edeece57d0f048392237
	tsginidx.o tsgistidx.o tsquery.o tsquery_cleanup.o tsquery_gist.o \
	tsquery_op.o tsquery_rewrite.o tsquery_util.o tsrank.o \
	tsvector.o tsvector_op.o tsvector_parser.o \
	txid.o uuid.o varbit.o varchar.o varlena.o version.o \
	windowfuncs.o xid.o xml.o

like.o: like.c like_match.c

include $(top_srcdir)/src/backend/common.mk
