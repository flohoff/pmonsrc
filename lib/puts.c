/* $Id: puts.c,v 1.2 1996/01/16 14:18:17 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  puts(p) put string to stdout
 */
puts (p)
     const char     *p;
{

    fputs (p, stdout);
    putc ('\n', stdout);
}
