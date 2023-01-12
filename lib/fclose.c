/* $Id: fclose.c,v 1.2 1996/01/16 14:17:50 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fclose(fp) close stream
 */
fclose (fp)
     FILE           *fp;
{
    close (fp->fd);
    fp->valid = 0;
}
