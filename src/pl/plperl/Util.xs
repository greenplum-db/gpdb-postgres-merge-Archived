/**********************************************************************
 * PostgreSQL::InServer::Util
 *
<<<<<<< HEAD
 * src/pl/plperl/Util.xs
=======
 * $PostgreSQL: pgsql/src/pl/plperl/Util.xs,v 1.1 2010/01/20 01:08:21 adunstan Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
 *
 * Defines plperl interfaces for general-purpose utilities.
 * This module is bootstrapped as soon as an interpreter is initialized.
 * Currently doesn't define a PACKAGE= so all subs are in main:: to avoid
 * the need for explicit importing.
 *
 **********************************************************************/

/* this must be first: */
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"
#include "utils/bytea.h"       /* for byteain & byteaout */
#include "mb/pg_wchar.h"       /* for GetDatabaseEncoding */
/* Defined by Perl */
#undef _

/* perl stuff */
#include "plperl.h"
<<<<<<< HEAD
#include "plperl_helpers.h"
=======

>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

/*
 * Implementation of plperl's elog() function
 *
 * If the error level is less than ERROR, we'll just emit the message and
 * return.  When it is ERROR, elog() will longjmp, which we catch and
 * turn into a Perl croak().  Note we are assuming that elog() can't have
 * any internal failures that are so bad as to require a transaction abort.
 *
 * This is out-of-line to suppress "might be clobbered by longjmp" warnings.
 */
static void
<<<<<<< HEAD
do_util_elog(int level, SV *msg)
{
	MemoryContext oldcontext = CurrentMemoryContext;
	char	   * volatile cmsg = NULL;

	PG_TRY();
	{
		cmsg = sv2cstr(msg);
		elog(level, "%s", cmsg);
		pfree(cmsg);
	}
	PG_CATCH();
	{
		ErrorData  *edata;

		/* Must reset elog.c's state */
		MemoryContextSwitchTo(oldcontext);
		edata = CopyErrorData();
		FlushErrorState();

		if (cmsg)
			pfree(cmsg);

		/* Punt the error to Perl */
		croak_cstr(edata->message);
	}
	PG_END_TRY();
=======
do_util_elog(int level, char *message)
{
    MemoryContext oldcontext = CurrentMemoryContext;

    PG_TRY();
    {
        elog(level, "%s", message);
    }
    PG_CATCH();
    {
        ErrorData  *edata;

        /* Must reset elog.c's state */
        MemoryContextSwitchTo(oldcontext);
        edata = CopyErrorData();
        FlushErrorState();

        /* Punt the error to Perl */
        croak("%s", edata->message);
    }
    PG_END_TRY();
}

static SV  *
newSVstring_len(const char *str, STRLEN len)
{
    SV         *sv;

    sv = newSVpvn(str, len);
#if PERL_BCDVERSION >= 0x5006000L
    if (GetDatabaseEncoding() == PG_UTF8)
        SvUTF8_on(sv);
#endif
    return sv;
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
}

static text *
sv2text(SV *sv)
{
<<<<<<< HEAD
	char	   *str = sv2cstr(sv);
	text	   *text;

	text = cstring_to_text(str);
	pfree(str);
	return text;
=======
    STRLEN    sv_len;
    char     *sv_pv;

    if (!sv)
        sv = &PL_sv_undef;
    sv_pv = SvPV(sv, sv_len);
    return cstring_to_text_with_len(sv_pv, sv_len);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
}

MODULE = PostgreSQL::InServer::Util PREFIX = util_

PROTOTYPES: ENABLE
VERSIONCHECK: DISABLE

int
_aliased_constants()
    PROTOTYPE:
    ALIAS:
        DEBUG   = DEBUG2
        LOG     = LOG
        INFO    = INFO
        NOTICE  = NOTICE
        WARNING = WARNING
        ERROR   = ERROR
    CODE:
    /* uses the ALIAS value as the return value */
    RETVAL = ix;
    OUTPUT:
    RETVAL


void
<<<<<<< HEAD
util_elog(level, msg)
    int level
    SV *msg
=======
util_elog(level, message)
    int level
    char* message
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
    CODE:
        if (level > ERROR)      /* no PANIC allowed thanks */
            level = ERROR;
        if (level < DEBUG5)
            level = DEBUG5;
<<<<<<< HEAD
        do_util_elog(level, msg);
=======
        do_util_elog(level, message);
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

SV *
util_quote_literal(sv)
    SV *sv
    CODE:
    if (!sv || !SvOK(sv)) {
        RETVAL = &PL_sv_undef;
    }
    else {
        text *arg = sv2text(sv);
<<<<<<< HEAD
		text *quoted = DatumGetTextP(DirectFunctionCall1(quote_literal, PointerGetDatum(arg)));
		char *str;

		pfree(arg);
		str = text_to_cstring(quoted);
		RETVAL = cstr2sv(str);
		pfree(str);
=======
        text *ret = DatumGetTextP(DirectFunctionCall1(quote_literal, PointerGetDatum(arg)));
        RETVAL = newSVstring_len(VARDATA(ret), (VARSIZE(ret) - VARHDRSZ));
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
    }
    OUTPUT:
    RETVAL

SV *
util_quote_nullable(sv)
    SV *sv
    CODE:
<<<<<<< HEAD
    if (!sv || !SvOK(sv))
	{
        RETVAL = cstr2sv("NULL");
    }
    else
	{
        text *arg = sv2text(sv);
		text *quoted = DatumGetTextP(DirectFunctionCall1(quote_nullable, PointerGetDatum(arg)));
		char *str;

		pfree(arg);
		str = text_to_cstring(quoted);
		RETVAL = cstr2sv(str);
		pfree(str);
=======
    if (!sv || !SvOK(sv)) 
	{
        RETVAL = newSVstring_len("NULL", 4);
    }
    else 
	{
        text *arg = sv2text(sv);
        text *ret = DatumGetTextP(DirectFunctionCall1(quote_nullable, PointerGetDatum(arg)));
        RETVAL = newSVstring_len(VARDATA(ret), (VARSIZE(ret) - VARHDRSZ));
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
    }
    OUTPUT:
    RETVAL

SV *
util_quote_ident(sv)
    SV *sv
    PREINIT:
        text *arg;
<<<<<<< HEAD
		text *quoted;
		char *str;
    CODE:
        arg = sv2text(sv);
		quoted = DatumGetTextP(DirectFunctionCall1(quote_ident, PointerGetDatum(arg)));

		pfree(arg);
		str = text_to_cstring(quoted);
		RETVAL = cstr2sv(str);
		pfree(str);
=======
        text *ret;
    CODE:
        arg = sv2text(sv);
        ret = DatumGetTextP(DirectFunctionCall1(quote_ident, PointerGetDatum(arg)));
        RETVAL = newSVstring_len(VARDATA(ret), (VARSIZE(ret) - VARHDRSZ));
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
    OUTPUT:
    RETVAL

SV *
util_decode_bytea(sv)
    SV *sv
    PREINIT:
        char *arg;
        text *ret;
    CODE:
<<<<<<< HEAD
        arg = SvPVbyte_nolen(sv);
        ret = DatumGetTextP(DirectFunctionCall1(byteain, PointerGetDatum(arg)));
        /* not cstr2sv because this is raw bytes not utf8'able */
=======
        arg = SvPV_nolen(sv);
        ret = DatumGetTextP(DirectFunctionCall1(byteain, PointerGetDatum(arg)));
        /* not newSVstring_len because this is raw bytes not utf8'able */
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
        RETVAL = newSVpvn(VARDATA(ret), (VARSIZE(ret) - VARHDRSZ));
    OUTPUT:
    RETVAL

SV *
util_encode_bytea(sv)
    SV *sv
    PREINIT:
        text *arg;
        char *ret;
<<<<<<< HEAD
		STRLEN len;
    CODE:
        /* not sv2text because this is raw bytes not utf8'able */
        ret = SvPVbyte(sv, len);
		arg = cstring_to_text_with_len(ret, len);
        ret = DatumGetCString(DirectFunctionCall1(byteaout, PointerGetDatum(arg)));
        RETVAL = cstr2sv(ret);
=======
    CODE:
        arg = sv2text(sv);
        ret = DatumGetCString(DirectFunctionCall1(byteaout, PointerGetDatum(arg)));
        RETVAL = newSVstring_len(ret, strlen(ret));
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
    OUTPUT:
    RETVAL

SV *
looks_like_number(sv)
    SV *sv
    CODE:
    if (!SvOK(sv))
        RETVAL = &PL_sv_undef;
    else if ( looks_like_number(sv) )
        RETVAL = &PL_sv_yes;
    else
        RETVAL = &PL_sv_no;
    OUTPUT:
    RETVAL

<<<<<<< HEAD
SV *
encode_typed_literal(sv, typname)
	SV 	   *sv
	char   *typname;
	PREINIT:
		char 	*outstr;
	CODE:
		outstr = plperl_sv_to_literal(sv, typname);
		if (outstr == NULL)
			RETVAL = &PL_sv_undef;
		else
			RETVAL = cstr2sv(outstr);
	OUTPUT:
	RETVAL
=======
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2

BOOT:
    items = 0;  /* avoid 'unused variable' warning */
