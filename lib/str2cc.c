/* $Id: str2cc.c,v 1.2 1996/01/16 14:18:30 chris Exp $ */
#include "string.h"

/** str2cc(p) convert the string p into a control char */
str2cc (p)
     char           *p;
{
    int             len;

    if (!p)
	return (0);
    len = strlen (p);
    if (len < 1 || len > 2)
	return (0);
    if (len == 1)
	return (p[0]);
    if (p[1] < 'A' || p[1] > '~')
	return (0);
    if (p[0] != '^')
	return (0);
    return (p[1] & 0x1f);
}
