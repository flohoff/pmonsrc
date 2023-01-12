/* $Id: strempty.c,v 1.2 1996/01/16 14:18:37 chris Exp $ */
#include "string.h"

/** strempty(p) returns 1 if p contains nothing but isspace */
strempty (p)
     char           *p;
{

    if (!p)
	return (1);
    for (; *p; p++)
	if (!isspace (*p))
	    return (0);
    return (1);
}
