# src/bin/pg_rewind/nls.mk
CATALOG_NAME     = pg_rewind
<<<<<<< HEAD
AVAIL_LANGUAGES  =cs de es fr it ko pl pt_BR ru zh_CN
GETTEXT_FILES    = copy_fetch.c datapagemap.c fetch.c file_ops.c filemap.c libpq_fetch.c logging.c parsexlog.c pg_rewind.c timeline.c ../../common/fe_memutils.c ../../common/restricted_token.c xlogreader.c

GETTEXT_TRIGGERS = pg_log:2 pg_fatal report_invalid_record:2
GETTEXT_FLAGS    = pg_log:2:c-format \
=======
AVAIL_LANGUAGES  =de es fr it ja ko pl pt_BR ru sv tr zh_CN
GETTEXT_FILES    = $(FRONTEND_COMMON_GETTEXT_FILES) copy_fetch.c datapagemap.c fetch.c file_ops.c filemap.c libpq_fetch.c parsexlog.c pg_rewind.c timeline.c ../../common/fe_memutils.c ../../common/restricted_token.c xlogreader.c
GETTEXT_TRIGGERS = $(FRONTEND_COMMON_GETTEXT_TRIGGERS) pg_fatal report_invalid_record:2
GETTEXT_FLAGS    = $(FRONTEND_COMMON_GETTEXT_FLAGS) \
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
    pg_fatal:1:c-format \
    report_invalid_record:2:c-format
