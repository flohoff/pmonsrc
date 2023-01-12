/* $Id: fflush.c,v 1.2 1996/01/16 14:17:51 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fflush(fp) flush output buffer
 *      Output in PMON is not buffered, so no flush is necessary
 */
fflush (fp)
     FILE           *fp;
{
}
