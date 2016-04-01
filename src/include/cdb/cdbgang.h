/*-------------------------------------------------------------------------
 *
 * cdbgang.h
 *
 * Copyright (c) 2005-2008, Greenplum inc
 *
 *-------------------------------------------------------------------------
 */
#ifndef _CDBGANG_H_
#define _CDBGANG_H_

#include "cdb/cdbutil.h"
#include "executor/execdesc.h"
#include <pthread.h>

struct Port;                    /* #include "libpq/libpq-be.h" */
struct QueryDesc;               /* #include "executor/execdesc.h" */

/*
 * A gang represents a single worker on each connected segDB
 *
 */

typedef struct Gang
{
	GangType	type;
	int			gang_id;
	int			size;			/* segment_count or segdb_count ? */

	/* MPP-6253: on *writer* gangs keep track of dispatcher use
	 * (reader gangs already track this properly, since they get
	 * allocated from a list of available gangs.*/
	bool		dispatcherActive; 

	/* the named portal that owns this gang, NULL if none */
	char		*portal_name;

	/* Array of QEs/segDBs that make up this gang */
	struct SegmentDatabaseDescriptor *db_descriptors;	

	/* For debugging purposes only. These do not add any actual functionality. */
	bool		active;
	bool		all_valid_segdbs_connected;
	bool		allocated;

	/* should be destroyed in cleanupGang() if set*/
	bool		noReuse;

	/* MPP-24003: pointer to array of segment database info for each reader and writer gang. */
	struct		CdbComponentDatabaseInfo *segment_database_info;
} Gang;

extern Gang *allocateGang(GangType type, int size, int content, char *portal_name);
extern Gang *allocateWriterGang(void);

struct DirectDispatchInfo;
extern List *getCdbProcessList(Gang *gang, int sliceIndex, struct DirectDispatchInfo *directDispatch);

extern bool gangOK(Gang *gp);

extern List *getCdbProcessesForQD(int isPrimary);

extern void freeGangsForPortal(char *portal_name);

extern void disconnectAndDestroyAllGangs(void);

extern void CheckForResetSession(void);

extern List * getAllReaderGangs(void);

extern List * getAllIdleReaderGangs(void);

extern List * getAllBusyReaderGangs(void);

extern void detectFailedConnections(void);

extern CdbComponentDatabases *getComponentDatabases(void);

extern bool gangsExist(void);

struct SegmentDatabaseDescriptor *getSegmentDescriptorFromGang(const Gang *gp, int seg);

extern Gang *findGangById(int gang_id);


/*
 * cleanupIdleReaderGangs() and cleanupAllIdleGangs().
 *
 * These two routines are used when a session has been idle for a while (waiting for the
 * client to send us SQL to execute).  The idea is to consume less resources while sitting idle.
 *
 * Only call these from an idle session.
 */
extern void cleanupIdleReaderGangs(void);

extern void cleanupAllIdleGangs(void);

extern void cleanupPortalGangs(Portal portal);

extern int largestGangsize(void);

extern int gp_pthread_create(pthread_t *thread, void *(*start_routine)(void *), void *arg, const char *caller);
/*
 * cdbgang_parse_gpqeid_params
 *
 * Called very early in backend initialization, to interpret the "gpqeid"
 * parameter value that a qExec receives from its qDisp.
 *
 * At this point, client authentication has not been done; the backend
 * command line options have not been processed; GUCs have the settings
 * inherited from the postmaster; etc; so don't try to do too much in here.
 */
void
cdbgang_parse_gpqeid_params(struct Port *port, const char* gpqeid_value);

void
cdbgang_parse_gpqdid_params(struct Port *port, const char* gpqdid_value);

void
cdbgang_parse_gpdaid_params(struct Port *port, const char* gpdaid_value);

/* ----------------
 * MPP Worker Process information
 *
 * This structure represents the global information about a worker process.
 * It is constructed on the entry process (QD) and transmitted as part of
 * the global slice table to the involved QEs.  Note that this is an
 * immutable, fixed-size structure so it can be held in a contiguous
 * array. In the Slice node, however, it is held in a List.
 * ----------------
 */
typedef struct CdbProcess
{

	NodeTag type;

	/* These fields are established at connection (libpq) time and are
	 * available to the QD in PGconn structure associated with connected
	 * QE.  It needs to be explicitly transmitted to QE's,  however,
	 */

	char *listenerAddr; /* Interconnect listener IPv4 address, a C-string */

	int listenerPort; /* Interconnect listener port */
	int pid; /* Backend PID of the process. */

	/* Unclear about why we need these, however, it is no trouble to carry
	 * them.
	 */

	int contentid;
} CdbProcess;

struct EState;

extern void InitSliceTable(struct EState *estate, int nMotions, int nSubplans);
extern Slice *getCurrentSlice(struct EState* estate, int sliceIndex);
extern bool sliceRunsOnQD(Slice *slice);
extern bool sliceRunsOnQE(Slice *slice);
extern int sliceCalculateNumSendingProcesses(Slice *slice, int numSegmentsInCluster);

extern void InitRootSlices(QueryDesc *queryDesc);
extern void AssignGangs(QueryDesc *queryDesc, int utility_segment_index);
extern void ReleaseGangs(QueryDesc *queryDesc);

#ifdef USE_ASSERT_CHECKING
struct PlannedStmt;

extern void AssertSliceTableIsValid(SliceTable *st, struct PlannedStmt *pstmt);
#endif

#endif   /* _CDBGANG_H_ */
