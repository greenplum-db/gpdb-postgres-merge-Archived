#include "libpq-fe.h"

extern const char *progname;
<<<<<<< HEAD
extern char *connection_string;
=======
>>>>>>> 80edfd76591fdb9beec061de3c05ef4e9d96ce56
extern char *dbhost;
extern char *dbuser;
extern char *dbport;
extern int	dbgetpassword;

/* Connection kept global so we can disconnect easily */
extern PGconn *conn;

#define disconnect_and_exit(code)				\
	{											\
	if (conn != NULL) PQfinish(conn);			\
	exit(code);									\
	}


char	   *xstrdup(const char *s);
void	   *xmalloc0(int size);

PGconn	   *GetConnection(void);
