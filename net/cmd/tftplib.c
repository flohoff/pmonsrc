/* $Id: tftplib.c,v 1.3 1998/09/20 23:53:53 chris Exp $ */
/* Many bug fixes are from Jim Guyton <guyton@rand-unix> */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/syslog.h>

#include <netinet/in.h>
#include <arpa/tftp.h>

#include <machine/endian.h>

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>

#include <netio.h>

#ifdef TFTPDBG
static int	tftptrace;
#endif

static const char spinner[] = "-\\|/";

static const int tftperrmap[] = {
    EIO,	/* EUNDEF	not defined */
    ENOENT,	/* ENOTFOUND	file not found */
    EACCES,	/* EACCESS	access violation */
    ENOSPC,	/* ENOSPACE	disk full or allocation exceeded */
    EIO,	/* EBADOP	illegal TFTP operation */
    EINVAL,	/* EBADID	unknown transfer ID */
    EEXIST,	/* EEXISTS	file already exists */
    EACCES	/* ENOUSER	no such user */
};

#define	TIMEOUT		5		/* secs between rexmts */
#define MAXREXMT	5		/* no of rexmts */
#define PKTSIZE		(SEGSIZE+4)

struct tftpfile {
    struct sockaddr_in sin;
    u_short	block;
    short	flags;
    short	eof;
    int		sock;
    int 	start;
    int		end;
    int		foffs;
    char	buf[PKTSIZE];
};

static netfh_t	tftpopen (const char *, int);
static int	tftpread (netfh_t, void *, int);
static int	tftpwrite (netfh_t, const void *, int);
static long	tftplseek (netfh_t, long, int);
static int	tftpioctl (netfh_t, int, void *);
static int	tftpclose (netfh_t);

const struct netfileops tftpops = {
    tftpopen,
    tftpread,
    tftpwrite,
    tftplseek,
    tftpioctl,
    tftpclose
};


static int	tftprrq (struct tftpfile *tfp, struct tftphdr *req, int size);
static int	tftpwrq (struct tftpfile *tfp, struct tftphdr *req, int size);
static void	tftpnxtblk (struct tftpfile *tfp);
static int	makerequest (int request, const char *name, struct tftphdr *tp,
			     const char *mode);
static void	tpacket (char *s, struct tftphdr *tp, int n);
static int	synchnet (int f);


static netfh_t
tftpopen (path, flags)
    const char *path;
    int flags;
{
    extern char *getenv(), *strchr();
    struct hostent *hp;
    struct servent *sp;
    const char *name;
    const char *mode;
    char *host;
    char hbuf[MAXHOSTNAMELEN];
    char reqbuf[PKTSIZE];
    struct tftpfile *tfp;
    struct tftphdr *rp;
    int oflags, size;

    oflags = flags & O_ACCMODE; 
    if (oflags != O_RDONLY && oflags != O_WRONLY) {
	errno = EACCES;
	return NULL;
    }

    sp = getservbyname("tftp", "udp");
    if (!sp) {
	errno = EPROTONOSUPPORT;
	return NULL;
    }

#ifdef TFTPDBG
    tftptrace = getenv ("tftptrace") ? 1 : 0;
#endif

    if (name = strchr (path, ':')) {
	int len = name - path;
	if (len + 1 > sizeof(hbuf)) {
	    errno = ENAMETOOLONG;
	    return NULL;
	}
	bcopy (path, host = hbuf, len);
	host[len] = 0;
	name++;
    } else {
	host = getenv ("tftphost");
	if (!host) {
	    log (LOG_INFO, "tftp: missing/bad host name: %s\n", path);
	    errno = EDESTADDRREQ;
	    return NULL;
	}
	name = path;
    }

    tfp = (struct tftpfile *) malloc (sizeof (struct tftpfile));
    if (!tfp) {
	errno = ENOBUFS;
	return NULL;
    }
    bzero (tfp, sizeof (struct tftpfile));

    tfp->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (tfp->sock < 0)
      goto error;

    tfp->sin.sin_family = AF_INET;
    if (bind(tfp->sock, (struct sockaddr *)&tfp->sin, sizeof (tfp->sin)) < 0)
	goto error;

    hp = gethostbyname(host);
    if (hp) {
	tfp->sin.sin_family = hp->h_addrtype;
	bcopy(hp->h_addr, &tfp->sin.sin_addr, hp->h_length);
	strncpy (hbuf, hp->h_name, sizeof(hbuf) - 1);
	hbuf[sizeof(hbuf)-1] = '\0';
	host = hbuf;
    } else {
	tfp->sin.sin_family = AF_INET;
	tfp->sin.sin_addr.s_addr = inet_addr (host);
	if (tfp->sin.sin_addr.s_addr == -1) {
	    log (LOG_INFO, "tftp: bad internet address: %s\n", host);
	    errno = EADDRNOTAVAIL;
	    goto error;
	}
    }

    tfp->sin.sin_port = sp->s_port;

    rp = (struct tftphdr *)reqbuf;
    tfp->flags = flags;
    mode = (oflags & O_ASCII) ? "netascii" : "octet";
    if (oflags == O_RDONLY) {
	tfp->block = 1;
	size = makerequest(RRQ, name, rp, mode);
	size = tftprrq (tfp, rp, size);
	tfp->end = tfp->start + size;
    } else {
	tfp->block = 0;
	size = makerequest(WRQ, name, rp, mode);
	size = tftpwrq (tfp, rp, size - 4);
    }

    if (size >= 0)
	return (netfh_t)tfp;

error:
    if (tfp->sock >= 0)
	close (tfp->sock);
    free (tfp);
    return NULL;
}

static int
tftpread (handle, buf, nread)
    netfh_t handle;
    void *buf;
    int nread;
{
    struct tftpfile *tfp = (struct tftpfile *)handle;
    struct tftphdr *rp;
    int nb, n;

    if ((tfp->flags & O_ACCMODE) != O_RDONLY) {
	errno = EPERM;
	return (-1);
    }

    rp = (struct tftphdr *) tfp->buf;

    /* continue while more bytes wanted, and more available */
    for (nb = nread; nb != 0 && tfp->start < tfp->end; ) {

	if (tfp->foffs >= tfp->start && tfp->foffs < tfp->end) {
	    /* got some data that's in range */
	    n = tfp->end - tfp->foffs;
	    if (n > nb) n = nb;
	    bcopy (&rp->th_data[tfp->foffs - tfp->start], buf, n);
	    tfp->foffs += n;
	    buf += n;
	    nb -= n;
	} 

	if (tfp->foffs >= tfp->end) {
	    /* buffer is empty, ack last packet and refill */
	    struct tftphdr ack;
	    ack.th_opcode = htons((u_short)ACK);
	    ack.th_block = htons((u_short)tfp->block);
	    tftpnxtblk (tfp);
	    n = tftprrq (tfp, &ack, 4);
	    if (n < 0)
	      return (-1);
	    tfp->start = tfp->end;
	    tfp->end   = tfp->start + n;
	}
    }

    return nread - nb;
}

static int
tftpwrite (handle, buf, nwrite)
    netfh_t handle;
    const void *buf;
    int nwrite;
{
    struct tftpfile *tfp = (struct tftpfile *)handle;
    struct tftphdr *dp;
    int nb, n;

    if ((tfp->flags & O_ACCMODE) != O_WRONLY) {
	errno = EPERM;
	return (-1);
    }

    dp = (struct tftphdr *)tfp->buf;
    for (nb = nwrite; nb != 0; ) {
	n = SEGSIZE - tfp->end;
	if (n > nb) n = nb;
	bcopy (buf, &dp->th_data[tfp->end], n);
	tfp->end += n;
	tfp->foffs += n;
	buf += n;
	nb -= n;

	if (tfp->end == SEGSIZE) {
	    /* buffer is full, send it */
	    tftpnxtblk (tfp);
	    dp->th_opcode = htons((u_short)DATA);
	    dp->th_block = htons((u_short)tfp->block);
	    n = tftpwrq (tfp, dp, tfp->end);
	    if (n < 0)
	      return (-1);
	    tfp->end = 0;
	}
    }
    return nwrite - nb;
}

static long
tftplseek (handle, offs, how)
    netfh_t handle;
    long offs;
    int how;
{
    struct tftpfile *tfp = (struct tftpfile *)handle;
    long noffs;

    switch (how) {
    case 0:
	noffs = offs; 
	break;
    case 1:
	noffs = tfp->foffs + offs; 
	break;
    case 2:
    default:
	return -1;
    }

    if ((tfp->flags & O_ACCMODE) == O_WRONLY) {
	if (noffs != tfp->foffs) {
	    errno = ESPIPE;
	    return (-1);
	}
    } else {
	if (noffs < tfp->start) {
	    errno = ESPIPE;
	    return (-1);
	}
	tfp->foffs = noffs;
    }

    return (tfp->foffs);
}


static int
tftpioctl (handle, op, argp)
    netfh_t handle;
    int op;
    void *argp;
{
    errno = ENOTTY;
    return -1;
}


static int
tftpclose (handle)
    netfh_t handle;
{
    struct tftpfile *tfp = (struct tftpfile *)handle;
    struct tftphdr *tp;

    tp = (struct tftphdr *) tfp->buf;
    if ((tfp->flags & O_ACCMODE) == O_WRONLY) {
	int n = 0;
	/* flush last block */
	tftpnxtblk (tfp);
	tp->th_opcode = htons((u_short)DATA);
	tp->th_block = htons((u_short)tfp->block);
	n = tftpwrq (tfp, tp, tfp->end);
	if (n >= 0 && tfp->end == SEGSIZE) {
	    /* last block == SEGSIZE, so send empty one to terminate */
	    tftpnxtblk (tfp);
	    tp->th_opcode = htons((u_short)DATA);
	    tp->th_block = htons((u_short)tfp->block);
	    (void) tftpwrq (tfp, tp, 0);
	}
    } else {
	if (tfp->foffs < tfp->end || !tfp->eof) {
	    const char *msg;
	    int length;
	    
	    /* didn't reach eof, so send a nak to terminate */
	    tp->th_opcode = htons((u_short)ERROR);
	    tp->th_code = htons((u_short)EUNDEF);
	    msg = "file closed";
	    strcpy(tp->th_msg, msg);
	    length = strlen(msg) + 4;
#ifdef TFTPDBG
	    if (tftptrace)
	      tpacket("sent", tp, length);
#endif
	    if (sendto(tfp->sock, tp, length, 0, (struct sockaddr *)&tfp->sin,
		       sizeof (tfp->sin)) != length)
	      perror("sendto(eof)");
	}
    }
    close (tfp->sock);
    free (tfp);
    return (0);
}

static void
tftpnxtblk (tfp)
    struct tftpfile *tfp;
{
    tfp->block++;
    if (tfp->flags & O_NONBLOCK)
      dotik (20000, 0);
}


static int 
tftprrq (tfp, req, size)
    struct tftpfile *tfp;
    struct tftphdr *req;
    int size;
{
    struct tftphdr *rp;
    struct sockaddr_in from;
    fd_set ifds;
    int fromlen, n;
    struct timeval timo;
    int rexmt = 0;

    while (1) {
#ifdef TFTPDBG
	if (tftptrace)
	  tpacket("sent", req, size);
#endif
	if (sendto(tfp->sock, req, size, 0, (struct sockaddr *)&tfp->sin,
		   sizeof (tfp->sin)) != size) {
	    perror("tftp: sendto");
	    return (-1);
	}

	if (tfp->eof)
	  /* reached eof, no more to read */
	  return 0;

	FD_ZERO(&ifds);
	FD_SET(tfp->sock, &ifds);
	timo.tv_sec = TIMEOUT; timo.tv_usec = 0;
	switch (select (tfp->sock + 1, &ifds, 0, 0, &timo)) {
	case -1:
	    perror("tftp: select");
	    return (-1);
	case 0:
	    if (++rexmt > MAXREXMT) {
		errno = ETIMEDOUT;
		return (-1);
	    }
	    log (LOG_INFO, "tftp: timeout, retry %d\n", rexmt);
	    continue;
	}

	fromlen = sizeof (from);
	rp = (struct tftphdr *) tfp->buf;
	n = recvfrom(tfp->sock, rp, PKTSIZE, 0,
		     (struct sockaddr *)&from, &fromlen);
	if (n < 0) {
	    perror("tftp: recvfrom");
	    return (-1);
	}
#ifdef TFTPDBG
	if (tftptrace)
	  tpacket("received", rp, n);
#endif

	if (tfp->block <= 1)
	  tfp->sin.sin_port = from.sin_port;
	else if (from.sin_port != tfp->sin.sin_port)
	  continue;
	/* should verify client address completely? */

	rp->th_opcode = ntohs(rp->th_opcode);
	rp->th_block = ntohs(rp->th_block);
	if (rp->th_opcode == ERROR) {
	    if (rp->th_msg[0])
	      log(LOG_INFO, "tftp: %s\n", rp->th_msg);
	    errno = tftperrmap[rp->th_code & 7];
	    return (-1);
	}

	if (rp->th_opcode == DATA) {
	    int j;
	    if (rp->th_block == tfp->block) {
		/* got the packet */
		n -= 4;
		if (n < SEGSIZE)
		  tfp->eof = 1;
		return (n);
	    }

	    /* On an error, try to synchronize
	     * both sides.
	     */
	    j = synchnet(tfp->sock);
	    if (j)
	      log (LOG_INFO, "tftp: discarded %d packets\n", j);

	    if (rp->th_block != tfp->block - 1)
	      return (-1);
	}
    }
}

static int 
tftpwrq (tfp, req, size)
    struct tftpfile *tfp;
    struct tftphdr *req;
    int size;
{
    char ackbuf[PKTSIZE];
    struct tftphdr *rp;
    struct sockaddr_in from;
    fd_set ifds;
    int fromlen, n;
    struct timeval timo;
    int rexmt = 0;

    while (1) {
#ifdef TFTPDBG
	if (tftptrace)
	  tpacket("sent", req, size);
#endif
	if (sendto(tfp->sock, req, size+4, 0, (struct sockaddr *)&tfp->sin,
		   sizeof (tfp->sin)) != size+4) {
	    perror("tftp: sendto");
	    return (-1);
	}

	FD_ZERO(&ifds);
	FD_SET(tfp->sock, &ifds);
	timo.tv_sec = TIMEOUT; timo.tv_usec = 0;
	switch (select (tfp->sock + 1, &ifds, 0, 0, &timo)) {
	case -1:
	    perror("tftp: select");
	    return (-1);
	case 0:
	    if (++rexmt > MAXREXMT) {
		errno = ETIMEDOUT;
		return (-1);
	    }
	    log (LOG_INFO, "tftp: timeout, retry %d\n", rexmt);
	    continue;
	}

	fromlen = sizeof (from);
	rp = (struct tftphdr *) ackbuf;
	n = recvfrom(tfp->sock, rp, PKTSIZE, 0,
		     (struct sockaddr *)&from, &fromlen);
	if (n < 0) {
	    perror("tftp: recvfrom");
	    return (-1);
	}
#ifdef TFTPDBG
	if (tftptrace)
	  tpacket("received", rp, n);
#endif

	if (tfp->block == 0)
	  tfp->sin.sin_port = from.sin_port;
	else if (from.sin_port != tfp->sin.sin_port)
	  continue;
	/* should verify client address completely? */

	rp->th_opcode = ntohs(rp->th_opcode);
	rp->th_block = ntohs(rp->th_block);
	if (rp->th_opcode == ERROR) {
	    if (rp->th_msg[0])
	      log(LOG_INFO, "tftp: %s\n", rp->th_msg);
	    errno = tftperrmap[rp->th_code & 7];
	    return (-1);
	}

	if (rp->th_opcode == ACK) {
	    int j;
	    if (rp->th_block == tfp->block)
	      /* acknowledged packet */
	      return (size);

	    /* On an error, try to synchronize
	     * both sides.
	     */
	    j = synchnet(tfp->sock);
	    if (j)
	      log (LOG_INFO, "tftp: discarded %d packets\n", j);

	    if (rp->th_block != tfp->block - 1)
	      return (-1);
	}
    }
}

static int
makerequest(request, name, tp, mode)
	int request;
	const char *name, *mode;
	struct tftphdr *tp;
{
	register char *cp;

	tp->th_opcode = htons((u_short)request);
	cp = tp->th_stuff;
	strcpy(cp, name);
	cp += strlen(name);
	*cp++ = '\0';
	strcpy(cp, mode);
	cp += strlen(mode);
	*cp++ = '\0';
	return (cp - (char *)tp);
}


#ifdef TFTPDBG
static void
tpacket(s, tp, n)
	char *s;
	struct tftphdr *tp;
	int n;
{
	static const char * const opcodes[] =
	   { "#0", "RRQ", "WRQ", "DATA", "ACK", "ERROR" };
	register char *cp, *file;
	u_short op = ntohs(tp->th_opcode);

	if (op < RRQ || op > ERROR)
		printf("%s opcode=%x ", s, op);
	else
		printf("%s %s ", s, opcodes[op]);
	switch (op) {

	case RRQ:
	case WRQ:
		n -= 2;
		file = cp = tp->th_stuff;
		cp += strlen(cp);
		printf("<file=%s, mode=%s>\n", file, cp + 1);
		break;

	case DATA:
		printf("<block=%d, %d bytes>\n", ntohs(tp->th_block), n - 4);
		break;

	case ACK:
		printf("<block=%d>\n", ntohs(tp->th_block));
		break;

	case ERROR:
		printf("<code=%d, msg=%s>\n", ntohs(tp->th_code), tp->th_msg);
		break;
	}
}
#endif


/* When an error has occurred, it is possible that the two sides
 * are out of synch.  Ie: that what I think is the other side's
 * response to packet N is really their response to packet N-1.
 *
 * So, to try to prevent that, we flush all the input queued up
 * for us on the network connection on our host.
 *
 * We return the number of packets we flushed (mostly for reporting
 * when trace is active).
 */

static int
synchnet(f)
int	f;		/* socket to flush */
{
	int i, j = 0;
	char rbuf[PKTSIZE];
	struct sockaddr_in from;
	int fromlen;

	while (1) {
		(void) ioctl(f, FIONREAD, &i);
		if (i) {
			j++;
			fromlen = sizeof from;
			(void) recvfrom(f, rbuf, sizeof (rbuf), 0,
				(struct sockaddr *)&from, &fromlen);
		} else {
			return(j);
		}
	}
}
