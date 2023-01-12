/* $Id: isxdigit.c,v 1.2 1996/01/16 14:18:10 chris Exp $ */
/** isxdigit(c) returns true if c is a hex digit */
isxdigit (c)
     int             c;
{
    if (c >= '0' && c <= '9')
	return (1);
    if (c >= 'a' && c <= 'f')
	return (1);
    if (c >= 'A' && c <= 'F')
	return (1);
    return (0);
}
