# contrib/Makefile

subdir = contrib
top_builddir = ..
include $(top_builddir)/src/Makefile.global

SUBDIRS = \
<<<<<<< HEAD
=======
		adminpack	\
		auth_delay	\
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		auto_explain	\
		btree_gin	\
		citext		\
		dblink		\
		dict_int	\
		dict_xsyn	\
<<<<<<< HEAD
=======
		dummy_seclabel	\
		earthdistance	\
		file_fdw	\
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
		fuzzystrmatch	\
		hstore		\
		intarray	\
		pageinspect	\
		passwordcheck	\
		pg_archivecleanup \
		pg_buffercache	\
		pg_freespacemap \
		pg_standby	\
		pg_stat_statements \
		pg_test_fsync	\
		pg_trgm		\
		pg_upgrade	\
		pg_upgrade_support \
		pgbench		\
		pgcrypto	\
		test_parser

# Greenplum-specific additions (to ease merge pain).
SUBDIRS += \
		xlogdump \
		formatter \
		formatter_fixedwidth \
		extprotocol \
		gp_distribution_policy \
		gp_internal_tools \
		gp_inject_fault \
		gp_cancel_query \
		indexscan \

ifeq ($(with_openssl),yes)
SUBDIRS += sslinfo
endif

ifeq ($(with_ossp_uuid),yes)
SUBDIRS += uuid-ossp
endif

ifeq ($(with_libxml),yes)
SUBDIRS += xml2
endif

<<<<<<< HEAD
ifeq ($(enable_orca),yes)
SUBDIRS += orca_debug
endif

SUBDIRS += gp_replica_check

=======
ifeq ($(with_selinux),yes)
SUBDIRS += sepgsql
endif

>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
# Missing:
#		start-scripts	\ (does not have a makefile)


$(recurse)
