/* $Id: iscntrl.c,v 1.2 1996/01/16 14:18:06 chris Exp $ */
/** iscntrl(c) returns true if c is delete or control character */
iscntrl (c)
     int             c;
{
    if (c == 0x7f)
	return (1);
    if (c < ' ')
	return (1);
    return (0);
}
