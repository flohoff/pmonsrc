/* $Id: isdigit.c,v 1.2 1996/01/16 14:18:07 chris Exp $ */
/** isdigit(c) returns true if c is decimal digit */
isdigit (c)
     int             c;
{
    if (c >= '0' && c <= '9')
	return (1);
    return (0);
}
