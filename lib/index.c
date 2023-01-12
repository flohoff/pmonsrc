/* $Id: index.c,v 1.2 1996/01/16 14:18:04 chris Exp $ */
#include "string.h"

/** char *index(s,c) returns ptr to 1st c in s, else 0 */
char           *
index (s, c)
     char           *s;
     int             c;
{

    if (s == 0)
	return (0);

    for (; *s; s++) {
	if (*s == c)
	    return (s);
    }
    return (0);
}
