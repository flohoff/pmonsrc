/* $Id: strichr.c,v 1.2 1996/01/16 14:18:38 chris Exp $ */
#include "string.h"

/** char *strichr(p,c) inserts c as the first char of the string p */
char           *
strichr (p, c)
     char           *p;
     int             c;
{
    char           *t;

    if (!p)
	return (p);
    for (t = p; *t; t++);
    for (; t >= p; t--)
	*(t + 1) = *t;
    *p = c;
    return (p);
}
