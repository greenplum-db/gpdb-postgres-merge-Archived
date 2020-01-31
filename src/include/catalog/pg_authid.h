/*-------------------------------------------------------------------------
 *
 * pg_authid.h
 *	  definition of the "authorization identifier" system catalog (pg_authid)
 *
 *	  pg_shadow and pg_group are now publicly accessible views on pg_authid.
 *
 *
 * Portions Copyright (c) 2006-2010, Greenplum inc.
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_authid.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_AUTHID_H
#define PG_AUTHID_H

#include "catalog/genbki.h"
#include "catalog/pg_authid_d.h"

/* ----------------
 *		pg_authid definition.  cpp turns this into
 *		typedef struct FormData_pg_authid
 * ----------------
 */
CATALOG(pg_authid,1260,AuthIdRelationId) BKI_SHARED_RELATION BKI_ROWTYPE_OID(2842,AuthIdRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
	Oid			oid;			/* oid */
	NameData	rolname;		/* name of role */
	bool		rolsuper;		/* read this field via superuser() only! */
	bool		rolinherit;		/* inherit privileges from other roles? */
	bool		rolcreaterole;	/* allowed to create more roles? */
	bool		rolcreatedb;	/* allowed to create databases? */
	bool		rolcanlogin;	/* allowed to log in as session user? */
	bool		rolreplication; /* role used for streaming replication */
	bool		rolbypassrls;	/* bypasses row level security? */
	int32		rolconnlimit;	/* max connections allowed (-1=no limit) */

	/* remaining fields may be null; use heap_getattr to read them! */
#ifdef CATALOG_VARLEN			/* variable-length fields start here */
	text		rolpassword;	/* password, if any */
	timestamptz rolvaliduntil;	/* password expiration time, if any */

	/* GP added fields */
	Oid			rolresqueue;	/* ID of resource queue for this role */
	bool		rolcreaterextgpfd;	/* allowed to create readable gpfdist tbl?  */
	bool		rolcreaterexthttp;	/* allowed to create readable http tbl?  */
	bool		rolcreatewextgpfd;	/* allowed to create writable gpfdist tbl?  */
	Oid			rolresgroup;		/* ID of resource group for this role  */
#endif
} FormData_pg_authid;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(rolresqueue REFERENCES pg_resqueue(oid));
FOREIGN_KEY(rolresgroup REFERENCES pg_resgroup(oid));

/* ----------------
 *		Form_pg_authid corresponds to a pointer to a tuple with
 *		the format of pg_authid relation.
 * ----------------
 */
typedef FormData_pg_authid *Form_pg_authid;

<<<<<<< HEAD

/* ----------------
 *		compiler constants for pg_authid
 * ----------------
 */
#define Natts_pg_authid						16
#define Anum_pg_authid_rolname				1
#define Anum_pg_authid_rolsuper				2
#define Anum_pg_authid_rolinherit			3
#define Anum_pg_authid_rolcreaterole		4
#define Anum_pg_authid_rolcreatedb			5
#define Anum_pg_authid_rolcanlogin			6
#define Anum_pg_authid_rolreplication		7
#define Anum_pg_authid_rolbypassrls			8
#define Anum_pg_authid_rolconnlimit			9
#define Anum_pg_authid_rolpassword			10
#define Anum_pg_authid_rolvaliduntil		11
#define Anum_pg_authid_rolresqueue			12
#define Anum_pg_authid_rolcreaterextgpfd	13
#define Anum_pg_authid_rolcreaterexthttp	14
#define Anum_pg_authid_rolcreatewextgpfd	15
#define Anum_pg_authid_rolresgroup			16

/* ----------------
 *		initial contents of pg_authid
 *
 * The uppercase quantities will be replaced at initdb time with
 * user choices.
 *
 * If adding new default roles or changing the OIDs below, be sure to add or
 * update the #defines which follow as appropriate.
 *
 * add default queue DEFAULTRESQUEUE_OID 6055
 * add default group ADMINRESGROUP_OID 6438
 * ----------------
 */
DATA(insert OID = 10 ( "POSTGRES" t t t t t t t -1 _null_ _null_ 6055 t t t 6438 ));
DATA(insert OID = 4200 ( "pg_signal_backend" f t f f f f f -1 _null_ _null_ 6055 t t t 6438 ));

#define BOOTSTRAP_SUPERUSERID			10

#define DEFAULT_ROLE_SIGNAL_BACKENDID	4200

#endif   /* PG_AUTHID_H */
=======
#endif							/* PG_AUTHID_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
