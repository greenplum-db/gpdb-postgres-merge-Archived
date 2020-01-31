/*-------------------------------------------------------------------------
 *
 * pg_class.h
 *	  definition of the "relation" system catalog (pg_class)
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_class.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_CLASS_H
#define PG_CLASS_H

#include "catalog/genbki.h"
#include "catalog/pg_class_d.h"

/* ----------------
 *		postgres.h contains the system type definitions and the
 *		CATALOG(), BKI_BOOTSTRAP and DATA() sugar words so this file
 *		can be read by both genbki.sh and the C compiler.
 * ----------------
 */
CATALOG(pg_class,1259,RelationRelationId) BKI_BOOTSTRAP BKI_ROWTYPE_OID(83,RelationRelation_Rowtype_Id) BKI_SCHEMA_MACRO
{
	/* oid */
	Oid			oid;

	/* class name */
	NameData	relname;

	/* OID of namespace containing this class */
	Oid			relnamespace BKI_DEFAULT(PGNSP);

	/* OID of entry in pg_type for table's implicit row type */
	Oid			reltype BKI_LOOKUP(pg_type);

	/* OID of entry in pg_type for underlying composite type */
	Oid			reloftype BKI_DEFAULT(0) BKI_LOOKUP(pg_type);

	/* class owner */
	Oid			relowner BKI_DEFAULT(PGUID);

	/* access method; 0 if not a table / index */
	Oid			relam BKI_LOOKUP(pg_am);

	/* identifier of physical storage file */
	/* relfilenode == 0 means it is a "mapped" relation, see relmapper.c */
<<<<<<< HEAD
	Oid			reltablespace;	/* identifier of table space for relation */
	int32		relpages;		/* # of blocks (not always up-to-date) */
	float4		reltuples;		/* # of tuples (not always up-to-date) */
	int32		relallvisible;	/* # of all-visible blocks (not always
								 * up-to-date) */
	Oid			reltoastrelid;	/* OID of toast table; 0 if none */
	bool		relhasindex;	/* T if has (or has had) any indexes */
	bool		relisshared;	/* T if shared across databases */
	char		relpersistence; /* see RELPERSISTENCE_xxx constants below */
	char		relkind;		/* see RELKIND_xxx constants below */
	char		relstorage;		/* see RELSTORAGE_xxx constants below */
	int16		relnatts;		/* number of user attributes */
=======
	Oid			relfilenode;

	/* identifier of table space for relation (0 means default for database) */
	Oid			reltablespace BKI_DEFAULT(0) BKI_LOOKUP(pg_tablespace);

	/* # of blocks (not always up-to-date) */
	int32		relpages;

	/* # of tuples (not always up-to-date) */
	float4		reltuples;

	/* # of all-visible blocks (not always up-to-date) */
	int32		relallvisible;

	/* OID of toast table; 0 if none */
	Oid			reltoastrelid;

	/* T if has (or has had) any indexes */
	bool		relhasindex;

	/* T if shared across databases */
	bool		relisshared;

	/* see RELPERSISTENCE_xxx constants below */
	char		relpersistence;

	/* see RELKIND_xxx constants below */
	char		relkind;

	/* number of user attributes */
	int16		relnatts;
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

	/*
	 * Class pg_attribute must contain exactly "relnatts" user attributes
	 * (with attnums ranging from 1 to relnatts) for this class.  It may also
	 * contain entries with negative attnums for system attributes.
	 */

	/* # of CHECK constraints for class */
	int16		relchecks;

	/* has (or has had) any rules */
	bool		relhasrules;

	/* has (or has had) any TRIGGERs */
	bool		relhastriggers;

	/* has (or has had) child tables or indexes */
	bool		relhassubclass;

	/* row security is enabled or not */
	bool		relrowsecurity;

	/* row security forced for owners or not */
	bool		relforcerowsecurity;

	/* matview currently holds query results */
	bool		relispopulated;

	/* see REPLICA_IDENTITY_xxx constants */
	char		relreplident;

	/* is relation a partition? */
	bool		relispartition;

	/* heap for rewrite during DDL, link to original rel */
	Oid			relrewrite BKI_DEFAULT(0);

	/* all Xids < this are frozen in this rel */
	TransactionId relfrozenxid;

	/* all multixacts in this rel are >= this; it is really a MultiXactId */
	TransactionId relminmxid;

#ifdef CATALOG_VARLEN			/* variable-length fields start here */
	/* NOTE: These fields are not present in a relcache entry's rd_rel field. */
	/* access permissions */
	aclitem		relacl[1];

	/* access-method-specific options */
	text		reloptions[1];

	/* partition bound node tree */
	pg_node_tree relpartbound;
#endif
} FormData_pg_class;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(relnamespace REFERENCES pg_namespace(oid));
FOREIGN_KEY(reltype REFERENCES pg_type(oid));
FOREIGN_KEY(relowner REFERENCES pg_authid(oid));
FOREIGN_KEY(relam REFERENCES pg_am(oid));
FOREIGN_KEY(reltablespace REFERENCES pg_tablespace(oid));
FOREIGN_KEY(reltoastrelid REFERENCES pg_class(oid));

/* Size of fixed part of pg_class tuples, not counting var-length fields */
#define CLASS_TUPLE_SIZE \
	 (offsetof(FormData_pg_class,relminmxid) + sizeof(TransactionId))

/* ----------------
 *		Form_pg_class corresponds to a pointer to a tuple with
 *		the format of pg_class relation.
 * ----------------
 */
typedef FormData_pg_class *Form_pg_class;

#ifdef EXPOSE_TO_CLIENT_CODE

<<<<<<< HEAD
#define Natts_pg_class					32
#define Anum_pg_class_relname			1
#define Anum_pg_class_relnamespace		2
#define Anum_pg_class_reltype			3
#define Anum_pg_class_reloftype			4
#define Anum_pg_class_relowner			5
#define Anum_pg_class_relam				6
#define Anum_pg_class_relfilenode		7
#define Anum_pg_class_reltablespace		8
#define Anum_pg_class_relpages			9
#define Anum_pg_class_reltuples			10
#define Anum_pg_class_relallvisible		11
#define Anum_pg_class_reltoastrelid		12
#define Anum_pg_class_relhasindex		13
#define Anum_pg_class_relisshared		14
#define Anum_pg_class_relpersistence	15
#define Anum_pg_class_relkind			16
#define Anum_pg_class_relstorage		17 /* GPDB specific */
GPDB_COLUMN_DEFAULT(relstorage, h);
#define Anum_pg_class_relnatts			18
#define Anum_pg_class_relchecks			19
#define Anum_pg_class_relhasoids		20
#define Anum_pg_class_relhaspkey		21
#define Anum_pg_class_relhasrules		22
#define Anum_pg_class_relhastriggers	23
#define Anum_pg_class_relhassubclass	24
#define Anum_pg_class_relrowsecurity	25
#define Anum_pg_class_relforcerowsecurity	26
#define Anum_pg_class_relispopulated	27
#define Anum_pg_class_relreplident		28
#define Anum_pg_class_relfrozenxid		29
#define Anum_pg_class_relminmxid		30
#define Anum_pg_class_relacl			31
#define Anum_pg_class_reloptions		32

/* ----------------
 *		initial contents of pg_class
 *
 * NOTE: only "bootstrapped" relations need to be declared here.  Be sure that
 * the OIDs listed here match those given in their CATALOG macros, and that
 * the relnatts values are correct.
 * ----------------
 */

/*
 * Note: "3" in the relfrozenxid column stands for FirstNormalTransactionId;
 * similarly, "1" in relminmxid stands for FirstMultiXactId
 */
DATA(insert OID = 1247 (  pg_type		PGNSP 71 0 PGUID 0 0 0 0 0 0 0 f f p r 30 0 t f f f f f f t n 3 1 _null_ _null_ ));
DESCR("");
DATA(insert OID = 1249 (  pg_attribute	PGNSP 75 0 PGUID 0 0 0 0 0 0 0 f f p r 21 0 f f f f f f f t n 3 1 _null_ _null_ ));
DESCR("");
DATA(insert OID = 1255 (  pg_proc		PGNSP 81 0 PGUID 0 0 0 0 0 0 0 f f p r 31 0 t f f f f f f t n 3 1 _null_ _null_ ));
DESCR("");
DATA(insert OID = 1259 (  pg_class		PGNSP 83 0 PGUID 0 0 0 0 0 0 0 f f p r 32 0 t f f f f f f t n 3 1 _null_ _null_ ));
DESCR("");


#define		  RELKIND_RELATION		  'r'		/* ordinary table */
#define		  RELKIND_INDEX			  'i'		/* secondary index */
#define		  RELKIND_SEQUENCE		  'S'		/* sequence object */
#define		  RELKIND_TOASTVALUE	  't'		/* for out-of-line values */
#define		  RELKIND_VIEW			  'v'		/* view */
#define		  RELKIND_COMPOSITE_TYPE  'c'		/* composite type */
#define		  RELKIND_FOREIGN_TABLE   'f'		/* foreign table */
#define		  RELKIND_UNCATALOGED	  'u'		/* not yet cataloged */
#define		  RELKIND_MATVIEW		  'm'		/* materialized view */
#define		  RELKIND_AOSEGMENTS	  'o'		/* AO segment files and eof's */
#define		  RELKIND_AOBLOCKDIR	  'b'		/* AO block directory */
#define		  RELKIND_AOVISIMAP		  'M'		/* AO visibility map */

#define		  RELPERSISTENCE_PERMANENT	'p'		/* regular table */
#define		  RELPERSISTENCE_UNLOGGED	'u'		/* unlogged permanent table */
#define		  RELPERSISTENCE_TEMP		't'		/* temporary table */
=======
#define		  RELKIND_RELATION		  'r'	/* ordinary table */
#define		  RELKIND_INDEX			  'i'	/* secondary index */
#define		  RELKIND_SEQUENCE		  'S'	/* sequence object */
#define		  RELKIND_TOASTVALUE	  't'	/* for out-of-line values */
#define		  RELKIND_VIEW			  'v'	/* view */
#define		  RELKIND_MATVIEW		  'm'	/* materialized view */
#define		  RELKIND_COMPOSITE_TYPE  'c'	/* composite type */
#define		  RELKIND_FOREIGN_TABLE   'f'	/* foreign table */
#define		  RELKIND_PARTITIONED_TABLE 'p' /* partitioned table */
#define		  RELKIND_PARTITIONED_INDEX 'I' /* partitioned index */

#define		  RELPERSISTENCE_PERMANENT	'p' /* regular table */
#define		  RELPERSISTENCE_UNLOGGED	'u' /* unlogged permanent table */
#define		  RELPERSISTENCE_TEMP		't' /* temporary table */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/* default selection for replica identity (primary key or nothing) */
#define		  REPLICA_IDENTITY_DEFAULT	'd'
/* no replica identity is logged for this relation */
#define		  REPLICA_IDENTITY_NOTHING	'n'
/* all columns are logged as replica identity */
#define		  REPLICA_IDENTITY_FULL		'f'
/*
 * an explicitly chosen candidate key's columns are used as replica identity.
 * Note this will still be set if the index has been dropped; in that case it
 * has the same meaning as 'd'.
 */
#define		  REPLICA_IDENTITY_INDEX	'i'

/*
<<<<<<< HEAD
 * relstorage describes how a relkind is physically stored in the database.
 *
 * RELSTORAGE_HEAP    - stored on disk using heap storage.
 * RELSTORAGE_AOROWS  - stored on disk using append only storage.
 * RELSTORAGE_AOCOLS  - stored on dist using append only column storage.
 * RELSTORAGE_VIRTUAL - has virtual storage, meaning, relation has no
 *						data directly stored forit  (right now this
 *						relates to views and comp types).
 * RELSTORAGE_FOREIGN - stored in another server.  
 *
 * GPDB 6.x and below used RELSTORAGE_EXTERNAL ('x') for external tables.
 * Now they look like foreign tables.
 */
#define		  RELSTORAGE_HEAP	'h'
#define		  RELSTORAGE_AOROWS	'a'
#define 	  RELSTORAGE_AOCOLS	'c'
#define		  RELSTORAGE_VIRTUAL	'v'
#define		  RELSTORAGE_FOREIGN 'f'

static inline bool relstorage_is_heap(char c)
{
	return (c == RELSTORAGE_HEAP);
}

static inline bool relstorage_is_ao(char c)
{
	return (c == RELSTORAGE_AOROWS || c == RELSTORAGE_AOCOLS);
}

#endif   /* PG_CLASS_H */
=======
 * Relation kinds that have physical storage. These relations normally have
 * relfilenode set to non-zero, but it can also be zero if the relation is
 * mapped.
 */
#define RELKIND_HAS_STORAGE(relkind) \
	((relkind) == RELKIND_RELATION || \
	 (relkind) == RELKIND_INDEX || \
	 (relkind) == RELKIND_SEQUENCE || \
	 (relkind) == RELKIND_TOASTVALUE || \
	 (relkind) == RELKIND_MATVIEW)


#endif							/* EXPOSE_TO_CLIENT_CODE */

#endif							/* PG_CLASS_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
