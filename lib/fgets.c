/* $Id: fgets.c,v 1.2 1996/01/16 14:17:53 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  char *fgets(dst,max,fp) get string from stream
 */
char           *
fgets (dst, max, fp)
     char           *dst;
     int             max;
     FILE           *fp;
{
    int             c;
    char           *p;

/* get max bytes or upto a newline */

    for (p = dst, max--; max > 0; max--) {
	if ((c = fgetc (fp)) == EOF)
	    break;
	*p++ = c;
	if (c == '\n')
	    break;
    }
    *p = 0;
    if (p == dst)
	return (0);
    return (p);
}
