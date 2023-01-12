/* $Id: fileno.c,v 1.2 1996/01/16 14:17:53 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fileno(fp)
 *      convert an fp to an fd
 */
fileno (fp)
     FILE           *fp;
{

    return (fp->fd);
}

/*************************************************************
 *  clearerr(fp)
 *      Clear file read error. Provided as a stub to aid in porting
 *      customer applications to run under PMON.
 */
void
clearerr (fp)
     FILE           *fp;
{
}

/*************************************************************
 *  ferror(fp)
 *      Return file read error status. Provided as a stub to aid in 
 *      porting customer applications to run under PMON.
 */
ferror (fp)
     FILE           *fp;
{
    return (0);
}
