/* $Id: isalpha.c,v 1.2 1996/01/16 14:18:05 chris Exp $ */
/** isalpha(c) returns true if c is alphabetic */
isalpha (c)
     int             c;
{
    if (c >= 'a' && c <= 'z')
	return (1);
    if (c >= 'A' && c <= 'Z')
	return (1);
    return (0);
}
