/* $Id: isalnum.c,v 1.2 1996/01/16 14:18:05 chris Exp $ */
/** isalnum(c) returns true if c is alphanumeric */
isalnum (c)
     int             c;
{

    if (isalpha (c))
	return (1);
    if (isdigit (c))
	return (1);
    return (0);
}
