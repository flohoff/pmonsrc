/* $Id: realloc.c,v 1.2 1996/01/16 14:18:23 chris Exp $ */
char           *malloc ();

char           *
realloc (ptr, size)
     char           *ptr;
     unsigned int    size;
{
    char           *p;
    unsigned int    sz;

    p = malloc (size);
    if (!p)
	return (p);
    sz = allocsize (ptr);
    memcpy (p, ptr, (sz > size) ? size : sz);
    free (ptr);
    return (p);
}
