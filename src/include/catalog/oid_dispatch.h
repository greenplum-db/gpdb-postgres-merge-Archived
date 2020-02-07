/*-------------------------------------------------------------------------
 *
 * oid_dispatch.h
 *	  prototypes for functions in backend/catalog/oid_dispatch.c
 *
 *
 * Portions Copyright 2016 Pivotal Software, Inc.
 *
 * src/include/catalog/oid_dispatch.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef OID_DISPATCH_H
#define OID_DISPATCH_H

#include "utils/relcache.h"
#include "access/htup.h"

/* Functions used in master */
extern List *GetAssignedOidsForDispatch(void);

/* Functions used in QE nodes */
extern void AddPreassignedOids(List *l);
extern void AddPreassignedOidFromBinaryUpgrade(Oid oid, Oid catalog,
			char *objname, Oid namespaceOid, Oid keyOid1, Oid keyOid2);

/* Wrapper functions for GetNewOidWithIndex(), used in QD and QE nodes */
extern Oid GetNewOidForAccessMethodRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											  char *amname);
extern Oid GetNewOidForAccessMethodOperatorRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
													  Oid amopfamily, Oid amoplefttype, Oid amoprighttype,
													  Oid amopstrategy);
extern Oid GetNewOidForAttrDefaultRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											 Oid adrelid, int16 adnum);
extern Oid GetNewOidForAuthIdRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										char *rolname);
extern Oid GetNewOidForCastRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
									  Oid castsource, Oid casttarget);
extern Oid GetNewOidForCollationRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										   Oid collnamespace, char *collname);
extern Oid GetNewOidForConstraintRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											Oid conrelid, Oid contypid, char *conname);
extern Oid GetNewOidForConversionRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											Oid connamespace, char *conname);
extern Oid GetNewOidForDatabaseRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *datname);
extern Oid GetNewOidForDefaultAclRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											Oid defaclrole, Oid defaclnamespace, char defaclobjtype);
extern Oid GetNewOidForEnumRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
									  Oid enumtypid, char *enumlabel);
extern Oid GetNewOidForExtensionRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										   char *extname);
extern Oid GetNewOidForExtprotocolRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											 char *ptcname);
extern Oid GetNewOidForForeignDataWrapperRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
													char *fdwname);
extern Oid GetNewOidForForeignServerRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											   char *srvname);
extern Oid GetNewOidForLanguageRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *lanname);
extern Oid GetNewOidForNamespaceRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										   char *nspname);
extern Oid GetNewOidForOperatorRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *oprname, Oid oprleft, Oid oprright, Oid oprnamespace);
extern Oid GetNewOidForOperatorClassRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											   Oid opcmethod, char *opcname, Oid opcnamespace);
extern Oid GetNewOidForOperatorFamilyRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
												Oid opfmethod, char *opfname, Oid opfnamespace);
extern Oid GetNewOidForPolicyRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										Oid polrelid, char *polname);
extern Oid GetNewOidForProcedureRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										   char *proname, oidvector proargtypes, Oid pronamespace);
extern Oid GetNewOidForRelationRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *relname, Oid relnamespace);
extern Oid GetNewOidForResQueueRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *rsqname);
extern Oid GetNewOidForRewriteRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										 Oid ev_class, char *rulename);
extern Oid GetNewOidForTableSpaceRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											char *spcname);
extern Oid GetNewOidForTransformRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										   Oid trftype, Oid trflang);
extern Oid GetNewOidForTSParserRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *prsname, Oid prsnamespace);
extern Oid GetNewOidForTSDictionaryRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											  char *dictname, Oid dictnamespace);
extern Oid GetNewOidForTSTemplateRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											char *tmplname, Oid tmplnamespace);
extern Oid GetNewOidForTSConfigRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *cfgname, Oid cfgnamespace);
extern Oid GetNewOidForTypeRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
									  char *typname, Oid typnamespace);
extern Oid GetNewOidForResGroupRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
										  char *rsgname);
extern Oid GetNewOidForUserMappingRelationId(Relation relation, Oid indexiId, AttrNumber oidcolumn,
											 Oid umuser, Oid umserver);
extern Oid GetNewOidForPublication(Relation relation, Oid indexiId, AttrNumber oidcolumn,
								   char *pubname);
extern Oid GetNewOidForPublicationRel(Relation relation, Oid indexiId, AttrNumber oidcolumn,
									  Oid prrelid, Oid prpubid);



/* Functions used in binary upgrade */
extern bool IsOidAcceptable(Oid oid);
extern void MarkOidPreassignedFromBinaryUpgrade(Oid oid);

extern void AtEOXact_DispatchOids(bool isCommit);

#endif   /* OID_DISPATCH_H */
