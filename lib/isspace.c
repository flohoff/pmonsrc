/* $Id: isspace.c,v 1.2 1996/01/16 14:18:09 chris Exp $ */
/** isspace(c) returns 1 if c == tab newline or space */
isspace (c)
     int             c;
{
    switch (c) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
    case '\f':
    case '\v':
	return (1);
    }
    return (0);
}
