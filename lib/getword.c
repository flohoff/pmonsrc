/* $Id: getword.c,v 1.2 1996/01/16 14:18:03 chris Exp $ */
#include "string.h"

/** char *getword(dst,p) copies next word from p into dst, else rtns 0 */
char           *
getword (dst, p)
     char           *p, *dst;
{
    char           *a;

    if (!dst || !p)
	return (0);

    dst[0] = 0;
    while (isspace (*p))
	p++;
    if (*p == 0)
	return (0);
    a = p;
    while (!isspace (*p) && *p != 0)
	p++;
    strncpy (dst, a, p - a);
    dst[p - a] = 0;
    return (p);
}

/* int wordsz(p) return size of first word in p */
int 
wordsz (p)
     char           *p;
{
    int             n;

    if (!p)
	return (0);

    while (isspace (*p))
	p++;
    for (n = 0; !*p && !isspace (*p); n++)
	p++;
    return (n);
}
