/* $Id: strbequ.c,v 1.2 1996/01/16 14:18:32 chris Exp $ */
#include "string.h"

/** int strbequ(s1,s2) return 1 if s2 matches 1st part of s1 */
int 
strbequ (s1, s2)
     char           *s1, *s2;
{

    if (!s1 || !s2)
	return (0);
    for (; *s1 && *s2; s1++, s2++)
	if (*s1 != *s2)
	    return (0);
    if (!*s2)
	return (1);
    return (0);
}
