/* $Id: putchar.c,v 1.2 1996/01/16 14:18:17 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  putchar(c) put char to stdout
 */
putchar (c)
     int             c;
{
    putc (c, stdout);
}
