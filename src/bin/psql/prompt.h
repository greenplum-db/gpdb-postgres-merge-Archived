/*
 * psql - the PostgreSQL interactive terminal
 *
<<<<<<< HEAD
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
 * src/bin/psql/prompt.h
=======
 * Copyright (c) 2000-2008, PostgreSQL Global Development Group
 *
 * $PostgreSQL: pgsql/src/bin/psql/prompt.h,v 1.19 2008/01/01 19:45:56 momjian Exp $
>>>>>>> 632e7b6353a99dd139b999efce4cb78db9a1e588
 */
#ifndef PROMPT_H
#define PROMPT_H

typedef enum _promptStatus
{
	PROMPT_READY,
	PROMPT_CONTINUE,
	PROMPT_COMMENT,
	PROMPT_SINGLEQUOTE,
	PROMPT_DOUBLEQUOTE,
	PROMPT_DOLLARQUOTE,
	PROMPT_PAREN,
	PROMPT_COPY
} promptStatus_t;

char	   *get_prompt(promptStatus_t status);

#endif   /* PROMPT_H */
