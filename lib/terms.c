/* $Id: terms.c,v 1.2 1996/01/16 14:18:56 chris Exp $ */
#include "stdio.h"
#include "termio.h"

tvi920 (fd, op, a1, a2)
     int             fd, op, a1, a2;
{
    switch (op) {
    case TT_CM:
	printf ("\033=%c%c", a2 + ' ', a1 + ' ');
	break;
    case TT_CLR:
	printf ("\032");
	break;
    case TT_CUROFF:
	printf ("\033.0");
	break;
    case TT_CURON:
	printf ("\033.2");
	break;
    default:
	return (-1);
    }
    return (0);
}

vt100 (fd, op, a1, a2)
     int             fd, op, a1, a2;
{
    switch (op) {
    case TT_CM:
	printf ("\033[%d;%dH", a2 + 1, a1 + 1);
	break;
    case TT_CLR:
	printf ("\033[H\033[J");
	break;
    case TT_CUROFF:
	printf ("\033[?25l");
	break;
    case TT_CURON:
	printf ("\033[?25h");
	break;
    default:
	return (-1);
    }
    return (0);
}
