/*
 * variable.h
 *		Routines for handling specialized SET variables.
 *
<<<<<<< HEAD
 * Portions Copyright (c) 1996-2009, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/variable.h,v 1.33 2009/01/01 17:23:58 momjian Exp $
=======
 * Portions Copyright (c) 1996-2008, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * $PostgreSQL: pgsql/src/include/commands/variable.h,v 1.32 2008/01/01 19:45:57 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef VARIABLE_H
#define VARIABLE_H

#include "utils/guc.h"


extern const char *assign_datestyle(const char *value,
				 bool doit, GucSource source);
extern const char *assign_timezone(const char *value,
				bool doit, GucSource source);
extern const char *show_timezone(void);
extern const char *assign_log_timezone(const char *value,
					bool doit, GucSource source);
extern const char *show_log_timezone(void);
extern const char *assign_XactIsoLevel(const char *value,
					bool doit, GucSource source);
extern const char *show_XactIsoLevel(void);
extern bool assign_random_seed(double value,
				   bool doit, GucSource source);
extern const char *show_random_seed(void);
extern const char *assign_client_encoding(const char *value,
					   bool doit, GucSource source);
extern const char *assign_role(const char *value,
			bool doit, GucSource source);
extern const char *show_role(void);
extern const char *assign_session_authorization(const char *value,
							 bool doit, GucSource source);
extern const char *show_session_authorization(void);

#endif   /* VARIABLE_H */
