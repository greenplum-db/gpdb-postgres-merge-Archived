/*-------------------------------------------------------------------------
 *
 * prepare.c
 *	  Prepareable SQL statements via PREPARE, EXECUTE and DEALLOCATE
 *
 * This module also implements storage of prepared statements that are
 * accessed via the extended FE/BE query protocol.
 *
 *
<<<<<<< HEAD
 * Copyright (c) 2002-2009, PostgreSQL Global Development Group
=======
 * Copyright (c) 2002-2008, PostgreSQL Global Development Group
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 *
 * IDENTIFICATION
 *	  $PostgreSQL: pgsql/src/backend/commands/prepare.c,v 1.80.2.2 2009/12/29 17:41:18 heikki Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "access/heapam.h"
#include "access/xact.h"
#include "catalog/gp_policy.h"
#include "catalog/pg_type.h"
#include "commands/explain.h"
#include "commands/prepare.h"
<<<<<<< HEAD
#include "funcapi.h"
#include "miscadmin.h"
=======
#include "miscadmin.h"
#include "parser/analyze.h"
#include "parser/parse_coerce.h"
#include "parser/parse_expr.h"
#include "parser/parse_type.h"
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#include "rewrite/rewriteHandler.h"
#include "tcop/pquery.h"
#include "tcop/tcopprot.h"
#include "tcop/utility.h"
#include "utils/builtins.h"
#include "utils/memutils.h"

extern char *savedSeqServerHost;
extern int savedSeqServerPort;

/*
 * The hash table in which prepared queries are stored. This is
 * per-backend: query plans are not shared between backends.
 * The keys for this hash table are the arguments to PREPARE and EXECUTE
 * (statement names); the entries are PreparedStatement structs.
 */
static HTAB *prepared_queries = NULL;

static void InitQueryHashTable(void);
static ParamListInfo EvaluateParams(PreparedStatement *pstmt, List *params,
			   const char *queryString, EState *estate);
static Datum build_regtype_array(Oid *param_types, int num_params);

/*
 * Implements the 'PREPARE' utility statement.
 */
void
PrepareQuery(PrepareStmt *stmt, const char *queryString)
{
<<<<<<< HEAD
	const char	*commandTag = NULL;
	Query		*query = NULL;
	List		*query_list = NIL;
	List		*plan_list = NIL;
	List		*query_list_copy = NIL;
	NodeTag		srctag;  /* GPDB */
=======
	Oid		   *argtypes = NULL;
	int			nargs;
	Query	   *query;
	List	   *query_list,
			   *plan_list;
	int			i;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	/*
	 * Disallow empty-string statement name (conflicts with protocol-level
	 * unnamed statement).
	 */
	if (!stmt->name || stmt->name[0] == '\0')
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PSTATEMENT_DEFINITION),
				 errmsg("invalid statement name: must not be empty")));

	/* Transform list of TypeNames to array of type OIDs */
	nargs = list_length(stmt->argtypes);

	if (nargs)
	{
		ParseState *pstate;
		ListCell   *l;

		/*
		 * typenameTypeId wants a ParseState to carry the source query string.
		 * Is it worth refactoring its API to avoid this?
		 */
		pstate = make_parsestate(NULL);
		pstate->p_sourcetext = queryString;

		argtypes = (Oid *) palloc(nargs * sizeof(Oid));
		i = 0;

		foreach(l, stmt->argtypes)
		{
			TypeName   *tn = lfirst(l);
			Oid			toid = typenameTypeId(pstate, tn, NULL);

			argtypes[i++] = toid;
		}
	}

	/*
	 * Analyze the statement using these parameter types (any parameters
	 * passed in from above us will not be visible to it), allowing
	 * information about unknown parameters to be deduced from context.
	 *
	 * Because parse analysis scribbles on the raw querytree, we must make a
	 * copy to ensure we have a pristine raw tree to cache.  FIXME someday.
	 */
	query = parse_analyze_varparams((Node *) copyObject(stmt->query),
									queryString,
									&argtypes, &nargs);

	/*
	 * Check that all parameter types were determined.
	 */
	for (i = 0; i < nargs; i++)
	{
		Oid			argtype = argtypes[i];

		if (argtype == InvalidOid || argtype == UNKNOWNOID)
			ereport(ERROR,
					(errcode(ERRCODE_INDETERMINATE_DATATYPE),
					 errmsg("could not determine data type of parameter $%d",
							i + 1)));
	}

	/*
	 * grammar only allows OptimizableStmt, so this check should be redundant
	 */
	switch (query->commandType)
	{
		case CMD_SELECT:
<<<<<<< HEAD
			commandTag = "SELECT";
			srctag = T_SelectStmt;
			break;
		case CMD_INSERT:
			commandTag = "INSERT";
			srctag = T_InsertStmt;
			break;
		case CMD_UPDATE:
			commandTag = "UPDATE";
			srctag = T_UpdateStmt;
			break;
		case CMD_DELETE:
			commandTag = "DELETE";
			srctag = T_DeleteStmt;
=======
		case CMD_INSERT:
		case CMD_UPDATE:
		case CMD_DELETE:
			/* OK */
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
			break;
		default:
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PSTATEMENT_DEFINITION),
					 errmsg("utility statements cannot be prepared")));
<<<<<<< HEAD
			commandTag = NULL;	/* keep compiler quiet */
			srctag = T_Query;
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
			break;
	}

	/* Rewrite the query. The result could be 0, 1, or many queries. */
	query_list = QueryRewrite(query);

	query_list_copy = copyObject(query_list); /* planner scribbles on query tree */
	
	/* Generate plans for queries.	Snapshot is already set. */
	plan_list = pg_plan_queries(query_list, 0, NULL, false);

	/*
	 * Save the results.
	 */
	StorePreparedStatement(stmt->name,
<<<<<<< HEAD
						   queryString, /* WAS global debug_query_string, */
						   srctag,
						   commandTag,
						   query_list_copy,
						   stmt->argtype_oids,
=======
						   stmt->query,
						   queryString,
						   CreateCommandTag((Node *) query),
						   argtypes,
						   nargs,
						   0,	/* default cursor options */
						   plan_list,
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
						   true);
}

/*
 * Implements the 'EXECUTE' utility statement.
 *
 * Note: this is one of very few places in the code that needs to deal with
 * two query strings at once.  The passed-in queryString is that of the
 * EXECUTE, which we might need for error reporting while processing the
 * parameter expressions.  The query_string that we copy from the plan
 * source is that of the original PREPARE.
 */
void
ExecuteQuery(ExecuteStmt *stmt, const char *queryString,
			 ParamListInfo params,
			 DestReceiver *dest, char *completionTag)
{
	PreparedStatement *entry;
<<<<<<< HEAD
	List	   *stmt_list;
	MemoryContext qcontext;
=======
	CachedPlan *cplan;
	List	   *plan_list;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	ParamListInfo paramLI = NULL;
	EState	   *estate = NULL;
	Portal		portal;

	/* Look it up in the hash table */
	entry = FetchPreparedStatement(stmt->name, true);

<<<<<<< HEAD
	qcontext = entry->context;
=======
	/* Shouldn't have a non-fully-planned plancache entry */
	if (!entry->plansource->fully_planned)
		elog(ERROR, "EXECUTE does not support unplanned prepared statements");
	/* Shouldn't get any non-fixed-result cached plan, either */
	if (!entry->plansource->fixed_result)
		elog(ERROR, "EXECUTE does not support variable-result cached plans");
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	/* Evaluate parameters, if any */
	if (entry->plansource->num_params > 0)
	{
		/*
		 * Need an EState to evaluate parameters; must not delete it till end
		 * of query, in case parameters are pass-by-reference.
		 */
		estate = CreateExecutorState();
		estate->es_param_list_info = params;
		paramLI = EvaluateParams(entry, stmt->params,
								 queryString, estate);
	}

	/* Create a new portal to run the query in */
	portal = CreateNewPortal();
	/* Don't display the portal in pg_cursors, it is for internal use only */
	portal->visible = false;

	/* Plan the query.  If this is a CTAS, copy the "into" information into
	 * the query so that we construct the plan correctly.  Else the table
	 * might not be created on the segments.  (MPP-8135) */
	{
		List *query_list = copyObject(entry->query_list); /* planner scribbles on query tree :( */
		
		if ( stmt->into )
		{
			Query *query = (Query*)linitial(query_list);
			Assert(IsA(query, Query) && query->intoClause == NULL);
			query->intoClause = copyObject(stmt->into);
		}
		
		stmt_list = pg_plan_queries(query_list, paramLI, false);
	}

	/*
	 * For CREATE TABLE / AS EXECUTE, we must make a copy of the stored query
	 * so that we can modify its destination (yech, but this has always been
	 * ugly).  For regular EXECUTE we can just use the cached query, since the
	 * executor is read-only.
	 */
	if (stmt->into)
	{
		MemoryContext oldContext;
<<<<<<< HEAD
		PlannedStmt	 *pstmt;

		if (list_length(stmt_list) != 1)
			ereport(ERROR,
					(errcode(ERRCODE_WRONG_OBJECT_TYPE),
					 errmsg("prepared statement is not a SELECT")));

		oldContext = MemoryContextSwitchTo(PortalGetHeapMemory(portal));

		stmt_list = copyObject(stmt_list);
		qcontext = PortalGetHeapMemory(portal);
		pstmt = (PlannedStmt *) linitial(stmt_list);
		pstmt->qdContext = qcontext;
		if (pstmt->commandType != CMD_SELECT)
			ereport(ERROR,
					(errcode(ERRCODE_WRONG_OBJECT_TYPE),
					 errmsg("prepared statement is not a SELECT")));
=======
		PlannedStmt *pstmt;

		/* Replan if needed, and increment plan refcount transiently */
		cplan = RevalidateCachedPlan(entry->plansource, true);

		/* Copy plan into portal's context, and modify */
		oldContext = MemoryContextSwitchTo(PortalGetHeapMemory(portal));

		plan_list = copyObject(cplan->stmt_list);

		if (list_length(plan_list) != 1)
			ereport(ERROR,
					(errcode(ERRCODE_WRONG_OBJECT_TYPE),
					 errmsg("prepared statement is not a SELECT")));
		pstmt = (PlannedStmt *) linitial(plan_list);
		if (!IsA(pstmt, PlannedStmt) ||
			pstmt->commandType != CMD_SELECT ||
			pstmt->utilityStmt != NULL)
			ereport(ERROR,
					(errcode(ERRCODE_WRONG_OBJECT_TYPE),
					 errmsg("prepared statement is not a SELECT")));
		pstmt->intoClause = copyObject(stmt->into);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

		pstmt->intoClause = copyObject(stmt->into);

		/* XXX  Is it legitimate to assign a constant default policy without 
		 *      even checking the relation?
		 */
		pstmt->intoPolicy = palloc0(sizeof(GpPolicy)- sizeof(pstmt->intoPolicy->attrs)
									+ 255 * sizeof(pstmt->intoPolicy->attrs[0]));
		pstmt->intoPolicy->nattrs = 1;			
		pstmt->intoPolicy->ptype = POLICYTYPE_PARTITIONED;
		pstmt->intoPolicy->attrs[0] = 1;
		
		MemoryContextSwitchTo(oldContext);

		/* We no longer need the cached plan refcount ... */
		ReleaseCachedPlan(cplan, true);
		/* ... and we don't want the portal to depend on it, either */
		cplan = NULL;
	}
	else
	{
		/* Replan if needed, and increment plan refcount for portal */
		cplan = RevalidateCachedPlan(entry->plansource, false);
		plan_list = cplan->stmt_list;
	}

<<<<<<< HEAD
	/* Copy the plan's saved query string into the portal's memory */
	Assert(entry->query_string != NULL); 
	char *query_string = MemoryContextStrdup(PortalGetHeapMemory(portal),
					   entry->query_string);

	PortalDefineQuery(portal,
					  NULL,
					  query_string,
					  entry->sourceTag,
					  entry->commandTag,
					  stmt_list,
					  qcontext);
=======
	/*
	 * Note: we don't bother to copy the source query string into the portal.
	 * Any errors it might be useful for will already have been reported.
	 */
	PortalDefineQuery(portal,
					  NULL,
					  NULL,
					  entry->plansource->commandTag,
					  plan_list,
					  cplan);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	/*
	 * Run the portal to completion.
	 */
	PortalStart(portal, paramLI, ActiveSnapshot,
				savedSeqServerHost, savedSeqServerPort);

<<<<<<< HEAD
	(void) PortalRun(portal, 
					FETCH_ALL, 
					 true, /* Effectively always top level. */
					 dest, 
					 dest, 
					 completionTag);
=======
	(void) PortalRun(portal, FETCH_ALL, false, dest, dest, completionTag);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	PortalDrop(portal, false);

	if (estate)
		FreeExecutorState(estate);

	/* No need to pfree other memory, MemoryContext will be reset */
}

/*
 * EvaluateParams: evaluate a list of parameters.
 *
 * pstmt: statement we are getting parameters for.
 * params: list of given parameter expressions (raw parser output!)
 * queryString: source text for error messages.
 * estate: executor state to use.
 *
 * Returns a filled-in ParamListInfo -- this can later be passed to
 * CreateQueryDesc(), which allows the executor to make use of the parameters
 * during query execution.
 */
static ParamListInfo
EvaluateParams(PreparedStatement *pstmt, List *params,
			   const char *queryString, EState *estate)
{
	Oid		   *param_types = pstmt->plansource->param_types;
	int			num_params = pstmt->plansource->num_params;
	int			nparams = list_length(params);
	ParseState *pstate;
	ParamListInfo paramLI;
	List	   *exprstates;
	ListCell   *l;
	int			i;

	if (nparams != num_params)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
		   errmsg("wrong number of parameters for prepared statement \"%s\"",
				  pstmt->stmt_name),
				 errdetail("Expected %d parameters but got %d.",
						   num_params, nparams)));

	/* Quick exit if no parameters */
	if (num_params == 0)
		return NULL;

	/*
	 * We have to run parse analysis for the expressions.  Since the parser is
	 * not cool about scribbling on its input, copy first.
	 */
	params = (List *) copyObject(params);

	pstate = make_parsestate(NULL);
	pstate->p_sourcetext = queryString;

	i = 0;
	foreach(l, params)
	{
		Node	   *expr = lfirst(l);
		Oid			expected_type_id = param_types[i];
		Oid			given_type_id;

		expr = transformExpr(pstate, expr);

		/* Cannot contain subselects or aggregates */
		if (pstate->p_hasSubLinks)
			ereport(ERROR,
					(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
					 errmsg("cannot use subquery in EXECUTE parameter")));
		if (pstate->p_hasAggs)
			ereport(ERROR,
					(errcode(ERRCODE_GROUPING_ERROR),
			  errmsg("cannot use aggregate function in EXECUTE parameter")));

		given_type_id = exprType(expr);

		expr = coerce_to_target_type(pstate, expr, given_type_id,
									 expected_type_id, -1,
									 COERCION_ASSIGNMENT,
									 COERCE_IMPLICIT_CAST);

		if (expr == NULL)
			ereport(ERROR,
					(errcode(ERRCODE_DATATYPE_MISMATCH),
					 errmsg("parameter $%d of type %s cannot be coerced to the expected type %s",
							i + 1,
							format_type_be(given_type_id),
							format_type_be(expected_type_id)),
			   errhint("You will need to rewrite or cast the expression.")));

		lfirst(l) = expr;
		i++;
	}

	/* Prepare the expressions for execution */
	exprstates = (List *) ExecPrepareExpr((Expr *) params, estate);

	/* sizeof(ParamListInfoData) includes the first array element */
	paramLI = (ParamListInfo)
		palloc(sizeof(ParamListInfoData) +
			   (num_params - 1) *sizeof(ParamExternData));
	paramLI->numParams = num_params;

	i = 0;
	foreach(l, exprstates)
	{
		ExprState  *n = lfirst(l);
		ParamExternData *prm = &paramLI->params[i];

		prm->ptype = param_types[i];
		prm->pflags = 0;
		prm->value = ExecEvalExprSwitchContext(n,
											   GetPerTupleExprContext(estate),
											   &prm->isnull,
											   NULL);

		i++;
	}

	return paramLI;
}


/*
 * Initialize query hash table upon first use.
 */
static void
InitQueryHashTable(void)
{
	HASHCTL		hash_ctl;

	MemSet(&hash_ctl, 0, sizeof(hash_ctl));

	hash_ctl.keysize = NAMEDATALEN;
	hash_ctl.entrysize = sizeof(PreparedStatement);

	prepared_queries = hash_create("Prepared Queries",
								   32,
								   &hash_ctl,
								   HASH_ELEM);
}

/*
 * Store all the data pertaining to a query in the hash table using
 * the specified key.  All the given data is copied into either the hashtable
 * entry or the underlying plancache entry, so the caller can dispose of its
 * copy.
 *
 * Exception: commandTag is presumed to be a pointer to a constant string,
 * or possibly NULL, so it need not be copied.	Note that commandTag should
 * be NULL only if the original query (before rewriting) was empty.
 * The original query nodetag is saved as well, only used if resource 
 * scheduling is enabled.
 */
void
StorePreparedStatement(const char *stmt_name,
					   Node *raw_parse_tree,
					   const char *query_string,
					   NodeTag	   sourceTag,
					   const char *commandTag,
<<<<<<< HEAD
					   List *query_list,
					   List *argtype_list,
=======
					   Oid *param_types,
					   int num_params,
					   int cursor_options,
					   List *stmt_list,
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
					   bool from_sql)
{
	PreparedStatement *entry;
	CachedPlanSource *plansource;
	bool		found;

	/* Initialize the hash table, if necessary */
	if (!prepared_queries)
		InitQueryHashTable();

	/* Check for pre-existing entry of same name */
	hash_search(prepared_queries, stmt_name, HASH_FIND, &found);

	if (found)
		ereport(ERROR,
				(errcode(ERRCODE_DUPLICATE_PSTATEMENT),
				 errmsg("prepared statement \"%s\" already exists",
						stmt_name)));

<<<<<<< HEAD
	/* Make a permanent memory context for the hashtable entry */
	entrycxt = AllocSetContextCreate(TopMemoryContext,
									 stmt_name,
									 ALLOCSET_SMALL_MINSIZE,
									 ALLOCSET_SMALL_INITSIZE,
									 ALLOCSET_SMALL_MAXSIZE);

	oldcxt = MemoryContextSwitchTo(entrycxt);

	/*
	 * We need to copy the data so that it is stored in the correct memory
	 * context.  Do this before making hashtable entry, so that an
	 * out-of-memory failure only wastes memory and doesn't leave us with an
	 * incomplete (ie corrupt) hashtable entry.
	 */
	qstring = query_string ? pstrdup(query_string) : NULL;
	query_list = (List *)copyObject(query_list);
	argtype_list = list_copy(argtype_list);
=======
	/* Create a plancache entry */
	plansource = CreateCachedPlan(raw_parse_tree,
								  query_string,
								  commandTag,
								  param_types,
								  num_params,
								  cursor_options,
								  stmt_list,
								  true,
								  true);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	/* Now we can add entry to hash table */
	entry = (PreparedStatement *) hash_search(prepared_queries,
											  stmt_name,
											  HASH_ENTER,
											  &found);

	/* Shouldn't get a duplicate entry */
	if (found)
		elog(ERROR, "duplicate prepared statement \"%s\"",
			 stmt_name);

<<<<<<< HEAD
	/* Fill in the hash table entry with copied data */
	entry->query_string = qstring;
	entry->sourceTag = sourceTag;
	entry->commandTag = commandTag;
	entry->query_list = query_list;
	entry->argtype_list = argtype_list;
	entry->context = entrycxt;
	entry->prepare_time = GetCurrentStatementStartTimestamp();
=======
	/* Fill in the hash table entry */
	entry->plansource = plansource;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	entry->from_sql = from_sql;
	entry->prepare_time = GetCurrentStatementStartTimestamp();
}

/*
 * Lookup an existing query in the hash table. If the query does not
 * actually exist, throw ereport(ERROR) or return NULL per second parameter.
 *
 * Note: this does not force the referenced plancache entry to be valid,
 * since not all callers care.
 */
PreparedStatement *
FetchPreparedStatement(const char *stmt_name, bool throwError)
{
	PreparedStatement *entry;

	/*
	 * If the hash table hasn't been initialized, it can't be storing
	 * anything, therefore it couldn't possibly store our plan.
	 */
	if (prepared_queries)
		entry = (PreparedStatement *) hash_search(prepared_queries,
												  stmt_name,
												  HASH_FIND,
												  NULL);
	else
		entry = NULL;

	if (!entry && throwError)
		ereport(ERROR,
				(errcode(ERRCODE_UNDEFINED_PSTATEMENT),
				 errmsg("prepared statement \"%s\" does not exist",
						stmt_name)));

	return entry;
}

/*
 * Given a prepared statement, determine the result tupledesc it will
 * produce.  Returns NULL if the execution will not return tuples.
 *
 * Note: the result is created or copied into current memory context.
 */
TupleDesc
FetchPreparedStatementResultDesc(PreparedStatement *stmt)
{
<<<<<<< HEAD
	Query	   *query;

	switch (ChoosePortalStrategy(stmt->query_list))
	{
		case PORTAL_ONE_SELECT:
			query = (Query *) linitial(stmt->query_list);
			return ExecCleanTypeFromTL(query->targetList, false);

		case PORTAL_ONE_RETURNING:
			query = (Query *) PortalListGetPrimaryStmt(stmt->query_list);
			return ExecCleanTypeFromTL(query->returningList, false);

		case PORTAL_UTIL_SELECT:
			query = (Query *) linitial(stmt->query_list);
			return UtilityTupleDescriptor(query->utilityStmt);

		case PORTAL_MULTI_QUERY:
			/* will not return tuples */
			break;
	}
	return NULL;
}

/*
 * Given a prepared statement, determine whether it will return tuples.
 *
 * Note: this is used rather than just testing the result of
 * FetchPreparedStatementResultDesc() because that routine can fail if
 * invoked in an aborted transaction.  This one is safe to use in any
 * context.  Be sure to keep the two routines in sync!
 */
bool
PreparedStatementReturnsTuples(PreparedStatement *stmt)
{
	switch (ChoosePortalStrategy(stmt->query_list))
	{
		case PORTAL_ONE_SELECT:
		case PORTAL_ONE_RETURNING:
		case PORTAL_UTIL_SELECT:
			return true;

		case PORTAL_MULTI_QUERY:
			/* will not return tuples */
			break;
	}
	return false;
=======
	/*
	 * Since we don't allow prepared statements' result tupdescs to change,
	 * there's no need for a revalidate call here.
	 */
	Assert(stmt->plansource->fixed_result);
	if (stmt->plansource->resultDesc)
		return CreateTupleDescCopy(stmt->plansource->resultDesc);
	else
		return NULL;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
}

/*
 * Given a prepared statement that returns tuples, extract the query
 * targetlist.	Returns NIL if the statement doesn't have a determinable
 * targetlist.
 *
 * Note: this is pretty ugly, but since it's only used in corner cases like
 * Describe Statement on an EXECUTE command, we don't worry too much about
 * efficiency.
<<<<<<< HEAD

 * Note: do not modify the result.
 *
 * XXX be careful to keep this in sync with FetchPortalTargetList,
 * and with UtilityReturnsTuples.
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
List *
FetchPreparedStatementTargetList(PreparedStatement *stmt)
{
	List	   *tlist;
	CachedPlan *cplan;

<<<<<<< HEAD
	if (strategy == PORTAL_ONE_SELECT)
		return ((Query *) linitial(stmt->query_list))->targetList;
	if (strategy == PORTAL_ONE_RETURNING)
		return ((Query *)(PortalListGetPrimaryStmt(stmt->query_list)))->returningList;
	if (strategy == PORTAL_UTIL_SELECT)
	{
		Node	   *utilityStmt;
=======
	/* No point in looking if it doesn't return tuples */
	if (stmt->plansource->resultDesc == NULL)
		return NIL;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

	/* Make sure the plan is up to date */
	cplan = RevalidateCachedPlan(stmt->plansource, true);

	/* Get the primary statement and find out what it returns */
	tlist = FetchStatementTargetList(PortalListGetPrimaryStmt(cplan->stmt_list));

	/* Copy into caller's context so we can release the plancache entry */
	tlist = (List *) copyObject(tlist);

	ReleaseCachedPlan(cplan, true);

	return tlist;
}

/*
 * Implements the 'DEALLOCATE' utility statement: deletes the
 * specified plan from storage.
 */
void
DeallocateQuery(DeallocateStmt *stmt)
{
	if (stmt->name)
		DropPreparedStatement(stmt->name, true);
	else
		DropAllPreparedStatements();
}

/*
 * Internal version of DEALLOCATE
 *
 * If showError is false, dropping a nonexistent statement is a no-op.
 */
void
DropPreparedStatement(const char *stmt_name, bool showError)
{
	PreparedStatement *entry;

	/* Find the query's hash table entry; raise error if wanted */
	entry = FetchPreparedStatement(stmt_name, showError);

	if (entry)
	{
		/* Release the plancache entry */
		DropCachedPlan(entry->plansource);

		/* Now we can remove the hash table entry */
		hash_search(prepared_queries, entry->stmt_name, HASH_REMOVE, NULL);
	}
}

/*
 * Drop all cached statements.
 */
void
DropAllPreparedStatements(void)
{
	HASH_SEQ_STATUS seq;
	PreparedStatement *entry;

	/* nothing cached */
	if (!prepared_queries)
		return;

	/* walk over cache */
	hash_seq_init(&seq, prepared_queries);
	while ((entry = hash_seq_search(&seq)) != NULL)
	{
		/* Release the plancache entry */
		DropCachedPlan(entry->plansource);

		/* Now we can remove the hash table entry */
		hash_search(prepared_queries, entry->stmt_name, HASH_REMOVE, NULL);
	}
}

/*
 * Implements the 'EXPLAIN EXECUTE' utility statement.
 */
void
ExplainExecuteQuery(ExecuteStmt *execstmt, ExplainStmt *stmt,
					const char *queryString,
					ParamListInfo params, TupOutputState *tstate)
{
	PreparedStatement *entry;
<<<<<<< HEAD
	ListCell   *q,
			   *p;
	List	   *query_list,
			   *stmt_list;
=======
	CachedPlan *cplan;
	List	   *plan_list;
	ListCell   *p;
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	ParamListInfo paramLI = NULL;
	EState	   *estate = NULL;

	/* Look it up in the hash table */
	entry = FetchPreparedStatement(execstmt->name, true);

<<<<<<< HEAD
=======
	/* Shouldn't have a non-fully-planned plancache entry */
	if (!entry->plansource->fully_planned)
		elog(ERROR, "EXPLAIN EXECUTE does not support unplanned prepared statements");
	/* Shouldn't get any non-fixed-result cached plan, either */
	if (!entry->plansource->fixed_result)
		elog(ERROR, "EXPLAIN EXECUTE does not support variable-result cached plans");

	/* Replan if needed, and acquire a transient refcount */
	cplan = RevalidateCachedPlan(entry->plansource, true);

	plan_list = cplan->stmt_list;

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	/* Evaluate parameters, if any */
	if (entry->plansource->num_params)
	{
		/*
		 * Need an EState to evaluate parameters; must not delete it till end
		 * of query, in case parameters are pass-by-reference.
		 */
		estate = CreateExecutorState();
		estate->es_param_list_info = params;
		paramLI = EvaluateParams(entry, execstmt->params,
								 queryString, estate);
	}

	query_list = copyObject(entry->query_list); /* planner scribbles on query tree */
	stmt_list = pg_plan_queries(query_list, paramLI, false);

	Assert(list_length(query_list) == list_length(stmt_list));

	/* Explain each query */
<<<<<<< HEAD
	forboth(q, query_list, p, stmt_list)
	{
		PlannedStmt *pstmt = (PlannedStmt *) lfirst(p);
		Query	   *query = (Query *) lfirst(q);
=======
	foreach(p, plan_list)
	{
		PlannedStmt *pstmt = (PlannedStmt *) lfirst(p);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
		bool		is_last_query;

		is_last_query = (lnext(p) == NULL);

		if (IsA(pstmt, PlannedStmt))
		{
<<<<<<< HEAD
			if (query->utilityStmt && IsA(query->utilityStmt, NotifyStmt))
				do_text_output_oneline(tstate, "NOTIFY");
			else
				do_text_output_oneline(tstate, "UTILITY");
		}
		else
		{
=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
			if (execstmt->into)
			{
				if (pstmt->commandType != CMD_SELECT ||
					pstmt->utilityStmt != NULL)
					ereport(ERROR,
							(errcode(ERRCODE_WRONG_OBJECT_TYPE),
							 errmsg("prepared statement is not a SELECT")));

				/* Copy the stmt so we can modify it */
				pstmt = copyObject(pstmt);

<<<<<<< HEAD
				if ( execstmt->into )
				{
					Assert(query->intoClause == NULL);
					query->intoClause = makeNode(IntoClause);
					query->intoClause->rel = execstmt->into->rel;
				}
			}


			ExplainOnePlan(pstmt, stmt, "EXECUTE", paramLI, tstate);
=======
				pstmt->intoClause = execstmt->into;
			}

			ExplainOnePlan(pstmt, paramLI, stmt, tstate);
		}
		else
		{
			ExplainOneUtility((Node *) pstmt, stmt, queryString,
							  params, tstate);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
		}

		/* No need for CommandCounterIncrement, as ExplainOnePlan did it */

		/* put a blank line between plans */
		if (!is_last_query)
			do_text_output_oneline(tstate, "");
	}

	if (estate)
		FreeExecutorState(estate);

	ReleaseCachedPlan(cplan, true);
}

/*
 * This set returning function reads all the prepared statements and
 * returns a set of (name, statement, prepare_time, param_types, from_sql).
 */
Datum
pg_prepared_statement(PG_FUNCTION_ARGS)
{
	ReturnSetInfo *rsinfo = (ReturnSetInfo *) fcinfo->resultinfo;
	TupleDesc	tupdesc;
	Tuplestorestate *tupstore;
	MemoryContext per_query_ctx;
	MemoryContext oldcontext;

	/* check to see if caller supports us returning a tuplestore */
	if (rsinfo == NULL || !IsA(rsinfo, ReturnSetInfo))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("set-valued function called in context that cannot accept a set")));
	if (!(rsinfo->allowedModes & SFRM_Materialize))
		ereport(ERROR,
				(errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				 errmsg("materialize mode required, but it is not " \
						"allowed in this context")));
<<<<<<< HEAD

	/* need to build tuplestore in query context */
	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);

=======

	/* need to build tuplestore in query context */
	per_query_ctx = rsinfo->econtext->ecxt_per_query_memory;
	oldcontext = MemoryContextSwitchTo(per_query_ctx);

>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	/*
	 * build tupdesc for result tuples. This must match the definition of the
	 * pg_prepared_statements view in system_views.sql
	 */
	tupdesc = CreateTemplateTupleDesc(5, false);
	TupleDescInitEntry(tupdesc, (AttrNumber) 1, "name",
					   TEXTOID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 2, "statement",
					   TEXTOID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 3, "prepare_time",
					   TIMESTAMPTZOID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 4, "parameter_types",
					   REGTYPEARRAYOID, -1, 0);
	TupleDescInitEntry(tupdesc, (AttrNumber) 5, "from_sql",
					   BOOLOID, -1, 0);
<<<<<<< HEAD

	/*
	 * We put all the tuples into a tuplestore in one scan of the hashtable.
	 * This avoids any issue of the hashtable possibly changing between calls.
	 */
	tupstore = tuplestore_begin_heap(true, false, work_mem);

	/* hash table might be uninitialized */
	if (prepared_queries)
	{
		HASH_SEQ_STATUS hash_seq;
		PreparedStatement *prep_stmt;

		hash_seq_init(&hash_seq, prepared_queries);
		while ((prep_stmt = hash_seq_search(&hash_seq)) != NULL)
		{
			HeapTuple	tuple;
			Datum		values[5];
			bool		nulls[5];

			/* generate junk in short-term context */
			MemoryContextSwitchTo(oldcontext);
=======

	/*
	 * We put all the tuples into a tuplestore in one scan of the hashtable.
	 * This avoids any issue of the hashtable possibly changing between calls.
	 */
	tupstore = tuplestore_begin_heap(true, false, work_mem);

	/* generate junk in short-term context */
	MemoryContextSwitchTo(oldcontext);

	/* hash table might be uninitialized */
	if (prepared_queries)
	{
		HASH_SEQ_STATUS hash_seq;
		PreparedStatement *prep_stmt;

		hash_seq_init(&hash_seq, prepared_queries);
		while ((prep_stmt = hash_seq_search(&hash_seq)) != NULL)
		{
			HeapTuple	tuple;
			Datum		values[5];
			bool		nulls[5];
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

			MemSet(nulls, 0, sizeof(nulls));

			values[0] = DirectFunctionCall1(textin,
									  CStringGetDatum(prep_stmt->stmt_name));

<<<<<<< HEAD
			if (prep_stmt->query_string == NULL)
				nulls[1] = true;
			else
				values[1] = DirectFunctionCall1(textin,
								   CStringGetDatum(prep_stmt->query_string));

			values[2] = TimestampTzGetDatum(prep_stmt->prepare_time);
			values[3] = build_regtype_array(prep_stmt->argtype_list);
			values[4] = BoolGetDatum(prep_stmt->from_sql);

			tuple = heap_form_tuple(tupdesc, values, nulls);

			/* switch to appropriate context while storing the tuple */
			MemoryContextSwitchTo(per_query_ctx);
=======
			if (prep_stmt->plansource->query_string == NULL)
				nulls[1] = true;
			else
				values[1] = DirectFunctionCall1(textin,
					   CStringGetDatum(prep_stmt->plansource->query_string));

			values[2] = TimestampTzGetDatum(prep_stmt->prepare_time);
			values[3] = build_regtype_array(prep_stmt->plansource->param_types,
										  prep_stmt->plansource->num_params);
			values[4] = BoolGetDatum(prep_stmt->from_sql);

			tuple = heap_form_tuple(tupdesc, values, nulls);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
			tuplestore_puttuple(tupstore, tuple);
		}
	}

	/* clean up and return the tuplestore */
	tuplestore_donestoring(tupstore);

<<<<<<< HEAD
	MemoryContextSwitchTo(oldcontext);

=======
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
	rsinfo->returnMode = SFRM_Materialize;
	rsinfo->setResult = tupstore;
	rsinfo->setDesc = tupdesc;

	return (Datum) 0;
}

/*
 * This utility function takes a C array of Oids, and returns a Datum
 * pointing to a one-dimensional Postgres array of regtypes. An empty
 * array is returned as a zero-element array, not NULL.
 */
static Datum
build_regtype_array(Oid *param_types, int num_params)
{
	Datum	   *tmp_ary;
	ArrayType  *result;
	int			i;

	tmp_ary = (Datum *) palloc(num_params * sizeof(Datum));

	for (i = 0; i < num_params; i++)
		tmp_ary[i] = ObjectIdGetDatum(param_types[i]);

	/* XXX: this hardcodes assumptions about the regtype type */
	result = construct_array(tmp_ary, num_params, REGTYPEOID, 4, true, 'i');
	return PointerGetDatum(result);
}
