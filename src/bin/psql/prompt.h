/*
 * psql - the PostgreSQL interactive terminal
 *
 * Copyright (c) 2000-2010, PostgreSQL Global Development Group
 *
<<<<<<< HEAD
 * src/bin/psql/prompt.h
=======
 * $PostgreSQL: pgsql/src/bin/psql/prompt.h,v 1.21 2010/01/02 16:57:59 momjian Exp $
>>>>>>> 1084f317702e1a039696ab8a37caf900e55ec8f2
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
