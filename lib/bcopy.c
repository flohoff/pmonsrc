/* $Id: bcopy.c,v 1.2 1996/01/16 14:17:39 chris Exp $ */
/** bcopy(src,dst,bytes) copy bytes from src to destination */
bcopy (src, dst, bytes)
     char           *src, *dst;
     unsigned int   bytes;
{

    if (dst >= src && dst < src + bytes) {
	/* do overlapping copy backwards, slowly! */
	src += bytes;
	dst += bytes;
	while (bytes--)
	  *--dst = *--src;
    } else {
	/* use the assembler code memcpy() */
	memcpy (dst, src, bytes);
    }
}
