/* $Id: strset.c,v 1.2 1996/01/16 14:18:50 chris Exp $ */
#include "string.h"

/** char *strset(p,set) returns a ptr to 1st char from set in p, else 0 */
char           *
strset (p, set)
     char           *p, *set;
{

    for (; *p; p++) {
	if (strchr (set, *p))
	    return (p);
    }
    return (0);
}
