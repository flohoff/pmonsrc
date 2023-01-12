/* $Id: strnchr.c,v 1.1 1996/09/27 00:45:04 chris Exp $ */
#include "string.h"

/** char *strnchr(p,c, n) return a ptr to c in p, return NUL if not found */
char           *
strnchr (p, c, n)
     char           *p, c;
     int	    n;
{

    if (!p)
	return (0);

    while (n-- > 0) {
	if (*p == c)
	    return (p);
	p++;
    }
    return (0);
}
