/* $Id: write.c,v 1.2 1996/01/16 14:19:00 chris Exp $ */
#include <termio.h>

write (fd, buf, n)
     int             fd, n;
     char           *buf;
{

    if (fd < FILEOFFSET)
	return (_write (fd, buf, n));

/* we don't support write to ram-based files */
    return (0);
}
