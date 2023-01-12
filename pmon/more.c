/* $Id: more.c,v 1.3 1997/06/16 11:30:48 chris Exp $ */
#include "stdio.h"
#include "termio.h"
#include "mips.h"
#include "pmon.h"

#define BEL 007

unsigned int	moresz = 10;
char            more_pat[LINESZ];
int             tik_cnt;
const char      more_tiks[] = "|/-\\";
const char     *more_tik;
const char * const more_msg = "more... ";

int
chg_moresz (name, value)
    char *name, *value;
{
    unsigned long v;

    if (atob (&v, value, 10) && v > 2 && v < 100) {
	moresz = v;
	return 1;
    }
    printf ("%s: invalid moresz value\n", value);
    return 0;
}


const Optdesc         more_opts[] =
{
    {"/str", "search for str"},
    {"n", "repeat last search"},
    {"SPACE", "next page"},
    {"CR", "next line"},
    {"q", "quit"},
    {0}};

/*************************************************************
 *  more(p,cnt,size)
 *      The 'more' paginator    
 */
more (p, cnt, size)
     char           *p;
     int            *cnt, size;
{
    int             r;

    if (*cnt == 0) {
	if (size == 0)
	    return (1);		/* if cnt=0 and size=0 we're done */
	while ((r = getinp (cnt, size)) == -1);
	if (r) {		/* quit */
	    *p = 0;
	    return (1);
	}
    }
    if (*cnt == -1) {		/* search in progress */
	if (!strposn (p, more_pat)) {
	    dotik (256, 0);
	    return (0);		/* not found yet */
	} else {		/* found */
	    *cnt = size;
	    printf ("\b \n");
	}
    }
    printf ("%s\n", p);
    *p = 0;
    (*cnt)--;
    return (0);
}

/*************************************************************
 *  getinp(cnt,size)
 */
getinp (cnt, size)
     int            *cnt, size;
{
    int             i, c;

    printf ("%s", more_msg);
    loop {
	c = getchar ();
	if (strchr ("nq/ \n", c))
	    break;
	putchar (BEL);
    }
    era_line (more_msg);
    if (c == 'q')
	return (1);
    switch (c) {
    case ' ':
	*cnt = size;
	break;
    case '\n':
	*cnt = 1;
	break;
    case '/':
	/* get pattern */
	putchar ('/');
	for (i = 0;;) {
	    c = getchar ();
	    if (c == '\n')
		break;
	    if (c == '\b') {
		if (i > 0) {
		    putchar (c);
		    i--;
		} else {
		    putchar ('\b');
		    return (-1);
		}
	    } else {
		putchar (c);
		more_pat[i++] = c;
	    }
	}
	more_pat[i] = 0;
	printf ("  ");
	*cnt = -1;		/* enter search mode */
	break;
    case 'n':
	printf ("/%s  ", more_pat);
	*cnt = -1;		/* enter search mode */
	break;
    }
    return (0);
}

/*************************************************************
 *  dotik(rate,use_ret)
 */
dotik (rate, use_ret)
     int             rate, use_ret;
{

    tik_cnt -= rate;
    if (tik_cnt > 0)
	return;
    tik_cnt = TIKRATE;
    if (more_tik == 0)
	more_tik = more_tiks;
    if (*more_tik == 0)
	more_tik = more_tiks;
    if (use_ret)
	printf (" %c\r", *more_tik);
    else
	printf ("\b%c", *more_tik);
    more_tik++;
}
