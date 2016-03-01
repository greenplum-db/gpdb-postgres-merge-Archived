# $PostgreSQL: pgsql/contrib/Makefile,v 1.82.2.1 2008/05/08 16:49:47 tgl Exp $

subdir = contrib
top_builddir = ..
include $(top_builddir)/src/Makefile.global

WANTED_DIRS = \
<<<<<<< HEAD
		xlogdump \
		pgbench \
		changetrackingdump \
		formatter \
		formatter_fixedwidth \
		extprotocol \
		gp_distribution_policy \
		gp_internal_tools \
		gp_cancel_query \
		indexscan

=======
		adminpack	\
		btree_gist	\
		chkpass		\
		cube		\
		dblink		\
		dict_int	\
		dict_xsyn	\
		earthdistance	\
		fuzzystrmatch	\
		hstore		\
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
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

ifeq ($(with_openssl),yes)
WANTED_DIRS += sslinfo
endif

ifeq ($(with_ossp_uuid),yes)
WANTED_DIRS += uuid-ossp
endif

ifeq ($(with_libxml),yes)
WANTED_DIRS += xml2
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
