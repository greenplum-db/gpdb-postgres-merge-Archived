<<<<<<< HEAD
# $PostgreSQL: pgsql/src/pl/plpython/nls.mk,v 1.6.2.1 2009/09/03 21:01:24 petere Exp $
CATALOG_NAME	:= plpython
AVAIL_LANGUAGES	:= de es fr it ja pt_BR tr
=======
# $PostgreSQL: pgsql/src/pl/plpython/nls.mk,v 1.6 2009/06/10 23:42:44 petere Exp $
CATALOG_NAME	:= plpython
AVAIL_LANGUAGES	:= de es fr ja pt_BR tr
>>>>>>> 4d53a2f9699547bdc12831d2860c9d44c465e805
GETTEXT_FILES	:= plpython.c
GETTEXT_TRIGGERS:= errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext PLy_elog:2 PLy_exception_set:2 PLy_exception_set_plural:2,3
