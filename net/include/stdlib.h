/* $Id: stdlib.h,v 1.2 1996/01/16 14:19:59 chris Exp $ */
/*
 * stdlib.h : ANSI stdlib definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#ifndef __STDLIB_H
#define __STDLIB_H

/* 
 * This is a ANSI-C header file only
*/

/* --- Inclusions --- */
#include <stddef.h>

/* --- Constants --- */
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#ifdef PROM
#define EXIT_USAGE	-1
#endif

/* --- Prototype --- */
#include <sys/cdefs.h>

long	strtol	__P((const char *, char **, int));
unsigned long strtoul __P((const char *, char **, int));

/* --- Memory management --- */
void	free	__P((void *));
void	*malloc	__P((size_t));
void	*realloc __P((void *, size_t));
void 	*calloc	__P((size_t, size_t));
void	cfree __P((void *));

/* system hooks */
void	exit	__P((int));

/* enviroment hooks */
char	*getenv	__P((const char *));

/* --- Macros --- */
#define atoi(nptr)	((int) strtol((nptr), NULL, 10))
#define atol(nptr)	strtol((nptr), NULL, 10)

#endif /* !defined __STDLIB_H */
