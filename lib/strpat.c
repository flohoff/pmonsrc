/* $Id: strpat.c,v 1.2 1996/01/16 14:18:45 chris Exp $ */
#include <stdio.h>
#include "string.h"

#define BANCHOR		(0x80|'^')
#define EANCHOR		(0x80|'$')

/** int strpat(p,pat) return 1 if pat matches p, else 0; wildcards * and ? */
int 
strpat (s1, s2)
     char           *s1, *s2;
{
    char           *p, *pat;
    char           *t, tmp[MAXLN];
    char            src1[MAXLN], src2[MAXLN];

    if (!s1 || !s2)
	return (0);

    p = src1;
    pat = src2;
    *p++ = BANCHOR;
    while (*s1)
	*p++ = *s1++;
    *p++ = EANCHOR;
    *p = 0;
    *pat++ = BANCHOR;
    while (*s2)
	*pat++ = *s2++;
    *pat++ = EANCHOR;
    *pat = 0;

    p = src1;
    pat = src2;
    for (; *p && *pat;) {
	if (*pat == '*') {
	    pat++;
	    for (t = pat; *t && *t != '*' && *t != '?'; t++);
	    strncpy (tmp, pat, t - pat);
	    tmp[t - pat] = '\0';
	    pat = t;
	    t = strposn (p, tmp);
	    if (t == 0)
		return (0);
	    p = t + strlen (tmp);
	} else if (*pat == '?' || *pat == *p) {
	    pat++;
	    p++;
	} else
	    return (0);
    }
    if (!*p && !*pat)
	return (1);
    if (!*p && *pat == '*' && !*(pat + 1))
	return (1);
    return (0);
}
