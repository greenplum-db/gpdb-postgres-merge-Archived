<<<<<<< HEAD
# $PostgreSQL: pgsql/src/bin/scripts/nls.mk,v 1.23.2.1 2009/09/03 21:01:11 petere Exp $
CATALOG_NAME    := pgscripts
AVAIL_LANGUAGES := cs de es fr it ja ko pt_BR ro sv ta tr
=======
# $PostgreSQL: pgsql/src/bin/scripts/nls.mk,v 1.21 2007/11/29 08:50:35 petere Exp $
CATALOG_NAME    := pgscripts
AVAIL_LANGUAGES := cs de es fr it ko pt_BR ro ru sk sl sv ta tr zh_CN zh_TW
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
GETTEXT_FILES   := createdb.c createlang.c createuser.c \
                   dropdb.c droplang.c dropuser.c \
                   clusterdb.c vacuumdb.c reindexdb.c \
                   common.c
GETTEXT_TRIGGERS:= _ simple_prompt yesno_prompt
