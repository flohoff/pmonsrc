/* $Id: fprintf.c,v 1.3 1998/06/17 00:49:32 chris Exp $ */
#include <stdio.h>
#include <stdarg.h>

/*************************************************************
 *  fprintf(fp,fmt,va_alist) send formatted string to stream
 */
int 
fprintf (FILE *fp, const char *fmt, ...)
{
    int             len;
    va_list ap;

    va_start(ap, fmt);
    len = vfprintf (fp, fmt, ap);
    va_end(ap);
    return (len);
}
