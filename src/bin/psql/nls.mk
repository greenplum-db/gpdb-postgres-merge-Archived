<<<<<<< HEAD
# src/bin/psql/nls.mk
=======
# $PostgreSQL: pgsql/src/bin/psql/nls.mk,v 1.23 2009/06/26 19:33:50 petere Exp $
>>>>>>> 4d53a2f9699547bdc12831d2860c9d44c465e805
CATALOG_NAME	:= psql
AVAIL_LANGUAGES	:= cs de es fr ja pt_BR sv tr
GETTEXT_FILES	:= command.c common.c copy.c help.c input.c large_obj.c \
                   mainloop.c print.c startup.c describe.c sql_help.h sql_help.c \
                   ../../port/exec.c
GETTEXT_TRIGGERS:= _ N_ psql_error simple_prompt
