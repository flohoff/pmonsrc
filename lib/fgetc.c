/* $Id: fgetc.c,v 1.2 1996/01/16 14:17:52 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fgetc(fp) get char from stream
 */
fgetc (fp)
     FILE           *fp;
{
    char            c;

    if (fp->ungetcflag) {
	fp->ungetcflag = 0;
	return (fp->ungetchar);
    }
    if (read (fp->fd, &c, 1) == 0)
	return (EOF);
    return (c);
}
