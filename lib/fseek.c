/* $Id: fseek.c,v 1.2 1996/01/16 14:17:58 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fseek(fp,offset,whence)
 */
fseek (fp, offset, whence)
     FILE           *fp;
     long            offset;
     int             whence;
{
    int             n;

    fp->ungetcflag = 0;
    n = lseek (fp->fd, offset, whence);
    return (n);
}
