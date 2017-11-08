<<<<<<< HEAD
# src/pl/plperl/nls.mk
CATALOG_NAME	:= plperl
AVAIL_LANGUAGES	:= cs de es fr it ja pl pt_BR ro ru tr zh_CN zh_TW
GETTEXT_FILES	:= plperl.c SPI.c
GETTEXT_TRIGGERS:= errmsg errmsg_plural:1,2 errdetail errdetail_log errdetail_plural:1,2 errhint errcontext
=======
# $PostgreSQL: pgsql/src/pl/plperl/nls.mk,v 1.1 2008/10/09 17:24:05 alvherre Exp $
CATALOG_NAME	:= plperl
AVAIL_LANGUAGES	:=
GETTEXT_FILES	:= plperl.c SPI.xs
GETTEXT_TRIGGERS:= _ errmsg errdetail errdetail_log errhint errcontext write_stderr croak Perl_croak
>>>>>>> 38e9348282e
