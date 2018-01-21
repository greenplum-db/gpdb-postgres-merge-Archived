<<<<<<< HEAD
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.18.2.1 2009/09/03 21:01:07 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= cs de es fr it ko pt_BR ro ru sk sl sv ta tr zh_CN zh_TW
=======
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.19 2009/10/20 18:23:24 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= de es fr it ja ko pt_BR ru sv ta tr
>>>>>>> 78a09145e0
GETTEXT_FILES	:= pg_ctl.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
