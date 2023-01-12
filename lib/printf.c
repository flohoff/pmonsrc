/* $Id: printf.c,v 1.3 1998/06/17 00:49:33 chris Exp $ */
#include <stdio.h>
#include <stdarg.h>

/** printf(fmt,va_alist) print formatted output to stdout */
int 
printf (const char *fmt, ...)
{
    int             len;
    va_list	    ap;

    va_start(ap, fmt);
    len = vfprintf (stdout, fmt, ap);
    va_end(ap);
    return (len);
}
