/* $Id: strcspn.c,v 1.2 1996/01/16 14:18:36 chris Exp $ */

strcspn (p, s)
     char           *p, *s;
{
    int             i, j;

    for (i = 0; p[i]; i++) {
	for (j = 0; s[j]; j++) {
	    if (s[j] == p[i])
		break;
	}
	if (s[j])
	    break;
    }
    return (i);
}
