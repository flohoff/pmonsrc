/* $Id: strcat.c,v 1.2 1996/01/16 14:18:32 chris Exp $ */
#include "string.h"

/** char *strcat(dst,src)  concatinate src string to dst string */
char           *
strcat (dst, src)
     char           *dst, *src;
{
    char           *d;

    if (!dst || !src)
	return (dst);
    d = dst;
    for (; *d; d++);
    for (; *src; src++)
	*d++ = *src;
    *d = 0;
    return (dst);
}
