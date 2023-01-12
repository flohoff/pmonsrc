/* $Id: read.c,v 1.2 1996/01/16 14:18:22 chris Exp $ */
#include <termio.h>

/** read(fd,buf,n) read n bytes into buf from fd */
read (fd, buf, n)
     int             fd, n;
     char           *buf;
{
    Ramfile        *p;

    if (fd < FILEOFFSET)
	return (_read (fd, buf, n));

    fd -= FILEOFFSET;
    if (_mfile[fd].open != 1)
	return (0);

    p = &_mfile[fd];
    if (p->posn + n > p->size)
	n = p->size - p->posn;
    memcpy (buf, (char *)p->base + p->posn, n);
    p->posn += n;
    return (n);
}
