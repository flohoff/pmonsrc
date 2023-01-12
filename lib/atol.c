/* $Id: atol.c,v 1.2 1996/01/16 14:17:39 chris Exp $ */
#include "string.h"

/** long atol(p) converts p to long */
long 
atol (p)
     char           *p;
{
    int             digit, isneg;
    long            value;

    isneg = 0;
    value = 0;
    for (; isspace (*p); p++);	/* gobble up leading whitespace */

/* do I have a sign? */
    if (*p == '-') {
	isneg = 1;
	p++;
    } else if (*p == '+')
	p++;

    for (; *p; p++) {
	value *= 10;
	if (*p >= '0' && *p <= '9')
	    digit = *p - '0';
	else
	    break;
	value += digit;
    }

    if (isneg)
	value = 0 - value;
    return (value);
}
