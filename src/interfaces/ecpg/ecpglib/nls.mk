<<<<<<< HEAD
# $PostgreSQL: pgsql/src/interfaces/ecpg/ecpglib/nls.mk,v 1.5.2.1 2009/09/03 21:01:11 petere Exp $
CATALOG_NAME	 = ecpglib
AVAIL_LANGUAGES	 = de es fr it ja pt_BR tr
GETTEXT_FILES	 = connect.c error.c execute.c misc.c
GETTEXT_TRIGGERS = ecpg_gettext
=======
# $PostgreSQL $
CATALOG_NAME	 = ecpglib
AVAIL_LANGUAGES	 =
GETTEXT_FILES	 = ../compatlib/informix.c connect.c data.c descriptor.c error.c execute.c misc.c prepare.c
GETTEXT_TRIGGERS = _ ecpg_gettext ecpg_log
>>>>>>> b0a6ad70a12b6949fdebffa8ca1650162bf0254a
