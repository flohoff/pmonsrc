/* $Id: strrset.c,v 1.2 1996/01/16 14:18:49 chris Exp $ */
#include "string.h"

/** char *strrset(p,set) returns a ptr to last char from set in p, else 0 */
char           *
strrset (p, set)
     char           *p, *set;
{
    int             i;

    for (i = strlen (p); i >= 0; i--) {
	if (strchr (set, p[i]))
	    return (&p[i]);
    }
    return (0);
}
