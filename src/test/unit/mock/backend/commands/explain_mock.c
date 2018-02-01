/*
 *
 * Mock source for explain.c with parts from explain_gp.c
 *
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include "cmockery.h"


#include "postgres.h"
#include "portability/instr_time.h"

#include "libpq-fe.h"
#include "libpq-int.h"
#include "cdb/cdbconn.h"		
#include "cdb/cdbdispatchresult.h"	
#include "cdb/cdbexplain.h"		
#include "cdb/cdbpartition.h"
#include "cdb/cdbvars.h"		
#include "commands/explain.h"
#include "executor/execUtils.h"
#include "executor/instrument.h"	
#include "lib/stringinfo.h"		
#include "libpq/pqformat.h"		
#include "utils/memutils.h"		
#include "cdb/memquota.h"
#include "utils/vmem_tracker.h"
#include "parser/parsetree.h"

#define NUM_SORT_METHOD 5

#define TOP_N_HEAP_SORT_STR "top-N heapsort"
#define QUICK_SORT_STR "quicksort"
#define EXTERNAL_SORT_STR "external sort"
#define EXTERNAL_MERGE_STR "external merge"
#define IN_PROGRESS_SORT_STR "sort still in progress"

#define NUM_SORT_SPACE_TYPE 2
#define MEMORY_STR_SORT_SPACE_TYPE "Memory"
#define DISK_STR_SORT_SPACE_TYPE "Disk"



typedef enum
{
	UNINITIALIZED_SORT = 0,
	TOP_N_HEAP_SORT = 1,
	QUICK_SORT = 2,
	EXTERNAL_SORT = 3,
	EXTERNAL_MERGE = 4,
	IN_PROGRESS_SORT = 5
} ExplainSortMethod;

typedef enum
{
	UNINITIALIZED_SORT_SPACE_TYPE = 0,
	MEMORY_SORT_SPACE_TYPE = 1,
	DISK_SORT_SPACE_TYPE = 2
} ExplainSortSpaceType;


const char *sort_method_enum_str[] = {TOP_N_HEAP_SORT_STR, QUICK_SORT_STR, EXTERNAL_SORT_STR, EXTERNAL_MERGE_STR, IN_PROGRESS_SORT_STR};


typedef struct CdbExplain_StatInst
{
	NodeTag		pstype;			
	bool		running;		
	instr_time	starttime;		
	instr_time	counter;		
	double		firsttuple;		
	double		startup;		
	double		total;			
	double		ntuples;		
	double		nloops;			
	double		execmemused;	
	double		workmemused;	
	double		workmemwanted;	
	bool		workfileCreated;	
	instr_time	firststart;		
	double		peakMemBalance; 
	int			numPartScanned; 
	ExplainSortMethod sortMethod;	
	ExplainSortSpaceType sortSpaceType; 
	long		sortSpaceUsed;	
	int			bnotes;			
	int			enotes;			
} CdbExplain_StatInst;



typedef struct CdbExplain_SliceWorker
{
	double		peakmemused;	
	double		vmem_reserved;	
	double		memory_accounting_global_peak;	
} CdbExplain_SliceWorker;



typedef struct CdbExplain_StatHdr
{
	NodeTag		type;			
	int			segindex;		
	int			nInst;			
	int			bnotes;			
	int			enotes;			

	int			memAccountCount;	
	int			memAccountStartOffset;	

	CdbExplain_SliceWorker worker;	

	
	CdbExplain_StatInst inst[1];

	
} CdbExplain_StatHdr;



typedef struct CdbExplain_DispatchSummary
{
	int			nResult;
	int			nOk;
	int			nError;
	int			nCanceled;
	int			nNotDispatched;
	int			nIgnorableError;
} CdbExplain_DispatchSummary;



typedef struct CdbExplain_NodeSummary
{
	
	CdbExplain_Agg ntuples;
	CdbExplain_Agg execmemused;
	CdbExplain_Agg workmemused;
	CdbExplain_Agg workmemwanted;
	CdbExplain_Agg totalWorkfileCreated;
	CdbExplain_Agg peakMemBalance;
	
	CdbExplain_Agg totalPartTableScanned;
	
	CdbExplain_Agg sortSpaceUsed[NUM_SORT_SPACE_TYPE][NUM_SORT_METHOD];

	
	int			segindex0;		
	int			ninst;			

	
	CdbExplain_StatInst insts[1];	
} CdbExplain_NodeSummary;



typedef struct CdbExplain_SliceSummary
{
	Slice	   *slice;

	
	int			nworker;		
	int			segindex0;		
	CdbExplain_SliceWorker *workers;	

	
	void	  **memoryAccounts; 
	MemoryAccountIdType *memoryAccountCount;	

	CdbExplain_Agg peakmemused; 

	CdbExplain_Agg vmem_reserved;	

	CdbExplain_Agg memory_accounting_global_peak;	

	
	double		workmemused_max;
	double		workmemwanted_max;

	
	CdbExplain_DispatchSummary dispatchSummary;
} CdbExplain_SliceSummary;



typedef struct CdbExplain_ShowStatCtx
{
	StringInfoData extratextbuf;
	instr_time	querystarttime;

	
	double		workmemused_max;
	double		workmemwanted_max;

	
	int			nslice;			
	CdbExplain_SliceSummary *slices;	
} CdbExplain_ShowStatCtx;



typedef struct CdbExplain_SendStatCtx
{
	StringInfoData *notebuf;
	StringInfoData buf;
	CdbExplain_StatHdr hdr;
} CdbExplain_SendStatCtx;



typedef struct CdbExplain_RecvStatCtx
{
	
	int			iStatInst;

	
	int			nStatInst;

	
	int			segindexMin;

	
	int			segindexMax;

	
	int			sliceIndex;

	
	int			nmsgptr;
	
	CdbExplain_StatHdr **msgptrs;
	CdbDispatchResults *dispatchResults;
	StringInfoData *extratextbuf;
	CdbExplain_ShowStatCtx *showstatctx;

	
	double		workmemused_max;
	double		workmemwanted_max;
} CdbExplain_RecvStatCtx;



typedef struct CdbExplain_LocalStatCtx
{
	CdbExplain_SendStatCtx send;
	CdbExplain_RecvStatCtx recv;
	CdbExplain_StatHdr *msgptrs[1];
} CdbExplain_LocalStatCtx;


static CdbVisitOpt
			cdbexplain_localStatWalker(PlanState *planstate, void *context);
static CdbVisitOpt
			cdbexplain_sendStatWalker(PlanState *planstate, void *context);
static CdbVisitOpt
			cdbexplain_recvStatWalker(PlanState *planstate, void *context);

static void cdbexplain_collectSliceStats(PlanState *planstate,
							 CdbExplain_SliceWorker *out_worker);
static void cdbexplain_depositSliceStats(CdbExplain_StatHdr *hdr,
							 CdbExplain_RecvStatCtx *recvstatctx);
static void
			cdbexplain_collectStatsFromNode(PlanState *planstate, CdbExplain_SendStatCtx *ctx);
static void
			cdbexplain_depositStatsToNode(PlanState *planstate, CdbExplain_RecvStatCtx *ctx);
static int
			cdbexplain_collectExtraText(PlanState *planstate, StringInfo notebuf);
static int
			cdbexplain_countLeafPartTables(PlanState *planstate);



static ExplainSortMethod
String2ExplainSortMethod(const char * sortMethod)
{
	return (ExplainSortMethod) mock();
}


static ExplainSortSpaceType
String2ExplainSortSpaceType(const char * sortSpaceType, ExplainSortMethod sortMethod)
{
	return (ExplainSortSpaceType) mock();
}


void
cdbexplain_localExecStats(struct PlanState * planstate, struct CdbExplain_ShowStatCtx * showstatctx)
{
	check_expected(planstate);
	check_expected(showstatctx);
	optional_assignment(planstate);
	optional_assignment(showstatctx);
	mock();
}


CdbVisitOpt
cdbexplain_localStatWalker(PlanState * planstate, void * context)
{
	check_expected(planstate);
	check_expected(context);
	optional_assignment(planstate);
	optional_assignment(context);
	return (CdbVisitOpt) mock();
}


void
cdbexplain_sendExecStats(QueryDesc * queryDesc)
{
	check_expected(queryDesc);
	optional_assignment(queryDesc);
	mock();
}


CdbVisitOpt
cdbexplain_sendStatWalker(PlanState * planstate, void * context)
{
	check_expected(planstate);
	check_expected(context);
	optional_assignment(planstate);
	optional_assignment(context);
	return (CdbVisitOpt) mock();
}


void
cdbexplain_recvExecStats(struct PlanState * planstate, struct CdbDispatchResults * dispatchResults, int sliceIndex, struct CdbExplain_ShowStatCtx * showstatctx)
{
	check_expected(planstate);
	check_expected(dispatchResults);
	check_expected(sliceIndex);
	check_expected(showstatctx);
	optional_assignment(planstate);
	optional_assignment(dispatchResults);
	optional_assignment(showstatctx);
	mock();
}


CdbVisitOpt
cdbexplain_recvStatWalker(PlanState * planstate, void * context)
{
	check_expected(planstate);
	check_expected(context);
	optional_assignment(planstate);
	optional_assignment(context);
	return (CdbVisitOpt) mock();
}

void
ExplainOnePlan(PlannedStmt *plannedstmt, ExplainState *es,
			   const char *queryString, ParamListInfo params)
{
	mock();
}

void
ExplainOneUtility(Node *utilityStmt, ExplainState *es,
				  const char *queryString, ParamListInfo params)
{
	mock();
}

void
ExplainQuery(ExplainStmt *stmt, const char *queryString,
			 ParamListInfo params, DestReceiver *dest)
{
	mock();
}

TupleDesc
ExplainResultDesc(ExplainStmt *stmt)
{
	return (TupleDesc) mock();
}

void
ExplainSeparatePlans(ExplainState *es)
{
	mock();
}

static void
cdbexplain_collectSliceStats(PlanState * planstate, CdbExplain_SliceWorker * out_worker)
{
	mock();
}


static void
cdbexplain_depositSliceStats(CdbExplain_StatHdr * hdr, CdbExplain_RecvStatCtx * recvstatctx)
{
	mock();
}


static void
cdbexplain_collectStatsFromNode(PlanState * planstate, CdbExplain_SendStatCtx * ctx)
{
	mock();
}

typedef struct CdbExplain_DepStatAcc
{
	
	CdbExplain_Agg agg;
	
	CdbExplain_StatHdr *rshmax;
	
	CdbExplain_StatInst *rsimax;
	
	CdbExplain_StatInst *nsimax;
	
	double		max_total;
	
	instr_time	firststart_of_max_total;
} CdbExplain_DepStatAcc;


static void
cdbexplain_depStatAcc_init0(CdbExplain_DepStatAcc * acc)
{
	mock();
}


inline void
cdbexplain_depStatAcc_upd(CdbExplain_DepStatAcc * acc, double v, CdbExplain_StatHdr * rsh, CdbExplain_StatInst * rsi, CdbExplain_StatInst * nsi)
{
	mock();
}


static void
cdbexplain_depStatAcc_saveText(CdbExplain_DepStatAcc * acc, StringInfoData * extratextbuf, bool * saved_inout)
{
	mock();
}


static void
cdbexplain_depositStatsToNode(PlanState * planstate, CdbExplain_RecvStatCtx * ctx)
{
	mock();
}


int
cdbexplain_collectExtraText(PlanState * planstate, StringInfo notebuf)
{
	check_expected(planstate);
	check_expected(notebuf);
	optional_assignment(planstate);
	return (int) mock();
}


static void
cdbexplain_formatExtraText(StringInfo str, int indent, int segindex, const char * notes, int notelen)
{
	mock();
}


static void
cdbexplain_formatMemory(char * outbuf, int bufsize, double bytes)
{
	mock();
}


static void
cdbexplain_formatSeconds(char * outbuf, int bufsize, double seconds)
{
	mock();
}


static void
cdbexplain_formatSeg(char * outbuf, int bufsize, int segindex, int nInst)
{
	mock();
}


struct CdbExplain_ShowStatCtx *
cdbexplain_showExecStatsBegin(struct QueryDesc * queryDesc, instr_time querystarttime)
{
	check_expected(queryDesc);
	check_expected(&querystarttime);
	optional_assignment(queryDesc);
	return (struct CdbExplain_ShowStatCtx *) mock();
}


static bool
nodeSupportWorkfileCaching(PlanState * planstate)
{
	return (bool) mock();
}


static void
show_cumulative_sort_info(struct StringInfoData * str, int indent, const char * sort_method, const char * sort_space_type, CdbExplain_Agg * agg)
{
	mock();
}


void
cdbexplain_showExecStats(struct PlanState * planstate, struct StringInfoData * str, int indent, struct CdbExplain_ShowStatCtx * ctx)
{
	check_expected(planstate);
	check_expected(str);
	check_expected(indent);
	check_expected(ctx);
	optional_assignment(planstate);
	optional_assignment(str);
	optional_assignment(ctx);
	mock();
}


void
cdbexplain_showExecStatsEnd(struct PlannedStmt * stmt, struct CdbExplain_ShowStatCtx * showstatctx, struct StringInfoData * str, struct EState * estate)
{
	check_expected(stmt);
	check_expected(showstatctx);
	check_expected(str);
	check_expected(estate);
	optional_assignment(stmt);
	optional_assignment(showstatctx);
	optional_assignment(str);
	optional_assignment(estate);
	mock();
}


static int
cdbexplain_countLeafPartTables(PlanState * planstate)
{
	return (int) mock();
}
