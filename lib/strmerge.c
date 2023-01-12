/* $Id: strmerge.c,v 1.2 1996/01/16 14:18:42 chris Exp $ */
/************************************************************** 
 *  strmerge(d,s)
 */
strmerge (d, s)
     char           *d, *s;
{
    int             i;

    if (strlen (d) < strlen (s)) {
	for (i = strlen (d); i < strlen (s); i++)
	    d[i] = ' ';
	d[i] = 0;
    }
    for (; *d && *s; d++, s++)
	if (*s != ' ')
	    *d = *s;
}
