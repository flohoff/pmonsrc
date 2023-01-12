/* $Id: strncat.c,v 1.2 1996/01/16 14:18:42 chris Exp $ */
#include "string.h"

/** char *strncat(dst,src,n) concatinate n chars from src to dst */
char           *
strncat (dst, src, n)
     char           *dst, *src;
     int             n;
{
    char           *d;

    if (!dst || !src)
	return (dst);
    d = dst;
    for (; *d; d++);
    for (; *src && n; src++, n--)
	*d++ = *src;
    *d = 0;
    return (dst);
}
