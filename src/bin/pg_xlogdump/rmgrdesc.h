/*
 * rmgrdesc.h
 *
 * pg_xlogdump resource managers declaration
 *
 * src/bin/pg_xlogdump/rmgrdesc.h
 */
#ifndef RMGRDESC_H
#define RMGRDESC_H

#include "lib/stringinfo.h"

typedef struct RmgrDescData
{
	const char *rm_name;
<<<<<<< HEAD:contrib/pg_xlogdump/rmgrdesc.h
	void		(*rm_desc) (StringInfo buf, struct XLogRecord *record);
=======
	void		(*rm_desc) (StringInfo buf, XLogReaderState *record);
	const char *(*rm_identify) (uint8 info);
>>>>>>> ab93f90cd3a4fcdd891cee9478941c3cc65795b8:src/bin/pg_xlogdump/rmgrdesc.h
} RmgrDescData;

extern const RmgrDescData RmgrDescTable[];

#endif   /* RMGRDESC_H */
