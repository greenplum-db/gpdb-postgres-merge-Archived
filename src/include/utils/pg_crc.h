/*
 * pg_crc.h
 *
 * PostgreSQL CRC support
 *
 * See Ross Williams' excellent introduction
 * A PAINLESS GUIDE TO CRC ERROR DETECTION ALGORITHMS, available from
 * http://www.ross.net/crc/ or several other net sites.
 *
 * We have three slightly different variants of a 32-bit CRC calculation:
 * CRC-32C (Castagnoli polynomial), CRC-32 (Ethernet polynomial), and a legacy
 * CRC-32 version that uses the lookup table in a funny way. They all consist
 * of four macros:
 *
 * INIT_<variant>(crc)
 *		Initialize a CRC accumulator
 *
 * COMP_<variant>(crc, data, len)
 *		Accumulate some (more) bytes into a CRC
 *
 * FIN_<variant>(crc)
 *		Finish a CRC calculation
 *
 * EQ_<variant>(c1, c2)
 *		Check for equality of two CRCs.
 *
 * The CRC-32C variant is in port/pg_crc32c.h.
 *
 * Portions Copyright (c) 1996-2015, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
<<<<<<< HEAD
 * src/include/utils/pg_crc.h
=======
 * $PostgreSQL: pgsql/src/include/utils/pg_crc.h,v 1.20 2008/11/14 20:21:07 tgl Exp $
>>>>>>> 38e9348282e
 */
#ifndef PG_CRC_H
#define PG_CRC_H

<<<<<<< HEAD
=======
/* ugly hack to let this be used in frontend and backend code on Cygwin */
#ifdef FRONTEND
#define CRCDLLIMPORT
#else
#define CRCDLLIMPORT PGDLLIMPORT
#endif

>>>>>>> 38e9348282e
typedef uint32 pg_crc32;

/*
 * CRC-32, the same used e.g. in Ethernet.
 *
 * This is currently only used in ltree and hstore contrib modules. It uses
 * the same lookup table as the legacy algorithm below. New code should
 * use the Castagnoli version instead.
 */
#define INIT_TRADITIONAL_CRC32(crc) ((crc) = 0xFFFFFFFF)
#define FIN_TRADITIONAL_CRC32(crc)	((crc) ^= 0xFFFFFFFF)
#define COMP_TRADITIONAL_CRC32(crc, data, len)	\
	COMP_CRC32_NORMAL_TABLE(crc, data, len, pg_crc32_table)
#define EQ_TRADITIONAL_CRC32(c1, c2) ((c1) == (c2))

/* Sarwate's algorithm, for use with a "normal" lookup table */
#define COMP_CRC32_NORMAL_TABLE(crc, data, len, table)			  \
do {															  \
	const unsigned char *__data = (const unsigned char *) (data); \
	uint32		__len = (len); \
\
	while (__len-- > 0) \
	{ \
		int		__tab_index = ((int) (crc) ^ *__data++) & 0xFF; \
		(crc) = table[__tab_index] ^ ((crc) >> 8); \
	} \
} while (0)

/*
 * The CRC algorithm used for WAL et al in pre-9.5 versions.
 *
 * This closely resembles the normal CRC-32 algorithm, but is subtly
 * different. Using Williams' terms, we use the "normal" table, but with
 * "reflected" code. That's bogus, but it was like that for years before
 * anyone noticed. It does not correspond to any polynomial in a normal CRC
 * algorithm, so it's not clear what the error-detection properties of this
 * algorithm actually are.
 *
 * We still need to carry this around because it is used in a few on-disk
 * structures that need to be pg_upgradeable. It should not be used in new
 * code.
 */
#define INIT_LEGACY_CRC32(crc) ((crc) = 0xFFFFFFFF)
#define FIN_LEGACY_CRC32(crc)	((crc) ^= 0xFFFFFFFF)
#define COMP_LEGACY_CRC32(crc, data, len)	\
	COMP_CRC32_REFLECTED_TABLE(crc, data, len, pg_crc32_table)
#define EQ_LEGACY_CRC32(c1, c2) ((c1) == (c2))

/*
 * Sarwate's algorithm, for use with a "reflected" lookup table (but in the
 * legacy algorithm, we actually use it on a "normal" table, see above)
 */
#define COMP_CRC32_REFLECTED_TABLE(crc, data, len, table) \
do {															  \
	const unsigned char *__data = (const unsigned char *) (data); \
	uint32		__len = (len); \
\
	while (__len-- > 0) \
	{ \
		int		__tab_index = ((int) ((crc) >> 24) ^ *__data++) & 0xFF; \
		(crc) = table[__tab_index] ^ ((crc) << 8); \
	} \
} while (0)

<<<<<<< HEAD
=======
/* Check for equality of two CRCs */
#define EQ_CRC32(c1,c2)  ((c1) == (c2))

/* Constant table for CRC calculation */
extern CRCDLLIMPORT const uint32 pg_crc32_table[];


#ifdef PROVIDE_64BIT_CRC

/*
 * If we have a 64-bit integer type, then a 64-bit CRC looks just like the
 * usual sort of implementation.  If we have no working 64-bit type, then
 * fake it with two 32-bit registers.  (Note: experience has shown that the
 * two-32-bit-registers code is as fast as, or even much faster than, the
 * 64-bit code on all but true 64-bit machines.  INT64_IS_BUSTED is therefore
 * probably the wrong control symbol to use to select the implementation.)
 */

#ifdef INT64_IS_BUSTED

>>>>>>> 38e9348282e
/*
 * Constant table for the CRC-32 polynomials. The same table is used by both
 * the normal and traditional variants.
 */
<<<<<<< HEAD
extern PGDLLIMPORT const uint32 pg_crc32_table[256];
=======
typedef struct pg_crc64
{
	uint32		crc0;
	uint32		crc1;
}	pg_crc64;

/* Initialize a CRC accumulator */
#define INIT_CRC64(crc) ((crc).crc0 = 0xffffffff, (crc).crc1 = 0xffffffff)

/* Finish a CRC calculation */
#define FIN_CRC64(crc)	((crc).crc0 ^= 0xffffffff, (crc).crc1 ^= 0xffffffff)

/* Accumulate some (more) bytes into a CRC */
#define COMP_CRC64(crc, data, len)	\
do { \
	uint32		__crc0 = (crc).crc0; \
	uint32		__crc1 = (crc).crc1; \
	unsigned char *__data = (unsigned char *) (data); \
	uint32		__len = (len); \
\
	while (__len-- > 0) \
	{ \
		int		__tab_index = ((int) (__crc1 >> 24) ^ *__data++) & 0xFF; \
		__crc1 = pg_crc64_table1[__tab_index] ^ ((__crc1 << 8) | (__crc0 >> 24)); \
		__crc0 = pg_crc64_table0[__tab_index] ^ (__crc0 << 8); \
	} \
	(crc).crc0 = __crc0; \
	(crc).crc1 = __crc1; \
} while (0)

/* Check for equality of two CRCs */
#define EQ_CRC64(c1,c2)  ((c1).crc0 == (c2).crc0 && (c1).crc1 == (c2).crc1)

/* Constant table for CRC calculation */
extern CRCDLLIMPORT const uint32 pg_crc64_table0[];
extern CRCDLLIMPORT const uint32 pg_crc64_table1[];
#else							/* int64 works */

typedef struct pg_crc64
{
	uint64		crc0;
}	pg_crc64;

/* Initialize a CRC accumulator */
#define INIT_CRC64(crc) ((crc).crc0 = UINT64CONST(0xffffffffffffffff))

/* Finish a CRC calculation */
#define FIN_CRC64(crc)	((crc).crc0 ^= UINT64CONST(0xffffffffffffffff))

/* Accumulate some (more) bytes into a CRC */
#define COMP_CRC64(crc, data, len)	\
do { \
	uint64		__crc0 = (crc).crc0; \
	unsigned char *__data = (unsigned char *) (data); \
	uint32		__len = (len); \
\
	while (__len-- > 0) \
	{ \
		int		__tab_index = ((int) (__crc0 >> 56) ^ *__data++) & 0xFF; \
		__crc0 = pg_crc64_table[__tab_index] ^ (__crc0 << 8); \
	} \
	(crc).crc0 = __crc0; \
} while (0)

/* Check for equality of two CRCs */
#define EQ_CRC64(c1,c2)  ((c1).crc0 == (c2).crc0)

/* Constant table for CRC calculation */
extern CRCDLLIMPORT const uint64 pg_crc64_table[];
#endif   /* INT64_IS_BUSTED */
#endif   /* PROVIDE_64BIT_CRC */
>>>>>>> 38e9348282e

#endif   /* PG_CRC_H */
