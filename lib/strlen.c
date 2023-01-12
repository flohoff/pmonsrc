/* $Id: strlen.c,v 1.2 1996/01/16 14:18:41 chris Exp $ */
#include "string.h"

/** int strlen(p) returns the length of p */
int 
strlen (p)
     char           *p;
{
    int             n;

    if (!p)
	return (0);
    for (n = 0; *p; p++)
	n++;
    return (n);
}
