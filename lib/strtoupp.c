/* $Id: strtoupp.c,v 1.2 1996/01/16 14:18:53 chris Exp $ */
#include "string.h"

/** strtoupper(p) convert p to uppercase */
strtoupper (p)
     char           *p;
{
    if (!p)
	return;
    for (; *p; p++)
	*p = toupper (*p);
}
