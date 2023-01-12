/* $Id: ungetc.c,v 1.2 1996/01/16 14:18:58 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  ungetc(fp) unget a char
 */
ungetc (c, fp)
     int             c;
     FILE           *fp;
{

    fp->ungetcflag = 1;
    fp->ungetchar = c;
    return (c);
}
