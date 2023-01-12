/* $Id: cc2str.c,v 1.2 1996/01/16 14:17:42 chris Exp $ */
#include "stdio.h"
#include "string.h"

/** char *cc2str(p,c) convert a control char to a string, result in p */
char           *
cc2str (p, c)
     char           *p, c;
{

    if (iscntrl (c)) {
	p[0] = '^';
	p[1] = c | 0x40;
	p[2] = 0;
    } else {
	p[0] = c;
	p[1] = 0;
    }
    return (p);
}
