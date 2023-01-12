/* $Id: strsort.c,v 1.2 1996/01/16 14:18:50 chris Exp $ */

/*
 * ** Uses a rather ugly bubble sort. Ugh!
 */
strsort (p)
     char           *p;
{
    int             i, flag;
    char            t;

    if (strlen (p) < 2)
	return;
    for (;;) {
	flag = 0;
	for (i = 0; p[i + 1]; i++) {
	    if (p[i] > p[i + 1]) {
		t = p[i + 1];
		p[i + 1] = p[i];
		p[i] = t;
		flag = 1;
	    }
	}
	if (flag == 0)
	    break;
    }
}
