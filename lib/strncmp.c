/* $Id: strncmp.c,v 1.2 1996/01/16 14:18:43 chris Exp $ */
#include "string.h"

/** int strncmp(s1,s2,n) as strcmp, but compares at most n characters */
int 
strncmp (s1, s2, n)
     char           *s1, *s2;
     int             n;
{

    if (!s1 || !s2)
	return (0);

    while (n && (*s1 == *s2)) {
	if (*s1 == 0)
	  return (0);
	s1++;
	s2++;
	n--;
    }
    if (n)
      return (*s1 - *s2);
    return (0);
}
