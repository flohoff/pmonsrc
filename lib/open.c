/* $Id: open.c,v 1.2 1996/01/16 14:18:15 chris Exp $ */
#include <termio.h>

extern unsigned long _filebase;

open (fname, mode)
     char           *fname;
     int             mode;
{
    int             fd, i;

    if ((fd = _open (fname, mode)) != -1)
	return (fd);

    if (mode != 0)
	return (-1);		/* no writeable files yet */

    if (_mfile[0].base == 0) {
	for (i = 0; _mfile[i].name; i++)
	    _mfile[i].base += _filebase;
    }
    for (i = 0; _mfile[i].name; i++) {
	if (strequ (fname, _mfile[i].name))
	    break;
    }
    if (_mfile[i].name == 0)
	return (-1);

    if (_mfile[i].open)
	return (-1);
    _mfile[i].open = 1;
    _mfile[i].posn = 0;
    return (i + FILEOFFSET);
}
