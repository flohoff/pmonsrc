/* $Id: strspn.c,v 1.2 1996/01/16 14:18:51 chris Exp $ */

/* return length of initial segment of p that consists entirely of
 * characters from s */

strspn (p, s)
     char           *p, *s;
{
    int             i, j;

    for (i = 0; p[i]; i++) {
	for (j = 0; s[j]; j++) {
	    if (s[j] == p[i])
		break;
	}
	if (!s[j])
	    break;
    }
    return (i);
}
