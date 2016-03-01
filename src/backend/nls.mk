<<<<<<< HEAD
# $PostgreSQL: pgsql/src/backend/nls.mk,v 1.27 2009/06/26 19:33:43 petere Exp $
CATALOG_NAME	:= postgres
AVAIL_LANGUAGES	:= de es fr ja pt_BR tr
GETTEXT_FILES	:= + gettext-files
GETTEXT_TRIGGERS:= _ errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext write_stderr yyerror
=======
# $PostgreSQL: pgsql/src/backend/nls.mk,v 1.21.2.2 2008/10/27 19:37:28 tgl Exp $
CATALOG_NAME	:= postgres
AVAIL_LANGUAGES	:= af cs de es fr hr hu it ko nb nl pl pt_BR ro ru sk sl sv tr zh_CN zh_TW
GETTEXT_FILES	:= + gettext-files
GETTEXT_TRIGGERS:= _ errmsg errdetail errhint errcontext write_stderr yyerror
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

gettext-files: distprep
	find $(srcdir)/ $(srcdir)/../port/ -name '*.c' -print >$@

my-maintainer-clean:
	rm -f gettext-files

.PHONY: my-maintainer-clean
maintainer-clean: my-maintainer-clean
