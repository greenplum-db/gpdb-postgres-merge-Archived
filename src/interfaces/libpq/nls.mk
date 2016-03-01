<<<<<<< HEAD
# $PostgreSQL: pgsql/src/interfaces/libpq/nls.mk,v 1.25 2010/05/13 15:56:42 petere Exp $
CATALOG_NAME	:= libpq
AVAIL_LANGUAGES	:= cs de es fr it ja ko pt_BR ru sv ta tr zh_CN
=======
# $PostgreSQL: pgsql/src/interfaces/libpq/nls.mk,v 1.21 2007/10/27 00:13:43 petere Exp $
CATALOG_NAME	:= libpq
AVAIL_LANGUAGES	:= af cs de es fr hr it ko nb pl pt_BR ru sk sl sv ta tr zh_CN zh_TW
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
GETTEXT_FILES	:= fe-auth.c fe-connect.c fe-exec.c fe-lobj.c fe-misc.c fe-protocol2.c fe-protocol3.c fe-secure.c
GETTEXT_TRIGGERS:= libpq_gettext pqInternalNotice:2
