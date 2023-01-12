/* $Id: termio.c,v 1.11 1999/05/13 11:45:12 nigel Exp $ */
#include "queue.h"
#include "stdio.h"
#include "termio.h"
#include "mips.h"

#ifdef TEST
# define read(x,y,z) xread(x,y,z)
# define write(x,y,z) xwrite(x,y,z)
# define ioctl(x,y,z) xioctl(x,y,z)
int             SIOBASE[16];

#endif

extern ConfigEntry ConfigTable[];
extern char *getenv();

DevEntry        DevTable[DEV_MAX];

File            _file[OPEN_MAX] =
{
#ifdef NETIO
    {0, 1, -1},
    {0, 1, -1},
    {0, 1, -1}
#else
    {0, 1},
    {0, 1},
    {0, 1}
#endif
};

struct TermEntry {
    char           *name;
    iFunc          *func;
};

int             tvi920 (), vt100 ();

struct TermEntry TermTable[] =
{
    {"tvi920", tvi920},
    {"vt100", vt100},
    {0}};

int            *curlst;		/* list of files open in the current context */

#ifdef TEST
int             fd;

main ()
{

    devinit ();
    fd = 0;
    for (;;) {
	if (!docmd ("main>"))
	    break;
    }
}

docmd (s)
     char           *s;
{
    char            buf[40];
    struct termio   t;
    int             i, value, reg;
    DevEntry       *p;

    printf ("fd=%d ", fd);
    p = &DevTable[_file[fd].dev];
    printf ("txoff=%d rxq=", p->txoff);
    prqueue (p->rxq);
    for (i = 0; i < 16; i++)
	printf ("%02x ", SIOBASE[i]);
    printf ("\n");
    printf ("pgfscq.%s", s);
    gets (buf);
    if (buf[0] == 0)
	return (1);
    switch (buf[0]) {
    case 'q':
	return (0);
	break;
    case 'p':
	sscanf (buf, "p %x", &value);
	buf[0] = value;
	write (fd, buf, 1);
	break;
    case 'G':
	value = read (fd, buf, 40);
	for (i = 0; i < value; i++)
	    printf ("%02x ", buf[i]);
	printf ("\n");
	break;
    case 'g':
	read (fd, buf, 1);
	printf ("%02x\n", buf[0]);
	break;
    case 'f':
	sscanf (buf, "f %d", &fd);
	break;
    case 'c':
	ioctl (fd, TCGETA, &t);
	printf ("iflag=%04x oflag=%04x cflag=%04x lflag=%04x\n",
		t.c_iflag, t.c_oflag, t.c_cflag, t.c_lflag);
	if (strlen (buf) == 1)
	    break;
	sscanf (buf, "c %1s %04x", buf, &value);
	if (buf[0] == 'i')
	    t.c_iflag = value;
	else if (buf[0] == 'o')
	    t.c_oflag = value;
	else if (buf[0] == 'c')
	    t.c_cflag = value;
	else if (buf[0] == 'l')
	    t.c_lflag = value;
	printf ("iflag=%04x oflag=%04x cflag=%04x lflag=%04x\n",
		t.c_iflag, t.c_oflag, t.c_cflag, t.c_lflag);
	ioctl (fd, TCSETAF, &t);
	break;
    case 's':
	sscanf (buf, "s %d %x", &reg, &value);
	SIOBASE[reg] = value;
	break;
    }
    return (1);
}

prqueue (q)
     Queue          *q;
{
    int             i, ch;

    printf ("[");
    i = 0;
    while ((ch = Qread (q, i)) != 0) {
	printf ("%02x ", ch);
	i++;
    }
    printf ("]\n");
}
#endif

reschedule (p)
     char           *p;
{
#ifdef TEST
    char            buf[80];
    scandevs ();
    sprintf (buf, "%s>>", p);
    docmd (buf);
#else
    scandevs ();
#endif
}

#ifndef TEST
#define reschedule(p)    scandevs()
#endif


/** _write(fd,buf,n) write n bytes from buf to fd */
_write (fd, buf, n)
     int             fd, n;
     char           *buf;
{
    int             i;
    DevEntry       *p;
    char           *t;


    if (!_file[fd].valid)
        return (-1);

#ifdef NETIO
    if (_file[fd].netfd >= 0)
      return netwrite (_file[fd].netfd, buf, n);
#endif

    p = &DevTable[_file[fd].dev];
    for (i = 0; i < n; i++) {
	if (p->t.c_oflag & ONLCR && buf[i] == '\n') {
	    _chwrite (p, '\r');
	    scandevs ();
	}
	_chwrite (p, buf[i]);
	scandevs ();
    }
    return (i);
}

_chwrite (p, ch)
     DevEntry       *p;
     char            ch;
{

    while (p->txoff)
	reschedule ("write txoff");
    if (p->handler) {
	while (!(*p->handler) (OP_TXRDY, p->sio, p->chan))
	  reschedule ("write txrdy");
	(*p->handler) (OP_TX, p->sio, p->chan, ch);
    } else {
#if 0
	/* early error? use main console (and hope someone has programmed it!) */
	ConfigEntry *q = &ConfigTable[0];
	while (!(*q->handler) (OP_TXRDY, q->devinfo, q->chan))
	  continue;
	(*q->handler) (OP_TX, q->devinfo, q->chan, ch);
#endif
    }
}

scandevs ()
{
    int             c, n;
    DevEntry       *p;

    for (p = DevTable; p->qsize != 0; p++) {
	if (!p->rxq)
	    continue;
	while ((*p->handler) (OP_RXRDY, p->sio, p->chan)) {
	    c = (*p->handler) (OP_RX, p->sio, p->chan);
	    if (p->t.c_iflag & ISTRIP)
		c &= 0x7f;
	    if (p->t.c_lflag & ISIG) {
		if (c == p->t.c_cc[VINTR]) {
		    gsignal (p->intr/*XXX*/, 2 /*SIGINT*/);
		    continue;
		}
	    }
	    if (p->t.c_iflag & IXON) {
		if (p->t.c_iflag & IXANY && p->txoff) {
		    p->txoff = 0;
		    continue;
		}
		if (c == p->t.c_cc[V_STOP]) {
		    p->txoff = 1;
		    continue;
		}
		if (c == p->t.c_cc[V_START]) {
		    p->txoff = 0;
		    continue;
		}
	    }

	    n = Qspace (p->rxq);
	    if (n > 0) {
		Qput (p->rxq, c);
		if (n < 20 && !p->rxoff) {
		    (*p->handler) (OP_RXSTOP, p->sio, p->chan, p->rxoff = 1);
		    if (p->t.c_iflag & IXOFF)
			_chwrite (p, CNTRL ('S'));
		}
	    }
	}
    }
#ifdef R4000
    cerrpoll ();
#endif
#ifdef NETIO
    netpoll ();
#endif
    sbdpoll ();
}

/** ioctl(fd,op,argp) perform control operation on fd */
ioctl (fd, op, argp)
     int             fd, op, *argp;
{
    DevEntry       *p;
    struct termio  *at;
    int             i;

    if (!_file[fd].valid)
        return (-1);

#ifdef NETIO
    if (_file[fd].netfd >= 0)
      return netioctl (_file[fd].netfd, op, argp);
#endif

    if (_file[fd].dev < 0)
	return (-1);
    p = &DevTable[_file[fd].dev];

    switch (op) {
    case TCGETA:
	*(struct termio *)argp = p->t;
	break;
    case TCSETAF:		/* after flush of input queue */
	while (!Qempty (p->rxq))
	    Qget (p->rxq);
	(*p->handler) (OP_FLUSH, p->sio, p->chan, 1);
	if (p->rxoff) {
	    (*p->handler) (OP_RXSTOP, p->sio, p->chan, p->rxoff = 0);
	    if (p->t.c_iflag & IXOFF)
		_chwrite (p, CNTRL ('Q'));
	}
    case TCSETAW:		/* after write */
	/* no txq, so no delay needed */
	at = (struct termio *)argp;
	if (p->t.c_cflag != at->c_cflag) {
	    if (at->c_cflag & CXBAUD) {
		if ((*p->handler) (OP_XBAUD, p->sio, p->chan, at->c_cflag))
		    return (-1);
	    }
	    else {
		if ((*p->handler) (OP_BAUD, p->sio, p->chan,
				   CBAUD & at->c_cflag))
		    return (-1);
	    }
	}
	p->t = *at;
	/* smart devices may handle other options (e.g. flow cntrl) */
	(*p->handler) (OP_TCSET, p->sio, p->chan, (int)at);
	break;
    case FIONREAD:
	scandevs ();
	*argp = Qused (p->rxq);
	break;
    case SETINTR:
	p->intr = (jmp_buf *) argp;
	break;
    case GETINTR:
	*(jmp_buf **) argp = p->intr;
	break;
    case SETSANE:
	(*p->handler) (OP_RESET, p->sio, p->chan, 0);
	p->t.c_iflag |= (ISTRIP | ICRNL | IXON);
	p->t.c_oflag = (ONLCR);
	p->t.c_lflag &= (CSIZE | CSTOPB | PARENB | PARODD); /* keep these */
	p->t.c_lflag |= (ICANON | ISIG | ECHO | ECHOE);
	p->t.c_cc[VINTR] = CNTRL ('c');
	p->t.c_cc[VEOL] = '\n';
	p->t.c_cc[VEOL2] = CNTRL ('c');
	p->t.c_cc[VERASE] = CNTRL ('h');
	p->t.c_cc[V_STOP] = CNTRL ('s');
	p->t.c_cc[V_START] = CNTRL ('q');
	(*p->handler) (OP_TCSET, p->sio, p->chan, (int)&p->t);
	break;
    case SETNCNE:
	if (argp)
	  *(struct termio *)argp = p->t;
	p->t.c_lflag &= ~(ICANON | ECHO | ECHOE);
	p->t.c_cc[4] = 1;
	break;
    case CBREAK:
	if (argp)
	  *(struct termio *)argp = p->t;
	p->t.c_lflag &= ~(ICANON | ECHO);
	p->t.c_cc[4] = 1;
	break;
    case GETTERM:
	*argp = 0;
	if (p->tfunc == 0)
	    return (-1);
	strcpy ((char *)argp, p->tname);
	break;
    case SETTERM:
	for (i = 0; TermTable[i].name; i++) {
	    if (strequ (argp, TermTable[i].name))
		break;
	}
	if (TermTable[i].name == 0)
	    return (-1);
	p->tname = TermTable[i].name;
	p->tfunc = TermTable[i].func;
	break;
    case TERMTYPE:
	if (TermTable[fd].name == 0)
	    return (-1);
	strcpy ((char *)argp, TermTable[fd].name);
	return (fd + 1);
    default:
	return (-1);
    }
    return (0);
}

int _pmon_in_ram;

devinit ()
{
    int             i;
    ConfigEntry    *q;
    DevEntry       *p;
    char	   dname[5];
    int		   init_required;

    strcpy (dname, "ttyx");
    for (i = 0; ConfigTable[i].devinfo && i < DEV_MAX; i++) {
	q = &ConfigTable[i];
	p = &DevTable[i];
	p->txoff = 0;
	p->rxoff = 0;
	p->qsize = q->rxqsize;

	init_required = !_pmon_in_ram;
	if (p->qsize < 0) {
	    p->qsize = -p->qsize;
	    init_required = 1;
	}

	if (q->chan == 0 && init_required)
	    if ((*q->handler) (OP_INIT, q->devinfo, 0, p->qsize))
		continue;
	p->rxq = Qcreate (p->qsize);
	if (p->rxq == 0)
	    return (-1);

	if (init_required) {
	    /* set default baudrate in case anything goes wrong */
	    if (q->brate & CXBAUD)
		(void)(*q->handler) (OP_XBAUD, q->devinfo, q->chan, q->brate);
	    else
		(void) (*q->handler) (OP_BAUD, q->devinfo, q->chan, q->brate);
	}

	p->sio = q->devinfo;
	p->chan = q->chan;
	p->handler = q->handler;
	p->intr = 0;
	p->tfunc = 0;
	p->t.c_iflag = (ISTRIP | ICRNL | IXON);
	p->t.c_oflag = (ONLCR);
	p->t.c_lflag = (CS8 | ICANON | ISIG | ECHO | ECHOE);
	p->t.c_cflag = q->brate;
	p->t.c_cc[VINTR] = CNTRL ('c');
	p->t.c_cc[VEOL] = '\n';
	p->t.c_cc[VEOL2] = CNTRL ('c');
	p->t.c_cc[VERASE] = CNTRL ('h');
	p->t.c_cc[V_STOP] = CNTRL ('s');
	p->t.c_cc[V_START] = CNTRL ('q');

	if (init_required) {
	    char *s;
	    dname[3] = (i < 10) ? i + '0' : i - 10 + 'a';
	    if (s = getenv (dname)) {
		struct termio save = p->t;
		if (parsettymode (s, &p->t)
		    || (*p->handler) ((p->t.c_cflag & CXBAUD) ? OP_XBAUD : OP_BAUD, 
				      p->sio, p->chan, p->t.c_cflag))
		    /* restore old settings if baud rate fails */
		    p->t = save;
	    }
	    /* ignore failure in TCSET */
	    (void) (*p->handler) (OP_TCSET, p->sio, p->chan, (int)&p->t);
	    _chwrite (p, CNTRL ('Q'));
	}
    }
    return (0);
}

/** ttctl(fd,op,a1,a2) perform terminal specific operation */
ttctl (fd, op, a1, a2)
     int             fd, op, a1, a2;
{
    DevEntry       *p;
    int             r;

    if (!_file[fd].valid || _file[fd].dev < 0)
        return (-1);

    p = &DevTable[_file[fd].dev];
    if (p->tfunc == 0)
	return (-1);
    r = (*p->tfunc) (fd, op, a1, a2);
    return (r);
}

/** _open(fname,mode,perms) return fd for fname */
_open (fname, mode, perms)
     char           *fname;
     int             mode, perms;
{
    int             i, c, dev, len;
    char            *dname;

    for (i = 0; i < OPEN_MAX && _file[i].valid; i++);
    if (i == OPEN_MAX)
	return (-1);

    dname = fname;
    if (strncmp (dname, "/dev/", 5) == 0)
      dname += 5;

    if (strlen (dname) == 4 && strncmp (dname, "tty", 3) == 0) {
	c = dname[3];
	if (c >= 'a' && c <= 'z')
	  dev = c - 'a';
	else if (c >= 'A' && c <= 'Z')
	  dev = c - 'A';
	else if (c >= '0' && c <= '9')
	  dev = c - '0';
	if (dev >= DEV_MAX || DevTable[dev].rxq == 0)
	  return (-1);
	_file[i].dev = dev;
#ifdef NETIO
	_file[i].netfd = -1;
#endif
	_file[i].valid = 1;
	{
	    DevEntry *p = &DevTable[dev];
	    (*p->handler) (OP_OPEN, p->sio, p->chan, 0);
	}
	ioctl (i, SETSANE);
    } else {
#ifdef NETIO
	if ((_file[i].netfd = netopen (fname, mode)) < 0)
	  return (-1);
	_file[i].dev = -1;
	_file[i].valid = 1;
#else
	return (-1);
#endif
    }
    if (curlst)
	*curlst |= (1 << i);
    return (i);
}

/** _close(fd) close fd */
_close (fd)
     int             fd;
{
    if (_file[fd].valid) {
#ifdef NETIO
	if (_file[fd].netfd >= 0) {
	    netclose (_file[fd].netfd);
	    _file[fd].netfd = -1;
	}
	else
#endif
	{
	    DevEntry *p = &DevTable[_file[fd].dev];
	    (*p->handler) (OP_CLOSE, p->sio, p->chan, 0);
	}
	_file[fd].valid = 0;
	if (curlst)
	  *curlst &= ~(1 << fd);
    }
}

/** _read(fd,buf,n) read n bytes into buf from fd */
_read (fd, buf, n)
     int             fd, n;
     char           *buf;
{
    int             i, used;
    DevEntry       *p;
    char            ch;

    if (!_file[fd].valid)
        return (-1);

#ifdef NETIO
    if (_file[fd].netfd >= 0)
      return netread (_file[fd].netfd, buf, n);
#endif

    p = &DevTable[_file[fd].dev];
    for (i = 0; i < n;) {
	scandevs ();
	while ((used = Qused (p->rxq)) == 0)
	    reschedule ("read Qempty");
	if (used < 20 && p->rxoff) {
	    (*p->handler) (OP_RXSTOP, p->sio, p->chan, p->rxoff = 0);
	    if (p->t.c_iflag & IXOFF)
		_chwrite (p, CNTRL ('Q'));
	}
	ch = Qget (p->rxq);
	if (p->t.c_iflag & ICRNL && ch == '\r')
	    ch = '\n';
	if (p->t.c_lflag & ICANON) {
	    if (ch == p->t.c_cc[VERASE]) {
		if (i > 0) {
		    i--;
		    if (p->t.c_lflag & ECHOE)
		      write (fd, "\b \b", 3);
		    else if (p->t.c_lflag & ECHO)
		      write (fd, "\b", 1);
		}
		continue;
	    }
	    if (p->t.c_lflag & ECHO)
	      write (fd, &ch, 1);
	    buf[i++] = ch;
	    if (ch == p->t.c_cc[VEOL] || ch == p->t.c_cc[VEOL2])
	      break;
	} else {
	    buf[i++] = ch;
	}
    }
    return (i);
}
