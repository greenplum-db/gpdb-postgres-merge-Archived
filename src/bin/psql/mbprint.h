<<<<<<< HEAD
/* $PostgreSQL: pgsql/src/bin/psql/mbprint.h,v 1.15 2009/11/25 20:26:31 tgl Exp $ */
=======
/* $PostgreSQL: pgsql/src/bin/psql/mbprint.h,v 1.11.4.1 2008/05/29 19:34:43 tgl Exp $ */
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
#ifndef MBPRINT_H
#define MBPRINT_H


struct lineptr
{
	unsigned char *ptr;
	int			width;
};

extern unsigned char *mbvalidate(unsigned char *pwcs, int encoding);
extern int	pg_wcswidth(const unsigned char *pwcs, size_t len, int encoding);
extern void pg_wcsformat(unsigned char *pwcs, size_t len, int encoding, struct lineptr * lines, int count);
extern void pg_wcssize(unsigned char *pwcs, size_t len, int encoding,
<<<<<<< HEAD
		   int *width, int *height, int *format_size);
=======
					   int *width, int *height, int *format_size);
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588

#endif   /* MBPRINT_H */
