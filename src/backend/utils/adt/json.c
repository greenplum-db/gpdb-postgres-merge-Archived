/*-------------------------------------------------------------------------
 *
 * json.c
 *		JSON data type support.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2013, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2012, PostgreSQL Global Development Group
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	  src/backend/utils/adt/json.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

<<<<<<< HEAD
#include "access/transam.h"
#include "catalog/pg_cast.h"
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
#include "catalog/pg_type.h"
#include "executor/spi.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "mb/pg_wchar.h"
<<<<<<< HEAD
#include "miscadmin.h"
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
#include "parser/parse_coerce.h"
#include "utils/array.h"
#include "utils/builtins.h"
#include "utils/lsyscache.h"
#include "utils/json.h"
<<<<<<< HEAD
#include "utils/jsonapi.h"
#include "utils/typcache.h"
#include "utils/syscache.h"

/*
 * The context of the parser is maintained by the recursive descent
 * mechanism, but is passed explicitly to the error reporting routine
 * for better diagnostics.
 */
typedef enum					/* contexts of JSON parser */
{
	JSON_PARSE_VALUE,			/* expecting a value */
	JSON_PARSE_STRING,			/* expecting a string (for a field name) */
=======
#include "utils/typcache.h"

typedef enum					/* types of JSON values */
{
	JSON_VALUE_INVALID,			/* non-value tokens are reported as this */
	JSON_VALUE_STRING,
	JSON_VALUE_NUMBER,
	JSON_VALUE_OBJECT,
	JSON_VALUE_ARRAY,
	JSON_VALUE_TRUE,
	JSON_VALUE_FALSE,
	JSON_VALUE_NULL
} JsonValueType;

typedef struct					/* state of JSON lexer */
{
	char	   *input;			/* whole string being parsed */
	char	   *token_start;	/* start of current token within input */
	char	   *token_terminator; /* end of previous or current token */
	JsonValueType token_type;	/* type of current token, once it's known */
} JsonLexContext;

typedef enum					/* states of JSON parser */
{
	JSON_PARSE_VALUE,			/* expecting a value */
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
	JSON_PARSE_ARRAY_START,		/* saw '[', expecting value or ']' */
	JSON_PARSE_ARRAY_NEXT,		/* saw array element, expecting ',' or ']' */
	JSON_PARSE_OBJECT_START,	/* saw '{', expecting label or '}' */
	JSON_PARSE_OBJECT_LABEL,	/* saw object label, expecting ':' */
	JSON_PARSE_OBJECT_NEXT,		/* saw object value, expecting ',' or '}' */
<<<<<<< HEAD
	JSON_PARSE_OBJECT_COMMA,	/* saw object ',', expecting next label */
	JSON_PARSE_END				/* saw the end of a document, expect nothing */
} JsonParseContext;

typedef enum					/* type categories for datum_to_json */
{
	JSONTYPE_NULL,				/* null, so we didn't bother to identify */
	JSONTYPE_BOOL,				/* boolean (built-in types only) */
	JSONTYPE_NUMERIC,			/* numeric (ditto) */
	JSONTYPE_JSON,				/* JSON itself */
	JSONTYPE_ARRAY,				/* array */
	JSONTYPE_COMPOSITE,			/* composite */
	JSONTYPE_CAST,				/* something with an explicit cast to JSON */
	JSONTYPE_OTHER				/* all else */
} JsonTypeCategory;

static inline void json_lex(JsonLexContext *lex);
static inline void json_lex_string(JsonLexContext *lex);
static inline void json_lex_number(JsonLexContext *lex, char *s,
				bool *num_err, int *total_len);
static inline void parse_scalar(JsonLexContext *lex, JsonSemAction *sem);
static void parse_object_field(JsonLexContext *lex, JsonSemAction *sem);
static void parse_object(JsonLexContext *lex, JsonSemAction *sem);
static void parse_array_element(JsonLexContext *lex, JsonSemAction *sem);
static void parse_array(JsonLexContext *lex, JsonSemAction *sem);
static void report_parse_error(JsonParseContext ctx, JsonLexContext *lex);
static void report_invalid_token(JsonLexContext *lex);
static int	report_json_context(JsonLexContext *lex);
static char *extract_mb_char(char *s);
static void composite_to_json(Datum composite, StringInfo result,
				  bool use_line_feeds);
static void array_dim_to_json(StringInfo result, int dim, int ndims, int *dims,
				  Datum *vals, bool *nulls, int *valcount,
				  JsonTypeCategory tcategory, Oid outfuncoid,
				  bool use_line_feeds);
static void array_to_json_internal(Datum array, StringInfo result,
					   bool use_line_feeds);
static void json_categorize_type(Oid typoid,
					 JsonTypeCategory *tcategory,
					 Oid *outfuncoid);
static void datum_to_json(Datum val, bool is_null, StringInfo result,
			  JsonTypeCategory tcategory, Oid outfuncoid);
static text *catenate_stringinfo_string(StringInfo buffer, const char *addon);

/* the null action object used for pure validation */
static JsonSemAction nullSemAction =
{
	NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL
};

/* Recursive Descent parser support routines */

/*
 * lex_peek
 *
 * what is the current look_ahead token?
*/
static inline JsonTokenType
lex_peek(JsonLexContext *lex)
{
	return lex->token_type;
}

/*
 * lex_accept
 *
 * accept the look_ahead token and move the lexer to the next token if the
 * look_ahead token matches the token parameter. In that case, and if required,
 * also hand back the de-escaped lexeme.
 *
 * returns true if the token matched, false otherwise.
 */
static inline bool
lex_accept(JsonLexContext *lex, JsonTokenType token, char **lexeme)
{
	if (lex->token_type == token)
	{
		if (lexeme != NULL)
		{
			if (lex->token_type == JSON_TOKEN_STRING)
			{
				if (lex->strval != NULL)
					*lexeme = pstrdup(lex->strval->data);
			}
			else
			{
				int			len = (lex->token_terminator - lex->token_start);
				char	   *tokstr = palloc(len + 1);

				memcpy(tokstr, lex->token_start, len);
				tokstr[len] = '\0';
				*lexeme = tokstr;
			}
		}
		json_lex(lex);
		return true;
	}
	return false;
}

/*
 * lex_accept
 *
 * move the lexer to the next token if the current look_ahead token matches
 * the parameter token. Otherwise, report an error.
 */
static inline void
lex_expect(JsonParseContext ctx, JsonLexContext *lex, JsonTokenType token)
{
	if (!lex_accept(lex, token, NULL))
		report_parse_error(ctx, lex);
}

=======
	JSON_PARSE_OBJECT_COMMA		/* saw object ',', expecting next label */
} JsonParseState;

typedef struct JsonParseStack	/* the parser state has to be stackable */
{
	JsonParseState state;
	/* currently only need the state enum, but maybe someday more stuff */
} JsonParseStack;

typedef enum					/* required operations on state stack */
{
	JSON_STACKOP_NONE,			/* no-op */
	JSON_STACKOP_PUSH,			/* push new JSON_PARSE_VALUE stack item */
	JSON_STACKOP_PUSH_WITH_PUSHBACK, /* push, then rescan current token */
	JSON_STACKOP_POP			/* pop, or expect end of input if no stack */
} JsonStackOp;

static void json_validate_cstring(char *input);
static void json_lex(JsonLexContext *lex);
static void json_lex_string(JsonLexContext *lex);
static void json_lex_number(JsonLexContext *lex, char *s);
static void report_parse_error(JsonParseStack *stack, JsonLexContext *lex);
static void report_invalid_token(JsonLexContext *lex);
static int report_json_context(JsonLexContext *lex);
static char *extract_mb_char(char *s);
static void composite_to_json(Datum composite, StringInfo result,
							  bool use_line_feeds);
static void array_dim_to_json(StringInfo result, int dim, int ndims, int *dims,
				  Datum *vals, bool *nulls, int *valcount,
				  TYPCATEGORY tcategory, Oid typoutputfunc,
				  bool use_line_feeds);
static void array_to_json_internal(Datum array, StringInfo result,
								   bool use_line_feeds);

/* fake type category for JSON so we can distinguish it in datum_to_json */
#define TYPCATEGORY_JSON 'j'
/* letters appearing in numeric output that aren't valid in a JSON number */
#define NON_NUMERIC_LETTER "NnAaIiFfTtYy"
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
/* chars to consider as part of an alphanumeric token */
#define JSON_ALPHANUMERIC_CHAR(c)  \
	(((c) >= 'a' && (c) <= 'z') || \
	 ((c) >= 'A' && (c) <= 'Z') || \
	 ((c) >= '0' && (c) <= '9') || \
	 (c) == '_' || \
	 IS_HIGHBIT_SET(c))

<<<<<<< HEAD
/*
 * Utility function to check if a string is a valid JSON number.
 *
 * str is of length len, and need not be null-terminated.
 */
bool
IsValidJsonNumber(const char *str, int len)
{
	bool		numeric_error;
	int			total_len;
	JsonLexContext dummy_lex;

	if (len <= 0)
		return false;

	/*
	 * json_lex_number expects a leading  '-' to have been eaten already.
	 *
	 * having to cast away the constness of str is ugly, but there's not much
	 * easy alternative.
	 */
	if (*str == '-')
	{
		dummy_lex.input = (char *) str + 1;
		dummy_lex.input_length = len - 1;
	}
	else
	{
		dummy_lex.input = (char *) str;
		dummy_lex.input_length = len;
	}

	json_lex_number(&dummy_lex, dummy_lex.input, &numeric_error, &total_len);

	return (!numeric_error) && (total_len == dummy_lex.input_length);
}
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/*
 * Input.
 */
Datum
json_in(PG_FUNCTION_ARGS)
{
<<<<<<< HEAD
	char	   *json = PG_GETARG_CSTRING(0);
	text	   *result = cstring_to_text(json);
	JsonLexContext *lex;

	/* validate it */
	lex = makeJsonLexContext(result, false);
	pg_parse_json(lex, &nullSemAction);

	/* Internal representation is the same as text, for now */
	PG_RETURN_TEXT_P(result);
=======
	char	   *text = PG_GETARG_CSTRING(0);

	json_validate_cstring(text);

	/* Internal representation is the same as text, for now */
	PG_RETURN_TEXT_P(cstring_to_text(text));
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
}

/*
 * Output.
 */
Datum
json_out(PG_FUNCTION_ARGS)
{
	/* we needn't detoast because text_to_cstring will handle that */
	Datum		txt = PG_GETARG_DATUM(0);

	PG_RETURN_CSTRING(TextDatumGetCString(txt));
}

/*
 * Binary send.
 */
Datum
json_send(PG_FUNCTION_ARGS)
{
	text	   *t = PG_GETARG_TEXT_PP(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendtext(&buf, VARDATA_ANY(t), VARSIZE_ANY_EXHDR(t));
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*
 * Binary receive.
 */
Datum
json_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	text	   *result;
	char	   *str;
	int			nbytes;
<<<<<<< HEAD
	JsonLexContext *lex;

	str = pq_getmsgtext(buf, buf->len - buf->cursor, &nbytes);

	result = palloc(nbytes + VARHDRSZ);
	SET_VARSIZE(result, nbytes + VARHDRSZ);
	memcpy(VARDATA(result), str, nbytes);

	/* Validate it. */
	lex = makeJsonLexContext(result, false);
	pg_parse_json(lex, &nullSemAction);
=======

	str = pq_getmsgtext(buf, buf->len - buf->cursor, &nbytes);

	/*
	 * We need a null-terminated string to pass to json_validate_cstring().
	 * Rather than make a separate copy, make the temporary result one byte
	 * bigger than it needs to be.
	 */
	result = palloc(nbytes + 1 + VARHDRSZ);
	SET_VARSIZE(result, nbytes + VARHDRSZ);
	memcpy(VARDATA(result), str, nbytes);
	str = VARDATA(result);
	str[nbytes] = '\0';

	/* Validate it. */
	json_validate_cstring(str);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	PG_RETURN_TEXT_P(result);
}

/*
<<<<<<< HEAD
 * makeJsonLexContext
 *
 * lex constructor, with or without StringInfo object
 * for de-escaped lexemes.
 *
 * Without is better as it makes the processing faster, so only make one
 * if really required.
 */
JsonLexContext *
makeJsonLexContext(text *json, bool need_escapes)
{
	JsonLexContext *lex = palloc0(sizeof(JsonLexContext));

	lex->input = lex->token_terminator = lex->line_start = VARDATA(json);
	lex->line_number = 1;
	lex->input_length = VARSIZE(json) - VARHDRSZ;
	if (need_escapes)
		lex->strval = makeStringInfo();
	return lex;
}

/*
 * pg_parse_json
 *
 * Publicly visible entry point for the JSON parser.
 *
 * lex is a lexing context, set up for the json to be processed by calling
 * makeJsonLexContext(). sem is a strucure of function pointers to semantic
 * action routines to be called at appropriate spots during parsing, and a
 * pointer to a state object to be passed to those routines.
 */
void
pg_parse_json(JsonLexContext *lex, JsonSemAction *sem)
{
	JsonTokenType tok;

	/* get the initial token */
	json_lex(lex);

	tok = lex_peek(lex);

	/* parse by recursive descent */
	switch (tok)
	{
		case JSON_TOKEN_OBJECT_START:
			parse_object(lex, sem);
			break;
		case JSON_TOKEN_ARRAY_START:
			parse_array(lex, sem);
			break;
		default:
			parse_scalar(lex, sem);		/* json can be a bare scalar */
	}

	lex_expect(JSON_PARSE_END, lex, JSON_TOKEN_END);

}

/*
 *	Recursive Descent parse routines. There is one for each structural
 *	element in a json document:
 *	  - scalar (string, number, true, false, null)
 *	  - array  ( [ ] )
 *	  - array element
 *	  - object ( { } )
 *	  - object field
 */
static inline void
parse_scalar(JsonLexContext *lex, JsonSemAction *sem)
{
	char	   *val = NULL;
	json_scalar_action sfunc = sem->scalar;
	char	  **valaddr;
	JsonTokenType tok = lex_peek(lex);

	valaddr = sfunc == NULL ? NULL : &val;

	/* a scalar must be a string, a number, true, false, or null */
	switch (tok)
	{
		case JSON_TOKEN_TRUE:
			lex_accept(lex, JSON_TOKEN_TRUE, valaddr);
			break;
		case JSON_TOKEN_FALSE:
			lex_accept(lex, JSON_TOKEN_FALSE, valaddr);
			break;
		case JSON_TOKEN_NULL:
			lex_accept(lex, JSON_TOKEN_NULL, valaddr);
			break;
		case JSON_TOKEN_NUMBER:
			lex_accept(lex, JSON_TOKEN_NUMBER, valaddr);
			break;
		case JSON_TOKEN_STRING:
			lex_accept(lex, JSON_TOKEN_STRING, valaddr);
			break;
		default:
			report_parse_error(JSON_PARSE_VALUE, lex);
	}

	if (sfunc != NULL)
		(*sfunc) (sem->semstate, val, tok);
}

static void
parse_object_field(JsonLexContext *lex, JsonSemAction *sem)
{
	/*
	 * an object field is "fieldname" : value where value can be a scalar,
	 * object or array
	 */

	char	   *fname = NULL;	/* keep compiler quiet */
	json_ofield_action ostart = sem->object_field_start;
	json_ofield_action oend = sem->object_field_end;
	bool		isnull;
	char	  **fnameaddr = NULL;
	JsonTokenType tok;

	if (ostart != NULL || oend != NULL)
		fnameaddr = &fname;

	if (!lex_accept(lex, JSON_TOKEN_STRING, fnameaddr))
		report_parse_error(JSON_PARSE_STRING, lex);

	lex_expect(JSON_PARSE_OBJECT_LABEL, lex, JSON_TOKEN_COLON);

	tok = lex_peek(lex);
	isnull = tok == JSON_TOKEN_NULL;

	if (ostart != NULL)
		(*ostart) (sem->semstate, fname, isnull);

	switch (tok)
	{
		case JSON_TOKEN_OBJECT_START:
			parse_object(lex, sem);
			break;
		case JSON_TOKEN_ARRAY_START:
			parse_array(lex, sem);
			break;
		default:
			parse_scalar(lex, sem);
	}

	if (oend != NULL)
		(*oend) (sem->semstate, fname, isnull);

	if (fname != NULL)
		pfree(fname);
}

static void
parse_object(JsonLexContext *lex, JsonSemAction *sem)
{
	/*
	 * an object is a possibly empty sequence of object fields, separated by
	 * commas and surrounde by curly braces.
	 */
	json_struct_action ostart = sem->object_start;
	json_struct_action oend = sem->object_end;
	JsonTokenType tok;

	check_stack_depth();

	if (ostart != NULL)
		(*ostart) (sem->semstate);

	/*
	 * Data inside an object at at a higher nesting level than the object
	 * itself. Note that we increment this after we call the semantic routine
	 * for the object start and restore it before we call the routine for the
	 * object end.
	 */
	lex->lex_level++;

	/* we know this will succeeed, just clearing the token */
	lex_expect(JSON_PARSE_OBJECT_START, lex, JSON_TOKEN_OBJECT_START);

	tok = lex_peek(lex);
	switch (tok)
	{
		case JSON_TOKEN_STRING:
			parse_object_field(lex, sem);
			while (lex_accept(lex, JSON_TOKEN_COMMA, NULL))
				parse_object_field(lex, sem);
			break;
		case JSON_TOKEN_OBJECT_END:
			break;
		default:
			/* case of an invalid initial token inside the object */
			report_parse_error(JSON_PARSE_OBJECT_START, lex);
	}

	lex_expect(JSON_PARSE_OBJECT_NEXT, lex, JSON_TOKEN_OBJECT_END);

	lex->lex_level--;

	if (oend != NULL)
		(*oend) (sem->semstate);
}

static void
parse_array_element(JsonLexContext *lex, JsonSemAction *sem)
{
	json_aelem_action astart = sem->array_element_start;
	json_aelem_action aend = sem->array_element_end;
	JsonTokenType tok = lex_peek(lex);

	bool		isnull;

	isnull = tok == JSON_TOKEN_NULL;

	if (astart != NULL)
		(*astart) (sem->semstate, isnull);

	/* an array element is any object, array or scalar */
	switch (tok)
	{
		case JSON_TOKEN_OBJECT_START:
			parse_object(lex, sem);
			break;
		case JSON_TOKEN_ARRAY_START:
			parse_array(lex, sem);
			break;
		default:
			parse_scalar(lex, sem);
	}

	if (aend != NULL)
		(*aend) (sem->semstate, isnull);
}

static void
parse_array(JsonLexContext *lex, JsonSemAction *sem)
{
	/*
	 * an array is a possibly empty sequence of array elements, separated by
	 * commas and surrounded by square brackets.
	 */
	json_struct_action astart = sem->array_start;
	json_struct_action aend = sem->array_end;

	check_stack_depth();

	if (astart != NULL)
		(*astart) (sem->semstate);

	/*
	 * Data inside an array at at a higher nesting level than the array
	 * itself. Note that we increment this after we call the semantic routine
	 * for the array start and restore it before we call the routine for the
	 * array end.
	 */
	lex->lex_level++;

	lex_expect(JSON_PARSE_ARRAY_START, lex, JSON_TOKEN_ARRAY_START);
	if (lex_peek(lex) != JSON_TOKEN_ARRAY_END)
	{

		parse_array_element(lex, sem);

		while (lex_accept(lex, JSON_TOKEN_COMMA, NULL))
			parse_array_element(lex, sem);
	}

	lex_expect(JSON_PARSE_ARRAY_NEXT, lex, JSON_TOKEN_ARRAY_END);

	lex->lex_level--;

	if (aend != NULL)
		(*aend) (sem->semstate);
=======
 * Check whether supplied input is valid JSON.
 */
static void
json_validate_cstring(char *input)
{
	JsonLexContext lex;
	JsonParseStack *stack,
			   *stacktop;
	int			stacksize;

	/* Set up lexing context. */
	lex.input = input;
	lex.token_terminator = lex.input;

	/* Set up parse stack. */
	stacksize = 32;
	stacktop = (JsonParseStack *) palloc(sizeof(JsonParseStack) * stacksize);
	stack = stacktop;
	stack->state = JSON_PARSE_VALUE;

	/* Main parsing loop. */
	for (;;)
	{
		JsonStackOp op;

		/* Fetch next token. */
		json_lex(&lex);

		/* Check for unexpected end of input. */
		if (lex.token_start == NULL)
			report_parse_error(stack, &lex);

redo:
		/* Figure out what to do with this token. */
		op = JSON_STACKOP_NONE;
		switch (stack->state)
		{
			case JSON_PARSE_VALUE:
				if (lex.token_type != JSON_VALUE_INVALID)
					op = JSON_STACKOP_POP;
				else if (lex.token_start[0] == '[')
					stack->state = JSON_PARSE_ARRAY_START;
				else if (lex.token_start[0] == '{')
					stack->state = JSON_PARSE_OBJECT_START;
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_ARRAY_START:
				if (lex.token_type != JSON_VALUE_INVALID)
					stack->state = JSON_PARSE_ARRAY_NEXT;
				else if (lex.token_start[0] == ']')
					op = JSON_STACKOP_POP;
				else if (lex.token_start[0] == '[' ||
						 lex.token_start[0] == '{')
				{
					stack->state = JSON_PARSE_ARRAY_NEXT;
					op = JSON_STACKOP_PUSH_WITH_PUSHBACK;
				}
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_ARRAY_NEXT:
				if (lex.token_type != JSON_VALUE_INVALID)
					report_parse_error(stack, &lex);
				else if (lex.token_start[0] == ']')
					op = JSON_STACKOP_POP;
				else if (lex.token_start[0] == ',')
					op = JSON_STACKOP_PUSH;
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_OBJECT_START:
				if (lex.token_type == JSON_VALUE_STRING)
					stack->state = JSON_PARSE_OBJECT_LABEL;
				else if (lex.token_type == JSON_VALUE_INVALID &&
						 lex.token_start[0] == '}')
					op = JSON_STACKOP_POP;
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_OBJECT_LABEL:
				if (lex.token_type == JSON_VALUE_INVALID &&
					lex.token_start[0] == ':')
				{
					stack->state = JSON_PARSE_OBJECT_NEXT;
					op = JSON_STACKOP_PUSH;
				}
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_OBJECT_NEXT:
				if (lex.token_type != JSON_VALUE_INVALID)
					report_parse_error(stack, &lex);
				else if (lex.token_start[0] == '}')
					op = JSON_STACKOP_POP;
				else if (lex.token_start[0] == ',')
					stack->state = JSON_PARSE_OBJECT_COMMA;
				else
					report_parse_error(stack, &lex);
				break;
			case JSON_PARSE_OBJECT_COMMA:
				if (lex.token_type == JSON_VALUE_STRING)
					stack->state = JSON_PARSE_OBJECT_LABEL;
				else
					report_parse_error(stack, &lex);
				break;
			default:
				elog(ERROR, "unexpected json parse state: %d",
					 (int) stack->state);
		}

		/* Push or pop the state stack, if needed. */
		switch (op)
		{
			case JSON_STACKOP_PUSH:
			case JSON_STACKOP_PUSH_WITH_PUSHBACK:
				stack++;
				if (stack >= &stacktop[stacksize])
				{
					/* Need to enlarge the stack. */
					int			stackoffset = stack - stacktop;

					stacksize += 32;
					stacktop = (JsonParseStack *)
						repalloc(stacktop,
								 sizeof(JsonParseStack) * stacksize);
					stack = stacktop + stackoffset;
				}
				stack->state = JSON_PARSE_VALUE;
				if (op == JSON_STACKOP_PUSH_WITH_PUSHBACK)
					goto redo;
				break;
			case JSON_STACKOP_POP:
				if (stack == stacktop)
				{
					/* Expect end of input. */
					json_lex(&lex);
					if (lex.token_start != NULL)
						report_parse_error(NULL, &lex);
					return;
				}
				stack--;
				break;
			case JSON_STACKOP_NONE:
				/* nothing to do */
				break;
		}
	}
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
}

/*
 * Lex one token from the input stream.
 */
<<<<<<< HEAD
static inline void
json_lex(JsonLexContext *lex)
{
	char	   *s;
	int			len;

	/* Skip leading whitespace. */
	s = lex->token_terminator;
	len = s - lex->input;
	while (len < lex->input_length &&
		   (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r'))
	{
		if (*s == '\n')
			++lex->line_number;
		++s;
		++len;
	}
	lex->token_start = s;

	/* Determine token type. */
	if (len >= lex->input_length)
	{
		lex->token_start = NULL;
		lex->prev_token_terminator = lex->token_terminator;
		lex->token_terminator = s;
		lex->token_type = JSON_TOKEN_END;
	}
	else
		switch (*s)
		{
				/* Single-character token, some kind of punctuation mark. */
			case '{':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_OBJECT_START;
				break;
			case '}':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_OBJECT_END;
				break;
			case '[':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_ARRAY_START;
				break;
			case ']':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_ARRAY_END;
				break;
			case ',':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_COMMA;
				break;
			case ':':
				lex->prev_token_terminator = lex->token_terminator;
				lex->token_terminator = s + 1;
				lex->token_type = JSON_TOKEN_COLON;
				break;
			case '"':
				/* string */
				json_lex_string(lex);
				lex->token_type = JSON_TOKEN_STRING;
				break;
			case '-':
				/* Negative number. */
				json_lex_number(lex, s + 1, NULL, NULL);
				lex->token_type = JSON_TOKEN_NUMBER;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				/* Positive number. */
				json_lex_number(lex, s, NULL, NULL);
				lex->token_type = JSON_TOKEN_NUMBER;
				break;
			default:
				{
					char	   *p;

					/*
					 * We're not dealing with a string, number, legal
					 * punctuation mark, or end of string.  The only legal
					 * tokens we might find here are true, false, and null,
					 * but for error reporting purposes we scan until we see a
					 * non-alphanumeric character.  That way, we can report
					 * the whole word as an unexpected token, rather than just
					 * some unintuitive prefix thereof.
					 */
					for (p = s; p - s < lex->input_length - len && JSON_ALPHANUMERIC_CHAR(*p); p++)
						 /* skip */ ;

					/*
					 * We got some sort of unexpected punctuation or an
					 * otherwise unexpected character, so just complain about
					 * that one character.
					 */
					if (p == s)
					{
						lex->prev_token_terminator = lex->token_terminator;
						lex->token_terminator = s + 1;
						report_invalid_token(lex);
					}

					/*
					 * We've got a real alphanumeric token here.  If it
					 * happens to be true, false, or null, all is well.  If
					 * not, error out.
					 */
					lex->prev_token_terminator = lex->token_terminator;
					lex->token_terminator = p;
					if (p - s == 4)
					{
						if (memcmp(s, "true", 4) == 0)
							lex->token_type = JSON_TOKEN_TRUE;
						else if (memcmp(s, "null", 4) == 0)
							lex->token_type = JSON_TOKEN_NULL;
						else
							report_invalid_token(lex);
					}
					else if (p - s == 5 && memcmp(s, "false", 5) == 0)
						lex->token_type = JSON_TOKEN_FALSE;
					else
						report_invalid_token(lex);

				}
		}						/* end of switch */
=======
static void
json_lex(JsonLexContext *lex)
{
	char	   *s;

	/* Skip leading whitespace. */
	s = lex->token_terminator;
	while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
		s++;
	lex->token_start = s;

	/* Determine token type. */
	if (strchr("{}[],:", s[0]) != NULL)
	{
		/* strchr() is willing to match a zero byte, so test for that. */
		if (s[0] == '\0')
		{
			/* End of string. */
			lex->token_start = NULL;
			lex->token_terminator = s;
		}
		else
		{
			/* Single-character token, some kind of punctuation mark. */
			lex->token_terminator = s + 1;
		}
		lex->token_type = JSON_VALUE_INVALID;
	}
	else if (*s == '"')
	{
		/* String. */
		json_lex_string(lex);
		lex->token_type = JSON_VALUE_STRING;
	}
	else if (*s == '-')
	{
		/* Negative number. */
		json_lex_number(lex, s + 1);
		lex->token_type = JSON_VALUE_NUMBER;
	}
	else if (*s >= '0' && *s <= '9')
	{
		/* Positive number. */
		json_lex_number(lex, s);
		lex->token_type = JSON_VALUE_NUMBER;
	}
	else
	{
		char	   *p;

		/*
		 * We're not dealing with a string, number, legal punctuation mark, or
		 * end of string.  The only legal tokens we might find here are true,
		 * false, and null, but for error reporting purposes we scan until we
		 * see a non-alphanumeric character.  That way, we can report the
		 * whole word as an unexpected token, rather than just some
		 * unintuitive prefix thereof.
		 */
		for (p = s; JSON_ALPHANUMERIC_CHAR(*p); p++)
			/* skip */ ;

		if (p == s)
		{
			/*
			 * We got some sort of unexpected punctuation or an otherwise
			 * unexpected character, so just complain about that one
			 * character.  (It can't be multibyte because the above loop
			 * will advance over any multibyte characters.)
			 */
			lex->token_terminator = s + 1;
			report_invalid_token(lex);
		}

		/*
		 * We've got a real alphanumeric token here.  If it happens to be
		 * true, false, or null, all is well.  If not, error out.
		 */
		lex->token_terminator = p;
		if (p - s == 4)
		{
			if (memcmp(s, "true", 4) == 0)
				lex->token_type = JSON_VALUE_TRUE;
			else if (memcmp(s, "null", 4) == 0)
				lex->token_type = JSON_VALUE_NULL;
			else
				report_invalid_token(lex);
		}
		else if (p - s == 5 && memcmp(s, "false", 5) == 0)
			lex->token_type = JSON_VALUE_FALSE;
		else
			report_invalid_token(lex);
	}
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
}

/*
 * The next token in the input stream is known to be a string; lex it.
 */
<<<<<<< HEAD
static inline void
json_lex_string(JsonLexContext *lex)
{
	char	   *s;
	int			len;
	int			hi_surrogate = -1;

	if (lex->strval != NULL)
		resetStringInfo(lex->strval);

	Assert(lex->input_length > 0);
	s = lex->token_start;
	len = lex->token_start - lex->input;
	for (;;)
	{
		s++;
		len++;
		/* Premature end of the string. */
		if (len >= lex->input_length)
		{
			lex->token_terminator = s;
			report_invalid_token(lex);
		}
		else if (*s == '"')
			break;
		else if ((unsigned char) *s < 32)
		{
			/* Per RFC4627, these characters MUST be escaped. */
=======
static void
json_lex_string(JsonLexContext *lex)
{
	char	   *s;

	for (s = lex->token_start + 1; *s != '"'; s++)
	{
		/* Per RFC4627, these characters MUST be escaped. */
		if ((unsigned char) *s < 32)
		{
			/* A NUL byte marks the (premature) end of the string. */
			if (*s == '\0')
			{
				lex->token_terminator = s;
				report_invalid_token(lex);
			}
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			/* Since *s isn't printable, exclude it from the context string */
			lex->token_terminator = s;
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
					 errmsg("invalid input syntax for type json"),
					 errdetail("Character with value 0x%02x must be escaped.",
							   (unsigned char) *s),
					 report_json_context(lex)));
		}
		else if (*s == '\\')
		{
			/* OK, we have an escape character. */
			s++;
<<<<<<< HEAD
			len++;
			if (len >= lex->input_length)
=======
			if (*s == '\0')
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			{
				lex->token_terminator = s;
				report_invalid_token(lex);
			}
			else if (*s == 'u')
			{
				int			i;
				int			ch = 0;

				for (i = 1; i <= 4; i++)
				{
					s++;
<<<<<<< HEAD
					len++;
					if (len >= lex->input_length)
=======
					if (*s == '\0')
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
					{
						lex->token_terminator = s;
						report_invalid_token(lex);
					}
					else if (*s >= '0' && *s <= '9')
						ch = (ch * 16) + (*s - '0');
					else if (*s >= 'a' && *s <= 'f')
						ch = (ch * 16) + (*s - 'a') + 10;
					else if (*s >= 'A' && *s <= 'F')
						ch = (ch * 16) + (*s - 'A') + 10;
					else
					{
						lex->token_terminator = s + pg_mblen(s);
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								 errmsg("invalid input syntax for type json"),
								 errdetail("\"\\u\" must be followed by four hexadecimal digits."),
								 report_json_context(lex)));
					}
				}
<<<<<<< HEAD
				if (lex->strval != NULL)
				{
					char		utf8str[5];
					int			utf8len;

					if (ch >= 0xd800 && ch <= 0xdbff)
					{
						if (hi_surrogate != -1)
							ereport(ERROR,
							   (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								errmsg("invalid input syntax for type json"),
								errdetail("Unicode high surrogate must not follow a high surrogate."),
								report_json_context(lex)));
						hi_surrogate = (ch & 0x3ff) << 10;
						continue;
					}
					else if (ch >= 0xdc00 && ch <= 0xdfff)
					{
						if (hi_surrogate == -1)
							ereport(ERROR,
							   (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								errmsg("invalid input syntax for type json"),
								errdetail("Unicode low surrogate must follow a high surrogate."),
								report_json_context(lex)));
						ch = 0x10000 + hi_surrogate + (ch & 0x3ff);
						hi_surrogate = -1;
					}

					if (hi_surrogate != -1)
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								 errmsg("invalid input syntax for type json"),
								 errdetail("Unicode low surrogate must follow a high surrogate."),
								 report_json_context(lex)));

					/*
					 * For UTF8, replace the escape sequence by the actual utf8
					 * character in lex->strval. Do this also for other encodings
					 * if the escape designates an ASCII character, otherwise
					 * raise an error. We don't ever unescape a \u0000, since that
					 * would result in an impermissible nul byte.
					 */

					if (ch == 0)
					{
						appendStringInfoString(lex->strval, "\\u0000");
					}
					else if (GetDatabaseEncoding() == PG_UTF8)
					{
						unicode_to_utf8(ch, (unsigned char *) utf8str);
						utf8len = pg_utf_mblen((unsigned char *) utf8str);
						appendBinaryStringInfo(lex->strval, utf8str, utf8len);
					}
					else if (ch <= 0x007f)
					{
						/*
						 * This is the only way to designate things like a form feed
						 * character in JSON, so it's useful in all encodings.
						 */
						appendStringInfoChar(lex->strval, (char) ch);
					}
					else
					{
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								 errmsg("invalid input syntax for type json"),
								 errdetail("Unicode escape values cannot be used for code point values above 007F when the server encoding is not UTF8."),
								 report_json_context(lex)));
					}

				}
			}
			else if (lex->strval != NULL)
			{
				if (hi_surrogate != -1)
					ereport(ERROR,
							(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
							 errmsg("invalid input syntax for type json"),
							 errdetail("Unicode low surrogate must follow a high surrogate."),
							 report_json_context(lex)));

				switch (*s)
				{
					case '"':
					case '\\':
					case '/':
						appendStringInfoChar(lex->strval, *s);
						break;
					case 'b':
						appendStringInfoChar(lex->strval, '\b');
						break;
					case 'f':
						appendStringInfoChar(lex->strval, '\f');
						break;
					case 'n':
						appendStringInfoChar(lex->strval, '\n');
						break;
					case 'r':
						appendStringInfoChar(lex->strval, '\r');
						break;
					case 't':
						appendStringInfoChar(lex->strval, '\t');
						break;
					default:
						/* Not a valid string escape, so error out. */
						lex->token_terminator = s + pg_mblen(s);
						ereport(ERROR,
								(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
								 errmsg("invalid input syntax for type json"),
							errdetail("Escape sequence \"\\%s\" is invalid.",
									  extract_mb_char(s)),
								 report_json_context(lex)));
				}
			}
			else if (strchr("\"\\/bfnrt", *s) == NULL)
			{
				/*
				 * Simpler processing if we're not bothered about de-escaping
				 *
				 * It's very tempting to remove the strchr() call here and
				 * replace it with a switch statement, but testing so far has
				 * shown it's not a performance win.
				 */
=======
			}
			else if (strchr("\"\\/bfnrt", *s) == NULL)
			{
				/* Not a valid string escape, so error out. */
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
				lex->token_terminator = s + pg_mblen(s);
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Escape sequence \"\\%s\" is invalid.",
								   extract_mb_char(s)),
						 report_json_context(lex)));
			}
<<<<<<< HEAD

		}
		else if (lex->strval != NULL)
		{
			if (hi_surrogate != -1)
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Unicode low surrogate must follow a high surrogate."),
						 report_json_context(lex)));

			appendStringInfoChar(lex->strval, *s);
		}

	}

	if (hi_surrogate != -1)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type json"),
		errdetail("Unicode low surrogate must follow a high surrogate."),
				 report_json_context(lex)));

	/* Hooray, we found the end of the string! */
	lex->prev_token_terminator = lex->token_terminator;
	lex->token_terminator = s + 1;
}

/*
=======
		}
	}

	/* Hooray, we found the end of the string! */
	lex->token_terminator = s + 1;
}

/*-------------------------------------------------------------------------
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 * The next token in the input stream is known to be a number; lex it.
 *
 * In JSON, a number consists of four parts:
 *
 * (1) An optional minus sign ('-').
 *
 * (2) Either a single '0', or a string of one or more digits that does not
 *	   begin with a '0'.
 *
 * (3) An optional decimal part, consisting of a period ('.') followed by
<<<<<<< HEAD
 *	   one or more digits.  (Note: While this part can be omitted
=======
 *	   one or more digits.	(Note: While this part can be omitted
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 *	   completely, it's not OK to have only the decimal point without
 *	   any digits afterwards.)
 *
 * (4) An optional exponent part, consisting of 'e' or 'E', optionally
<<<<<<< HEAD
 *	   followed by '+' or '-', followed by one or more digits.  (Note:
=======
 *	   followed by '+' or '-', followed by one or more digits.	(Note:
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
 *	   As with the decimal part, if 'e' or 'E' is present, it must be
 *	   followed by at least one digit.)
 *
 * The 's' argument to this function points to the ostensible beginning
<<<<<<< HEAD
 * of part 2 - i.e. the character after any optional minus sign, or the
 * first character of the string if there is none.
 *
 * If num_err is not NULL, we return an error flag to *num_err rather than
 * raising an error for a badly-formed number.  Also, if total_len is not NULL
 * the distance from lex->input to the token end+1 is returned to *total_len.
 */
static inline void
json_lex_number(JsonLexContext *lex, char *s,
				bool *num_err, int *total_len)
{
	bool		error = false;
	int			len = s - lex->input;
=======
 * of part 2 - i.e. the character after any optional minus sign, and the
 * first character of the string if there is none.
 *
 *-------------------------------------------------------------------------
 */
static void
json_lex_number(JsonLexContext *lex, char *s)
{
	bool		error = false;
	char	   *p;
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	/* Part (1): leading sign indicator. */
	/* Caller already did this for us; so do nothing. */

	/* Part (2): parse main digit string. */
<<<<<<< HEAD
	if (len < lex->input_length && *s == '0')
	{
		s++;
		len++;
	}
	else if (len < lex->input_length && *s >= '1' && *s <= '9')
=======
	if (*s == '0')
		s++;
	else if (*s >= '1' && *s <= '9')
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
	{
		do
		{
			s++;
<<<<<<< HEAD
			len++;
		} while (len < lex->input_length && *s >= '0' && *s <= '9');
=======
		} while (*s >= '0' && *s <= '9');
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
	}
	else
		error = true;

	/* Part (3): parse optional decimal portion. */
<<<<<<< HEAD
	if (len < lex->input_length && *s == '.')
	{
		s++;
		len++;
		if (len == lex->input_length || *s < '0' || *s > '9')
=======
	if (*s == '.')
	{
		s++;
		if (*s < '0' || *s > '9')
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			error = true;
		else
		{
			do
			{
				s++;
<<<<<<< HEAD
				len++;
			} while (len < lex->input_length && *s >= '0' && *s <= '9');
=======
			} while (*s >= '0' && *s <= '9');
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		}
	}

	/* Part (4): parse optional exponent. */
<<<<<<< HEAD
	if (len < lex->input_length && (*s == 'e' || *s == 'E'))
	{
		s++;
		len++;
		if (len < lex->input_length && (*s == '+' || *s == '-'))
		{
			s++;
			len++;
		}
		if (len == lex->input_length || *s < '0' || *s > '9')
=======
	if (*s == 'e' || *s == 'E')
	{
		s++;
		if (*s == '+' || *s == '-')
			s++;
		if (*s < '0' || *s > '9')
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			error = true;
		else
		{
			do
			{
				s++;
<<<<<<< HEAD
				len++;
			} while (len < lex->input_length && *s >= '0' && *s <= '9');
=======
			} while (*s >= '0' && *s <= '9');
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		}
	}

	/*
	 * Check for trailing garbage.  As in json_lex(), any alphanumeric stuff
	 * here should be considered part of the token for error-reporting
	 * purposes.
	 */
<<<<<<< HEAD
	for (; len < lex->input_length && JSON_ALPHANUMERIC_CHAR(*s); s++, len++)
		error = true;

	if (total_len != NULL)
		*total_len = len;

	if (num_err != NULL)
	{
		/* let the caller handle any error */
		*num_err = error;
	}
	else
	{
		/* return token endpoint */
		lex->prev_token_terminator = lex->token_terminator;
		lex->token_terminator = s;
		/* handle error if any */
		if (error)
			report_invalid_token(lex);
	}
=======
	for (p = s; JSON_ALPHANUMERIC_CHAR(*p); p++)
		error = true;
	lex->token_terminator = p;
	if (error)
		report_invalid_token(lex);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
}

/*
 * Report a parse error.
 *
 * lex->token_start and lex->token_terminator must identify the current token.
 */
static void
<<<<<<< HEAD
report_parse_error(JsonParseContext ctx, JsonLexContext *lex)
=======
report_parse_error(JsonParseStack *stack, JsonLexContext *lex)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
{
	char	   *token;
	int			toklen;

	/* Handle case where the input ended prematurely. */
<<<<<<< HEAD
	if (lex->token_start == NULL || lex->token_type == JSON_TOKEN_END)
=======
	if (lex->token_start == NULL)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type json"),
				 errdetail("The input string ended unexpectedly."),
				 report_json_context(lex)));

	/* Separate out the current token. */
	toklen = lex->token_terminator - lex->token_start;
	token = palloc(toklen + 1);
	memcpy(token, lex->token_start, toklen);
	token[toklen] = '\0';

	/* Complain, with the appropriate detail message. */
<<<<<<< HEAD
	if (ctx == JSON_PARSE_END)
=======
	if (stack == NULL)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("invalid input syntax for type json"),
				 errdetail("Expected end of input, but found \"%s\".",
						   token),
				 report_json_context(lex)));
	else
	{
<<<<<<< HEAD
		switch (ctx)
=======
		switch (stack->state)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		{
			case JSON_PARSE_VALUE:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Expected JSON value, but found \"%s\".",
								   token),
						 report_json_context(lex)));
				break;
<<<<<<< HEAD
			case JSON_PARSE_STRING:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Expected string, but found \"%s\".",
								   token),
						 report_json_context(lex)));
				break;
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			case JSON_PARSE_ARRAY_START:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Expected array element or \"]\", but found \"%s\".",
								   token),
						 report_json_context(lex)));
				break;
			case JSON_PARSE_ARRAY_NEXT:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
<<<<<<< HEAD
					  errdetail("Expected \",\" or \"]\", but found \"%s\".",
								token),
=======
						 errdetail("Expected \",\" or \"]\", but found \"%s\".",
								   token),
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
						 report_json_context(lex)));
				break;
			case JSON_PARSE_OBJECT_START:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
<<<<<<< HEAD
					 errdetail("Expected string or \"}\", but found \"%s\".",
							   token),
=======
						 errdetail("Expected string or \"}\", but found \"%s\".",
								   token),
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
						 report_json_context(lex)));
				break;
			case JSON_PARSE_OBJECT_LABEL:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Expected \":\", but found \"%s\".",
								   token),
						 report_json_context(lex)));
				break;
			case JSON_PARSE_OBJECT_NEXT:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
<<<<<<< HEAD
					  errdetail("Expected \",\" or \"}\", but found \"%s\".",
								token),
=======
						 errdetail("Expected \",\" or \"}\", but found \"%s\".",
								   token),
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
						 report_json_context(lex)));
				break;
			case JSON_PARSE_OBJECT_COMMA:
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
						 errmsg("invalid input syntax for type json"),
						 errdetail("Expected string, but found \"%s\".",
								   token),
						 report_json_context(lex)));
				break;
			default:
<<<<<<< HEAD
				elog(ERROR, "unexpected json parse state: %d", ctx);
=======
				elog(ERROR, "unexpected json parse state: %d",
					 (int) stack->state);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		}
	}
}

/*
 * Report an invalid input token.
 *
 * lex->token_start and lex->token_terminator must identify the token.
 */
static void
report_invalid_token(JsonLexContext *lex)
{
	char	   *token;
	int			toklen;

	/* Separate out the offending token. */
	toklen = lex->token_terminator - lex->token_start;
	token = palloc(toklen + 1);
	memcpy(token, lex->token_start, toklen);
	token[toklen] = '\0';

	ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("invalid input syntax for type json"),
			 errdetail("Token \"%s\" is invalid.", token),
			 report_json_context(lex)));
}

/*
 * Report a CONTEXT line for bogus JSON input.
 *
 * lex->token_terminator must be set to identify the spot where we detected
 * the error.  Note that lex->token_start might be NULL, in case we recognized
 * error at EOF.
 *
 * The return value isn't meaningful, but we make it non-void so that this
 * can be invoked inside ereport().
 */
static int
report_json_context(JsonLexContext *lex)
{
	const char *context_start;
	const char *context_end;
	const char *line_start;
	int			line_number;
	char	   *ctxt;
	int			ctxtlen;
	const char *prefix;
	const char *suffix;

	/* Choose boundaries for the part of the input we will display */
	context_start = lex->input;
	context_end = lex->token_terminator;
	line_start = context_start;
	line_number = 1;
	for (;;)
	{
<<<<<<< HEAD
		/* Always advance over newlines */
		if (context_start < context_end && *context_start == '\n')
=======
		/* Always advance over newlines (context_end test is just paranoia) */
		if (*context_start == '\n' && context_start < context_end)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		{
			context_start++;
			line_start = context_start;
			line_number++;
			continue;
		}
		/* Otherwise, done as soon as we are close enough to context_end */
		if (context_end - context_start < 50)
			break;
		/* Advance to next multibyte character */
		if (IS_HIGHBIT_SET(*context_start))
			context_start += pg_mblen(context_start);
		else
			context_start++;
	}

	/*
	 * We add "..." to indicate that the excerpt doesn't start at the
	 * beginning of the line ... but if we're within 3 characters of the
	 * beginning of the line, we might as well just show the whole line.
	 */
	if (context_start - line_start <= 3)
		context_start = line_start;

	/* Get a null-terminated copy of the data to present */
	ctxtlen = context_end - context_start;
	ctxt = palloc(ctxtlen + 1);
	memcpy(ctxt, context_start, ctxtlen);
	ctxt[ctxtlen] = '\0';

	/*
	 * Show the context, prefixing "..." if not starting at start of line, and
	 * suffixing "..." if not ending at end of line.
	 */
	prefix = (context_start > line_start) ? "..." : "";
<<<<<<< HEAD
	suffix = (lex->token_type != JSON_TOKEN_END && context_end - lex->input < lex->input_length && *context_end != '\n' && *context_end != '\r') ? "..." : "";
=======
	suffix = (*context_end != '\0' && *context_end != '\n' && *context_end != '\r') ? "..." : "";
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	return errcontext("JSON data, line %d: %s%s%s",
					  line_number, prefix, ctxt, suffix);
}

/*
 * Extract a single, possibly multi-byte char from the input string.
 */
static char *
extract_mb_char(char *s)
{
	char	   *res;
	int			len;

	len = pg_mblen(s);
	res = palloc(len + 1);
	memcpy(res, s, len);
	res[len] = '\0';

	return res;
}

/*
<<<<<<< HEAD
 * Determine how we want to print values of a given type in datum_to_json.
 *
 * Given the datatype OID, return its JsonTypeCategory, as well as the type's
 * output function OID.  If the returned category is JSONTYPE_CAST, we
 * return the OID of the type->JSON cast function instead.
 */
static void
json_categorize_type(Oid typoid,
					 JsonTypeCategory *tcategory,
					 Oid *outfuncoid)
{
	bool		typisvarlena;

	/* Look through any domain */
	typoid = getBaseType(typoid);

	/* We'll usually need to return the type output function */
	getTypeOutputInfo(typoid, outfuncoid, &typisvarlena);

	/* Check for known types */
	switch (typoid)
	{
		case BOOLOID:
			*tcategory = JSONTYPE_BOOL;
			break;

		case INT2OID:
		case INT4OID:
		case INT8OID:
		case FLOAT4OID:
		case FLOAT8OID:
		case NUMERICOID:
			*tcategory = JSONTYPE_NUMERIC;
			break;

		case JSONOID:
			*tcategory = JSONTYPE_JSON;
			break;

		default:
			/* Check for arrays and composites */
			if (OidIsValid(get_element_type(typoid)))
				*tcategory = JSONTYPE_ARRAY;
			else if (type_is_rowtype(typoid))
				*tcategory = JSONTYPE_COMPOSITE;
			else
			{
				/* It's probably the general case ... */
				*tcategory = JSONTYPE_OTHER;
				/* but let's look for a cast to json, if it's not built-in */
				if (typoid >= FirstNormalObjectId)
				{
					HeapTuple	tuple;

					tuple = SearchSysCache2(CASTSOURCETARGET,
											ObjectIdGetDatum(typoid),
											ObjectIdGetDatum(JSONOID));
					if (HeapTupleIsValid(tuple))
					{
						Form_pg_cast castForm = (Form_pg_cast) GETSTRUCT(tuple);

						if (castForm->castmethod == COERCION_METHOD_FUNCTION)
						{
							*tcategory = JSONTYPE_CAST;
							*outfuncoid = castForm->castfunc;
						}

						ReleaseSysCache(tuple);
					}
				}
			}
			break;
	}
}

/*
 * Turn a Datum into JSON text, appending the string to "result".
 *
 * tcategory and outfuncoid are from a previous call to json_categorize_type,
 * except that if is_null is true then they can be invalid.
 */
static void
datum_to_json(Datum val, bool is_null, StringInfo result,
			  JsonTypeCategory tcategory, Oid outfuncoid)
{
	char	   *outputstr;
	text	   *jsontext;

	check_stack_depth();
=======
 * Turn a scalar Datum into JSON, appending the string to "result".
 *
 * Hand off a non-scalar datum to composite_to_json or array_to_json_internal
 * as appropriate.
 */
static void
datum_to_json(Datum val, bool is_null, StringInfo result,
			  TYPCATEGORY tcategory, Oid typoutputfunc)
{
	char	   *outputstr;
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	if (is_null)
	{
		appendStringInfoString(result, "null");
		return;
	}

	switch (tcategory)
	{
<<<<<<< HEAD
		case JSONTYPE_ARRAY:
			array_to_json_internal(val, result, false);
			break;
		case JSONTYPE_COMPOSITE:
			composite_to_json(val, result, false);
			break;
		case JSONTYPE_BOOL:
=======
		case TYPCATEGORY_ARRAY:
			array_to_json_internal(val, result, false);
			break;
		case TYPCATEGORY_COMPOSITE:
			composite_to_json(val, result, false);
			break;
		case TYPCATEGORY_BOOLEAN:
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			if (DatumGetBool(val))
				appendStringInfoString(result, "true");
			else
				appendStringInfoString(result, "false");
			break;
<<<<<<< HEAD
		case JSONTYPE_NUMERIC:
			outputstr = OidOutputFunctionCall(outfuncoid, val);

			/*
			 * Don't call escape_json for a non-key if it's a valid JSON
			 * number.
			 */
			if (IsValidJsonNumber(outputstr, strlen(outputstr)))
=======
		case TYPCATEGORY_NUMERIC:
			outputstr = OidOutputFunctionCall(typoutputfunc, val);

			/*
			 * Don't call escape_json here if it's a valid JSON number.
			 * Numeric output should usually be a valid JSON number and JSON
			 * numbers shouldn't be quoted. Quote cases like "Nan" and
			 * "Infinity", however.
			 */
			if (strpbrk(outputstr, NON_NUMERIC_LETTER) == NULL)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
				appendStringInfoString(result, outputstr);
			else
				escape_json(result, outputstr);
			pfree(outputstr);
			break;
<<<<<<< HEAD
		case JSONTYPE_JSON:
			/* JSON will already be escaped */
			outputstr = OidOutputFunctionCall(outfuncoid, val);
			appendStringInfoString(result, outputstr);
			pfree(outputstr);
			break;
		case JSONTYPE_CAST:
			/* outfuncoid refers to a cast function, not an output function */
			jsontext = DatumGetTextP(OidFunctionCall1(outfuncoid, val));
			outputstr = text_to_cstring(jsontext);
			appendStringInfoString(result, outputstr);
			pfree(outputstr);
			pfree(jsontext);
			break;
		default:
			outputstr = OidOutputFunctionCall(outfuncoid, val);
=======
		case TYPCATEGORY_JSON:
			/* JSON will already be escaped */
			outputstr = OidOutputFunctionCall(typoutputfunc, val);
			appendStringInfoString(result, outputstr);
			pfree(outputstr);
			break;
		default:
			outputstr = OidOutputFunctionCall(typoutputfunc, val);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			escape_json(result, outputstr);
			pfree(outputstr);
			break;
	}
}

/*
 * Process a single dimension of an array.
 * If it's the innermost dimension, output the values, otherwise call
 * ourselves recursively to process the next dimension.
 */
static void
array_dim_to_json(StringInfo result, int dim, int ndims, int *dims, Datum *vals,
<<<<<<< HEAD
				  bool *nulls, int *valcount, JsonTypeCategory tcategory,
				  Oid outfuncoid, bool use_line_feeds)
=======
				  bool *nulls, int *valcount, TYPCATEGORY tcategory,
				  Oid typoutputfunc, bool use_line_feeds)
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
{
	int			i;
	const char *sep;

	Assert(dim < ndims);

	sep = use_line_feeds ? ",\n " : ",";

	appendStringInfoChar(result, '[');

	for (i = 1; i <= dims[dim]; i++)
	{
		if (i > 1)
			appendStringInfoString(result, sep);

		if (dim + 1 == ndims)
		{
			datum_to_json(vals[*valcount], nulls[*valcount], result, tcategory,
<<<<<<< HEAD
						  outfuncoid);
=======
						  typoutputfunc);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
			(*valcount)++;
		}
		else
		{
			/*
			 * Do we want line feeds on inner dimensions of arrays? For now
			 * we'll say no.
			 */
			array_dim_to_json(result, dim + 1, ndims, dims, vals, nulls,
<<<<<<< HEAD
							  valcount, tcategory, outfuncoid, false);
=======
							  valcount, tcategory, typoutputfunc, false);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
		}
	}

	appendStringInfoChar(result, ']');
}

/*
 * Turn an array into JSON.
 */
static void
array_to_json_internal(Datum array, StringInfo result, bool use_line_feeds)
{
	ArrayType  *v = DatumGetArrayTypeP(array);
	Oid			element_type = ARR_ELEMTYPE(v);
	int		   *dim;
	int			ndim;
	int			nitems;
	int			count = 0;
	Datum	   *elements;
	bool	   *nulls;
	int16		typlen;
	bool		typbyval;
<<<<<<< HEAD
	char		typalign;
	JsonTypeCategory tcategory;
	Oid			outfuncoid;
=======
	char		typalign,
				typdelim;
	Oid			typioparam;
	Oid			typoutputfunc;
	TYPCATEGORY tcategory;
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	ndim = ARR_NDIM(v);
	dim = ARR_DIMS(v);
	nitems = ArrayGetNItems(ndim, dim);

	if (nitems <= 0)
	{
		appendStringInfoString(result, "[]");
		return;
	}

<<<<<<< HEAD
	get_typlenbyvalalign(element_type,
						 &typlen, &typbyval, &typalign);

	json_categorize_type(element_type,
						 &tcategory, &outfuncoid);
=======
	get_type_io_data(element_type, IOFunc_output,
					 &typlen, &typbyval, &typalign,
					 &typdelim, &typioparam, &typoutputfunc);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	deconstruct_array(v, element_type, typlen, typbyval,
					  typalign, &elements, &nulls,
					  &nitems);

<<<<<<< HEAD
	array_dim_to_json(result, 0, ndim, dim, elements, nulls, &count, tcategory,
					  outfuncoid, use_line_feeds);
=======
	if (element_type == RECORDOID)
		tcategory = TYPCATEGORY_COMPOSITE;
	else if (element_type == JSONOID)
		tcategory = TYPCATEGORY_JSON;
	else
		tcategory = TypeCategory(element_type);

	array_dim_to_json(result, 0, ndim, dim, elements, nulls, &count, tcategory,
					  typoutputfunc, use_line_feeds);
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

	pfree(elements);
	pfree(nulls);
}

/*
 * Turn a composite / record into JSON.
 */
static void
composite_to_json(Datum composite, StringInfo result, bool use_line_feeds)
{
	HeapTupleHeader td;
	Oid			tupType;
	int32		tupTypmod;
	TupleDesc	tupdesc;
	HeapTupleData tmptup,
			   *tuple;
	int			i;
	bool		needsep = false;
	const char *sep;

	sep = use_line_feeds ? ",\n " : ",";

	td = DatumGetHeapTupleHeader(composite);

	/* Extract rowtype info and find a tupdesc */
	tupType = HeapTupleHeaderGetTypeId(td);
	tupTypmod = HeapTupleHeaderGetTypMod(td);
	tupdesc = lookup_rowtype_tupdesc(tupType, tupTypmod);

	/* Build a temporary HeapTuple control structure */
	tmptup.t_len = HeapTupleHeaderGetDatumLength(td);
	tmptup.t_data = td;
	tuple = &tmptup;

	appendStringInfoChar(result, '{');

	for (i = 0; i < tupdesc->natts; i++)
	{
<<<<<<< HEAD
		Datum		val;
		bool		isnull;
		char	   *attname;
		JsonTypeCategory tcategory;
		Oid			outfuncoid;
=======
		Datum		val,
					origval;
		bool		isnull;
		char	   *attname;
		TYPCATEGORY tcategory;
		Oid			typoutput;
		bool		typisvarlena;
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

		if (tupdesc->attrs[i]->attisdropped)
			continue;

		if (needsep)
			appendStringInfoString(result, sep);
		needsep = true;

		attname = NameStr(tupdesc->attrs[i]->attname);
		escape_json(result, attname);
		appendStringInfoChar(result, ':');

<<<<<<< HEAD
		val = heap_getattr(tuple, i + 1, tupdesc, &isnull);

		if (isnull)
		{
			tcategory = JSONTYPE_NULL;
			outfuncoid = InvalidOid;
		}
		else
			json_categorize_type(tupdesc->attrs[i]->atttypid,
								 &tcategory, &outfuncoid);

		datum_to_json(val, isnull, result, tcategory, outfuncoid);
=======
		origval = heap_getattr(tuple, i + 1, tupdesc, &isnull);

		if (tupdesc->attrs[i]->atttypid == RECORDARRAYOID)
			tcategory = TYPCATEGORY_ARRAY;
		else if (tupdesc->attrs[i]->atttypid == RECORDOID)
			tcategory = TYPCATEGORY_COMPOSITE;
		else if (tupdesc->attrs[i]->atttypid == JSONOID)
			tcategory = TYPCATEGORY_JSON;
		else
			tcategory = TypeCategory(tupdesc->attrs[i]->atttypid);

		getTypeOutputInfo(tupdesc->attrs[i]->atttypid,
						  &typoutput, &typisvarlena);

		/*
		 * If we have a toasted datum, forcibly detoast it here to avoid
		 * memory leakage inside the type's output routine.
		 */
		if (typisvarlena && !isnull)
			val = PointerGetDatum(PG_DETOAST_DATUM(origval));
		else
			val = origval;

		datum_to_json(val, isnull, result, tcategory, typoutput);

		/* Clean up detoasted copy, if any */
		if (val != origval)
			pfree(DatumGetPointer(val));
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
	}

	appendStringInfoChar(result, '}');
	ReleaseTupleDesc(tupdesc);
}

/*
 * SQL function array_to_json(row)
 */
extern Datum
array_to_json(PG_FUNCTION_ARGS)
{
	Datum		array = PG_GETARG_DATUM(0);
	StringInfo	result;

	result = makeStringInfo();

	array_to_json_internal(array, result, false);

	PG_RETURN_TEXT_P(cstring_to_text(result->data));
<<<<<<< HEAD
}
=======
};
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/*
 * SQL function array_to_json(row, prettybool)
 */
extern Datum
array_to_json_pretty(PG_FUNCTION_ARGS)
{
	Datum		array = PG_GETARG_DATUM(0);
	bool		use_line_feeds = PG_GETARG_BOOL(1);
	StringInfo	result;

	result = makeStringInfo();

	array_to_json_internal(array, result, use_line_feeds);

	PG_RETURN_TEXT_P(cstring_to_text(result->data));
<<<<<<< HEAD
}
=======
};
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/*
 * SQL function row_to_json(row)
 */
extern Datum
row_to_json(PG_FUNCTION_ARGS)
{
	Datum		array = PG_GETARG_DATUM(0);
	StringInfo	result;

	result = makeStringInfo();

	composite_to_json(array, result, false);

	PG_RETURN_TEXT_P(cstring_to_text(result->data));
<<<<<<< HEAD
}
=======
};
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/*
 * SQL function row_to_json(row, prettybool)
 */
extern Datum
row_to_json_pretty(PG_FUNCTION_ARGS)
{
	Datum		array = PG_GETARG_DATUM(0);
	bool		use_line_feeds = PG_GETARG_BOOL(1);
	StringInfo	result;

	result = makeStringInfo();

	composite_to_json(array, result, use_line_feeds);

	PG_RETURN_TEXT_P(cstring_to_text(result->data));
<<<<<<< HEAD
}

/*
 * SQL function to_json(anyvalue)
 */
Datum
to_json(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	Oid			val_type = get_fn_expr_argtype(fcinfo->flinfo, 0);
	StringInfo	result;
	JsonTypeCategory tcategory;
	Oid			outfuncoid;

	if (val_type == InvalidOid)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("could not determine input data type")));

	json_categorize_type(val_type,
						 &tcategory, &outfuncoid);

	result = makeStringInfo();

	datum_to_json(val, false, result, tcategory, outfuncoid);

	PG_RETURN_TEXT_P(cstring_to_text(result->data));
}

/*
 * json_agg transition function
 *
 * aggregate input column as a json array value.
 */
Datum
json_agg_transfn(PG_FUNCTION_ARGS)
{
	Oid			val_type = get_fn_expr_argtype(fcinfo->flinfo, 1);
	MemoryContext aggcontext,
				oldcontext;
	StringInfo	state;
	Datum		val;
	JsonTypeCategory tcategory;
	Oid			outfuncoid;

	if (val_type == InvalidOid)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("could not determine input data type")));

	if (!AggCheckCallContext(fcinfo, &aggcontext))
	{
		/* cannot be called directly because of internal-type argument */
		elog(ERROR, "json_agg_transfn called in non-aggregate context");
	}

	if (PG_ARGISNULL(0))
	{
		/*
		 * Make this StringInfo in a context where it will persist for the
		 * duration of the aggregate call.  MemoryContextSwitchTo is only
		 * needed the first time, as the StringInfo routines make sure they
		 * use the right context to enlarge the object if necessary.
		 */
		oldcontext = MemoryContextSwitchTo(aggcontext);
		state = makeStringInfo();
		MemoryContextSwitchTo(oldcontext);

		appendStringInfoChar(state, '[');
	}
	else
	{
		state = (StringInfo) PG_GETARG_POINTER(0);
		appendStringInfoString(state, ", ");
	}

	/* fast path for NULLs */
	if (PG_ARGISNULL(1))
	{
		datum_to_json((Datum) 0, true, state, JSONTYPE_NULL, InvalidOid);
		PG_RETURN_POINTER(state);
	}

	val = PG_GETARG_DATUM(1);

	/* XXX we do this every time?? */
	json_categorize_type(val_type,
						 &tcategory, &outfuncoid);

	/* add some whitespace if structured type and not first item */
	if (!PG_ARGISNULL(0) &&
		(tcategory == JSONTYPE_ARRAY || tcategory == JSONTYPE_COMPOSITE))
	{
		appendStringInfoString(state, "\n ");
	}

	datum_to_json(val, false, state, tcategory, outfuncoid);

	/*
	 * The transition type for array_agg() is declared to be "internal", which
	 * is a pass-by-value type the same size as a pointer.  So we can safely
	 * pass the ArrayBuildState pointer through nodeAgg.c's machinations.
	 */
	PG_RETURN_POINTER(state);
}

/*
 * json_agg final function
 */
Datum
json_agg_finalfn(PG_FUNCTION_ARGS)
{
	StringInfo	state;

	/* cannot be called directly because of internal-type argument */
	Assert(AggCheckCallContext(fcinfo, NULL));

	state = PG_ARGISNULL(0) ? NULL : (StringInfo) PG_GETARG_POINTER(0);

	/* NULL result for no rows in, as is standard with aggregates */
	if (state == NULL)
		PG_RETURN_NULL();

	/* Else return state with appropriate array terminator added */
	PG_RETURN_TEXT_P(catenate_stringinfo_string(state, "]"));
}

/*
 * Helper function for aggregates: return given StringInfo's contents plus
 * specified trailing string, as a text datum.  We need this because aggregate
 * final functions are not allowed to modify the aggregate state.
 */
static text *
catenate_stringinfo_string(StringInfo buffer, const char *addon)
{
	/* custom version of cstring_to_text_with_len */
	int			buflen = buffer->len;
	int			addlen = strlen(addon);
	text	   *result = (text *) palloc(buflen + addlen + VARHDRSZ);

	SET_VARSIZE(result, buflen + addlen + VARHDRSZ);
	memcpy(VARDATA(result), buffer->data, buflen);
	memcpy(VARDATA(result) + buflen, addon, addlen);

	return result;
}
=======
};
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56

/*
 * Produce a JSON string literal, properly escaping characters in the text.
 */
void
escape_json(StringInfo buf, const char *str)
{
	const char *p;

	appendStringInfoCharMacro(buf, '\"');
	for (p = str; *p; p++)
	{
		switch (*p)
		{
			case '\b':
				appendStringInfoString(buf, "\\b");
				break;
			case '\f':
				appendStringInfoString(buf, "\\f");
				break;
			case '\n':
				appendStringInfoString(buf, "\\n");
				break;
			case '\r':
				appendStringInfoString(buf, "\\r");
				break;
			case '\t':
				appendStringInfoString(buf, "\\t");
				break;
			case '"':
				appendStringInfoString(buf, "\\\"");
				break;
			case '\\':
				appendStringInfoString(buf, "\\\\");
				break;
			default:
				if ((unsigned char) *p < ' ')
					appendStringInfo(buf, "\\u%04x", (int) *p);
				else
					appendStringInfoCharMacro(buf, *p);
				break;
		}
	}
	appendStringInfoCharMacro(buf, '\"');
}
