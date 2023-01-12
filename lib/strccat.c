/* $Id: strccat.c,v 1.2 1996/01/16 14:18:33 chris Exp $ */
#include "string.h"

/** char *strccat(dst,c)  concatinate char to dst string */
char           *
strccat (dst, c)
     char           *dst, c;
{
    int             len;

    if (!dst)
	return (dst);
    len = strlen (dst);
    dst[len] = c;
    dst[len + 1] = 0;
    return (dst);
}
