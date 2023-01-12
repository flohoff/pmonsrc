/* $Id: islower.c,v 1.2 1996/01/16 14:18:07 chris Exp $ */
/** islower(c) returns true if c is lower case */
islower (c)
     int             c;
{
    if (c >= 'a' && c <= 'z')
	return (1);
    return (0);
}


/** islower(c) returns true if c is lower case */
isupper (c)
     int             c;
{
    if (c >= 'A' && c <= 'Z')
	return (1);
    return (0);
}
