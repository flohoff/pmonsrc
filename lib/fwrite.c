/* $Id: fwrite.c,v 1.3 1998/06/17 00:49:33 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fwrite(dst,size,count,fp)
 */
size_t
fwrite (const void *dst, size_t size, size_t count, FILE *fp)
{
    int             n;

    n = write (fp->fd, dst, size * count);
    return (n / size);
}
