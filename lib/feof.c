/* $Id: feof.c,v 1.2 1996/01/16 14:17:50 chris Exp $ */
#include <termio.h>

/*************************************************************
 *  feof(fp)
 *      Test a file to see if it's at EOF
 */
feof (fp)
     FILE           *fp;
{
    int             fd;
    Ramfile        *p;

    fd = fp->fd;
    if (fd < FILEOFFSET)
	return (0);

    fd -= FILEOFFSET;
    if (_mfile[fd].open != 1)
	return (0);

    p = &_mfile[fd];
    if (p->posn >= p->size)
	return (1);
    return (0);
}
