/* $Id: strrpset.c,v 1.2 1996/01/16 14:18:48 chris Exp $ */
#include "string.h"

/** char *strrpset(str,set) like strrset except ignores inner parens */
char           *
strrpset (str, set)
     char           *str, *set;
{
    int             n;
    char           *p;

    n = 0;
    for (p = &str[strlen (str) - 1]; p > str; p--) {
	if (*p == '(')
	    n++;
	else if (*p == ')')
	    n--;
	else if (strchr (set, *p) && n == 0)
	    return (p);
    }
    return (0);
}
