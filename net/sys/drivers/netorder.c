/* $Id: netorder.c,v 1.2 1996/01/16 14:21:16 chris Exp $ */
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
    return (((x & 0x000000ff) << 24) |
	    ((x & 0x0000ff00) <<  8) |
	    ((x >> 8)  & 0x0000ff00) |
	    ((x >> 24) & 0x000000ff));
}

unsigned short	ntohs (unsigned short x)
{
    return (((x << 8) & 0xff00) |
	    ((x >> 8)  & 0x00ff));
}

unsigned long htonl(unsigned long x)
{
    return (((x & 0x000000ff) << 24) |
	    ((x & 0x0000ff00) <<  8) |
	    ((x >> 8)  & 0x0000ff00) |
	    ((x >> 24) & 0x000000ff));
}

unsigned short	htons (unsigned short x)
{
    return (((x << 8) & 0xff00) |
	    ((x >> 8)  & 0x00ff));
}


#endif
