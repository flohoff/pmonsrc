/* $Id: gets.c,v 1.2 1996/01/16 14:18:02 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  char *gets(p) get line from stdin
 *      unlike fgets, a trailing \n gets deleted
 */
char           *
gets (p)
     char           *p;
{
    char           *s;
    int             n;


    s = fgets (p, MAXLN, stdin);
    if (s == 0)
	return (0);
    n = strlen (p);
    if (n && p[n - 1] == '\n')
	p[n - 1] = 0;
    return (s);
}
