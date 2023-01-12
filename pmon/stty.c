/* $Id: stty.c,v 1.5 1999/03/22 16:46:42 nigel Exp $ */
#include "stdio.h"
#include "termio.h"
#include "string.h"
#include "pmon.h"

struct brate {
    char           *str;
    int             value;
};

struct brate    btable[] =
{
    {"50", B50},
    {"75", B75},
    {"110", B110},
    {"134", B134},
    {"200", B200},
    {"150", B150},
    {"300", B300},
    {"600", B600},
    {"1200", B1200},
    {"1800", B1800},
    {"2400", B2400},
    {"4800", B4800},
    {"9600", B9600},
    {"19200", B19200},
    {"38400", B38400},
    {0}};

static int getbaudrate (const char *);

const Optdesc         stty_opts[] =
{
    {"-v", "list possible baud rates and terminal types"},
    {"-a", "list all settings"},
    {"<baud>", "set baud rate"},
    {"<term>", "set terminal type"},
    {"sane", "set sane settings"},
    {"ixany", "allow any char to restart output"},
    {"-ixany", "allow only <start> to restart output"},
    {"ixoff", "enable s/w input flow control"},
    {"-ixoff", "disable s/w input flow control"},
    {"ixon", "enable s/w output flow control"},
    {"rts_iflow", "enable h/w input flow control"},
    {"cts_oflow", "enable h/w output flow control"},
    {"rtscts", "enable bidirectional h/w flow control"},
    {"cs8", "8 bit character size"},
    {"cs7", "7 bit character size"},
    {"cs6", "6 bit character size"},
    {"cs5", "5 bit character size"},
    {"cstopb", "2 stop bits"},
    {"-cstopb", "1 stop bit"},
    {"parenb", "enable parity"},
    {"-parenb", "disable parity"},
    {"parodd", "odd parity"},
    {"-parodd", "even parity"},
    {0}};

/** stty(ac,av), the 'stty' command */
stty (ac, av)
     int             ac;
     char           *av[];
{
    int             fd, i, flag, aflag, vflag, j;
    struct termio   tbuf;
    char            buf[16];

    fd = STDIN;
    ioctl (fd, TCGETA, &tbuf);
    flag = aflag = vflag = 0;
    for (i = 1; i < ac; i++) {
	if (av[i][0] == '-') {	/* option */
	    if (strequ (av[i], "-ixoff")) {
		flag = 1;
		tbuf.c_iflag &= ~IXOFF;
	    } else if (strequ (av[i], "-ixany")) {
		flag = 1;
		tbuf.c_iflag &= ~IXANY;
	    } else if (strequ (av[i], "-ixon")) {
		flag = 1;
		tbuf.c_iflag &= ~IXON;
	    } else if (strequ (av[i], "-cts_oflow")) {
		flag = 1;
		tbuf.c_iflag &= ~ICTS_OFLOW;
	    } else if (strequ (av[i], "-rts_iflow")) {
		flag = 1;
		tbuf.c_iflag &= ~IRTS_IFLOW;
	    } else if (strequ (av[i], "-rtscts")) {
		flag = 1;
		tbuf.c_iflag &= ~(IRTS_IFLOW | ICTS_OFLOW);
	    } else if (strequ (av[i], "-parenb")) {
		flag = 1;
		tbuf.c_lflag &= ~PARENB;
	    } else if (strequ (av[i], "-parodd")) {
		flag = 1;
		tbuf.c_lflag &= ~PARODD;
	    } else if (strequ (av[i], "-cstopb")) {
		flag = 1;
		tbuf.c_lflag &= ~CSTOPB;
	    } else
		for (j = 1; av[i][j] != 0; j++) {
		    if (av[i][j] == 'a')
			aflag = 1;
		    else if (av[i][j] == 'v')
			vflag = 1;
		    else
			return (-1);
		}
	} else if (isdigit (av[i][0])) {	/* set baud rate */
	    int cbaud;
	    flag = 1;
	    cbaud = getbaudrate (av[i]);
	    if (cbaud == 0) {
		printf ("%s: illegal baud rate\n", av[i]);
		return (-1);
	    }
	    tbuf.c_cflag = cbaud;
	} else if (strequ (av[i], "ixoff")) {
	    flag = 1;
	    tbuf.c_iflag |= IXOFF;
	} else if (strequ (av[i], "ixany")) {
	    flag = 1;
	    tbuf.c_iflag |= IXANY;
	} else if (strequ (av[i], "ixon")) {
	    flag = 1;
	    tbuf.c_iflag |= IXON;
	} else if (strequ (av[i], "cts_oflow")) {
	    flag = 1;
	    tbuf.c_iflag |= ICTS_OFLOW;
	} else if (strequ (av[i], "rts_iflow")) {
	    flag = 1;
	    tbuf.c_iflag |= IRTS_IFLOW;
	} else if (strequ (av[i], "rtscts")) {
	    flag = 1;
	    tbuf.c_iflag |= IRTS_IFLOW | ICTS_OFLOW;
	} else if (strequ (av[i], "parenb")) {
	    flag = 1;
	    tbuf.c_lflag |= PARENB;
	} else if (strequ (av[i], "parodd")) {
	    flag = 1;
	    tbuf.c_lflag |= PARODD;
	} else if (strequ (av[i], "cstopb")) {
	    flag = 1;
	    tbuf.c_lflag |= CSTOPB;
	} else if (strequ (av[i], "cs5")) {
	    flag = 1;
	    tbuf.c_lflag = (tbuf.c_lflag & ~CSIZE) | CS5;
	} else if (strequ (av[i], "cs6")) {
	    flag = 1;
	    tbuf.c_lflag = (tbuf.c_lflag & ~CSIZE) | CS6;
	} else if (strequ (av[i], "cs7")) {
	    flag = 1;
	    tbuf.c_lflag = (tbuf.c_lflag & ~CSIZE) | CS7;
	} else if (strequ (av[i], "cs8")) {
	    flag = 1;
	    tbuf.c_lflag = (tbuf.c_lflag & ~CSIZE) | CS8;
	} else if (strequ (av[i], "sane")) {
	    ioctl (fd, SETSANE);
	    ioctl (fd, TCGETA, &tbuf);
	} else if (strpat (av[i], "tty?")) {	/* select device */
	    fd = open (av[i], 0);
	    if (fd == -1) {
		printf ("can't open %s\n", av[i]);
		return (1);
	    }
	    ioctl (fd, TCGETA, &tbuf);
	} else {		/* set term type */
	    flag = 1;
	    if (ioctl (fd, SETTERM, av[i]) == -1) {
		printf ("term type %s not found\n", av[i]);
		return (-1);;
	    }
	}
    }

    if (vflag) {
	printf ("Baud rates:\n\t");
	for (i = 0; btable[i].str != 0; i++) {
	    printf ("%5s ", btable[i].str);
	    if (i == 8)
		printf ("\n\t");
	}
	printf ("\n");
	printf ("Terminal types:\n");
	for (i = 0; (i = ioctl (i, TERMTYPE, buf)) != -1;) {
	    if ((i % 6) == 0)
		printf ("\n");
	    printf ("%10s ", buf);
	}
	printf ("\n");
    }

    if (flag) {
	if (ioctl (fd, TCSETAF, &tbuf) < 0) {
	    printf ("baud rate %d not available\n", 
		    getbaudval (tbuf.c_cflag));
	    return (1);
	}
	if (!aflag)
	    return (0);
    }

    if (ioctl (fd, GETTERM, buf) != -1)
	printf ("term=%s ", buf);
    {
	int baud = getbaudval (tbuf.c_cflag);
	if (baud)
	    printf ("baud=%d ", baud);
    }
    printf ("\n");

    if (!aflag)
	return (0);

    switch (tbuf.c_lflag & CSIZE) {
    case CS5: printf("cs5 "); break;
    case CS6: printf("cs6 "); break;
    case CS7: printf("cs7 "); break;
    case CS8: printf("cs8 "); break;
    }

    printf ("%scstopb ", (tbuf.c_lflag & CSTOPB) ? "" : "-");
    printf ("%sparenb ", (tbuf.c_lflag & PARENB) ? "" : "-");
    printf ("%sparodd ", (tbuf.c_lflag & PARODD) ? "" : "-");
    printf ("%scts_oflow ", ((tbuf.c_iflag & ICTS_OFLOW) ? "" : "-"));
    printf ("%srts_iflow ", ((tbuf.c_iflag & IRTS_IFLOW) ? "" : "-"));
    printf ("\n");

    printf ("%sistrip ", ((tbuf.c_iflag & ISTRIP) ? "" : "-"));
    printf ("%sixon ", ((tbuf.c_iflag & IXON) ? "" : "-"));
    printf ("%sixany ", ((tbuf.c_iflag & IXANY) ? "" : "-"));
    printf ("%sixoff ", ((tbuf.c_iflag & IXOFF) ? "" : "-"));
    printf ("%sicanon ", ((tbuf.c_lflag & ICANON) ? "" : "-"));
    printf ("%secho ", ((tbuf.c_lflag & ECHO) ? "" : "-"));
    printf ("%sechoe ", ((tbuf.c_lflag & ECHOE) ? "" : "-"));
    printf ("%sicrnl ", ((tbuf.c_iflag & ICRNL) ? "" : "-"));
    printf ("%sonlcr ", ((tbuf.c_oflag & ONLCR) ? "" : "-"));
    printf ("\n");

    printf ("erase=%s ", cc2str (buf, tbuf.c_cc[VERASE]));
    printf ("stop=%s ", cc2str (buf, tbuf.c_cc[V_STOP]));
    printf ("start=%s ", cc2str (buf, tbuf.c_cc[V_START]));
    printf ("eol=%s ", cc2str (buf, tbuf.c_cc[VEOL]));
    printf ("eol2=%s ", cc2str (buf, tbuf.c_cc[VEOL2]));
    printf ("vintr=%s ", cc2str (buf, tbuf.c_cc[VINTR]));
    printf ("\n");
    return (0);
}


/* 
 * Parse compact tty mode string, e.g. 9600,8N1 
 */
const char *
parsettymode (const char *mode, struct termio *t)
{
    const char *s;
    int ncflag, nlflag, niflag;
    int len;

    ncflag = t->c_cflag;
    nlflag = t->c_lflag;
    niflag = t->c_iflag;

    /* get baud rate */
    for (s = mode; isdigit (*s); s++)
	continue;

    if (len = (s - mode)) {
	/* parse baud rate */
	char buf[8];
	if (len > sizeof(buf) - 1)
	    return "baud rate too long";
	memcpy (buf, mode, len);
	buf[len] = '\0';
	if ((ncflag = getbaudrate (buf)) == 0)
	    return "illegal baud rate";
    }

    /* skip space */
    while (*s == ',' || *s == ' ') 
	++s;

    if (*s) {
	/* parse character size */
	nlflag &= ~CSIZE;
	switch (*s++) {
	case '5':  nlflag |= CS5; break;
	case '6':  nlflag |= CS6; break;
	case '7':  nlflag |= CS7; break;
	case '8':  nlflag |= CS8; break;
	default: return "bad character size";
	}
    }

    /* skip space */
    while (*s == ',' || *s == ' ') 
	++s;

    if (*s) {
	/* parse parity */
	nlflag &= ~(PARENB | PARODD);
	switch (*s++) {
	case 'n': case 'N': break;
	case 'e': case 'E': nlflag |= PARENB; break;
	case 'o': case 'O': nlflag |= PARENB | PARODD; break;
	default: return "bad parity";
	}
    }

    /* skip space */
    while (*s == ',' || *s == ' ') 
	++s;

    if (*s) {
	/* parse stop bits */
	nlflag &= ~CSTOPB;
	switch (*s++) {
	case '1': break;
	case '2': nlflag |= CSTOPB; break;
	default: return "bad stop bits";
	}
    }

    /* skip space */
    while (*s == ',' || *s == ' ') 
	++s;

    if (*s) {
	/* parse flow-control mode */
	niflag &= ~(ICTS_OFLOW | IRTS_IFLOW | IXON | IXOFF | IXANY);
	while (*s && *s != ',') {
	    switch (*s++) {
	    case 'n': case 'N': break;
	    case 'c': case 'C': niflag |= ICTS_OFLOW; break;
	    case 'r': case 'R': niflag |= IRTS_IFLOW; break;
	    case 'h': case 'H': niflag |= ICTS_OFLOW | IRTS_IFLOW; break;
	    case 'o': case 'O': niflag |= IXON; break;
	    case 'i': case 'I': niflag |= IXOFF; break;
	    case 'x': case 'X': 
	    case 's': case 'S': niflag |= IXON | IXOFF; break;
	    case 'a': case 'A': niflag |= IXANY; break;
	    default: return "bad flow control";
	    }
	}
    }

    /* skip space */
    while (*s == ',' || *s == ' ') 
	++s;
    
    /* must be at end of string */
    if (*s)
	return "mode too long";

    t->c_cflag = ncflag;
    t->c_lflag = nlflag;
    t->c_iflag = niflag;
    return (0);
}


/* return a CBAUD value corresponding to the baudrate */
static int
getbaudrate (const char *p)
{
    int             j;

    /* try standard encoding */
    for (j = 0; btable[j].str != 0; j++) {
	if (strequ (p, btable[j].str))
	    return (btable[j].value);
    }

    /* try alternate encoding */
    {
	int baud = atoi (p);
	unsigned short cxbaud = CXBAUDENCODE(baud);
	if (CXBAUDDECODE(cxbaud) == baud)
	    return cxbaud;
    }

    return (0);
}


int
getbaudval (int cbaud)
{
    int j;

    if (cbaud & CXBAUD)
	return CXBAUDDECODE (cbaud);

    cbaud &= CBAUD;
    for (j = 0; btable[j].str != 0; j++) {
	if (btable[j].value == cbaud)
	    return (atoi(btable[j].str));
    }

    return 0;
}
