/* $Id: fputs.c,v 1.2 1996/01/16 14:17:57 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fputs(string,fp) put string to stream
 */
fputs (p, fp)
     const char     *p;
     FILE           *fp;
{

    write (fp->fd, p, strlen (p));
}
