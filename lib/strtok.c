/* $Id: strtok.c,v 1.2 1996/01/16 14:18:53 chris Exp $ */
#include <string.h>

#ifdef TEST
main (argc, argv)
     int             argc;
     char           *argv[];
{
    char           *p;

    if (argc != 3)
	exit (-1);

    p = strtok (argv[1], argv[2]);
    printf ("%s\n", p);
    while (p = strtok (0, argv[2]))
	printf ("%s\n", p);
}
#endif

char           *
strtok (p, tok)
     char           *p, *tok;
{
    static char    *t;
    char           *r;
    int             n;

    if (p)
	t = p;

    r = t + strspn (t, tok);
    if (!(n = strcspn (r, tok)))
	return (0);
    t = r + n;
    if (*t)
	*t++ = 0;
    return (r);
}
