/* $Id: close.c,v 1.2 1996/01/16 14:17:43 chris Exp $ */
#include <termio.h>

/** close(fd) close fd */
close (fd)
     int             fd;
{

    if (fd < FILEOFFSET)
	return (_close (fd));

    fd -= FILEOFFSET;
    _mfile[fd].open = 0;
}
