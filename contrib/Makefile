# $PostgreSQL: pgsql/contrib/Makefile,v 1.86 2009/01/04 22:19:59 tgl Exp $

subdir = contrib
top_builddir = ..
include $(top_builddir)/src/Makefile.global

WANTED_DIRS = \
		auto_explain	\
		citext		\
		dblink		\
		dict_int	\
		dict_xsyn	\
		fuzzystrmatch	\
		hstore		\
<<<<<<< HEAD
		intarray \
		pageinspect \
		pg_upgrade \
		pg_upgrade_support \
		test_parser \
		pgcrypto

# Greenplum-specific additions (to ease merge pain).
WANTED_DIRS += \
		xlogdump \
		pgbench \
		changetrackingdump \
		formatter \
		formatter_fixedwidth \
		extprotocol \
		gp_distribution_policy \
		gp_internal_tools \
		gp_inject_fault \
		gp_cancel_query \
		indexscan \
    passwordcheck
=======
		intagg		\
		intarray	\
		isn		\
		lo		\
		ltree		\
		oid2name	\
		pageinspect	\
		pg_buffercache	\
		pg_freespacemap \
		pg_standby	\
		pg_stat_statements \
		pg_trgm		\
		pgbench		\
		pgcrypto	\
		pgrowlocks	\
		pgstattuple	\
		seg		\
		spi		\
		tablefunc	\
		test_parser	\
		tsearch2	\
		vacuumlo
>>>>>>> b0a6ad70a12b6949fdebffa8ca1650162bf0254a

ifeq ($(with_openssl),yes)
WANTED_DIRS += sslinfo
endif

ifeq ($(with_ossp_uuid),yes)
WANTED_DIRS += uuid-ossp
endif

ifeq ($(with_libxml),yes)
WANTED_DIRS += xml2
endif

ifeq ($(enable_orca),yes)
WANTED_DIRS += orca_debug
endif

ifeq ($(enable_segwalrep),yes)
WANTED_DIRS += gp_replica_check
endif

# Missing:
#		start-scripts	\ (does not have a makefile)


all install installdirs uninstall distprep clean distclean maintainer-clean:
	@for dir in $(WANTED_DIRS); do \
		$(MAKE) -C $$dir $@ || exit; \
	done

# We'd like check operations to run all the subtests before failing.
check installcheck:
	@CHECKERR=0; for dir in $(WANTED_DIRS); do \
		$(MAKE) -C $$dir $@ || CHECKERR=$$?; \
	done; \
	exit $$CHECKERR
