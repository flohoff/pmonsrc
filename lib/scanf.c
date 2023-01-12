/* $Id: scanf.c,v 1.3 1998/06/17 00:49:33 chris Exp $ */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/*
 * ** fscanf --\    sscanf --\
 * **          |                  |
 * **  scanf --+-- vfscanf ----- vsscanf
 * **
 * ** This not been very well tested.. it probably has bugs
 */

#define ISSPACE " \t\n\r\f\v"

#ifdef PMCC
main (argc, argv)
     int             argc;
     char           *argv[];
{
    char            buf[40], s[30];
    int             a, b, n;
    char           *fmt = "%d %d";

    if (argc == 2) {
	fmt = argv[1];
    }
    printf ("fmt=[%s]\n", fmt);
    for (;;) {
	printf ("> ");
	gets (buf);
	/*
	 * n = sscanf(buf,fmt,&a,&b);
	 * printf("n=%d a=%d b=%d\n",n,a,b);
	 */
	n = sscanf (buf, fmt, s);
	printf ("n=%d s=[%s]\n", n, s);
    }
}
#endif


/*************************************************************
 *  scanf(fmt,va_alist) 
 */
int 
scanf (const char *fmt, ...)
{
    int             count;
    va_list ap;
    
    va_start (ap, fmt);
    count = vfscanf (stdin, fmt, ap);
    va_end (ap);
    return (count);
}

/*************************************************************
 *  fscanf(fp,fmt,va_alist)
 */
int 
fscanf (FILE *fp, const char *fmt, ...)
{
    int             count;
    va_list ap;

    va_start (ap, fmt);
    count = vfscanf (fp, fmt, ap);
    va_end (ap);
    return (count);
}

/*************************************************************
 *  sscanf(buf,fmt,va_alist)
 */
int 
sscanf (const char *buf, const char *fmt, ...)
{
    int             count;
    va_list ap;
    
    va_start (ap, fmt);
    count = vsscanf (buf, fmt, ap);
    va_end (ap);
    return (count);
}

/*************************************************************
 *  vfscanf(fp,fmt,ap) 
 */
vfscanf (FILE *fp, char *fmt, va_list ap)
{
    int             count;
    char            buf[MAXLN + 1];

    if (fgets (buf, MAXLN, fp) == 0)
	return (-1);
    count = vsscanf (buf, fmt, ap);
    return (count);
}


/*************************************************************
 *  vsscanf(buf,fmt,ap)
 */
int
vsscanf (char *buf, char *s, va_list ap)
{
    int             count, noassign, width, base, lflag;
    char           *t, tmp[MAXLN];

    count = noassign = width = lflag = 0;
    while (*s && *buf) {
	while (isspace (*s))
	    s++;
	if (*s == '%') {
	    s++;
	    for (; *s; s++) {
		if (strchr ("dibouxcsefg%", *s))
		    break;
		if (*s == '*')
		    noassign = 1;
		else if (*s == 'l' || *s == 'L')
		    lflag = 1;
		else if (*s >= '1' && *s <= '9') {
		    for (t = s; isdigit (*s); s++);
		    strncpy (tmp, t, s - t);
		    tmp[s - t] = '\0';
		    atob (&width, tmp, 10);
		    s--;
		}
	    }
	    if (*s == 's') {
		while (isspace (*buf))
		    buf++;
		if (!width)
		    width = strcspn (buf, ISSPACE);
		if (!noassign) {
		    strncpy (t = va_arg (ap, char *), buf, width);
		    t[width] = '\0';
		}
		buf += width;
	    } else if (*s == 'c') {
		if (!width)
		    width = 1;
		if (!noassign) {
		    strncpy (t = va_arg (ap, char *), buf, width);
		    t[width] = '\0';
		}
		buf += width;
	    } else if (strchr ("dobxu", *s)) {
		while (isspace (*buf))
		    buf++;
		if (*s == 'd' || *s == 'u')
		    base = 10;
		else if (*s == 'x')
		    base = 16;
		else if (*s == 'o')
		    base = 8;
		else if (*s == 'b')
		    base = 2;
		if (!width) {
		    if (isspace (*(s + 1)) || *(s + 1) == 0)
			width = strcspn (buf, ISSPACE);
		    else
			width = strchr (buf, *(s + 1)) - buf;
		}
		strncpy (tmp, buf, width);
		tmp[width] = '\0';
		buf += width;
		if (!noassign)
		    atob (va_arg (ap, int), tmp, base);
	    }
	    if (!noassign)
		count++;
	    width = noassign = lflag = 0;
	    s++;
	} else {
	    while (isspace (*buf))
		buf++;
	    if (*s != *buf)
		break;
	    else
		s++, buf++;
	}
    }
    return (count);
}
