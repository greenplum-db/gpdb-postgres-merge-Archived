%{
/*-------------------------------------------------------------------------
 *
 * specparse.y
 *	  bison grammar for the isolation test file format
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *-------------------------------------------------------------------------
 */

#include "postgres_fe.h"

#include "isolationtester.h"


TestSpec		parseresult;			/* result of parsing is left here */

%}

%expect 0
%name-prefix="spec_yy"

%union
{
	char	   *str;
	Session	   *session;
	Step	   *step;
	Permutation *permutation;
	struct
	{
		void  **elements;
		int		nelements;
	}			ptr_list;
}

%type <str>  opt_setup opt_teardown
%type <ptr_list> step_list session_list permutation_list opt_permutation_list
<<<<<<< HEAD
%type <ptr_list> string_literal_list
=======
%type <ptr_list> string_list
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
%type <session> session
%type <step> step
%type <permutation> permutation

<<<<<<< HEAD
%token <str> sqlblock string_literal
=======
%token <str> sqlblock string
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
%token PERMUTATION SESSION SETUP STEP TEARDOWN TEST

%%

TestSpec:
			opt_setup
			opt_teardown
			session_list
			opt_permutation_list
			{
				parseresult.setupsql = $1;
				parseresult.teardownsql = $2;
				parseresult.sessions = (Session **) $3.elements;
				parseresult.nsessions = $3.nelements;
				parseresult.permutations = (Permutation **) $4.elements;
				parseresult.npermutations = $4.nelements;
			}
		;

opt_setup:
			/* EMPTY */			{ $$ = NULL; }
			| SETUP sqlblock	{ $$ = $2; }
		;

opt_teardown:
			/* EMPTY */			{ $$ = NULL; }
			| TEARDOWN sqlblock	{ $$ = $2; }
		;

session_list:
			session_list session
			{
				$$.elements = realloc($1.elements,
									  ($1.nelements + 1) * sizeof(void *));
				$$.elements[$1.nelements] = $2;
				$$.nelements = $1.nelements + 1;
			}
			| session
			{
				$$.nelements = 1;
				$$.elements = malloc(sizeof(void *));
				$$.elements[0] = $1;
			}
		;

session:
<<<<<<< HEAD
			SESSION string_literal opt_setup step_list opt_teardown
=======
			SESSION string opt_setup step_list opt_teardown
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			{
				$$ = malloc(sizeof(Session));
				$$->name = $2;
				$$->setupsql = $3;
				$$->steps = (Step **) $4.elements;
				$$->nsteps = $4.nelements;
				$$->teardownsql = $5;
			}
		;

step_list:
			step_list step
			{
				$$.elements = realloc($1.elements,
									  ($1.nelements + 1) * sizeof(void *));
				$$.elements[$1.nelements] = $2;
				$$.nelements = $1.nelements + 1;
			}
			| step
			{
				$$.nelements = 1;
				$$.elements = malloc(sizeof(void *));
				$$.elements[0] = $1;
			}
		;


step:
<<<<<<< HEAD
			STEP string_literal sqlblock
=======
			STEP string sqlblock
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			{
				$$ = malloc(sizeof(Step));
				$$->name = $2;
				$$->sql = $3;
			}
		;


opt_permutation_list:
			permutation_list
			{
				$$ = $1;
			}
			| /* EMPTY */
			{
				$$.elements = NULL;
				$$.nelements = 0;
			}

permutation_list:
			permutation_list permutation
			{
				$$.elements = realloc($1.elements,
									  ($1.nelements + 1) * sizeof(void *));
				$$.elements[$1.nelements] = $2;
				$$.nelements = $1.nelements + 1;
			}
			| permutation
			{
				$$.nelements = 1;
				$$.elements = malloc(sizeof(void *));
				$$.elements[0] = $1;
			}
		;


permutation:
<<<<<<< HEAD
			PERMUTATION string_literal_list
=======
			PERMUTATION string_list
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			{
				$$ = malloc(sizeof(Permutation));
				$$->stepnames = (char **) $2.elements;
				$$->nsteps = $2.nelements;
			}
		;

<<<<<<< HEAD
string_literal_list:
			string_literal_list string_literal
=======
string_list:
			string_list string
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			{
				$$.elements = realloc($1.elements,
									  ($1.nelements + 1) * sizeof(void *));
				$$.elements[$1.nelements] = $2;
				$$.nelements = $1.nelements + 1;
			}
<<<<<<< HEAD
			| string_literal
=======
			| string
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
			{
				$$.nelements = 1;
				$$.elements = malloc(sizeof(void *));
				$$.elements[0] = $1;
			}
		;

%%

#include "specscanner.c"
