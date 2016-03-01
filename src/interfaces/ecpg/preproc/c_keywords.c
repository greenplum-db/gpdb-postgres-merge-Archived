/*-------------------------------------------------------------------------
 *
 * keywords.c
 *	  lexical token lookup for reserved words in postgres embedded SQL
 *
<<<<<<< HEAD
 * $PostgreSQL: pgsql/src/interfaces/ecpg/preproc/c_keywords.c,v 1.23 2009/06/11 14:49:13 momjian Exp $
=======
 * $PostgreSQL: pgsql/src/interfaces/ecpg/preproc/c_keywords.c,v 1.21 2007/08/22 08:20:58 meskes Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 * §
 *-------------------------------------------------------------------------
 */
#include "postgres_fe.h"

#include <ctype.h>

#include "extern.h"
#include "preproc.h"

/*
 * List of (keyword-name, keyword-token-value) pairs.
 *
 * !!WARNING!!: This list must be sorted, because binary
 *		 search is used to locate entries.
 */
static const ScanKeyword ScanCKeywords[] = {
<<<<<<< HEAD
	/* name, value, category */

	/*
	 * category is not needed in ecpg, it is only here so we can share the
	 * data structure with the backend
	 */
	{"VARCHAR", VARCHAR, 0},
	{"auto", S_AUTO, 0},
	{"bool", SQL_BOOL, 0},
	{"char", CHAR_P, 0},
	{"const", S_CONST, 0},
	{"enum", ENUM_P, 0},
	{"extern", S_EXTERN, 0},
	{"float", FLOAT_P, 0},
	{"hour", HOUR_P, 0},
	{"int", INT_P, 0},
	{"long", SQL_LONG, 0},
	{"minute", MINUTE_P, 0},
	{"month", MONTH_P, 0},
	{"register", S_REGISTER, 0},
	{"second", SECOND_P, 0},
	{"short", SQL_SHORT, 0},
	{"signed", SQL_SIGNED, 0},
	{"static", S_STATIC, 0},
	{"struct", SQL_STRUCT, 0},
	{"to", TO, 0},
	{"typedef", S_TYPEDEF, 0},
	{"union", UNION, 0},
	{"unsigned", SQL_UNSIGNED, 0},
	{"varchar", VARCHAR, 0},
	{"volatile", S_VOLATILE, 0},
	{"year", YEAR_P, 0},
};

const ScanKeyword *
ScanCKeywordLookup(const char *text)
=======
	/* name					value			*/
	{"VARCHAR", VARCHAR},
	{"auto", S_AUTO},
	{"bool", SQL_BOOL},
	{"char", CHAR_P},
	{"const", S_CONST},
	{"enum", ENUM_P},
	{"extern", S_EXTERN},
	{"float", FLOAT_P},
	{"hour", HOUR_P},
	{"int", INT_P},
	{"long", SQL_LONG},
	{"minute", MINUTE_P},
	{"month", MONTH_P},
	{"register", S_REGISTER},
	{"second", SECOND_P},
	{"short", SQL_SHORT},
	{"signed", SQL_SIGNED},
	{"static", S_STATIC},
	{"struct", SQL_STRUCT},
	{"to", TO},
	{"typedef", S_TYPEDEF},
	{"union", UNION},
	{"unsigned", SQL_UNSIGNED},
	{"varchar", VARCHAR},
	{"volatile", S_VOLATILE},
	{"year", YEAR_P},
};

const ScanKeyword *
ScanCKeywordLookup(char *text)
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
{
	return DoLookup(text, &ScanCKeywords[0], endof(ScanCKeywords) - 1);
}
