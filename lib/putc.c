/* $Id: putc.c,v 1.2 1996/01/16 14:18:16 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  putc(c,fp) put char to stream
 */
putc (c, fp)
     char            c;
     FILE           *fp;
{
    write (fp->fd, &c, 1);
}
