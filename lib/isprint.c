/* $Id: isprint.c,v 1.2 1996/01/16 14:18:08 chris Exp $ */
/** isprint(c) returns true if c is a printing character */
isprint (c)
     int             c;
{
    if (c >= ' ' && c <= '~')
	return (1);
    return (0);
}
