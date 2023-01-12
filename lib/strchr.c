/* $Id: strchr.c,v 1.2 1996/01/16 14:18:34 chris Exp $ */
#include "string.h"

/** char *strchr(p,c) return a ptr to c in p, return NUL if not found */
char           *
strchr (p, c)
     char           *p, c;
{

    if (!p)
	return (0);

    for (; *p; p++)
	if (*p == c)
	    return (p);
    return (0);
}
