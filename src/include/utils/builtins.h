/*-------------------------------------------------------------------------
 *
 * builtins.h
 *	  Declarations for operations on built-in types.
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 2005-2010, Greenplum inc
 * Portions Copyright (c) 2012-Present Pivotal Software, Inc.
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
=======
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/utils/builtins.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef BUILTINS_H
#define BUILTINS_H

#include "fmgr.h"
#include "nodes/nodes.h"
#include "utils/fmgrprotos.h"


/* bool.c */
extern bool parse_bool(const char *value, bool *result);
extern bool parse_bool_with_len(const char *value, size_t len, bool *result);

/* domains.c */
extern void domain_check(Datum value, bool isnull, Oid domainType,
						 void **extra, MemoryContext mcxt);
extern int	errdatatype(Oid datatypeOid);
extern int	errdomainconstraint(Oid datatypeOid, const char *conname);

/* encode.c */
extern unsigned hex_encode(const char *src, unsigned len, char *dst);
extern unsigned hex_decode(const char *src, unsigned len, char *dst);

<<<<<<< HEAD
/* encode.c */
extern Datum binary_encode(PG_FUNCTION_ARGS);
extern Datum binary_decode(PG_FUNCTION_ARGS);
extern unsigned hex_encode(const char *src, unsigned len, char *dst);
extern unsigned hex_decode(const char *src, unsigned len, char *dst);

/* enum.c */
extern Datum enum_in(PG_FUNCTION_ARGS);
extern Datum enum_out(PG_FUNCTION_ARGS);
extern Datum enum_recv(PG_FUNCTION_ARGS);
extern Datum enum_send(PG_FUNCTION_ARGS);
extern Datum enum_lt(PG_FUNCTION_ARGS);
extern Datum enum_le(PG_FUNCTION_ARGS);
extern Datum enum_eq(PG_FUNCTION_ARGS);
extern Datum enum_ne(PG_FUNCTION_ARGS);
extern Datum enum_ge(PG_FUNCTION_ARGS);
extern Datum enum_gt(PG_FUNCTION_ARGS);
extern Datum enum_cmp(PG_FUNCTION_ARGS);
extern Datum enum_smaller(PG_FUNCTION_ARGS);
extern Datum enum_larger(PG_FUNCTION_ARGS);
extern Datum enum_first(PG_FUNCTION_ARGS);
extern Datum enum_last(PG_FUNCTION_ARGS);
extern Datum enum_range_bounds(PG_FUNCTION_ARGS);
extern Datum enum_range_all(PG_FUNCTION_ARGS);

=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
/* int.c */
extern int2vector *buildint2vector(const int16 *int2s, int n);

/* name.c */
extern int	namecpy(Name n1, const NameData *n2);
extern int	namestrcpy(Name name, const char *str);
extern int	namestrcmp(Name name, const char *str);

/* numutils.c */
extern int32 pg_atoi(const char *s, int size, int c);
extern int16 pg_strtoint16(const char *s);
extern int32 pg_strtoint32(const char *s);
extern void pg_itoa(int16 i, char *a);
extern void pg_ltoa(int32 l, char *a);
extern void pg_lltoa(int64 ll, char *a);
extern char *pg_ltostr_zeropad(char *str, int32 value, int32 minwidth);
extern char *pg_ltostr(char *str, int32 value);
extern uint64 pg_strtouint64(const char *str, char **endptr, int base);

<<<<<<< HEAD
/*
 *		Per-opclass comparison functions for new btrees.  These are
 *		stored in pg_amproc; most are defined in access/nbtree/nbtcompare.c
 */
extern Datum btboolcmp(PG_FUNCTION_ARGS);
extern Datum btint2cmp(PG_FUNCTION_ARGS);
extern Datum btint4cmp(PG_FUNCTION_ARGS);
extern Datum btint8cmp(PG_FUNCTION_ARGS);
extern Datum btfloat4cmp(PG_FUNCTION_ARGS);
extern Datum btfloat8cmp(PG_FUNCTION_ARGS);
extern Datum btint48cmp(PG_FUNCTION_ARGS);
extern Datum btint84cmp(PG_FUNCTION_ARGS);
extern Datum btint24cmp(PG_FUNCTION_ARGS);
extern Datum btint42cmp(PG_FUNCTION_ARGS);
extern Datum btint28cmp(PG_FUNCTION_ARGS);
extern Datum btint82cmp(PG_FUNCTION_ARGS);
extern Datum btfloat48cmp(PG_FUNCTION_ARGS);
extern Datum btfloat84cmp(PG_FUNCTION_ARGS);
extern Datum btoidcmp(PG_FUNCTION_ARGS);
extern Datum btoidvectorcmp(PG_FUNCTION_ARGS);
extern Datum btabstimecmp(PG_FUNCTION_ARGS);
extern Datum btreltimecmp(PG_FUNCTION_ARGS);
extern Datum bttintervalcmp(PG_FUNCTION_ARGS);
extern Datum btcharcmp(PG_FUNCTION_ARGS);
extern Datum btnamecmp(PG_FUNCTION_ARGS);
extern Datum bttextcmp(PG_FUNCTION_ARGS);
extern Datum bttextsortsupport(PG_FUNCTION_ARGS);

/*
 *		Per-opclass sort support functions for new btrees.  Like the
 *		functions above, these are stored in pg_amproc; most are defined in
 *		access/nbtree/nbtcompare.c
 */
extern Datum btint2sortsupport(PG_FUNCTION_ARGS);
extern Datum btint4sortsupport(PG_FUNCTION_ARGS);
extern Datum btint8sortsupport(PG_FUNCTION_ARGS);
extern Datum btfloat4sortsupport(PG_FUNCTION_ARGS);
extern Datum btfloat8sortsupport(PG_FUNCTION_ARGS);
extern Datum btoidsortsupport(PG_FUNCTION_ARGS);
extern Datum btnamesortsupport(PG_FUNCTION_ARGS);

/* float.c */
extern PGDLLIMPORT int extra_float_digits;

extern double get_float8_infinity(void);
extern float get_float4_infinity(void);
extern double get_float8_nan(void);
extern float get_float4_nan(void);
extern int	is_infinite(double val);
extern double float8in_internal(char *num, char **endptr_p,
				  const char *type_name, const char *orig_string);
extern char *float8out_internal(double num);
extern int	float4_cmp_internal(float4 a, float4 b);
extern int	float8_cmp_internal(float8 a, float8 b);

extern Datum float4in(PG_FUNCTION_ARGS);
extern Datum float4out(PG_FUNCTION_ARGS);
extern Datum float4recv(PG_FUNCTION_ARGS);
extern Datum float4send(PG_FUNCTION_ARGS);
extern Datum float8in(PG_FUNCTION_ARGS);
extern Datum float8out(PG_FUNCTION_ARGS);
extern Datum float8recv(PG_FUNCTION_ARGS);
extern Datum float8send(PG_FUNCTION_ARGS);
extern Datum float4abs(PG_FUNCTION_ARGS);
extern Datum float4um(PG_FUNCTION_ARGS);
extern Datum float4up(PG_FUNCTION_ARGS);
extern Datum float4larger(PG_FUNCTION_ARGS);
extern Datum float4smaller(PG_FUNCTION_ARGS);
extern Datum float8abs(PG_FUNCTION_ARGS);
extern Datum float8um(PG_FUNCTION_ARGS);
extern Datum float8up(PG_FUNCTION_ARGS);
extern Datum float8larger(PG_FUNCTION_ARGS);
extern Datum float8smaller(PG_FUNCTION_ARGS);
extern Datum float4pl(PG_FUNCTION_ARGS);
extern Datum float4mi(PG_FUNCTION_ARGS);
extern Datum float4mul(PG_FUNCTION_ARGS);
extern Datum float4div(PG_FUNCTION_ARGS);
extern Datum float8pl(PG_FUNCTION_ARGS);
extern Datum float8mi(PG_FUNCTION_ARGS);
extern Datum float8mul(PG_FUNCTION_ARGS);
extern Datum float8div(PG_FUNCTION_ARGS);
extern Datum float4eq(PG_FUNCTION_ARGS);
extern Datum float4ne(PG_FUNCTION_ARGS);
extern Datum float4lt(PG_FUNCTION_ARGS);
extern Datum float4le(PG_FUNCTION_ARGS);
extern Datum float4gt(PG_FUNCTION_ARGS);
extern Datum float4ge(PG_FUNCTION_ARGS);
extern Datum float8eq(PG_FUNCTION_ARGS);
extern Datum float8ne(PG_FUNCTION_ARGS);
extern Datum float8lt(PG_FUNCTION_ARGS);
extern Datum float8le(PG_FUNCTION_ARGS);
extern Datum float8gt(PG_FUNCTION_ARGS);
extern Datum float8ge(PG_FUNCTION_ARGS);
extern Datum ftod(PG_FUNCTION_ARGS);
extern Datum i4tod(PG_FUNCTION_ARGS);
extern Datum i2tod(PG_FUNCTION_ARGS);
extern Datum dtof(PG_FUNCTION_ARGS);
extern Datum dtoi4(PG_FUNCTION_ARGS);
extern Datum dtoi2(PG_FUNCTION_ARGS);
extern Datum i4tof(PG_FUNCTION_ARGS);
extern Datum i2tof(PG_FUNCTION_ARGS);
extern Datum ftoi4(PG_FUNCTION_ARGS);
extern Datum ftoi2(PG_FUNCTION_ARGS);
extern Datum dround(PG_FUNCTION_ARGS);
extern Datum dceil(PG_FUNCTION_ARGS);
extern Datum dfloor(PG_FUNCTION_ARGS);
extern Datum dsign(PG_FUNCTION_ARGS);
extern Datum dtrunc(PG_FUNCTION_ARGS);
extern Datum dsqrt(PG_FUNCTION_ARGS);
extern Datum dcbrt(PG_FUNCTION_ARGS);
extern Datum dpow(PG_FUNCTION_ARGS);
extern Datum dexp(PG_FUNCTION_ARGS);
extern Datum dlog1(PG_FUNCTION_ARGS);
extern Datum dlog10(PG_FUNCTION_ARGS);
extern Datum dacos(PG_FUNCTION_ARGS);
extern Datum dasin(PG_FUNCTION_ARGS);
extern Datum datan(PG_FUNCTION_ARGS);
extern Datum datan2(PG_FUNCTION_ARGS);
extern Datum dcos(PG_FUNCTION_ARGS);
extern Datum dcosh(PG_FUNCTION_ARGS);
extern Datum dcot(PG_FUNCTION_ARGS);
extern Datum dsin(PG_FUNCTION_ARGS);
extern Datum dsinh(PG_FUNCTION_ARGS);
extern Datum dtan(PG_FUNCTION_ARGS);
extern Datum dtanh(PG_FUNCTION_ARGS);
extern Datum dacosd(PG_FUNCTION_ARGS);
extern Datum dasind(PG_FUNCTION_ARGS);
extern Datum datand(PG_FUNCTION_ARGS);
extern Datum datan2d(PG_FUNCTION_ARGS);
extern Datum dcosd(PG_FUNCTION_ARGS);
extern Datum dcotd(PG_FUNCTION_ARGS);
extern Datum dsind(PG_FUNCTION_ARGS);
extern Datum dtand(PG_FUNCTION_ARGS);
extern Datum degrees(PG_FUNCTION_ARGS);
extern Datum dpi(PG_FUNCTION_ARGS);
extern Datum radians(PG_FUNCTION_ARGS);
extern Datum drandom(PG_FUNCTION_ARGS);
extern Datum setseed(PG_FUNCTION_ARGS);
extern Datum float8_combine(PG_FUNCTION_ARGS);
extern Datum float8_accum(PG_FUNCTION_ARGS);
extern Datum float4_accum(PG_FUNCTION_ARGS);
extern Datum float8_avg(PG_FUNCTION_ARGS);
extern Datum float8_var_pop(PG_FUNCTION_ARGS);
extern Datum float8_var_samp(PG_FUNCTION_ARGS);
extern Datum float8_stddev_pop(PG_FUNCTION_ARGS);
extern Datum float8_stddev_samp(PG_FUNCTION_ARGS);
extern Datum float8_regr_accum(PG_FUNCTION_ARGS);
extern Datum float8_regr_combine(PG_FUNCTION_ARGS);
extern Datum float8_regr_sxx(PG_FUNCTION_ARGS);
extern Datum float8_regr_syy(PG_FUNCTION_ARGS);
extern Datum float8_regr_sxy(PG_FUNCTION_ARGS);
extern Datum float8_regr_avgx(PG_FUNCTION_ARGS);
extern Datum float8_regr_avgy(PG_FUNCTION_ARGS);
extern Datum float8_covar_pop(PG_FUNCTION_ARGS);
extern Datum float8_covar_samp(PG_FUNCTION_ARGS);
extern Datum float8_corr(PG_FUNCTION_ARGS);
extern Datum float8_regr_r2(PG_FUNCTION_ARGS);
extern Datum float8_regr_slope(PG_FUNCTION_ARGS);
extern Datum float8_regr_intercept(PG_FUNCTION_ARGS);
extern Datum float48pl(PG_FUNCTION_ARGS);
extern Datum float48mi(PG_FUNCTION_ARGS);
extern Datum float48mul(PG_FUNCTION_ARGS);
extern Datum float48div(PG_FUNCTION_ARGS);
extern Datum float84pl(PG_FUNCTION_ARGS);
extern Datum float84mi(PG_FUNCTION_ARGS);
extern Datum float84mul(PG_FUNCTION_ARGS);
extern Datum float84div(PG_FUNCTION_ARGS);
extern Datum float48eq(PG_FUNCTION_ARGS);
extern Datum float48ne(PG_FUNCTION_ARGS);
extern Datum float48lt(PG_FUNCTION_ARGS);
extern Datum float48le(PG_FUNCTION_ARGS);
extern Datum float48gt(PG_FUNCTION_ARGS);
extern Datum float48ge(PG_FUNCTION_ARGS);
extern Datum float84eq(PG_FUNCTION_ARGS);
extern Datum float84ne(PG_FUNCTION_ARGS);
extern Datum float84lt(PG_FUNCTION_ARGS);
extern Datum float84le(PG_FUNCTION_ARGS);
extern Datum float84gt(PG_FUNCTION_ARGS);
extern Datum float84ge(PG_FUNCTION_ARGS);
extern Datum width_bucket_float8(PG_FUNCTION_ARGS);
extern Datum pg_highest_oid(PG_FUNCTION_ARGS); /* MPP */
extern Datum gp_distributed_xid(PG_FUNCTION_ARGS); /* MPP */

/* dbsize.c */
extern Datum pg_tablespace_size_oid(PG_FUNCTION_ARGS);
extern Datum pg_tablespace_size_name(PG_FUNCTION_ARGS);
extern Datum pg_database_size_oid(PG_FUNCTION_ARGS);
extern Datum pg_database_size_name(PG_FUNCTION_ARGS);
extern Datum pg_relation_size(PG_FUNCTION_ARGS);
extern Datum pg_total_relation_size(PG_FUNCTION_ARGS);
extern Datum pg_size_pretty(PG_FUNCTION_ARGS);
extern Datum pg_size_pretty_numeric(PG_FUNCTION_ARGS);
extern Datum pg_size_bytes(PG_FUNCTION_ARGS);
extern Datum pg_table_size(PG_FUNCTION_ARGS);
extern Datum pg_indexes_size(PG_FUNCTION_ARGS);
extern Datum pg_relation_filenode(PG_FUNCTION_ARGS);
extern Datum pg_filenode_relation(PG_FUNCTION_ARGS);
extern Datum pg_relation_filepath(PG_FUNCTION_ARGS);

extern int64 get_size_from_segDBs(const char *cmd);

/* genfile.c */
extern Datum pg_stat_file(PG_FUNCTION_ARGS);
extern Datum pg_stat_file_1arg(PG_FUNCTION_ARGS);
extern Datum pg_read_file(PG_FUNCTION_ARGS);
extern Datum pg_read_file_off_len(PG_FUNCTION_ARGS);
extern Datum pg_read_file_all(PG_FUNCTION_ARGS);
extern Datum pg_read_binary_file(PG_FUNCTION_ARGS);
extern Datum pg_read_binary_file_off_len(PG_FUNCTION_ARGS);
extern Datum pg_read_binary_file_all(PG_FUNCTION_ARGS);
extern Datum pg_ls_dir(PG_FUNCTION_ARGS);
extern Datum pg_ls_dir_1arg(PG_FUNCTION_ARGS);
extern Datum pg_file_write(PG_FUNCTION_ARGS);
extern Datum pg_file_rename(PG_FUNCTION_ARGS);
extern Datum pg_file_unlink(PG_FUNCTION_ARGS);
extern Datum pg_logdir_ls(PG_FUNCTION_ARGS);
extern Datum pg_file_length(PG_FUNCTION_ARGS);

/* misc.c */
extern Datum pg_num_nulls(PG_FUNCTION_ARGS);
extern Datum pg_num_nonnulls(PG_FUNCTION_ARGS);
extern Datum current_database(PG_FUNCTION_ARGS);
extern Datum current_query(PG_FUNCTION_ARGS);
extern Datum pg_cancel_backend(PG_FUNCTION_ARGS);
extern Datum pg_terminate_backend(PG_FUNCTION_ARGS);
extern Datum pg_cancel_backend_msg(PG_FUNCTION_ARGS);
extern Datum pg_terminate_backend_msg(PG_FUNCTION_ARGS);
extern Datum pg_reload_conf(PG_FUNCTION_ARGS);
extern Datum pg_tablespace_databases(PG_FUNCTION_ARGS);
extern Datum pg_tablespace_location(PG_FUNCTION_ARGS);
extern Datum pg_rotate_logfile(PG_FUNCTION_ARGS);
extern Datum pg_sleep(PG_FUNCTION_ARGS);
extern Datum pg_get_keywords(PG_FUNCTION_ARGS);
extern Datum pg_typeof(PG_FUNCTION_ARGS);
extern Datum pg_collation_for(PG_FUNCTION_ARGS);
extern Datum pg_relation_is_updatable(PG_FUNCTION_ARGS);
extern Datum pg_column_is_updatable(PG_FUNCTION_ARGS);
extern Datum parse_ident(PG_FUNCTION_ARGS);

=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
/* oid.c */
extern oidvector *buildoidvector(const Oid *oids, int n);
extern Oid	oidparse(Node *node);
<<<<<<< HEAD

/* orderedsetaggs.c */
extern Datum ordered_set_transition(PG_FUNCTION_ARGS);
extern Datum ordered_set_transition_multi(PG_FUNCTION_ARGS);
extern Datum percentile_disc_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_float8_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_interval_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_timestamp_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_timestamptz_final(PG_FUNCTION_ARGS);
extern Datum percentile_disc_multi_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_float8_multi_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_interval_multi_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_timestamp_multi_final(PG_FUNCTION_ARGS);
extern Datum percentile_cont_timestamptz_multi_final(PG_FUNCTION_ARGS);
extern Datum mode_final(PG_FUNCTION_ARGS);
extern Datum hypothetical_rank_final(PG_FUNCTION_ARGS);
extern Datum hypothetical_percent_rank_final(PG_FUNCTION_ARGS);
extern Datum hypothetical_cume_dist_final(PG_FUNCTION_ARGS);
extern Datum hypothetical_dense_rank_final(PG_FUNCTION_ARGS);

/* pseudotypes.c */
extern Datum cstring_in(PG_FUNCTION_ARGS);
extern Datum cstring_out(PG_FUNCTION_ARGS);
extern Datum cstring_recv(PG_FUNCTION_ARGS);
extern Datum cstring_send(PG_FUNCTION_ARGS);
extern Datum any_in(PG_FUNCTION_ARGS);
extern Datum any_out(PG_FUNCTION_ARGS);
extern Datum anyarray_in(PG_FUNCTION_ARGS);
extern Datum anyarray_out(PG_FUNCTION_ARGS);
extern Datum anyarray_recv(PG_FUNCTION_ARGS);
extern Datum anyarray_send(PG_FUNCTION_ARGS);
extern Datum anynonarray_in(PG_FUNCTION_ARGS);
extern Datum anynonarray_out(PG_FUNCTION_ARGS);
extern Datum anyenum_in(PG_FUNCTION_ARGS);
extern Datum anyenum_out(PG_FUNCTION_ARGS);
extern Datum anyrange_in(PG_FUNCTION_ARGS);
extern Datum anyrange_out(PG_FUNCTION_ARGS);
extern Datum void_in(PG_FUNCTION_ARGS);
extern Datum void_out(PG_FUNCTION_ARGS);
extern Datum void_recv(PG_FUNCTION_ARGS);
extern Datum void_send(PG_FUNCTION_ARGS);
extern Datum trigger_in(PG_FUNCTION_ARGS);
extern Datum trigger_out(PG_FUNCTION_ARGS);
extern Datum event_trigger_in(PG_FUNCTION_ARGS);
extern Datum event_trigger_out(PG_FUNCTION_ARGS);
extern Datum language_handler_in(PG_FUNCTION_ARGS);
extern Datum language_handler_out(PG_FUNCTION_ARGS);
extern Datum fdw_handler_in(PG_FUNCTION_ARGS);
extern Datum fdw_handler_out(PG_FUNCTION_ARGS);
extern Datum index_am_handler_in(PG_FUNCTION_ARGS);
extern Datum index_am_handler_out(PG_FUNCTION_ARGS);
extern Datum tsm_handler_in(PG_FUNCTION_ARGS);
extern Datum tsm_handler_out(PG_FUNCTION_ARGS);
extern Datum internal_in(PG_FUNCTION_ARGS);
extern Datum internal_out(PG_FUNCTION_ARGS);
extern Datum opaque_in(PG_FUNCTION_ARGS);
extern Datum opaque_out(PG_FUNCTION_ARGS);
extern Datum anyelement_in(PG_FUNCTION_ARGS);
extern Datum anyelement_out(PG_FUNCTION_ARGS);
extern Datum shell_in(PG_FUNCTION_ARGS);
extern Datum shell_out(PG_FUNCTION_ARGS);
extern Datum pg_node_tree_in(PG_FUNCTION_ARGS);
extern Datum pg_node_tree_out(PG_FUNCTION_ARGS);
extern Datum pg_node_tree_recv(PG_FUNCTION_ARGS);
extern Datum pg_node_tree_send(PG_FUNCTION_ARGS);
extern Datum pg_ddl_command_in(PG_FUNCTION_ARGS);
extern Datum pg_ddl_command_out(PG_FUNCTION_ARGS);
extern Datum pg_ddl_command_recv(PG_FUNCTION_ARGS);
extern Datum pg_ddl_command_send(PG_FUNCTION_ARGS);
extern Datum anytable_in(PG_FUNCTION_ARGS);
extern Datum anytable_out(PG_FUNCTION_ARGS);
=======
extern int	oid_cmp(const void *p1, const void *p2);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/* regexp.c */
extern char *regexp_fixed_prefix(text *text_re, bool case_insensitive,
								 Oid collation, bool *exact);

/* ruleutils.c */
extern bool quote_all_identifiers;
<<<<<<< HEAD
extern Datum pg_get_ruledef(PG_FUNCTION_ARGS);
extern Datum pg_get_ruledef_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_viewdef(PG_FUNCTION_ARGS);
extern Datum pg_get_viewdef_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_viewdef_wrap(PG_FUNCTION_ARGS);
extern Datum pg_get_viewdef_name(PG_FUNCTION_ARGS);
extern Datum pg_get_viewdef_name_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_indexdef(PG_FUNCTION_ARGS);
extern Datum pg_get_indexdef_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_triggerdef(PG_FUNCTION_ARGS);
extern Datum pg_get_triggerdef_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_constraintdef(PG_FUNCTION_ARGS);
extern Datum pg_get_constraintdef_ext(PG_FUNCTION_ARGS);
extern char *pg_get_constraintexpr_string(Oid constraintId);
extern Datum pg_get_expr(PG_FUNCTION_ARGS);
extern Datum pg_get_expr_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_expr_ext_lock(PG_FUNCTION_ARGS);
extern Datum pg_get_userbyid(PG_FUNCTION_ARGS);
extern Datum pg_get_serial_sequence(PG_FUNCTION_ARGS);
extern Datum pg_get_functiondef(PG_FUNCTION_ARGS);
extern Datum pg_get_function_arguments(PG_FUNCTION_ARGS);
extern Datum pg_get_function_identity_arguments(PG_FUNCTION_ARGS);
extern Datum pg_get_function_result(PG_FUNCTION_ARGS);
extern Datum pg_get_function_arg_default(PG_FUNCTION_ARGS);
extern const char *quote_identifier(const char *ident);
extern char *quote_qualified_identifier(const char *qualifier,
						   const char *ident);
extern void generate_operator_clause(fmStringInfo buf,
						 const char *leftop, Oid leftoptype,
						 Oid opoid,
						 const char *rightop, Oid rightoptype);
extern List *set_deparse_context_planstate(List *dpcontext,
							  Node *planstate, List *ancestors);

extern Datum pg_get_partition_def(PG_FUNCTION_ARGS);
extern Datum pg_get_partition_def_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_partition_def_ext2(PG_FUNCTION_ARGS);
extern Datum pg_get_partition_rule_def(PG_FUNCTION_ARGS);
extern Datum pg_get_partition_rule_def_ext(PG_FUNCTION_ARGS);
extern Datum pg_get_partition_template_def(PG_FUNCTION_ARGS);

extern Datum pg_get_table_distributedby(PG_FUNCTION_ARGS);


/* tid.c */
extern Datum tidin(PG_FUNCTION_ARGS);
extern Datum tidout(PG_FUNCTION_ARGS);
extern Datum tidrecv(PG_FUNCTION_ARGS);
extern Datum tidsend(PG_FUNCTION_ARGS);
extern Datum tideq(PG_FUNCTION_ARGS);
extern Datum tidne(PG_FUNCTION_ARGS);
extern Datum tidlt(PG_FUNCTION_ARGS);
extern Datum tidle(PG_FUNCTION_ARGS);
extern Datum tidgt(PG_FUNCTION_ARGS);
extern Datum tidge(PG_FUNCTION_ARGS);
extern Datum bttidcmp(PG_FUNCTION_ARGS);
extern Datum tidlarger(PG_FUNCTION_ARGS);
extern Datum tidsmaller(PG_FUNCTION_ARGS);
extern Datum hashtid(PG_FUNCTION_ARGS);
extern Datum currtid_byreloid(PG_FUNCTION_ARGS);
extern Datum currtid_byrelname(PG_FUNCTION_ARGS);
=======
extern const char *quote_identifier(const char *ident);
extern char *quote_qualified_identifier(const char *qualifier,
										const char *ident);
extern void generate_operator_clause(fmStringInfo buf,
									 const char *leftop, Oid leftoptype,
									 Oid opoid,
									 const char *rightop, Oid rightoptype);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/* varchar.c */
extern int	bpchartruelen(char *s, int len);
<<<<<<< HEAD
extern Datum bpcharlen(PG_FUNCTION_ARGS);
extern Datum bpcharoctetlen(PG_FUNCTION_ARGS);
extern Datum hashbpchar(PG_FUNCTION_ARGS);
extern Datum bpchar_pattern_lt(PG_FUNCTION_ARGS);
extern Datum bpchar_pattern_le(PG_FUNCTION_ARGS);
extern Datum bpchar_pattern_gt(PG_FUNCTION_ARGS);
extern Datum bpchar_pattern_ge(PG_FUNCTION_ARGS);
extern Datum btbpchar_pattern_cmp(PG_FUNCTION_ARGS);
extern Datum btbpchar_pattern_sortsupport(PG_FUNCTION_ARGS);
extern Datum hashtext(PG_FUNCTION_ARGS);
extern Datum hashvarlena(PG_FUNCTION_ARGS);
=======
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/* popular functions from varlena.c */
extern text *cstring_to_text(const char *s);
extern text *cstring_to_text_with_len(const char *s, int len);
extern char *text_to_cstring(const text *t);
extern void text_to_cstring_buffer(const text *src, char *dst, size_t dst_len);

#define CStringGetTextDatum(s) PointerGetDatum(cstring_to_text(s))
#define TextDatumGetCString(d) text_to_cstring((text *) DatumGetPointer(d))

<<<<<<< HEAD
extern Datum textin(PG_FUNCTION_ARGS);
extern Datum textout(PG_FUNCTION_ARGS);
extern Datum textrecv(PG_FUNCTION_ARGS);
extern Datum textsend(PG_FUNCTION_ARGS);
extern Datum textcat(PG_FUNCTION_ARGS);
extern Datum texteq(PG_FUNCTION_ARGS);
extern Datum textne(PG_FUNCTION_ARGS);
extern Datum text_lt(PG_FUNCTION_ARGS);
extern Datum text_le(PG_FUNCTION_ARGS);
extern Datum text_gt(PG_FUNCTION_ARGS);
extern Datum text_ge(PG_FUNCTION_ARGS);
extern Datum text_larger(PG_FUNCTION_ARGS);
extern Datum text_smaller(PG_FUNCTION_ARGS);
extern Datum text_pattern_lt(PG_FUNCTION_ARGS);
extern Datum text_pattern_le(PG_FUNCTION_ARGS);
extern Datum text_pattern_gt(PG_FUNCTION_ARGS);
extern Datum text_pattern_ge(PG_FUNCTION_ARGS);
extern Datum bttext_pattern_cmp(PG_FUNCTION_ARGS);
extern Datum bttext_pattern_sortsupport(PG_FUNCTION_ARGS);
extern Datum textlen(PG_FUNCTION_ARGS);
extern Datum textoctetlen(PG_FUNCTION_ARGS);
extern Datum textpos(PG_FUNCTION_ARGS);
extern Datum text_substr(PG_FUNCTION_ARGS);
extern Datum text_substr_no_len(PG_FUNCTION_ARGS);
extern Datum textoverlay(PG_FUNCTION_ARGS);
extern Datum textoverlay_no_len(PG_FUNCTION_ARGS);
extern Datum name_text(PG_FUNCTION_ARGS);
extern Datum text_name(PG_FUNCTION_ARGS);
extern int	varstr_cmp(char *arg1, int len1, char *arg2, int len2, Oid collid);
extern void varstr_sortsupport(SortSupport ssup, Oid collid, bool bpchar);
extern int varstr_levenshtein(const char *source, int slen,
				   const char *target, int tlen,
				   int ins_c, int del_c, int sub_c,
				   bool trusted);
extern int varstr_levenshtein_less_equal(const char *source, int slen,
							  const char *target, int tlen,
							  int ins_c, int del_c, int sub_c,
							  int max_d, bool trusted);
extern List *textToQualifiedNameList(text *textval);
extern bool SplitIdentifierString(char *rawstring, char separator,
					  List **namelist);
extern bool SplitDirectoriesString(char *rawstring, char separator,
					   List **namelist);
extern bool SplitGUCList(char *rawstring, char separator,
			 List **namelist);
extern Datum replace_text(PG_FUNCTION_ARGS);
extern text *replace_text_regexp(text *src_text, void *regexp,
					text *replace_text, bool glob);
extern Datum split_text(PG_FUNCTION_ARGS);
extern Datum text_to_array(PG_FUNCTION_ARGS);
extern Datum array_to_text(PG_FUNCTION_ARGS);
extern Datum text_to_array_null(PG_FUNCTION_ARGS);
extern Datum array_to_text_null(PG_FUNCTION_ARGS);
extern Datum to_hex32(PG_FUNCTION_ARGS);
extern Datum to_hex64(PG_FUNCTION_ARGS);
extern Datum md5_text(PG_FUNCTION_ARGS);
extern Datum md5_bytea(PG_FUNCTION_ARGS);

extern Datum unknownin(PG_FUNCTION_ARGS);
extern Datum unknownout(PG_FUNCTION_ARGS);
extern Datum unknownrecv(PG_FUNCTION_ARGS);
extern Datum unknownsend(PG_FUNCTION_ARGS);

extern Datum pg_column_size(PG_FUNCTION_ARGS);

extern Datum bytea_string_agg_transfn(PG_FUNCTION_ARGS);
extern Datum bytea_string_agg_finalfn(PG_FUNCTION_ARGS);
extern Datum string_agg_transfn(PG_FUNCTION_ARGS);
extern Datum string_agg_finalfn(PG_FUNCTION_ARGS);

extern Datum text_concat(PG_FUNCTION_ARGS);
extern Datum text_concat_ws(PG_FUNCTION_ARGS);
extern Datum text_left(PG_FUNCTION_ARGS);
extern Datum text_right(PG_FUNCTION_ARGS);
extern Datum text_reverse(PG_FUNCTION_ARGS);
extern Datum text_format(PG_FUNCTION_ARGS);
extern Datum text_format_nv(PG_FUNCTION_ARGS);

/* version.c */
extern Datum pgsql_version(PG_FUNCTION_ARGS);

/* xid.c */
extern Datum xidin(PG_FUNCTION_ARGS);
extern Datum xidout(PG_FUNCTION_ARGS);
extern Datum xidrecv(PG_FUNCTION_ARGS);
extern Datum xidsend(PG_FUNCTION_ARGS);
extern Datum xideq(PG_FUNCTION_ARGS);
extern Datum xidlt(PG_FUNCTION_ARGS);
extern Datum xidgt(PG_FUNCTION_ARGS);
extern Datum xidle(PG_FUNCTION_ARGS);
extern Datum xidge(PG_FUNCTION_ARGS);
extern Datum btxidcmp(PG_FUNCTION_ARGS);
extern Datum xidneq(PG_FUNCTION_ARGS);
extern Datum xid_age(PG_FUNCTION_ARGS);
extern Datum mxid_age(PG_FUNCTION_ARGS);
=======
/* xid.c */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern int	xidComparator(const void *arg1, const void *arg2);

/* inet_cidr_ntop.c */
extern char *inet_cidr_ntop(int af, const void *src, int bits,
							char *dst, size_t size);

/* inet_net_pton.c */
extern int	inet_net_pton(int af, const char *src,
						  void *dst, size_t size);

/* network.c */
<<<<<<< HEAD
extern Datum inet_in(PG_FUNCTION_ARGS);
extern Datum inet_out(PG_FUNCTION_ARGS);
extern Datum inet_recv(PG_FUNCTION_ARGS);
extern Datum inet_send(PG_FUNCTION_ARGS);
extern Datum cidr_in(PG_FUNCTION_ARGS);
extern Datum cidr_out(PG_FUNCTION_ARGS);
extern Datum cidr_recv(PG_FUNCTION_ARGS);
extern Datum cidr_send(PG_FUNCTION_ARGS);
extern Datum network_cmp(PG_FUNCTION_ARGS);
extern Datum network_lt(PG_FUNCTION_ARGS);
extern Datum network_le(PG_FUNCTION_ARGS);
extern Datum network_eq(PG_FUNCTION_ARGS);
extern Datum network_ge(PG_FUNCTION_ARGS);
extern Datum network_gt(PG_FUNCTION_ARGS);
extern Datum network_ne(PG_FUNCTION_ARGS);
extern Datum network_smaller(PG_FUNCTION_ARGS);
extern Datum network_larger(PG_FUNCTION_ARGS);
extern Datum hashinet(PG_FUNCTION_ARGS);
extern Datum network_sub(PG_FUNCTION_ARGS);
extern Datum network_subeq(PG_FUNCTION_ARGS);
extern Datum network_sup(PG_FUNCTION_ARGS);
extern Datum network_supeq(PG_FUNCTION_ARGS);
extern Datum network_overlap(PG_FUNCTION_ARGS);
extern Datum network_network(PG_FUNCTION_ARGS);
extern Datum network_netmask(PG_FUNCTION_ARGS);
extern Datum network_hostmask(PG_FUNCTION_ARGS);
extern Datum network_masklen(PG_FUNCTION_ARGS);
extern Datum network_family(PG_FUNCTION_ARGS);
extern Datum network_broadcast(PG_FUNCTION_ARGS);
extern Datum network_host(PG_FUNCTION_ARGS);
extern Datum network_show(PG_FUNCTION_ARGS);
extern Datum inet_abbrev(PG_FUNCTION_ARGS);
extern Datum cidr_abbrev(PG_FUNCTION_ARGS);
extern double convert_network_to_scalar(Datum value, Oid typid, bool *failure);
extern Datum inet_to_cidr(PG_FUNCTION_ARGS);
extern Datum inet_set_masklen(PG_FUNCTION_ARGS);
extern Datum cidr_set_masklen(PG_FUNCTION_ARGS);
=======
extern double convert_network_to_scalar(Datum value, Oid typid, bool *failure);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
extern Datum network_scan_first(Datum in);
extern Datum network_scan_last(Datum in);
extern void clean_ipv6_addr(int addr_family, char *addr);

/* numeric.c */
<<<<<<< HEAD
extern Datum numeric_in(PG_FUNCTION_ARGS);
extern Datum numeric_out(PG_FUNCTION_ARGS);
extern Datum numeric_recv(PG_FUNCTION_ARGS);
extern Datum numeric_send(PG_FUNCTION_ARGS);
extern Datum numerictypmodin(PG_FUNCTION_ARGS);
extern Datum numerictypmodout(PG_FUNCTION_ARGS);
extern Datum numeric_transform(PG_FUNCTION_ARGS);
extern Datum numeric (PG_FUNCTION_ARGS);
extern Datum numeric_abs(PG_FUNCTION_ARGS);
extern Datum numeric_uminus(PG_FUNCTION_ARGS);
extern Datum numeric_uplus(PG_FUNCTION_ARGS);
extern Datum numeric_sign(PG_FUNCTION_ARGS);
extern Datum numeric_round(PG_FUNCTION_ARGS);
extern Datum numeric_trunc(PG_FUNCTION_ARGS);
extern Datum numeric_ceil(PG_FUNCTION_ARGS);
extern Datum numeric_floor(PG_FUNCTION_ARGS);
extern Datum numeric_sortsupport(PG_FUNCTION_ARGS);
extern Datum numeric_cmp(PG_FUNCTION_ARGS);
extern Datum numeric_eq(PG_FUNCTION_ARGS);
extern Datum numeric_ne(PG_FUNCTION_ARGS);
extern Datum numeric_gt(PG_FUNCTION_ARGS);
extern Datum numeric_ge(PG_FUNCTION_ARGS);
extern Datum numeric_lt(PG_FUNCTION_ARGS);
extern Datum numeric_le(PG_FUNCTION_ARGS);
extern Datum numeric_add(PG_FUNCTION_ARGS);
extern Datum numeric_sub(PG_FUNCTION_ARGS);
extern Datum numeric_mul(PG_FUNCTION_ARGS);
extern Datum numeric_div(PG_FUNCTION_ARGS);
extern Datum numeric_div_trunc(PG_FUNCTION_ARGS);
extern Datum numeric_mod(PG_FUNCTION_ARGS);
extern Datum numeric_inc(PG_FUNCTION_ARGS);
extern Datum numeric_dec(PG_FUNCTION_ARGS);
extern Datum numeric_smaller(PG_FUNCTION_ARGS);
extern Datum numeric_larger(PG_FUNCTION_ARGS);
extern Datum numeric_fac(PG_FUNCTION_ARGS);
extern Datum numeric_sqrt(PG_FUNCTION_ARGS);
extern Datum numeric_exp(PG_FUNCTION_ARGS);
extern Datum numeric_ln(PG_FUNCTION_ARGS);
extern Datum numeric_log(PG_FUNCTION_ARGS);
extern Datum numeric_power(PG_FUNCTION_ARGS);
extern Datum numeric_scale(PG_FUNCTION_ARGS);
extern Datum numeric_interval_bound(PG_FUNCTION_ARGS);
extern Datum numeric_interval_bound_shift(PG_FUNCTION_ARGS);
extern Datum numeric_interval_bound_shift_rbound(PG_FUNCTION_ARGS);
extern Datum int4_numeric(PG_FUNCTION_ARGS);
extern Datum numeric_int4(PG_FUNCTION_ARGS);
extern Datum int8_numeric(PG_FUNCTION_ARGS);
extern Datum numeric_int8(PG_FUNCTION_ARGS);
extern Datum int2_numeric(PG_FUNCTION_ARGS);
extern Datum numeric_int2(PG_FUNCTION_ARGS);
extern Datum float8_numeric(PG_FUNCTION_ARGS);
extern Datum numeric_float8(PG_FUNCTION_ARGS);
extern Datum numeric_float8_no_overflow(PG_FUNCTION_ARGS);
extern Datum float4_numeric(PG_FUNCTION_ARGS);
extern Datum numeric_float4(PG_FUNCTION_ARGS);
extern Datum numeric_accum(PG_FUNCTION_ARGS);
extern Datum numeric_combine(PG_FUNCTION_ARGS);
extern Datum numeric_avg_accum(PG_FUNCTION_ARGS);
extern Datum numeric_avg_combine(PG_FUNCTION_ARGS);
extern Datum numeric_avg_serialize(PG_FUNCTION_ARGS);
extern Datum numeric_avg_deserialize(PG_FUNCTION_ARGS);
extern Datum numeric_serialize(PG_FUNCTION_ARGS);
extern Datum numeric_deserialize(PG_FUNCTION_ARGS);
extern Datum numeric_accum_inv(PG_FUNCTION_ARGS);
extern Datum int2_accum(PG_FUNCTION_ARGS);
extern Datum int4_accum(PG_FUNCTION_ARGS);
extern Datum int8_accum(PG_FUNCTION_ARGS);
extern Datum numeric_poly_combine(PG_FUNCTION_ARGS);
extern Datum numeric_poly_serialize(PG_FUNCTION_ARGS);
extern Datum numeric_poly_deserialize(PG_FUNCTION_ARGS);
extern Datum int2_accum_inv(PG_FUNCTION_ARGS);
extern Datum int4_accum_inv(PG_FUNCTION_ARGS);
extern Datum int8_accum_inv(PG_FUNCTION_ARGS);
extern Datum int8_avg_accum(PG_FUNCTION_ARGS);
extern Datum int8_avg_combine(PG_FUNCTION_ARGS);
extern Datum int8_avg_serialize(PG_FUNCTION_ARGS);
extern Datum int8_avg_deserialize(PG_FUNCTION_ARGS);
extern Datum numeric_avg(PG_FUNCTION_ARGS);
extern Datum numeric_sum(PG_FUNCTION_ARGS);
extern Datum numeric_var_pop(PG_FUNCTION_ARGS);
extern Datum numeric_var_samp(PG_FUNCTION_ARGS);
extern Datum numeric_stddev_pop(PG_FUNCTION_ARGS);
extern Datum numeric_stddev_samp(PG_FUNCTION_ARGS);
extern Datum numeric_poly_sum(PG_FUNCTION_ARGS);
extern Datum numeric_poly_avg(PG_FUNCTION_ARGS);
extern Datum numeric_poly_var_pop(PG_FUNCTION_ARGS);
extern Datum numeric_poly_var_samp(PG_FUNCTION_ARGS);
extern Datum numeric_poly_stddev_pop(PG_FUNCTION_ARGS);
extern Datum numeric_poly_stddev_samp(PG_FUNCTION_ARGS);
extern Datum int2_sum(PG_FUNCTION_ARGS);
extern Datum int4_sum(PG_FUNCTION_ARGS);
extern Datum int8_sum(PG_FUNCTION_ARGS);
extern Datum int2_avg_accum(PG_FUNCTION_ARGS);
extern Datum int4_avg_accum(PG_FUNCTION_ARGS);
extern Datum int4_avg_combine(PG_FUNCTION_ARGS);
extern Datum int2_avg_accum_inv(PG_FUNCTION_ARGS);
extern Datum int4_avg_accum_inv(PG_FUNCTION_ARGS);
extern Datum int8_avg_accum_inv(PG_FUNCTION_ARGS);
extern Datum int8_avg(PG_FUNCTION_ARGS);
extern Datum float8_avg(PG_FUNCTION_ARGS);
extern Datum int2int4_sum(PG_FUNCTION_ARGS);
extern Datum width_bucket_numeric(PG_FUNCTION_ARGS);
extern Datum hash_numeric(PG_FUNCTION_ARGS);
extern Datum generate_series_numeric(PG_FUNCTION_ARGS);
extern Datum generate_series_step_numeric(PG_FUNCTION_ARGS);

/* complex_type.c */
extern Datum complex_cmp(PG_FUNCTION_ARGS);
extern Datum complex_lt(PG_FUNCTION_ARGS);
extern Datum complex_gt(PG_FUNCTION_ARGS);
extern Datum complex_gte(PG_FUNCTION_ARGS);
extern Datum complex_lte(PG_FUNCTION_ARGS);

/* ri_triggers.c */
extern Datum RI_FKey_check_ins(PG_FUNCTION_ARGS);
extern Datum RI_FKey_check_upd(PG_FUNCTION_ARGS);
extern Datum RI_FKey_noaction_del(PG_FUNCTION_ARGS);
extern Datum RI_FKey_noaction_upd(PG_FUNCTION_ARGS);
extern Datum RI_FKey_cascade_del(PG_FUNCTION_ARGS);
extern Datum RI_FKey_cascade_upd(PG_FUNCTION_ARGS);
extern Datum RI_FKey_restrict_del(PG_FUNCTION_ARGS);
extern Datum RI_FKey_restrict_upd(PG_FUNCTION_ARGS);
extern Datum RI_FKey_setnull_del(PG_FUNCTION_ARGS);
extern Datum RI_FKey_setnull_upd(PG_FUNCTION_ARGS);
extern Datum RI_FKey_setdefault_del(PG_FUNCTION_ARGS);
extern Datum RI_FKey_setdefault_upd(PG_FUNCTION_ARGS);

/* trigfuncs.c */
extern Datum suppress_redundant_updates_trigger(PG_FUNCTION_ARGS);

/* encoding support functions */
extern Datum getdatabaseencoding(PG_FUNCTION_ARGS);
extern Datum pg_client_encoding(PG_FUNCTION_ARGS);
extern Datum PG_encoding_to_char(PG_FUNCTION_ARGS);
extern Datum PG_char_to_encoding(PG_FUNCTION_ARGS);
extern Datum pg_convert(PG_FUNCTION_ARGS);
extern Datum pg_convert_to(PG_FUNCTION_ARGS);
extern Datum pg_convert_from(PG_FUNCTION_ARGS);
extern Datum length_in_encoding(PG_FUNCTION_ARGS);
extern Datum pg_encoding_max_length_sql(PG_FUNCTION_ARGS);
=======
extern Datum numeric_float8_no_overflow(PG_FUNCTION_ARGS);
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196

/* format_type.c */

/* Control flags for format_type_extended */
#define FORMAT_TYPE_TYPEMOD_GIVEN	0x01	/* typemod defined by caller */
#define FORMAT_TYPE_ALLOW_INVALID	0x02	/* allow invalid types */
#define FORMAT_TYPE_FORCE_QUALIFY	0x04	/* force qualification of type */
extern char *format_type_extended(Oid type_oid, int32 typemod, bits16 flags);

extern char *format_type_be(Oid type_oid);
extern char *format_type_be_qualified(Oid type_oid);
extern char *format_type_with_typemod(Oid type_oid, int32 typemod);

extern int32 type_maximum_size(Oid type_oid, int32 typemod);

/* quote.c */
extern char *quote_literal_cstr(const char *rawstr);

<<<<<<< HEAD
/* guc.c */
extern Datum show_config_by_name(PG_FUNCTION_ARGS);
extern Datum show_config_by_name_missing_ok(PG_FUNCTION_ARGS);
extern Datum set_config_by_name(PG_FUNCTION_ARGS);
extern Datum show_all_settings(PG_FUNCTION_ARGS);
extern Datum show_all_file_settings(PG_FUNCTION_ARGS);

/* pg_config.c */
extern Datum pg_config(PG_FUNCTION_ARGS);

/* pg_controldata.c */
extern Datum pg_control_checkpoint(PG_FUNCTION_ARGS);
extern Datum pg_control_system(PG_FUNCTION_ARGS);
extern Datum pg_control_init(PG_FUNCTION_ARGS);
extern Datum pg_control_recovery(PG_FUNCTION_ARGS);

/* rls.c */
extern Datum row_security_active(PG_FUNCTION_ARGS);
extern Datum row_security_active_name(PG_FUNCTION_ARGS);

/* lockfuncs.c */
extern Datum pg_lock_status(PG_FUNCTION_ARGS);
extern Datum pg_blocking_pids(PG_FUNCTION_ARGS);
extern Datum pg_advisory_lock_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_xact_lock_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_lock_shared_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_xact_lock_shared_int8(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_lock_int8(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_xact_lock_int8(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_lock_shared_int8(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_xact_lock_shared_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_unlock_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_unlock_shared_int8(PG_FUNCTION_ARGS);
extern Datum pg_advisory_lock_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_xact_lock_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_lock_shared_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_xact_lock_shared_int4(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_lock_int4(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_xact_lock_int4(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_lock_shared_int4(PG_FUNCTION_ARGS);
extern Datum pg_try_advisory_xact_lock_shared_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_unlock_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_unlock_shared_int4(PG_FUNCTION_ARGS);
extern Datum pg_advisory_unlock_all(PG_FUNCTION_ARGS);

/* txid.c */
extern Datum txid_snapshot_in(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_out(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_recv(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_send(PG_FUNCTION_ARGS);
extern Datum txid_current(PG_FUNCTION_ARGS);
extern Datum txid_current_snapshot(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_xmin(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_xmax(PG_FUNCTION_ARGS);
extern Datum txid_snapshot_xip(PG_FUNCTION_ARGS);
extern Datum txid_visible_in_snapshot(PG_FUNCTION_ARGS);

/* uuid.c */
extern Datum uuid_in(PG_FUNCTION_ARGS);
extern Datum uuid_out(PG_FUNCTION_ARGS);
extern Datum uuid_send(PG_FUNCTION_ARGS);
extern Datum uuid_recv(PG_FUNCTION_ARGS);
extern Datum uuid_lt(PG_FUNCTION_ARGS);
extern Datum uuid_le(PG_FUNCTION_ARGS);
extern Datum uuid_eq(PG_FUNCTION_ARGS);
extern Datum uuid_ge(PG_FUNCTION_ARGS);
extern Datum uuid_gt(PG_FUNCTION_ARGS);
extern Datum uuid_ne(PG_FUNCTION_ARGS);
extern Datum uuid_cmp(PG_FUNCTION_ARGS);
extern Datum uuid_sortsupport(PG_FUNCTION_ARGS);
extern Datum uuid_hash(PG_FUNCTION_ARGS);

/* windowfuncs.c */
extern Datum window_row_number(PG_FUNCTION_ARGS);
extern Datum window_rank(PG_FUNCTION_ARGS);
extern Datum window_dense_rank(PG_FUNCTION_ARGS);
extern Datum window_percent_rank(PG_FUNCTION_ARGS);
extern Datum window_cume_dist(PG_FUNCTION_ARGS);
extern Datum window_ntile(PG_FUNCTION_ARGS);
extern Datum window_lag(PG_FUNCTION_ARGS);
extern Datum window_lag_with_offset(PG_FUNCTION_ARGS);
extern Datum window_lag_with_offset_and_default(PG_FUNCTION_ARGS);
extern Datum window_lead(PG_FUNCTION_ARGS);
extern Datum window_lead_with_offset(PG_FUNCTION_ARGS);
extern Datum window_lead_with_offset_and_default(PG_FUNCTION_ARGS);
extern Datum window_first_value(PG_FUNCTION_ARGS);
extern Datum window_last_value(PG_FUNCTION_ARGS);
extern Datum window_nth_value(PG_FUNCTION_ARGS);

/* access/spgist/spgquadtreeproc.c */
extern Datum spg_quad_config(PG_FUNCTION_ARGS);
extern Datum spg_quad_choose(PG_FUNCTION_ARGS);
extern Datum spg_quad_picksplit(PG_FUNCTION_ARGS);
extern Datum spg_quad_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spg_quad_leaf_consistent(PG_FUNCTION_ARGS);

/* access/spgist/spgkdtreeproc.c */
extern Datum spg_kd_config(PG_FUNCTION_ARGS);
extern Datum spg_kd_choose(PG_FUNCTION_ARGS);
extern Datum spg_kd_picksplit(PG_FUNCTION_ARGS);
extern Datum spg_kd_inner_consistent(PG_FUNCTION_ARGS);

/* access/spgist/spgtextproc.c */
extern Datum spg_text_config(PG_FUNCTION_ARGS);
extern Datum spg_text_choose(PG_FUNCTION_ARGS);
extern Datum spg_text_picksplit(PG_FUNCTION_ARGS);
extern Datum spg_text_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spg_text_leaf_consistent(PG_FUNCTION_ARGS);

/* access/gin/ginarrayproc.c */
extern Datum ginarrayextract(PG_FUNCTION_ARGS);
extern Datum ginarrayextract_2args(PG_FUNCTION_ARGS);
extern Datum ginqueryarrayextract(PG_FUNCTION_ARGS);
extern Datum ginarrayconsistent(PG_FUNCTION_ARGS);
extern Datum ginarraytriconsistent(PG_FUNCTION_ARGS);

/* access/tablesample/bernoulli.c */
extern Datum tsm_bernoulli_handler(PG_FUNCTION_ARGS);

/* access/tablesample/system.c */
extern Datum tsm_system_handler(PG_FUNCTION_ARGS);

/* access/transam/twophase.c */
extern Datum pg_prepared_xact(PG_FUNCTION_ARGS);

/* access/transam/multixact.c */
extern Datum pg_get_multixact_members(PG_FUNCTION_ARGS);

/* access/transam/committs.c */
extern Datum pg_xact_commit_timestamp(PG_FUNCTION_ARGS);
extern Datum pg_last_committed_xact(PG_FUNCTION_ARGS);

/* catalogs/dependency.c */
extern Datum pg_describe_object(PG_FUNCTION_ARGS);
extern Datum pg_identify_object(PG_FUNCTION_ARGS);
extern Datum pg_identify_object_as_address(PG_FUNCTION_ARGS);

/* catalog/objectaddress.c */
extern Datum pg_get_object_address(PG_FUNCTION_ARGS);

/* commands/constraint.c */
extern Datum unique_key_recheck(PG_FUNCTION_ARGS);

/* commands/event_trigger.c */
extern Datum pg_event_trigger_dropped_objects(PG_FUNCTION_ARGS);
extern Datum pg_event_trigger_table_rewrite_oid(PG_FUNCTION_ARGS);
extern Datum pg_event_trigger_table_rewrite_reason(PG_FUNCTION_ARGS);
extern Datum pg_event_trigger_ddl_commands(PG_FUNCTION_ARGS);

/* commands/extension.c */
extern Datum pg_available_extensions(PG_FUNCTION_ARGS);
extern Datum pg_available_extension_versions(PG_FUNCTION_ARGS);
extern Datum pg_extension_update_paths(PG_FUNCTION_ARGS);
extern Datum pg_extension_config_dump(PG_FUNCTION_ARGS);

/* commands/prepare.c */
extern Datum pg_prepared_statement(PG_FUNCTION_ARGS);

/* utils/mmgr/portalmem.c */
extern Datum pg_cursor(PG_FUNCTION_ARGS);

/* utils/resscheduler/resqueue.c */
extern Datum pg_resqueue_status(PG_FUNCTION_ARGS);
extern Datum pg_resqueue_status_kv(PG_FUNCTION_ARGS);

/* utils/resgroup/resgroup.c */
extern Datum pg_resgroup_get_status(PG_FUNCTION_ARGS);
extern Datum pg_resgroup_get_status_kv(PG_FUNCTION_ARGS);

/* utils/gdd/gddfuncs.c */
extern Datum gp_dist_wait_status(PG_FUNCTION_ARGS);

/* utils/adt/matrix.c */
extern Datum matrix_add(PG_FUNCTION_ARGS);

/* utils/adt/pivot.c */
Datum int4_pivot_accum(PG_FUNCTION_ARGS);
Datum int8_pivot_accum(PG_FUNCTION_ARGS);
Datum float8_pivot_accum(PG_FUNCTION_ARGS);

/* utils/error/elog.c */
extern Datum gp_elog(PG_FUNCTION_ARGS);

/* utils/fmgr/deprecated.c */
extern Datum gp_deprecated(PG_FUNCTION_ARGS);

/* utils/gp/segadmin.c */
extern Datum gp_add_master_standby_port(PG_FUNCTION_ARGS);
extern Datum gp_add_master_standby(PG_FUNCTION_ARGS);
extern Datum gp_remove_master_standby(PG_FUNCTION_ARGS);
extern bool gp_activate_standby(void);

extern Datum gp_add_segment_primary(PG_FUNCTION_ARGS);
extern Datum gp_add_segment_mirror(PG_FUNCTION_ARGS);
extern Datum gp_remove_segment_mirror(PG_FUNCTION_ARGS);
extern Datum gp_add_segment(PG_FUNCTION_ARGS);
extern Datum gp_remove_segment(PG_FUNCTION_ARGS);

extern Datum gp_request_fts_probe_scan(PG_FUNCTION_ARGS);

/* storage/compress.c */
extern Datum zlib_constructor(PG_FUNCTION_ARGS);
extern Datum zlib_destructor(PG_FUNCTION_ARGS);
extern Datum zlib_compress(PG_FUNCTION_ARGS);
extern Datum zlib_decompress(PG_FUNCTION_ARGS);
extern Datum zlib_validator(PG_FUNCTION_ARGS);

extern Datum rle_type_constructor(PG_FUNCTION_ARGS);
extern Datum rle_type_destructor(PG_FUNCTION_ARGS);
extern Datum rle_type_compress(PG_FUNCTION_ARGS);
extern Datum rle_type_decompress(PG_FUNCTION_ARGS);
extern Datum rle_type_validator(PG_FUNCTION_ARGS);

extern Datum dummy_compression_constructor(PG_FUNCTION_ARGS);
extern Datum dummy_compression_destructor(PG_FUNCTION_ARGS);
extern Datum dummy_compression_compress(PG_FUNCTION_ARGS);
extern Datum dummy_compression_decompress(PG_FUNCTION_ARGS);
extern Datum dummy_compression_validator(PG_FUNCTION_ARGS);

/* gp_partition_functions.c */
struct EState;
extern void dumpDynamicTableScanPidIndex(struct EState *estate, int index);

/* XForms */
extern Datum disable_xform(PG_FUNCTION_ARGS);
extern Datum enable_xform(PG_FUNCTION_ARGS);

/* Optimizer's version */
extern Datum gp_opt_version(PG_FUNCTION_ARGS);

/* query_metrics.c */
extern Datum gp_instrument_shmem_summary(PG_FUNCTION_ARGS);

#endif   /* BUILTINS_H */
=======
#endif							/* BUILTINS_H */
>>>>>>> 9e1c9f959422192bbe1b842a2a1ffaf76b080196
