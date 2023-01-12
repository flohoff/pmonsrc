/* $Id: calloc.c,v 1.2 1996/01/16 14:17:41 chris Exp $ */
#include <malloc.h>

char           *
calloc (nelem, elsize)
     unsigned        nelem, elsize;
{
    char           *p;

    p = malloc (nelem * elsize);
    if (p == 0)
	return (p);

    bzero (p, nelem * elsize);
    return (p);
}
