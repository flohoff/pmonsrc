/* $Id: vfprintf.c,v 1.3 1998/06/17 00:49:34 chris Exp $ */
#include <stdio.h>
#include <stdarg.h>

char            errmsg[] = "\nvfprintf: out of memory";

int vprintf (const char *fmt, va_list ap)
{
    return vfprintf (stdout, fmt, ap);
}


/*************************************************************
 *  int vfprintf(fp,fmt,ap)
 */
int 
vfprintf (FILE *fp, const char *fmt, va_list ap)
{
    char           *p, buf[300];
    va_list	    *lp;
    int             n;

    n = strlen (fmt);
    if (strchr (fmt, '%'))
      n *= 3;	/* fudge factor for long format strings */

    if (n > sizeof(buf) - 1) {
	p = (char *)malloc (n);
	if (p) {
	    n = vsprintf (p, fmt, ap);
	    fputs (p, fp);
	    free (p);
	} else
	    fputs (errmsg, fp);
    } else {
	n = vsprintf (buf, fmt, ap);
	fputs (buf, fp);
    }
    return (n);
}
