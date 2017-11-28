<<<<<<< HEAD
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.18.2.1 2009/09/03 21:01:07 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= cs de es fr it ko pt_BR ro ru sk sl sv ta tr zh_CN zh_TW
=======
# $PostgreSQL: pgsql/src/bin/pg_ctl/nls.mk,v 1.18 2009/06/26 19:33:49 petere Exp $
CATALOG_NAME	:= pg_ctl
AVAIL_LANGUAGES	:= de es fr ja ko pt_BR ru sv ta tr
>>>>>>> 4d53a2f9699547bdc12831d2860c9d44c465e805
GETTEXT_FILES	:= pg_ctl.c ../../port/exec.c
GETTEXT_TRIGGERS:= _ simple_prompt
