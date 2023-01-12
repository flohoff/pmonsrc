/* $Id: strstr.c,v 1.2 1996/01/16 14:18:52 chris Exp $ */
#include "string.h"

/** char *strstr(p,q) returns a ptr to q in p, else 0 if not found */
char           *
strstr (p, q)
     char           *p, *q;
{
    char           *s, *t;

    if (!p || !q)
	return (0);

    if (!*q)
	return (p);
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
