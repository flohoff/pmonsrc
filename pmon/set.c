/* $Id: set.c,v 1.12 2001/09/04 17:40:07 chris Exp $ */
#include "mips.h"
#include "pmon.h"
#include "string.h"
#include "stdio.h"
#include "termio.h"

extern char *sbd_getenv();

struct stdenv {
    char           *name;
    char           *init;
    char           *values;
    int		   (*chgfunc)();
};

static int	envinited = 0;

extern int	chg_heaptop();
extern int	chg_moresz();
extern int	chg_validpc();

#define stringify(x)	#x

#define stringval(x)	stringify(x)

static const struct stdenv stdenvtab[] =
{
    {"brkcmd", "l -r @epc 1", 0},
#ifdef R4000
    {"datasz", "-b", "-b -h -w -d"},
#else
    {"datasz", "-b", "-b -h -w"},
#endif
    {"dlecho", "off", "off on lfeed"},
#ifdef SABLE
    {"dlproto", "none", "none XonXoff EtxAck"},
    {"hostport", "tty0", 0},
#else
    {"dlproto", "EtxAck", "none XonXoff EtxAck"},
#ifdef INET
    {"bootp",	"no", "no sec pri save"},
#ifdef DHCP
    {"dhcp",	"no", "no sec pri save"},
#endif
#endif
    {"hostport", "tty1", 0},
#endif
    {"inalpha", "hex", "hex symbol"},
    {"inbase", "16", "auto 8 10 16"},
    {"moresz", "10", 0, chg_moresz},
    {"prompt", "PMON> ", 0},
    {"regstyle", "sw", "hw sw"},
#ifdef R4000
    {"regsize", "32", "32 64"},
#endif
    {"rptcmd", "trace", "off on trace"},
    {"trabort", "^K", 0},
    {"ulcr", "cr", "cr lf crlf"},
    {"uleof", "%", 0},
    {"validpc", "_ftext etext", 0, chg_validpc},
    {"heaptop", stringval(CLIENTPC), 0, chg_heaptop},
    {"showsym", "yes", "no yes"},
#ifdef FLOATINGPT
    {"fpfmt", "both", "both double single none"},
    {"fpdis", "yes", "no yes"},
#endif
    {0}};


struct envpair {
    char	*name;
    char	*value;
};

#define NVAR	64
static struct envpair envvar[NVAR];



static int 
_matchval (sp, value)
    struct stdenv *sp;
    char *value;
{
    char *t, wrd[20];
    int j;

    for (j = 0, t = sp->values; ; j++) {
	t = getword (wrd, t);
	if (t == 0)
	    return (-2);
	if (striequ (wrd, value))
	    return (j);
    }
}


static const struct stdenv *
getstdenv (name)
    const char *name;
{
    const struct stdenv *sp;
    for (sp = stdenvtab; sp->name; sp++)
	if (striequ (name, sp->name))
	    return sp;
    return 0;
}


int
_setenv (name, value)
    char *name, *value;
{
    struct envpair *ep;
    struct envpair *bp = 0;
    const struct stdenv *sp;

    if (sp = getstdenv (name)) {
	if (sp->chgfunc && !(*sp->chgfunc) (name, value))
	    return 0;
	if (envinited && sp->values && _matchval (sp, value) < 0) {
	    printf ("%s: bad %s value, try [%s]\n", value, name, sp->values);
	    return 0;
	}
    }

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (!ep->name && !bp)
	  bp = ep;
	else if (ep->name && striequ (name, ep->name))
	  break;
    }
    
    if (ep < &envvar[NVAR]) {
	/* must have got a match, free old value */
	if (ep->value) {
	    free (ep->value); ep->value = 0;
	}
    } else if (bp) {
	/* new entry */
	int nb = strlen (name) + 1;
	ep = bp;
	if (!(ep->name = malloc (nb)))
	  return 0;
	memcpy (ep->name, name, nb);
    } else {
	return 0;
    }

    if (value) {
	int nb = strlen (value) + 1;
	if (!(ep->value = malloc (nb))) {
	    free (ep->name); ep->name = 0;
	    return 0;
	}
	memcpy (ep->value, value, nb);
    }

    return 1;
}


int
setenv (name, value)
    char *name, *value;
{
    if (_setenv (name, value)) {
	const struct stdenv *sp;
	if ((sp = getstdenv (name)) && striequ (value, sp->init))
	    /* set to default: remove from non-volatile ram */
	    return sbd_unsetenv (name);
	else
	    /* new value: save in non-volatile ram */
	    return sbd_setenv (name, value);
    }
    return 0;
}


static char *
_getenv (name)
    const char *name;
{
    struct envpair *ep;
    for (ep = envvar; ep < &envvar[NVAR]; ep++)
	if (ep->name && striequ (name, ep->name))
	    return ep->value ? ep->value : "";
    return 0;
}


char *
getenv (name)
    const char *name;
{
    if (envinited) {
	return _getenv (name);
    } else {
	const struct stdenv *sp;
	if (sp = getstdenv (name))
	    return sp->init;
    }
    return 0;
}


mapenv (pfunc)
    int (*pfunc)();
{
    struct envpair *ep;

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (ep->name)
	  (*pfunc) (ep->name, ep->value);
    }
}
 

unsetenv (pat)
    char *pat;
{
    const struct stdenv *sp;
    struct envpair *ep;
    int ndone = 0;

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (ep->name && strpat (ep->name, pat)) {
	    sp = getstdenv (ep->name);
	    if (sp) {
		/* unsetenving a standard variable resets it to initial value! */
		if (!setenv (sp->name, sp->init))
		  return 0;
	    } else {
		/* normal user variable: delete it */
		sbd_unsetenv (ep->name);	/* first from non-volatile ram */
		free (ep->name);		/* then from local copy */
		if (ep->value)
		  free (ep->value);
		ep->name = ep->value = 0;
	    }
	    ndone++;
	}
    }
    return ndone > 0;
}


#pragma weak _sbd_env_suppress
int
_sbd_env_suppress (void)
{
    return 0;
}

#pragma weak sbd_boardenv
void
sbd_boardenv (char *(*get)(const char *),
	       int (*set)(const char *, const char *))
{
}

envinit ()
{
    int             i;

    bzero (envvar, sizeof(envvar));

    if (!_sbd_env_suppress ()) {
	/* extract stored non-volatile variables into local copy */
	SBD_DISPLAY ("MAPV", CHKPNT_MAPV);
	sbd_mapenv (_setenv);
    }

    /* allow board-specific defaults */
    SBD_DISPLAY ("BRDV", CHKPNT_BRDV);
    sbd_boardenv (_getenv, _setenv);

    /* set PMON defaults (but only if not set above) */
    SBD_DISPLAY ("STDV", CHKPNT_STDV);
    for (i = 0; stdenvtab[i].name; i++) {
	if (!_getenv (stdenvtab[i].name))
	  _setenv (stdenvtab[i].name, stdenvtab[i].init);
    }

    envinited = 1;
}


do_set (ac, av)
     int             ac;
     char           *av[];
{
    static int   printvar();
    struct envpair *ep;
    char	*s;
    int		 ln;

    ln = moresz;
    switch (ac) {
    case 1:			/* display all variables */
	ioctl (STDIN, CBREAK, NULL);
	for (ep = envvar; ep < &envvar[NVAR]; ep++)
	  if (ep->name && printvar (ep->name, ep->value, &ln))
	    break;
	break;
    case 2:			/* display specific variable */
	if (s = getenv (av[1]))
	  printvar (av[1], s, &ln);
	else {
	    printf ("%s: not found\n", av[1]);
	    return (1);
	}
	break;
    case 3:			/* set specific variable */
	if (!setenv (av[1], av[2])) {
	    printf ("%s: cannot set variable\n", av[1]);
	    return (1);
	}
	break;
    default:
	return (-1);
    }
    return (0);
}


do_eset (ac, av)
     int             ac;
     char           *av[];
{
    char	name[LINESZ];
    char	val[LINESZ];
    char 	*s;
    int		i;

    for (i = 1; i < ac; i++) {
	strcpy (name, av[i]);
	strtoupper (name);

	s = getenv (av[i]);
	if (!s) {
	    printf ("%s: not found\n", name);
	    continue;
	}

	printf ("%s=", name); fflush (stdout);

	strcpy (val, s);
	get_line (val, 0);

	if (!strequ (val, s)) {
	    if (!setenv (av[i], val)) {
		printf ("%s: cannot set variable\n", name);
		return (1);
	    }
	}
    }

    return (0);
}


do_unset (ac, av)
     int             ac;
     char           *av[];
{
    int i;

    for (i = 1; i < ac; i++)
      if (!unsetenv (av[i])) {
	  printf ("%s: no matching variable\n", av[i]);
	  return (1);
      }
    return (0);
}


#if 0
do_envinit (ac, av)
     int             ac;
     char           *av[];
{
    int i;

    printf ("Reinitialising environment: are you sure (y/n) ");
    fflush (stdout);
    if ((c = getchar ()) != 'Y' && c != 'y') {
	printf ("\nAbandoned\n");
	return (0);
    }
    
    sbd_envreset ();
    envinit ();
    return (0);
}
#endif


static int
printvar (name, value, cntp)
    char *name;
    char *value;
    int  *cntp;
{
    const struct stdenv *ep;
    char buf[300];
    char *b;

    if (!value)
	sprintf (buf, "%10s", name);
    else if (strchr (value, ' '))
	sprintf (buf, "%10s = \"%s\"", name, value);
    else
	sprintf (buf, "%10s = %-10s", name, value);

    b = buf + strlen(buf);
    if ((ep = getstdenv (name)) && ep->values)
	sprintf (b, "  [%s]", ep->values);

    return more (buf, cntp, moresz);
}

/** matchenv(name) returns integer corresponding to current value */
matchenv (name)
     char           *name;
{
    int             i;
    const struct stdenv *ep;
    char           *value;

    if (!(ep = getstdenv (name)))
	return (-1);

    if (!(value = getenv (name)))
      value = "";

    return _matchval (ep, value);
}

/* Two helpers to build a traditional environment for clients */

void
envsize (int *ec, int *slen)
{
    struct envpair *ep;

    *ec = 0;
    *slen = 0;
    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (ep->name) {
	    *ec += 1;
	    if (ep->value)
		*slen += strlen (ep->name) + 1 + strlen (ep->value) + 1;
	    else
		*slen += strlen (ep->name) + 1 + 1;
	}
    }
}

void
envbuild (char **vsp, char *ssp)
{
    struct envpair *ep;

    for (ep = envvar; ep < &envvar[NVAR]; ep++) {
	if (ep->name) {
	    *vsp++ = ssp;
	    if (ep->value)
		ssp += sprintf (ssp, "%s=%s", ep->name, ep->value) + 1;
	    else
		ssp += sprintf (ssp, "%s=", ep->name) + 1;
	}
    }
    *vsp++ = (char *)0;
}
