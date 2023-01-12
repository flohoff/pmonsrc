/* $Id: sprintf.c,v 1.3 1998/06/17 00:49:34 chris Exp $ */

#include <stdio.h>
#include <stdarg.h>

/*************************************************************
 *  sprintf(buf,fmt,va_alist) send formatted string to buf
 */
int 
sprintf (char *buf, const char *fmt, ...)
{
    int             n;
    va_list ap;

    va_start(ap, fmt);
    n = vsprintf (buf, fmt, ap);
    va_end(ap);
    return (n);
}
