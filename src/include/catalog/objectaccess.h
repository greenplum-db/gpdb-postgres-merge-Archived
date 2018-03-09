/*
 * objectaccess.h
 *
 *		Object access hooks.
 *
 * Portions Copyright (c) 1996-2011, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 */

#ifndef OBJECTACCESS_H
#define OBJECTACCESS_H

/*
 * Object access hooks are intended to be called just before or just after
<<<<<<< HEAD
 * performing certain actions on a SQL object.  This is intended as
=======
 * performing certain actions on a SQL object.	This is intended as
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
 * infrastructure for security or logging pluggins.
 *
 * OAT_POST_CREATE should be invoked just after the the object is created.
 * Typically, this is done after inserting the primary catalog records and
 * associated dependencies.
 *
 * Other types may be added in the future.
 */
typedef enum ObjectAccessType
{
<<<<<<< HEAD
	OAT_POST_CREATE
=======
	OAT_POST_CREATE,
>>>>>>> a4bebdd92624e018108c2610fc3f2c1584b6c687
} ObjectAccessType;

/*
 * Hook, and a macro to invoke it.
 */

typedef void (*object_access_hook_type) (ObjectAccessType access,
													 Oid classId,
													 Oid objectId,
													 int subId);

extern PGDLLIMPORT object_access_hook_type object_access_hook;

#define InvokeObjectAccessHook(access,classId,objectId,subId)			\
	do {																\
		if (object_access_hook)											\
			(*object_access_hook)((access),(classId),(objectId),(subId)); \
	} while(0)

#endif   /* OBJECTACCESS_H */
