/* $Id: sbrk.c,v 1.2 1996/01/16 14:18:25 chris Exp $ */
#define NULL 0
#define ALLOCSIZE 32*1024


char            allocbuf[ALLOCSIZE];
char           *allocp1 = allocbuf;

/*************************************************************
 *  sbrk() is alloc() on page 101 of K & R  Edition 2
 *      This sbrk is used by PMON, the sbrk in crt1.s is used
 *      by clients.
 */
char           *
sbrk (n)
     int             n;
{

    if (allocp1 + n <= allocbuf + ALLOCSIZE) {
	allocp1 += n;
	return (allocp1 - n);
    } else
	return (NULL);
}
