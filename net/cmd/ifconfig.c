/* $Id: ifconfig.c,v 1.4 2000/04/15 00:50:18 chris Exp $ */
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/protosw.h>

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "bootparams.h"

#define SIN(x) ((struct sockaddr_in *)&(x))

extern char	*getenv(const char *);
extern int	setenv(const char *, const char *);
extern int	_setenv(const char *, const char *);

static void boot_getenv (struct bootparams *bootp);
static void boot_setenv (struct bootparams *bootp, int level);


static void
setsin (struct sockaddr_in *sa, int family, u_long addr)
{
    bzero (sa, sizeof (*sa));
    sa->sin_len = sizeof (*sa);
    sa->sin_family = family;
    sa->sin_addr.s_addr = addr;
}


int
ifconfig (ifname)
    char *ifname;
{
    struct sockaddr_in local, loop, zmask;
    struct bootparams bootp;
    struct ifaliasreq addreq;
    struct ifreq ifreq;
    char *gw;
    int s, bootplev;

    bzero (&bootp, sizeof (bootp));
    bootp.need = BOOT_ADDR;

    s = socket (AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
	perror("ifconfig: socket");
	return(0);
    }


    /*
     * Get the parameters for the ethernet interface 
     */
    bootplev = matchenv ("bootp");
    if (bootplev <= 1)
	/* get initial network parameters from environment */
	boot_getenv (&bootp);

    if (bootplev >= 1) {
	bzero (&addreq, sizeof(addreq));
	strncpy(addreq.ifra_name, ifname, IFNAMSIZ);
	setsin (SIN(addreq.ifra_addr), AF_INET, INADDR_ANY);
	setsin (SIN(addreq.ifra_broadaddr), AF_INET, INADDR_BROADCAST);
	if (ioctl(s, SIOCAIFADDR, &addreq) < 0) {
	    perror("ioctl (SIOCAIFADDR) dummy");
	    close (s);
	    return(0);
	}

	/* get network parameters from network */
	boot_bootp (&bootp);

	/* delete dummy address */
	(void) ioctl(s, SIOCDIFADDR, &addreq);

	/* get any remaining unknown parameters from environment */
	boot_getenv (&bootp);
    }


    if (!(bootp.have & BOOT_ADDR)) {
	if (bootplev >= 1)
	    fprintf (stderr, "\nERROR: bootp failed to set network address\n");
	else
	    fprintf (stderr, "\nNOTICE: set $netaddr or $bootp to enable network\n");
	close (s);
	return (0);
    }

    if (bootplev >= 1)
	/* save parameters in volatile or non-volatile environment */
	boot_setenv (&bootp, bootplev);

    /* got everyting we can, put them into the ifaddreq structure */
    bzero (&addreq, sizeof(addreq));
    strncpy(addreq.ifra_name, ifname, IFNAMSIZ);
    setsin (SIN(addreq.ifra_addr), AF_INET, bootp.addr.s_addr);
    if (bootp.have & BOOT_MASK)
	setsin (SIN(addreq.ifra_mask), 0, bootp.mask.s_addr);
    if (bootp.have & BOOT_BROADADDR)
	setsin (SIN(addreq.ifra_broadaddr), AF_INET, bootp.broadaddr.s_addr);
    /* remember our local address for later */
    local = *SIN(addreq.ifra_addr);

    /* now set our actual address */
    if (ioctl(s, SIOCAIFADDR, &addreq) < 0) {
	/* Assume this means no network interface to attach to */
	fprintf (stderr, "\nNOTICE: No network interface available\n");
	/*perror("ioctl (SIOCAIFADDR)"); */
	close (s);
	return(0);
    }


    /*
     * Configure the loopback i/f (lo0).
     */
    bzero (&addreq, sizeof(addreq));
    strncpy(addreq.ifra_name, "lo0", IFNAMSIZ);
    setsin (SIN(addreq.ifra_addr), AF_INET, htonl (0x7f000001));
    setsin (SIN(addreq.ifra_mask), 0, htonl (0xff000000));
    /* remember loopback address for later */
    loop = *SIN(addreq.ifra_addr);

    if (ioctl(s, SIOCAIFADDR, &addreq) < 0) {
	perror("ioctl (SIOCAIFADDR) loopback");
	close (s);
	return(0);
    }


    /*
     * Now setup the routing tables, equivalent to:
     *  route add default $gateway 0
     *  route add $netaddr 127.0.0.1 0
     */

    /* set a default routing mask */
    bzero (&zmask, sizeof(zmask));

    if ((bootp.have & BOOT_GATEIP) || (gw = getenv ("gateway"))) {
	/* install default route via the gateway */
	struct sockaddr_in dst;
	struct sockaddr_in gate;

	setsin (&dst, AF_INET, INADDR_ANY);
	if (bootp.have & BOOT_GATEIP)
	    setsin (&gate, AF_INET, bootp.gateip.s_addr);
	else {
	    /* try for a DNS lookup */
	    struct hostent *hp;
	    u_long addr = -1;
	    if ((hp = gethostbyname (gw)) && hp->h_addrtype == AF_INET)
		bcopy (hp->h_addr, &addr, sizeof(u_long));
	    setsin (&gate, AF_INET, addr);
	}

	if (gate.sin_addr.s_addr != (u_long)-1) {
	    errno = rtrequest (RTM_ADD, &dst, &gate, &zmask,
			       RTF_UP | RTF_GATEWAY, (struct rtentry **)0);
	    if (errno)
	      perror ("route add gateway");
	} else {
	    (void)fprintf (stderr, "gateway %s: unknown host\n", gw);
	}
    }

    /*
     * Route packets with local address via loopback i/f.
     */
    errno = rtrequest (RTM_ADD, &local, &loop, &zmask, RTF_UP | RTF_HOST,
		       (struct rtentry **)0);
    if (errno)
      perror ("route add loopback");

    close (s);
    return (1);
}



static int
ia_getenv (const char *name, struct in_addr *ia)
{
    char *s;
    if (s = getenv (name))
	return inet_aton (s, ia);
    return 0;
}


static void
boot_getenv (struct bootparams *bootp)
{
    /* these are the only ones that we have to know in this module */

    if (!(bootp->have & BOOT_ADDR)
	&& (ia_getenv ("netaddr", &bootp->addr)))
	bootp->have |= BOOT_ADDR;

    if (!(bootp->have & BOOT_MASK)
	&& (ia_getenv ("netmask", &bootp->mask)))
	bootp->have |= BOOT_MASK;

    if (!(bootp->have & BOOT_BROADADDR)
	&& (ia_getenv ("broadcast", &bootp->broadaddr)))
	bootp->have |= BOOT_BROADADDR;

    if (!(bootp->have & BOOT_GATEIP)
	&& (ia_getenv ("gateway", &bootp->gateip)))
	bootp->have |= BOOT_GATEIP;
}



static void
__setenv (const char *name, const char *val, int level)
{
    int (*set)(const char *, const char *) = level >= 3 ? setenv : _setenv;

    if (level <= 1 && getenv (name))
	return;
    if (level >= 3)
	/* write to non-volatile environment */
	setenv (name, val);
    else
	/* write to local environment only */
	_setenv (name, val);
}


static void
ia_setenv (const char *name, struct in_addr addr, int level)
{
    char *s = inet_ntoa (addr);
    __setenv (name, s, level);
}


static void
boot_setenv (struct bootparams *bootp, int level)
{
    if (bootp->have & BOOT_ADDR)
	ia_setenv ("netaddr", bootp->addr, level);

    if (bootp->have & BOOT_MASK)
	ia_setenv ("netmask", bootp->mask, level);

    if (bootp->have & BOOT_BROADADDR)
	ia_setenv ("broadcast", bootp->broadaddr, level);

    if (bootp->have & BOOT_GATEIP)
	ia_setenv ("gateway", bootp->gateip, level);

    if (bootp->have & BOOT_DNSIP)
	ia_setenv ("nameserver", bootp->dnsip, level);

    if (bootp->have & BOOT_BOOTIP)
	ia_setenv ("bootaddr", bootp->bootip, level);

    if (bootp->have & BOOT_HOSTNAME)
	__setenv ("hostname", bootp->hostname, level);

    if (bootp->have & BOOT_DOMAINNAME)
	__setenv ("localdomain", bootp->domainname, level);

    if (bootp->have & BOOT_BOOTFILE)
	__setenv ("bootfile", bootp->bootfile, level);
}
