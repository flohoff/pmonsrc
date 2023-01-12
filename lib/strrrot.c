/* $Id: strrrot.c,v 1.2 1996/01/16 14:18:48 chris Exp $ */

char           *
strrrot (p)
     char           *p;
{
    int             n, t;

    n = strlen (p);
    if (n < 2)
	return (p);

    t = p[--n];
    for (--n; n >= 0; n--)
	p[n + 1] = p[n];
    *p = t;
    return (p);
}
