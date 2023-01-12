/* $Id: atof.c,v 1.2 1996/01/16 14:17:37 chris Exp $ */
/*
 * ** should ignore leading white space, and give up on first bad char
 */
#ifdef FLOATINGPT
double 
atof (p)
     char           *p;
{
    double          d, t;
    int             len, val, sz, div, isneg;
    char            tmp[18];

    if (*p == '-') {
	isneg = 1;
	p++;
    } else
	isneg = 0;

    sz = strcspn (p, ".");
    if (sz > 0) {
	strncpy (tmp, p, sz);
	tmp[sz] = 0;
	if (!atob (&val, tmp, 10))
	    return (d);
    } else
	val = 0;

    d = (double)val;
    p += sz;
    if (*p)
	p++;
    if (*p) {
	len = strlen (p);
	if (!atob (&val, p, 10))
	    return (0);

	div = 1;
	for (; len > 0; len--)
	    div *= 10;

	t = (double)val;
	t /= div;

	d += t;
    }
    if (isneg)
	d = 0 - d;
    return (d);
}
#else
atof ()
{
    return (0);
}
#endif
