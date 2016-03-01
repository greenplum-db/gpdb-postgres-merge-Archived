<<<<<<< HEAD
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.18.2.1 2009/09/03 21:01:07 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= de es fr it ja ko pt_BR ru sv ta tr
=======
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.16 2007/11/15 20:38:15 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= cs de es fr ko pt_BR ro ru sk sl sv ta tr zh_CN zh_TW
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
GETTEXT_FILES	:= pg_ctl.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
