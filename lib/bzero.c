/* $Id: bzero.c,v 1.2 1996/01/16 14:17:40 chris Exp $ */
/** bzero(dst,length) clear length bytes in destination */
bzero (dst, length)
     char           *dst;
     int             length;
{
    memset (dst, 0, length);
}
