/* $Id: udptty.c,v 1.3 1998/07/08 22:01:20 chris Exp $ */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <machine/endian.h>

#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <stdlib.h>

#include <netio.h>
#include <termio.h>

#define PKTSIZ		1024

struct udptty {
    struct sockaddr_in sin;
    int		sock;
    int		first;
    int		count;
    int		connected;
    char	buf[PKTSIZ];
};

static netfh_t	uttyopen (const char *, int);
static int	uttyread (netfh_t, void *, int);
static int	uttywrite (netfh_t, const void *, int);
static long	uttylseek (netfh_t, long, int);
static int	uttyioctl (netfh_t, int, void *);
static int	uttyclose (netfh_t);

const struct netfileops udpttyops = {
    uttyopen,
    uttyread,
    uttywrite,
    uttylseek,
    uttyioctl,
    uttyclose
};


static netfh_t
uttyopen (path, flags)
    const char *path;
    int flags;
{
    struct servent *sp;
    struct udptty *utp;
    u_short port;

    if (strncmp (path, "udp", 3)) {
	errno = ENOENT;
	return NULL;
    }
    path += 3;

    if (*path) {
	char *ep;
	if (path[0] != ':' || path[1] == '\0') {
	    errno = ENOENT;
	    return NULL;
	}
	port = htons (strtoul (path + 1, &ep, 10));
	if (*ep) {
	    errno = ENOENT;
	    return NULL;
	}
    }
    else {
	sp = getservbyname("gdb", "udp");
	if (!sp) {
	    errno = EPROTONOSUPPORT;
	    return NULL;
	}
	port = sp->s_port;
    }

    utp = (struct udptty *) malloc (sizeof (struct udptty));
    if (!utp) {
	errno = ENOBUFS;
	return NULL;
    }
    bzero (utp, sizeof (struct udptty));

    utp->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (utp->sock < 0)
      goto error;

    /* initially receive packets from any host */
    utp->sin.sin_family = AF_INET;
    utp->sin.sin_addr.s_addr = INADDR_ANY;
    utp->sin.sin_port = port;
    if (bind (utp->sock, (struct sockaddr *)&utp->sin,
	      sizeof (utp->sin)) < 0) {
	perror ("udptty(bind)");
	goto error;
    }

    return (netfh_t)utp;

error:
    if (utp->sock >= 0)
	close (utp->sock);
    free (utp);
    return NULL;
}


static int
uttyfill (struct udptty *utp)
{
    struct sockaddr_in from;
    int fromlen = sizeof (from);

    utp->count = recvfrom (utp->sock, utp->buf, PKTSIZ, 0,
			   (struct sockaddr *)&from, &fromlen);
    if (utp->count < 0)
	return 0;
    utp->first = 0;
    
    if (!utp->connected) {
	/* once the first packet arrives from any peer, we
	   connect to it and ignore packets from other peers */
	utp->sin.sin_addr = from.sin_addr;
	if (connect (utp->sock, (struct sockaddr *)&utp->sin, 
		     sizeof (utp->sin)) < 0)
	    perror ("udptty(connect)");
	else
	    utp->connected = 1;
    }
    return 1;
}


static int
uttyread (handle, buf, nread)
    netfh_t handle;
    void *buf;
    int nread;
{
    struct udptty *utp = (struct udptty *)handle;
    int nb, n;

    /* continue while more bytes available */
    nb = nread;
    while (nb != 0) {
	if (utp->count > 0) {
	    n = (nb > utp->count) ? utp->count : nb;
	    bcopy (&utp->buf[utp->first], buf, n);
	    buf += n;
	    utp->first += n;
	    utp->count -= n;
	    nb -= n;
	}
	else {
	    /* if we've read anything at all, then return it */
	    if (nb < nread)
		break;
	    /* else try to refill the buffer */
	    if (!uttyfill (utp))
		return -1;
	}
    }

    return nread - nb;
}

static int
uttywrite (handle, buf, nwrite)
    netfh_t handle;
    const void *buf;
    int nwrite;
{
    struct udptty *utp = (struct udptty *)handle;
    int nb, n;

    if (!utp->connected)
	return nwrite;		/* XXX */
	
    nb = nwrite;
    while (nb != 0) {
	n = (nb > PKTSIZ) ? PKTSIZ : nb;
	if (send (utp->sock, buf, n, 0) != n) 
	    break;
	buf += n;
	nb -= n;
    }

    return nwrite - nb;
}

static long
uttylseek (handle, offs, how)
    netfh_t handle;
    long offs;
    int how;
{
    errno = EINVAL;
    return -1;
}


static int
uttyioctl (handle, op, argp)
    netfh_t handle;
    int op;
    void *argp;
{
    struct udptty *utp = (struct udptty *)handle;
    struct timeval timo;
    fd_set ifds, xfds;

    switch (op) {
    case FIONREAD:
	*(int *)argp = utp->count;
	if (utp->count <= 0) {
	    FD_ZERO(&ifds);
	    FD_ZERO(&xfds);
	    FD_SET(utp->sock, &ifds);
	    FD_SET(utp->sock, &xfds);
	    timo.tv_sec = timo.tv_usec = 0;
	    switch (select (utp->sock + 1, &ifds, 0, &xfds, &timo)) {
	    case -1:
		return -1;
	    case 1:
		if (uttyfill (utp))
		    *(int *)argp = utp->count;
		break;
	    case 0:
		scandevs ();
		break;
	    }
	}
	break;

    case TCGETA:
    case TCSETAF:
    case TCSETAW:
    case SETSANE:
    case CBREAK:
	/* dummies */
	break;

    default:
	return (-1);
    }
    return 0;
}


static int
uttyclose (handle)
    netfh_t handle;
{
    struct udptty *utp = (struct udptty *)handle;

    close (utp->sock);
    free (utp);
    return (0);
}
