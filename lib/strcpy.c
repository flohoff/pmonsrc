/* $Id: strcpy.c,v 1.2 1996/01/16 14:18:35 chris Exp $ */
#include "string.h"

/** char *strcpy(dst,src) copy src to dst */
char           *
strcpy (dstp, srcp)
     char           *dstp, *srcp;
{
    char           *dp = dstp;

    if (!dstp)
	return (0);
    *dp = 0;
    if (!srcp)
	return (dstp);

    while ((*dp++ = *srcp++) != 0);
    return (dstp);
}
