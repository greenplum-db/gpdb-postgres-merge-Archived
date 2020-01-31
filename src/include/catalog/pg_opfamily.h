/*-------------------------------------------------------------------------
 *
 * pg_opfamily.h
 *	  definition of the "operator family" system catalog (pg_opfamily)
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_opfamily.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_OPFAMILY_H
#define PG_OPFAMILY_H

#include "catalog/genbki.h"
#include "catalog/pg_opfamily_d.h"

/* ----------------
 *		pg_opfamily definition. cpp turns this into
 *		typedef struct FormData_pg_opfamily
 * ----------------
 */
CATALOG(pg_opfamily,2753,OperatorFamilyRelationId)
{
	Oid			oid;			/* oid */

	/* index access method opfamily is for */
	Oid			opfmethod BKI_LOOKUP(pg_am);

	/* name of this opfamily */
	NameData	opfname;

	/* namespace of this opfamily */
	Oid			opfnamespace BKI_DEFAULT(PGNSP);

	/* opfamily owner */
	Oid			opfowner BKI_DEFAULT(PGUID);
} FormData_pg_opfamily;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(opfmethod REFERENCES pg_am(oid));
FOREIGN_KEY(opfnamespace REFERENCES pg_namespace(oid));
FOREIGN_KEY(opfowner REFERENCES pg_authid(oid));

/* ----------------
 *		Form_pg_opfamily corresponds to a pointer to a tuple with
 *		the format of pg_opfamily relation.
 * ----------------
 */
typedef FormData_pg_opfamily *Form_pg_opfamily;

#ifdef EXPOSE_TO_CLIENT_CODE

#define IsBooleanOpfamily(opfamily) \
	((opfamily) == BOOL_BTREE_FAM_OID || (opfamily) == BOOL_HASH_FAM_OID)

#endif							/* EXPOSE_TO_CLIENT_CODE */

<<<<<<< HEAD
DATA(insert OID = 4054 (	3580	integer_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4055 (	3580	numeric_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4056 (	3580	text_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4058 (	3580	timetz_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4059 (	3580	datetime_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4062 (	3580	char_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4064 (	3580	bytea_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4065 (	3580	name_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4068 (	3580	oid_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4069 (	3580	tid_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4070 (	3580	float_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4072 (	3580	abstime_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4073 (	3580	reltime_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4074 (	3580	macaddr_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4075 (	3580	network_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4102 (	3580	network_inclusion_ops	PGNSP PGUID ));
DATA(insert OID = 4076 (	3580	bpchar_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4077 (	3580	time_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4078 (	3580	interval_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4079 (	3580	bit_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4080 (	3580	varbit_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4081 (	3580	uuid_minmax_ops			PGNSP PGUID ));
DATA(insert OID = 4103 (	3580	range_inclusion_ops		PGNSP PGUID ));
DATA(insert OID = 4082 (	3580	pg_lsn_minmax_ops		PGNSP PGUID ));
DATA(insert OID = 4104 (	3580	box_inclusion_ops		PGNSP PGUID ));
DATA(insert OID = 5000 (	4000	box_ops		PGNSP PGUID ));

/* Complex Number type */
DATA(insert OID = 6221 (	403		complex_ops		PGNSP PGUID ));
DATA(insert OID = 6224 (	405		complex_ops		PGNSP PGUID ));

/*
 * hash support for a few built-in datatypes that are missing it in upstream.
 */
DATA(insert OID = 7077 (	405		tid_ops		PGNSP PGUID ));
DATA(insert OID = 7078 (	405		bit_ops		PGNSP PGUID ));
DATA(insert OID = 7079 (	405		varbit_ops	PGNSP PGUID ));

/* Hash opfamilies to represent the legacy "cdbhash" function */
/* int2, int4, int8 */
DATA(insert OID = 7100 (	405		cdbhash_integer_ops	PGNSP PGUID ));
DATA(insert OID = 7101 (	405		cdbhash_float4_ops	PGNSP PGUID ));
DATA(insert OID = 7102 (	405		cdbhash_float8_ops	PGNSP PGUID ));
DATA(insert OID = 7103 (	405		cdbhash_numeric_ops	PGNSP PGUID ));
DATA(insert OID = 7104 (	405		cdbhash_char_ops	PGNSP PGUID ));
/* text is also for varchar */
DATA(insert OID = 7105 (	405		cdbhash_text_ops	PGNSP PGUID ));
/*
 * In the legacy hash functions, the hashing of 'text' and 'bpchar' are
 * actually compatible. But there are no cross-datatype operators between
 * them, we rely on casts. Better to put them in different operator families,
 * to avoid confusion.
 */
DATA(insert OID = 7106 (	405		cdbhash_bpchar_ops	PGNSP PGUID ));
DATA(insert OID = 7107 (	405		cdbhash_bytea_ops	PGNSP PGUID ));
DATA(insert OID = 7108 (	405		cdbhash_name_ops	PGNSP PGUID ));
DATA(insert OID = 7109 (	405		cdbhash_oid_ops		PGNSP PGUID ));
DATA(insert OID = 7110 (	405		cdbhash_tid_ops		PGNSP PGUID ));
DATA(insert OID = 7111 (	405		cdbhash_timestamp_ops	PGNSP PGUID ));
DATA(insert OID = 7112 (	405		cdbhash_timestamptz_ops	PGNSP PGUID ));
DATA(insert OID = 7113 (	405		cdbhash_date_ops	PGNSP PGUID ));
DATA(insert OID = 7114 (	405		cdbhash_time_ops	PGNSP PGUID ));
DATA(insert OID = 7115 (	405		cdbhash_timetz_ops	PGNSP PGUID ));
DATA(insert OID = 7116 (	405		cdbhash_interval_ops	PGNSP PGUID ));
DATA(insert OID = 7117 (	405		cdbhash_abstime_ops	PGNSP PGUID ));
DATA(insert OID = 7118 (	405		cdbhash_reltime_ops	PGNSP PGUID ));
DATA(insert OID = 7119 (	405		cdbhash_tinterval_ops	PGNSP PGUID ));
DATA(insert OID = 7120 (	405		cdbhash_inet_ops	PGNSP PGUID ));
DATA(insert OID = 7121 (	405		cdbhash_macaddr_ops	PGNSP PGUID ));
DATA(insert OID = 7122 (	405		cdbhash_bit_ops		PGNSP PGUID ));
DATA(insert OID = 7123 (	405		cdbhash_varbit_ops	PGNSP PGUID ));
DATA(insert OID = 7124 (	405		cdbhash_bool_ops	PGNSP PGUID ));
DATA(insert OID = 7125 (	405		cdbhash_array_ops	PGNSP PGUID ));
DATA(insert OID = 7126 (	405		cdbhash_oidvector_ops	PGNSP PGUID ));
DATA(insert OID = 7127 (	405		cdbhash_cash_ops	PGNSP PGUID ));
DATA(insert OID = 7128 (	405		cdbhash_complex_ops	PGNSP PGUID ));
DATA(insert OID = 7129 (	405		cdbhash_uuid_ops	PGNSP PGUID ));
DATA(insert OID = 7130 (	405		cdbhash_enum_ops	PGNSP PGUID ));

#endif   /* PG_OPFAMILY_H */
=======
#endif							/* PG_OPFAMILY_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
