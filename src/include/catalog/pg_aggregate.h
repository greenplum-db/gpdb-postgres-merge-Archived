/*-------------------------------------------------------------------------
 *
 * pg_aggregate.h
 *	  definition of the "aggregate" system catalog (pg_aggregate)
 *
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_aggregate.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_AGGREGATE_H
#define PG_AGGREGATE_H

#include "catalog/genbki.h"
#include "catalog/pg_aggregate_d.h"

#include "catalog/objectaddress.h"
#include "nodes/pg_list.h"

/* ----------------------------------------------------------------
 *		pg_aggregate definition.
 *		cpp turns this into typedef struct FormData_pg_aggregate
 * ----------------------------------------------------------------
 */
CATALOG(pg_aggregate,2600,AggregateRelationId)
{
	/* pg_proc OID of the aggregate itself */
	regproc		aggfnoid BKI_LOOKUP(pg_proc);

	/* aggregate kind, see AGGKIND_ categories below */
	char		aggkind BKI_DEFAULT(n);

	/* number of arguments that are "direct" arguments */
	int16		aggnumdirectargs BKI_DEFAULT(0);

	/* transition function */
	regproc		aggtransfn BKI_LOOKUP(pg_proc);

	/* final function (0 if none) */
	regproc		aggfinalfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* combine function (0 if none) */
	regproc		aggcombinefn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* function to convert transtype to bytea (0 if none) */
	regproc		aggserialfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* function to convert bytea to transtype (0 if none) */
	regproc		aggdeserialfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* forward function for moving-aggregate mode (0 if none) */
	regproc		aggmtransfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* inverse function for moving-aggregate mode (0 if none) */
	regproc		aggminvtransfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* final function for moving-aggregate mode (0 if none) */
	regproc		aggmfinalfn BKI_DEFAULT(-) BKI_LOOKUP(pg_proc);

	/* true to pass extra dummy arguments to aggfinalfn */
	bool		aggfinalextra BKI_DEFAULT(f);

	/* true to pass extra dummy arguments to aggmfinalfn */
	bool		aggmfinalextra BKI_DEFAULT(f);

	/* tells whether aggfinalfn modifies transition state */
	char		aggfinalmodify BKI_DEFAULT(r);

	/* tells whether aggmfinalfn modifies transition state */
	char		aggmfinalmodify BKI_DEFAULT(r);

	/* associated sort operator (0 if none) */
	Oid			aggsortop BKI_DEFAULT(0) BKI_LOOKUP(pg_operator);

	/* type of aggregate's transition (state) data */
	Oid			aggtranstype BKI_LOOKUP(pg_type);

	/* estimated size of state data (0 for default estimate) */
	int32		aggtransspace BKI_DEFAULT(0);

	/* type of moving-aggregate state data (0 if none) */
	Oid			aggmtranstype BKI_DEFAULT(0) BKI_LOOKUP(pg_type);

	/* estimated size of moving-agg state (0 for default est) */
	int32		aggmtransspace BKI_DEFAULT(0);

#ifdef CATALOG_VARLEN			/* variable-length fields start here */

	/* initial value for transition state (can be NULL) */
	text		agginitval BKI_DEFAULT(_null_);

	/* initial value for moving-agg state (can be NULL) */
	text		aggminitval BKI_DEFAULT(_null_);
#endif
} FormData_pg_aggregate;

/* GPDB added foreign key definitions for gpcheckcat. */
FOREIGN_KEY(aggfnoid REFERENCES pg_proc(oid));
FOREIGN_KEY(aggtransfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggcombinefn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggfinalfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggserialfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggdeserialfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggmtransfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggminvtransfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggmfinalfn REFERENCES pg_proc(oid));
FOREIGN_KEY(aggsortop REFERENCES pg_operator(oid));
FOREIGN_KEY(aggtranstype REFERENCES pg_type(oid));
FOREIGN_KEY(aggmtranstype REFERENCES pg_type(oid));

/* ----------------
 *		Form_pg_aggregate corresponds to a pointer to a tuple with
 *		the format of pg_aggregate relation.
 * ----------------
 */
typedef FormData_pg_aggregate *Form_pg_aggregate;

#ifdef EXPOSE_TO_CLIENT_CODE

/*
 * Symbolic values for aggkind column.  We distinguish normal aggregates
 * from ordered-set aggregates (which have two sets of arguments, namely
 * direct and aggregated arguments) and from hypothetical-set aggregates
 * (which are a subclass of ordered-set aggregates in which the last
 * direct arguments have to match up in number and datatypes with the
 * aggregated arguments).
 */
#define AGGKIND_NORMAL			'n'
#define AGGKIND_ORDERED_SET		'o'
#define AGGKIND_HYPOTHETICAL	'h'

/* Use this macro to test for "ordered-set agg including hypothetical case" */
#define AGGKIND_IS_ORDERED_SET(kind)  ((kind) != AGGKIND_NORMAL)

<<<<<<< HEAD
/*
 * Constant definitions of pg_proc.oid for aggregate functions.
 */
#define AGGFNOID_COUNT_ANY 2147 /* returns INT8OID */
#define AGGFNOID_SUM_BIGINT 2107 /* returns NUMERICOID */

/* ----------------
 * initial contents of pg_aggregate
 * ---------------
 */

/* avg */
DATA(insert ( 2100	n 0 int8_avg_accum		numeric_poly_avg	int8_avg_combine	int8_avg_serialize		int8_avg_deserialize	int8_avg_accum	int8_avg_accum_inv	numeric_poly_avg	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2101	n 0 int4_avg_accum		int8_avg			int4_avg_combine	-						-						int4_avg_accum	int4_avg_accum_inv	int8_avg			f f 0	1016	0	1016	0	"{0,0}" "{0,0}" ));
DATA(insert ( 2102	n 0 int2_avg_accum		int8_avg			int4_avg_combine	-						-						int2_avg_accum	int2_avg_accum_inv	int8_avg			f f 0	1016	0	1016	0	"{0,0}" "{0,0}" ));
DATA(insert ( 2103	n 0 numeric_avg_accum	numeric_avg			numeric_avg_combine numeric_avg_serialize	numeric_avg_deserialize numeric_avg_accum numeric_accum_inv numeric_avg			f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2104	n 0 float4_accum		float8_avg			float8_combine		-						-						-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2105	n 0 float8_accum		float8_avg			float8_combine		-						-						-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2106	n 0 interval_accum		interval_avg		interval_combine	-						-						interval_accum	interval_accum_inv	interval_avg		f f 0	1187	0	1187	0	"{0 second,0 second}" "{0 second,0 second}" ));

/* sum */
DATA(insert ( 2107	n 0 int8_avg_accum		numeric_poly_sum	int8_avg_combine	int8_avg_serialize		int8_avg_deserialize	int8_avg_accum	int8_avg_accum_inv	numeric_poly_sum	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2108	n 0 int4_sum			-					int8pl				-						-						int4_avg_accum	int4_avg_accum_inv	int2int4_sum		f f 0	20		0	1016	0	_null_ "{0,0}" ));
DATA(insert ( 2109	n 0 int2_sum			-					int8pl				-						-						int2_avg_accum	int2_avg_accum_inv	int2int4_sum		f f 0	20		0	1016	0	_null_ "{0,0}" ));
DATA(insert ( 2110	n 0 float4pl			-					float4pl			-						-						-				-					-					f f 0	700		0	0		0	_null_ _null_ ));
DATA(insert ( 2111	n 0 float8pl			-					float8pl			-						-						-				-					-					f f 0	701		0	0		0	_null_ _null_ ));
DATA(insert ( 2112	n 0 cash_pl				-					cash_pl				-						-						cash_pl			cash_mi				-					f f 0	790		0	790		0	_null_ _null_ ));
DATA(insert ( 2113	n 0 interval_pl			-					interval_pl			-						-						interval_pl		interval_mi			-					f f 0	1186	0	1186	0	_null_ _null_ ));
DATA(insert ( 2114	n 0 numeric_avg_accum	numeric_sum			numeric_avg_combine numeric_avg_serialize	numeric_avg_deserialize numeric_avg_accum numeric_accum_inv numeric_sum			f f 0	2281	128 2281	128 _null_ _null_ ));

/* max */
DATA(insert ( 2115	n 0 int8larger		-				int8larger			-	-	-				-				-				f f 413		20		0	0		0	_null_ _null_ ));
DATA(insert ( 2116	n 0 int4larger		-				int4larger			-	-	-				-				-				f f 521		23		0	0		0	_null_ _null_ ));
DATA(insert ( 2117	n 0 int2larger		-				int2larger			-	-	-				-				-				f f 520		21		0	0		0	_null_ _null_ ));
DATA(insert ( 2118	n 0 oidlarger		-				oidlarger			-	-	-				-				-				f f 610		26		0	0		0	_null_ _null_ ));
DATA(insert ( 2119	n 0 float4larger	-				float4larger		-	-	-				-				-				f f 623		700		0	0		0	_null_ _null_ ));
DATA(insert ( 2120	n 0 float8larger	-				float8larger		-	-	-				-				-				f f 674		701		0	0		0	_null_ _null_ ));
DATA(insert ( 2121	n 0 int4larger		-				int4larger			-	-	-				-				-				f f 563		702		0	0		0	_null_ _null_ ));
DATA(insert ( 2122	n 0 date_larger		-				date_larger			-	-	-				-				-				f f 1097	1082	0	0		0	_null_ _null_ ));
DATA(insert ( 2123	n 0 time_larger		-				time_larger			-	-	-				-				-				f f 1112	1083	0	0		0	_null_ _null_ ));
DATA(insert ( 2124	n 0 timetz_larger	-				timetz_larger		-	-	-				-				-				f f 1554	1266	0	0		0	_null_ _null_ ));
DATA(insert ( 2125	n 0 cashlarger		-				cashlarger			-	-	-				-				-				f f 903		790		0	0		0	_null_ _null_ ));
DATA(insert ( 2126	n 0 timestamp_larger	-			timestamp_larger	-	-	-				-				-				f f 2064	1114	0	0		0	_null_ _null_ ));
DATA(insert ( 2127	n 0 timestamptz_larger	-			timestamptz_larger	-	-	-				-				-				f f 1324	1184	0	0		0	_null_ _null_ ));
DATA(insert ( 2128	n 0 interval_larger -				interval_larger		-	-	-				-				-				f f 1334	1186	0	0		0	_null_ _null_ ));
DATA(insert ( 2129	n 0 text_larger		-				text_larger			-	-	-				-				-				f f 666		25		0	0		0	_null_ _null_ ));
DATA(insert ( 2130	n 0 numeric_larger	-				numeric_larger		-	-	-				-				-				f f 1756	1700	0	0		0	_null_ _null_ ));
DATA(insert ( 2050	n 0 array_larger	-				array_larger		-	-	-				-				-				f f 1073	2277	0	0		0	_null_ _null_ ));
DATA(insert ( 2244	n 0 bpchar_larger	-				bpchar_larger		-	-	-				-				-				f f 1060	1042	0	0		0	_null_ _null_ ));
DATA(insert ( 2797	n 0 tidlarger		-				tidlarger			-	-	-				-				-				f f 2800	27		0	0		0	_null_ _null_ ));
DATA(insert ( 3526	n 0 enum_larger		-				enum_larger			-	-	-				-				-				f f 3519	3500	0	0		0	_null_ _null_ ));
DATA(insert ( 3564	n 0 network_larger	-				network_larger		-	-	-				-				-				f f 1205	869		0	0		0	_null_ _null_ ));

/* min */
DATA(insert ( 2131	n 0 int8smaller		-				int8smaller			-	-	-				-				-				f f 412		20		0	0		0	_null_ _null_ ));
DATA(insert ( 2132	n 0 int4smaller		-				int4smaller			-	-	-				-				-				f f 97		23		0	0		0	_null_ _null_ ));
DATA(insert ( 2133	n 0 int2smaller		-				int2smaller			-	-	-				-				-				f f 95		21		0	0		0	_null_ _null_ ));
DATA(insert ( 2134	n 0 oidsmaller		-				oidsmaller			-	-	-				-				-				f f 609		26		0	0		0	_null_ _null_ ));
DATA(insert ( 2135	n 0 float4smaller	-				float4smaller		-	-	-				-				-				f f 622		700		0	0		0	_null_ _null_ ));
DATA(insert ( 2136	n 0 float8smaller	-				float8smaller		-	-	-				-				-				f f 672		701		0	0		0	_null_ _null_ ));
DATA(insert ( 2137	n 0 int4smaller		-				int4smaller			-	-	-				-				-				f f 562		702		0	0		0	_null_ _null_ ));
DATA(insert ( 2138	n 0 date_smaller	-				date_smaller		-	-	-				-				-				f f 1095	1082	0	0		0	_null_ _null_ ));
DATA(insert ( 2139	n 0 time_smaller	-				time_smaller		-	-	-				-				-				f f 1110	1083	0	0		0	_null_ _null_ ));
DATA(insert ( 2140	n 0 timetz_smaller	-				timetz_smaller		-	-	-				-				-				f f 1552	1266	0	0		0	_null_ _null_ ));
DATA(insert ( 2141	n 0 cashsmaller		-				cashsmaller			-	-	-				-				-				f f 902		790		0	0		0	_null_ _null_ ));
DATA(insert ( 2142	n 0 timestamp_smaller	-			timestamp_smaller	-	-	-				-				-				f f 2062	1114	0	0		0	_null_ _null_ ));
DATA(insert ( 2143	n 0 timestamptz_smaller -			timestamptz_smaller -	-	-				-				-				f f 1322	1184	0	0		0	_null_ _null_ ));
DATA(insert ( 2144	n 0 interval_smaller	-			interval_smaller	-	-	-				-				-				f f 1332	1186	0	0		0	_null_ _null_ ));
DATA(insert ( 2145	n 0 text_smaller	-				text_smaller		-	-	-				-				-				f f 664		25		0	0		0	_null_ _null_ ));
DATA(insert ( 2146	n 0 numeric_smaller -				numeric_smaller		-	-	-				-				-				f f 1754	1700	0	0		0	_null_ _null_ ));
DATA(insert ( 2051	n 0 array_smaller	-				array_smaller		-	-	-				-				-				f f 1072	2277	0	0		0	_null_ _null_ ));
DATA(insert ( 2245	n 0 bpchar_smaller	-				bpchar_smaller		-	-	-				-				-				f f 1058	1042	0	0		0	_null_ _null_ ));
DATA(insert ( 2798	n 0 tidsmaller		-				tidsmaller			-	-	-				-				-				f f 2799	27		0	0		0	_null_ _null_ ));
DATA(insert ( 3527	n 0 enum_smaller	-				enum_smaller		-	-	-				-				-				f f 3518	3500	0	0		0	_null_ _null_ ));
DATA(insert ( 3565	n 0 network_smaller -				network_smaller		-	-	-				-				-				f f 1203	869		0	0		0	_null_ _null_ ));

/* count */
DATA(insert ( 2147	n 0 int8inc_any		-				int8pl	-	-	int8inc_any		int8dec_any		-				f f 0		20		0	20		0	"0" "0" ));
DATA(insert ( 2803	n 0 int8inc			-				int8pl	-	-	int8inc			int8dec			-				f f 0		20		0	20		0	"0" "0" ));

/* var_pop */
DATA(insert ( 2718	n 0 int8_accum		numeric_var_pop			numeric_combine			numeric_serialize		numeric_deserialize			int8_accum		int8_accum_inv	numeric_var_pop			f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2719	n 0 int4_accum		numeric_poly_var_pop	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum		int4_accum_inv	numeric_poly_var_pop	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2720	n 0 int2_accum		numeric_poly_var_pop	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum		int2_accum_inv	numeric_poly_var_pop	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2721	n 0 float4_accum	float8_var_pop			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2722	n 0 float8_accum	float8_var_pop			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2723	n 0 numeric_accum	numeric_var_pop			numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum	numeric_accum_inv numeric_var_pop		f f 0	2281	128 2281	128 _null_ _null_ ));

/* var_samp */
DATA(insert ( 2641	n 0 int8_accum		numeric_var_samp		numeric_combine			numeric_serialize		numeric_deserialize			int8_accum		int8_accum_inv	numeric_var_samp		f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2642	n 0 int4_accum		numeric_poly_var_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum		int4_accum_inv	numeric_poly_var_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2643	n 0 int2_accum		numeric_poly_var_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum		int2_accum_inv	numeric_poly_var_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2644	n 0 float4_accum	float8_var_samp			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2645	n 0 float8_accum	float8_var_samp			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2646	n 0 numeric_accum	numeric_var_samp		numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum	numeric_accum_inv numeric_var_samp		f f 0	2281	128 2281	128 _null_ _null_ ));

/* variance: historical Postgres syntax for var_samp */
DATA(insert ( 2148	n 0 int8_accum		numeric_var_samp		numeric_combine			numeric_serialize		numeric_deserialize			int8_accum		int8_accum_inv	numeric_var_samp		f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2149	n 0 int4_accum		numeric_poly_var_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum		int4_accum_inv	numeric_poly_var_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2150	n 0 int2_accum		numeric_poly_var_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum		int2_accum_inv	numeric_poly_var_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2151	n 0 float4_accum	float8_var_samp			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2152	n 0 float8_accum	float8_var_samp			float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2153	n 0 numeric_accum	numeric_var_samp		numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum	numeric_accum_inv numeric_var_samp		f f 0	2281	128 2281	128 _null_ _null_ ));

/* stddev_pop */
DATA(insert ( 2724	n 0 int8_accum		numeric_stddev_pop		numeric_combine			numeric_serialize		numeric_deserialize			int8_accum		int8_accum_inv	numeric_stddev_pop		f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2725	n 0 int4_accum		numeric_poly_stddev_pop numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum		int4_accum_inv	numeric_poly_stddev_pop f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2726	n 0 int2_accum		numeric_poly_stddev_pop numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum		int2_accum_inv	numeric_poly_stddev_pop f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2727	n 0 float4_accum	float8_stddev_pop		float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2728	n 0 float8_accum	float8_stddev_pop		float8_combine			-						-							-				-				-						f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2729	n 0 numeric_accum	numeric_stddev_pop		numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum	numeric_accum_inv numeric_stddev_pop	f f 0	2281	128 2281	128 _null_ _null_ ));

/* stddev_samp */
DATA(insert ( 2712	n 0 int8_accum		numeric_stddev_samp			numeric_combine			numeric_serialize		numeric_deserialize			int8_accum	int8_accum_inv	numeric_stddev_samp			f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2713	n 0 int4_accum		numeric_poly_stddev_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum	int4_accum_inv	numeric_poly_stddev_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2714	n 0 int2_accum		numeric_poly_stddev_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum	int2_accum_inv	numeric_poly_stddev_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2715	n 0 float4_accum	float8_stddev_samp			float8_combine			-						-							-			-				-							f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2716	n 0 float8_accum	float8_stddev_samp			float8_combine			-						-							-			-				-							f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2717	n 0 numeric_accum	numeric_stddev_samp			numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum numeric_accum_inv numeric_stddev_samp		f f 0	2281	128 2281	128 _null_ _null_ ));

/* stddev: historical Postgres syntax for stddev_samp */
DATA(insert ( 2154	n 0 int8_accum		numeric_stddev_samp			numeric_combine			numeric_serialize		numeric_deserialize			int8_accum		int8_accum_inv	numeric_stddev_samp			f f 0	2281	128 2281	128 _null_ _null_ ));
DATA(insert ( 2155	n 0 int4_accum		numeric_poly_stddev_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int4_accum		int4_accum_inv	numeric_poly_stddev_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2156	n 0 int2_accum		numeric_poly_stddev_samp	numeric_poly_combine	numeric_poly_serialize	numeric_poly_deserialize	int2_accum		int2_accum_inv	numeric_poly_stddev_samp	f f 0	2281	48	2281	48	_null_ _null_ ));
DATA(insert ( 2157	n 0 float4_accum	float8_stddev_samp			float8_combine			-						-							-				-				-							f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2158	n 0 float8_accum	float8_stddev_samp			float8_combine			-						-							-				-				-							f f 0	1022	0	0		0	"{0,0,0}" _null_ ));
DATA(insert ( 2159	n 0 numeric_accum	numeric_stddev_samp			numeric_combine			numeric_serialize		numeric_deserialize			numeric_accum	numeric_accum_inv numeric_stddev_samp		f f 0	2281	128 2281	128 _null_ _null_ ));

/* SQL2003 binary regression aggregates */
DATA(insert ( 2818	n 0 int8inc_float8_float8	-					int8pl				-	-	-				-				-			f f 0	20		0	0		0	"0" _null_ ));
DATA(insert ( 2819	n 0 float8_regr_accum	float8_regr_sxx			float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2820	n 0 float8_regr_accum	float8_regr_syy			float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2821	n 0 float8_regr_accum	float8_regr_sxy			float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2822	n 0 float8_regr_accum	float8_regr_avgx		float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2823	n 0 float8_regr_accum	float8_regr_avgy		float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2824	n 0 float8_regr_accum	float8_regr_r2			float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2825	n 0 float8_regr_accum	float8_regr_slope		float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2826	n 0 float8_regr_accum	float8_regr_intercept	float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2827	n 0 float8_regr_accum	float8_covar_pop		float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2828	n 0 float8_regr_accum	float8_covar_samp		float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));
DATA(insert ( 2829	n 0 float8_regr_accum	float8_corr				float8_regr_combine -	-	-				-				-			f f 0	1022	0	0		0	"{0,0,0,0,0,0}" _null_ ));

/* boolean-and and boolean-or */
DATA(insert ( 2517	n 0 booland_statefunc	-	booland_statefunc	-	-	bool_accum	bool_accum_inv	bool_alltrue	f f 58	16	0	2281	16	_null_ _null_ ));
DATA(insert ( 2518	n 0 boolor_statefunc	-	boolor_statefunc	-	-	bool_accum	bool_accum_inv	bool_anytrue	f f 59	16	0	2281	16	_null_ _null_ ));
DATA(insert ( 2519	n 0 booland_statefunc	-	booland_statefunc	-	-	bool_accum	bool_accum_inv	bool_alltrue	f f 58	16	0	2281	16	_null_ _null_ ));

/* bitwise integer */
DATA(insert ( 2236	n 0 int2and		-				int2and -	-	-				-				-				f f 0	21		0	0		0	_null_ _null_ ));
DATA(insert ( 2237	n 0 int2or		-				int2or	-	-	-				-				-				f f 0	21		0	0		0	_null_ _null_ ));
DATA(insert ( 2238	n 0 int4and		-				int4and -	-	-				-				-				f f 0	23		0	0		0	_null_ _null_ ));
DATA(insert ( 2239	n 0 int4or		-				int4or	-	-	-				-				-				f f 0	23		0	0		0	_null_ _null_ ));
DATA(insert ( 2240	n 0 int8and		-				int8and -	-	-				-				-				f f 0	20		0	0		0	_null_ _null_ ));
DATA(insert ( 2241	n 0 int8or		-				int8or	-	-	-				-				-				f f 0	20		0	0		0	_null_ _null_ ));
DATA(insert ( 2242	n 0 bitand		-				bitand	-	-	-				-				-				f f 0	1560	0	0		0	_null_ _null_ ));
DATA(insert ( 2243	n 0 bitor		-				bitor	-	-	-				-				-				f f 0	1560	0	0		0	_null_ _null_ ));

/* xml */
DATA(insert ( 2901	n 0 xmlconcat2	-				-		-	-	-				-				-				f f 0	142		0	0		0	_null_ _null_ ));

/* array */
DATA(insert ( 2335	n 0 array_agg_transfn		array_agg_finalfn		-	-	-	-		-				-				t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 4053	n 0 array_agg_array_transfn array_agg_array_finalfn -	-	-	-		-				-				t f 0	2281	0	0		0	_null_ _null_ ));

/* text */
DATA(insert ( 3538	n 0 string_agg_transfn	string_agg_finalfn	-	-	-	-				-				-				f f 0	2281	0	0		0	_null_ _null_ ));

/* bytea */
DATA(insert ( 3545	n 0 bytea_string_agg_transfn	bytea_string_agg_finalfn	-	-	-	-				-				-		f f 0	2281	0	0		0	_null_ _null_ ));

/* json */
DATA(insert ( 3175	n 0 json_agg_transfn	json_agg_finalfn			-	-	-	-				-				-				f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3197	n 0 json_object_agg_transfn json_object_agg_finalfn -	-	-	-				-				-				f f 0	2281	0	0		0	_null_ _null_ ));

/* jsonb */
DATA(insert ( 3267	n 0 jsonb_agg_transfn	jsonb_agg_finalfn				-	-	-	-				-				-			f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3270	n 0 jsonb_object_agg_transfn jsonb_object_agg_finalfn	-	-	-	-				-				-			f f 0	2281	0	0		0	_null_ _null_ ));

/* ordered-set and hypothetical-set aggregates */
DATA(insert ( 3972	o 1 ordered_set_transition			percentile_disc_final					-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3974	o 1 ordered_set_transition			percentile_cont_float8_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3976	o 1 ordered_set_transition			percentile_cont_interval_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3978	o 1 ordered_set_transition			percentile_disc_multi_final				-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3980	o 1 ordered_set_transition			percentile_cont_float8_multi_final		-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3982	o 1 ordered_set_transition			percentile_cont_interval_multi_final	-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3984	o 0 ordered_set_transition			mode_final								-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3986	h 1 ordered_set_transition_multi	rank_final								-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3988	h 1 ordered_set_transition_multi	percent_rank_final						-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3990	h 1 ordered_set_transition_multi	cume_dist_final							-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 3992	h 1 ordered_set_transition_multi	dense_rank_final						-	-	-	-		-		-		t f 0	2281	0	0		0	_null_ _null_ ));

/* MPP Aggregate -- array_sum -- special for prospective customer. */
DATA(insert ( 5067	n 0 array_add	-				array_add	-	-			-	-	-	f f 0	1007 0	0	0	"{}" _null_ ));

/* sum(array[]) */
DATA(insert ( 6216  n 0 int2_matrix_accum		-	int8_matrix_accum	-	-	-	-	-	f f 0	1016 0	0	0	_null_ _null_ ));
DATA(insert ( 6217  n 0 int4_matrix_accum		-	int8_matrix_accum	-	-	-	-	-	f f 0	1016 0	0	0	_null_ _null_ ));
DATA(insert ( 6218  n 0 int8_matrix_accum		-	int8_matrix_accum	-	-	-	-	-	f f 0	1016 0	0	0	_null_ _null_ ));
DATA(insert ( 6219  n 0 float8_matrix_accum		-	float8_matrix_accum	-	-	-	-	-	f f 0	1022 0	0	0	_null_ _null_ ));

/* pivot_sum(...) */
DATA(insert ( 6226  n 0 int4_pivot_accum		-	int8_matrix_accum	-	-	-	-	-	f f 0	1007 0	0	0	_null_ _null_ ));
DATA(insert ( 6228  n 0 int8_pivot_accum		-	int8_matrix_accum	-	-	-	-	-	f f 0	1016 0	0	0	_null_ _null_ ));
DATA(insert ( 6230  n 0 float8_pivot_accum		-	float8_matrix_accum	-	-	-	-	-	f f 0	1022 0	0	0	_null_ _null_ ));

/* GPDB: additional variants of percentile_cont, for timestamps */
DATA(insert ( 6119	o 1 ordered_set_transition			percentile_cont_timestamp_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6121	o 1 ordered_set_transition			percentile_cont_timestamp_multi_final	-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6123	o 1 ordered_set_transition			percentile_cont_timestamptz_final		-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6125	o 1 ordered_set_transition			percentile_cont_timestamptz_multi_final	-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));

/* median */
DATA(insert ( 6127	o 1 ordered_set_transition			percentile_cont_float8_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6128	o 1 ordered_set_transition			percentile_cont_interval_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6129	o 1 ordered_set_transition			percentile_cont_timestamp_final			-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));
DATA(insert ( 6130	o 1 ordered_set_transition			percentile_cont_timestamptz_final		-	-	-	-		-		-		f f 0	2281	0	0		0	_null_ _null_ ));

/* hyperloglog */
DATA(insert ( 7164	n 0 gp_hyperloglog_add_item_agg_default gp_hyperloglog_comp		gp_hyperloglog_merge	-	-	-		-		-		f f 0	7157	0	0		0	_null_ _null_ ));

=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
/*
 * Symbolic values for aggfinalmodify and aggmfinalmodify columns.
 * Preferably, finalfns do not modify the transition state value at all,
 * but in some cases that would cost too much performance.  We distinguish
 * "pure read only" and "trashes it arbitrarily" cases, as well as the
 * intermediate case where multiple finalfn calls are allowed but the
 * transfn cannot be applied anymore after the first finalfn call.
 */
#define AGGMODIFY_READ_ONLY			'r'
#define AGGMODIFY_SHAREABLE			's'
#define AGGMODIFY_READ_WRITE		'w'

#endif							/* EXPOSE_TO_CLIENT_CODE */


extern ObjectAddress AggregateCreate(const char *aggName,
									 Oid aggNamespace,
									 bool replace,
									 char aggKind,
									 int numArgs,
									 int numDirectArgs,
									 oidvector *parameterTypes,
									 Datum allParameterTypes,
									 Datum parameterModes,
									 Datum parameterNames,
									 List *parameterDefaults,
									 Oid variadicArgType,
									 List *aggtransfnName,
									 List *aggfinalfnName,
									 List *aggcombinefnName,
									 List *aggserialfnName,
									 List *aggdeserialfnName,
									 List *aggmtransfnName,
									 List *aggminvtransfnName,
									 List *aggmfinalfnName,
									 bool finalfnExtraArgs,
									 bool mfinalfnExtraArgs,
									 char finalfnModify,
									 char mfinalfnModify,
									 List *aggsortopName,
									 Oid aggTransType,
									 int32 aggTransSpace,
									 Oid aggmTransType,
									 int32 aggmTransSpace,
									 const char *agginitval,
									 const char *aggminitval,
									 char proparallel);

#endif							/* PG_AGGREGATE_H */
