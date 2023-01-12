/* $Id: strdchr.c,v 1.2 1996/01/16 14:18:36 chris Exp $ */
#include "string.h"

/** char *strdchr(p) deletes the first char from the string p */
char           *
strdchr (p)
     char           *p;
{
    char           *t;

    if (!p)
	return (p);
    for (t = p; *t; t++)
	*t = *(t + 1);
    return (p);
}
