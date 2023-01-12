/* $Id: strrchr.c,v 1.2 1996/01/16 14:18:47 chris Exp $ */
#include "string.h"

/** char *strrchr(p,c) return a ptr to last c in p, return NUL if not found */
char           *
strrchr (p, c)
     char           *p, c;
{
    int             i;

    if (!p)
	return (0);

    for (i = strlen (p); i >= 0; i--)
	if (p[i] == c)
	    return (&p[i]);
    return (0);
}
