/*-------------------------------------------------------------------------
 *
 * cdbgroupingpaths.h
 *	  prototypes for cdbgroupingpaths.c.
 *
 *
 * Portions Copyright (c) 2019-Present Pivotal Software, Inc.
 *
 * IDENTIFICATION
 *	    src/include/cdb/cdbgroupingpaths.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef CDBGROUPINGPATHS_H
#define CDBGROUPINGPATHS_H

#include "nodes/relation.h"

extern void cdb_create_grouping_paths(PlannerInfo *root,
									  RelOptInfo *input_rel,
									  RelOptInfo *output_rel,
									  PathTarget *target,
									  PathTarget *partial_grouping_target,
									  const AggClauseCosts *agg_costs,
									  const AggClauseCosts *agg_partial_costs,
									  const AggClauseCosts *agg_final_costs);

#endif   /* CDBGROUPINGPATHS_H */
