/* $Id: strpbrk.c,v 1.2 1996/01/16 14:18:46 chris Exp $ */
#include "string.h"

/** char *strpbrk(s1,s2) returns ptr to 1st char from s2 found in s1 */
char           *
strpbrk (s1, s2)
     char           *s1, *s2;
{

    for (; *s1; s1++) {
	if (strchr (s2, *s1) != 0)
	    return (s1);
    }
    return (0);
}
