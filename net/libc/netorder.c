/* $Id: netorder.c,v 1.3 1996/12/09 20:10:42 nigel Exp $ */
/*
 * netorder.c
 * Generic network<->host byte order conversions
 */

#include <machine/endian.h>

#undef ntohl
#undef ntohs
#undef htonl
#undef htons

#if BYTE_ORDER == BIG_ENDIAN
unsigned long ntohl(unsigned long x)
{
    return (x);
}

unsigned short	ntohs (unsigned short x)
{
    return (x);
}

unsigned long htonl(unsigned long x)
{
    return (x);
}

unsigned short	htons (unsigned short x)
{
    return (x);
}

#else

unsigned long ntohl(unsigned long x)
{
    return ((x << 24) |
	    ((x & 0x0000ff00) <<  8) |
	    ((x >> 8)  & 0x0000ff00) |
	    (x >> 24));
}

unsigned short	ntohs (unsigned short x)
{
    return ((x << 8) | (x >> 8));
}

unsigned long htonl(unsigned long x)
{
    return ((x << 24) |
	    ((x & 0x0000ff00) <<  8) |
	    ((x >> 8)  & 0x0000ff00) |
	    (x >> 24));
}

unsigned short	htons (unsigned short x)
{
    return ((x << 8) | (x >> 8));
}


#endif
