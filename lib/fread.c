/* $Id: fread.c,v 1.3 1998/06/17 00:49:32 chris Exp $ */
#include <stdio.h>

/*************************************************************
 *  fread(src,size,count,fp)
 */
size_t
fread (void *src, size_t size, size_t count, FILE *fp)
{
    int             n;

    n = read (fp->fd, src, size * count);
    return (n / size);
}
