/* $Id: strnwrd.c,v 1.2 1996/01/16 14:18:44 chris Exp $ */
#include "string.h"

/** strnwrd(p) returns a count of words in p */
strnwrd (p)
     char           *p;
{
    int             n;

    if (!p)
	return (0);

    for (n = 0; *p; n++) {
	while (isspace (*p))
	    p++;
	if (!*p)
	    return (n);
	while (!isspace (*p) && *p != 0)
	    p++;
    }
    return (n);
}
