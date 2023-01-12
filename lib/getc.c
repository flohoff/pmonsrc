/* $Id: getc.c,v 1.2 1996/01/16 14:17:59 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  getc(fp) get char from stream
 */
getc (fp)
     FILE           *fp;
{
    return (fgetc (fp));
}
