/* $Id: toupper.c,v 1.2 1996/01/16 14:18:58 chris Exp $ */
#include "string.h"

/** toupper(c) translate c to uppercase */
int 
toupper (c)
     int             c;
{

    if (islower (c))
	return (c - ('a' - 'A'));
    return (c);
}
