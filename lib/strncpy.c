/* $Id: strncpy.c,v 1.2 1996/01/16 14:18:44 chris Exp $ */
#include "string.h"

/** char *strncpy(dst,src,n) copy n chars from src to dst */
char           *
strncpy (dst, src, n)
     char           *dst, *src;
     int             n;
{
    char           *d;

    if (!dst || !src)
	return (dst);
    d = dst;
    for (; *src && n; d++, src++, n--)
	*d = *src;
    while (n--)
	*d++ = '\0';
    return (dst);
}
