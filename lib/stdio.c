/* $Id: stdio.c,v 1.3 1998/06/17 00:49:34 chris Exp $ */
#include "stdio.h"
#include <fcntl.h>

/*************************************************************
 *
 *   fprintf --\    putchar ------ putc --\
 *             |                          |
 *    printf --+-- vfprintf --+-- fputs --+-- write
 *                            |           |
 *                            puts --/  fwrite --/
 *
 *
 *  getchar ----- getc --+-- fgetc ----- read
 *                      |
 *     gets ---- fgets --/
 *
 *************************************************************/

FILE            _iob[OPEN_MAX] =
{
    {0, 1},
    {1, 1},
    {2, 1}
};

/*************************************************************
 *  FILE *fopen(fname,mode) open stream
 */
FILE           *
fopen (fname, mode)
     const char     *fname;
     const char     *mode;
{
    int             i, fd, flags;

    for (i = 0; i < OPEN_MAX && _iob[i].valid; i++);
    if (i == OPEN_MAX)
	return (0);
    if (mode == 0)
	flags = O_RDONLY;
    else if (strequ (mode, "r"))
	flags = O_RDONLY;
    else if (strequ (mode, "w"))
	flags = O_WRONLY;
    else if (strequ (mode, "r+"))
	flags = O_RDWR;
    fd = open (fname, flags, 0);
    if (fd == -1)
	return (0);
    _iob[i].fd = fd;
    _iob[i].valid = 1;
    return (&_iob[i]);
}
