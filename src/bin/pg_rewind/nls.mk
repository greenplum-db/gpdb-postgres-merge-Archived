# src/bin/pg_rewind/nls.mk
CATALOG_NAME     = pg_rewind
<<<<<<< HEAD
AVAIL_LANGUAGES  =cs de es fr it ko pl pt_BR ru zh_CN
GETTEXT_FILES    = copy_fetch.c datapagemap.c fetch.c file_ops.c filemap.c libpq_fetch.c logging.c parsexlog.c pg_rewind.c timeline.c ../../common/fe_memutils.c ../../common/restricted_token.c xlogreader.c
=======
AVAIL_LANGUAGES  =de
GETTEXT_FILES    = copy_fetch.c datapagemap.c fetch.c file_ops.c filemap.c libpq_fetch.c logging.c parsexlog.c pg_rewind.c timeline.c ../../common/fe_memutils.c ../../common/restricted_token.c ../../../src/backend/access/transam/xlogreader.c
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8

GETTEXT_TRIGGERS = pg_log:2 pg_fatal report_invalid_record:2
GETTEXT_FLAGS    = pg_log:2:c-format \
    pg_fatal:1:c-format \
    report_invalid_record:2:c-format
