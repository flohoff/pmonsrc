/* $Id: strposn.c,v 1.2 1996/01/16 14:18:46 chris Exp $ */
#include "string.h"

/** char *strposn(p,q) returns a ptr to q in p, else 0 if not found */
char           *
strposn (p, q)
     char           *p, *q;
{
    char           *s, *t;

    if (!p || !q)
	return (0);

    if (!*q)
	return (p + strlen (p));
    for (; *p; p++) {
	if (*p == *q) {
	    t = p;
	    s = q;
	    for (; *t; s++, t++) {
		if (*t != *s)
		    break;
	    }
	    if (!*s)
		return (p);
	}
    }
    return (0);
}
