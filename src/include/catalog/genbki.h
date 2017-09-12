/*-------------------------------------------------------------------------
 *
 * genbki.h
 *	  Required include file for all POSTGRES catalog header files
 *
 * genbki.h defines CATALOG(), DATA(), BKI_BOOTSTRAP and related macros
 * so that the catalog header files can be read by the C compiler.
 * (These same words are recognized by genbki.sh to build the BKI
 * bootstrap file from these header files.)
 *
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/genbki.h,v 1.3 2009/06/11 14:49:09 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef GENBKI_H
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/catalog/genbki.h,v 1.1 2008/03/27 03:57:34 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#ifndef GENBKI_H 
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
#define GENBKI_H

/* Introduces a catalog's structure definition */
#define CATALOG(name,oid)	typedef struct CppConcat(FormData_,name)

/* Options that may appear after CATALOG (on the same line) */
#define BKI_BOOTSTRAP
#define BKI_SHARED_RELATION
#define BKI_WITHOUT_OIDS

/* Declarations that provide the initial content of a catalog */
/* In C, these need to expand into some harmless, repeatable declaration */
#define DATA(x)   extern int no_such_variable
#define DESCR(x)  extern int no_such_variable
#define SHDESCR(x) extern int no_such_variable

<<<<<<< HEAD
/* for process_col_defaults.pl */
#define GPDB_COLUMN_DEFAULT(col, default) extern int no_such_variable
#define GPDB_EXTRA_COL(x) extern int no_such_variable

/* for process_foreign_keys.pl */
#define FOREIGN_KEY(x) extern int no_such_variable

=======
>>>>>>> f260edb144c1e3f33d5ecc3d00d5359ab675d238
/* PHONY type definition for use in catalog structure definitions only */
typedef int aclitem;

#endif   /* GENBKI_H */
