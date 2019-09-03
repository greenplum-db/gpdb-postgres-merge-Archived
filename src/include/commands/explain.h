/*-------------------------------------------------------------------------
 *
 * explain.h
 *	  prototypes for explain.c
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994-5, Regents of the University of California
 *
 * src/include/commands/explain.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef EXPLAIN_H
#define EXPLAIN_H

#include "executor/executor.h"
#include "lib/stringinfo.h"

typedef enum ExplainFormat
{
	EXPLAIN_FORMAT_TEXT,
	EXPLAIN_FORMAT_XML,
	EXPLAIN_FORMAT_JSON,
	EXPLAIN_FORMAT_YAML
} ExplainFormat;

/* Crude hack to avoid changing sizeof(ExplainState) in released branches */
typedef struct ExplainStateExtra
{
	List	   *groupingstack;	/* format-specific grouping state */
	List	   *deparsecxt;		/* context list for deparsing expressions */
} ExplainStateExtra;

typedef struct ExplainState
{
	StringInfo	str;			/* output buffer */
	/* options */
	bool		verbose;		/* be verbose */
	bool		analyze;		/* print actual times */
	bool		costs;			/* print estimated costs */
	bool		buffers;		/* print buffer usage */
	bool		dxl;			/* CDB: print DXL */
	bool		timing;			/* print detailed node timing */
	bool		summary;		/* print total planning and execution timing */
	ExplainFormat format;		/* output format */
	/* state for output formatting --- not reset for each new plan tree */
	int			indent;			/* current indentation level */
	List	   *grouping_stack; /* format-specific grouping state */
	/* state related to the current plan tree (filled by ExplainPrintPlan) */
	PlannedStmt *pstmt;			/* top of plan */
	List	   *rtable;			/* range table */
	List	   *rtable_names;	/* alias names for RTEs */
<<<<<<< HEAD
	int			indent;			/* current indentation level */
	ExplainStateExtra *extra;	/* pointer to additional data */

    /* CDB */
    struct CdbExplain_ShowStatCtx  *showstatctx;    /* EXPLAIN ANALYZE info */
    Slice          *currentSlice;   /* slice whose nodes we are visiting */

	PlanState  *parentPlanState;
=======
	List	   *deparse_cxt;	/* context list for deparsing expressions */
	Bitmapset  *printed_subplans;		/* ids of SubPlans we've printed */
>>>>>>> b5bce6c1ec6061c8a4f730d927e162db7e2ce365
} ExplainState;

/* Hook for plugins to get control in ExplainOneQuery() */
typedef void (*ExplainOneQuery_hook_type) (Query *query,
													   IntoClause *into,
													   ExplainState *es,
													 const char *queryString,
													   ParamListInfo params);
extern PGDLLIMPORT ExplainOneQuery_hook_type ExplainOneQuery_hook;

/* Hook for plugins to get control in explain_get_index_name() */
typedef const char *(*explain_get_index_name_hook_type) (Oid indexId);
extern PGDLLIMPORT explain_get_index_name_hook_type explain_get_index_name_hook;


extern void ExplainQuery(ExplainStmt *stmt, const char *queryString,
			 ParamListInfo params, DestReceiver *dest);

extern ExplainState *NewExplainState(void);

extern TupleDesc ExplainResultDesc(ExplainStmt *stmt);

extern void ExplainOneUtility(Node *utilityStmt, IntoClause *into,
				  ExplainState *es,
				  const char *queryString, ParamListInfo params);

extern void ExplainOnePlan(PlannedStmt *plannedstmt, IntoClause *into,
			   ExplainState *es, const char *queryString,
			   ParamListInfo params, const instr_time *planduration);

extern void ExplainPrintPlan(ExplainState *es, QueryDesc *queryDesc);
extern void ExplainPrintTriggers(ExplainState *es, QueryDesc *queryDesc);

extern void ExplainQueryText(ExplainState *es, QueryDesc *queryDesc);

extern void ExplainBeginOutput(ExplainState *es);
extern void ExplainEndOutput(ExplainState *es);
extern void ExplainSeparatePlans(ExplainState *es);

extern void ExplainPropertyList(const char *qlabel, List *data,
					ExplainState *es);
extern void ExplainPropertyListNested(const char *qlabel, List *data,
						  ExplainState *es);
extern void ExplainPropertyText(const char *qlabel, const char *value,
					ExplainState *es);
extern void ExplainPropertyInteger(const char *qlabel, int value,
					   ExplainState *es);
extern void ExplainPropertyLong(const char *qlabel, long value,
					ExplainState *es);
extern void ExplainPropertyFloat(const char *qlabel, double value, int ndigits,
					 ExplainState *es);
extern void ExplainPropertyBool(const char *qlabel, bool value,
					ExplainState *es);

extern void ExplainOpenGroup(const char *objtype, const char *labelname,
				 bool labeled, ExplainState *es);
extern void ExplainCloseGroup(const char *objtype, const char *labelname,
				  bool labeled, ExplainState *es);

extern void ExplainPrintExecStatsEnd(ExplainState *es, QueryDesc *queryDesc);

#endif							/* EXPLAIN_H */
