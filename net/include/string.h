/* $Id: string.h,v 1.2 1996/01/16 14:19:59 chris Exp $ */
/*
 * string.h : ANSI string function definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */
#ifndef __STRING_H
#define __STRING_H

#include <stddef.h>

#include <sys/cdefs.h>

void	*memcpy	__P((void *, const void *, size_t));
int	memcmp	__P((const void *, const void *, size_t));
void	*memset	__P((void *, int , size_t));

char	*strcat	__P((char *, const char *));
char	*strcpy	__P((char *, const char *));
char	*strncpy __P((char *, const char *, size_t));
int	strcmp	__P((const char *, const char *));
char	*strchr	__P((const char *, int));
char	*strpbrk __P((const char *, const char *));
size_t	strlen	__P((const char *));

/* BSDisms */
#define index	strchr
#define rindex	strrchr

#endif /* !defined __STRING_H */
