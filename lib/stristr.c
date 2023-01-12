/* $Id: stristr.c,v 1.2 1996/01/16 14:18:40 chris Exp $ */
#include "string.h"

/** stristr(dst,p) insert string p into dst */
stristr (dst, p)
     char           *dst, *p;
{
    int             i;

    for (i = strlen (p); i > 0; i--)
	strichr (dst++, *p++);
}
