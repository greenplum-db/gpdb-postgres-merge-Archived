<<<<<<< HEAD
# $PostgreSQL: pgsql/src/bin/initdb/nls.mk,v 1.21.2.1 2009/09/03 21:01:00 petere Exp $
CATALOG_NAME	:= initdb
AVAIL_LANGUAGES	:= cs de es fr it ja pt_BR ru sv ta tr
=======
# $PostgreSQL: pgsql/src/bin/initdb/nls.mk,v 1.19 2007/10/27 00:13:42 petere Exp $
CATALOG_NAME	:= initdb
AVAIL_LANGUAGES	:= cs de es fr it ko pl pt_BR ro ru sk sl sv ta tr zh_CN zh_TW
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
GETTEXT_FILES	:= initdb.c ../../port/dirmod.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
