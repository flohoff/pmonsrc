/* $Id: bootp.c,v 1.6 2000/08/03 22:33:16 chris Exp $ */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/syslog.h>

#include <netinet/in.h>
#include <machine/endian.h>
#define KERNEL
#include <sys/time.h>

#define _TIME_
#include <net/if.h>
#include <net/if_dl.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <setjmp.h>
#include <errno.h>

#include "bootp.h"
#include "bootparams.h"

extern struct timeval	boottime;
extern struct timeval	time;

#define MAXTMO	20	/* seconds */
#define MINTMO	3	/* seconds */
#define MAX_MSG (3 * 512)

/* Returns true if n_long's on the same net */
#define	SAMENET(a1, a2, m) ((a1.s_addr & m.s_addr) == (a2.s_addr & m.s_addr))

static	char vm_zero[4];
static	char vm_rfc1048[4] = VM_RFC1048;
static	char vm_cmu[4] = VM_CMU;

static int nvend;
static char *vend[] = {
	vm_zero,			/* try no explicit vendor type first */
	vm_cmu,
	vm_rfc1048			/* try variable format last */
};

/* Local forwards */
static	int bootprecv (struct bootparams *, struct bootp *, int);
static	void vend_cmu (struct bootparams *, u_char *);
static	void vend_rfc1048 (struct bootparams *, u_char *, u_int);

static	int xid;
static	jmp_buf jmpb;


static void
finish (int signo)
{
    longjmp (jmpb, 1);
}



static int
getethaddr (unsigned char *eaddr)
{
#if 1
    extern struct ifnet *ifnet;
    struct ifnet *ifp;
    struct ifaddr *ifa;

    /* look for interface "en0" */
    for (ifp = ifnet; ifp; ifp = ifp->if_next)
	if (strcmp (ifp->if_name, "en") == 0 && ifp->if_unit == 0) {
	    /* find link address */
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_LINK) {
		    struct sockaddr_dl *sdl;
		    sdl = (struct sockaddr_dl *) ifa->ifa_addr;
		    bcopy (LLADDR(sdl), eaddr, ifp->if_addrlen);
		    return ifp->if_addrlen;
		}
	    }
	}
    return -1;
#else
    return sbdethaddr (wbp.bp_chaddr);
#endif
}


/* Fetch required bootp infomation */
void
boot_bootp(struct bootparams *bootp)
{
    struct bootp wbp;
    u_char rbuf[MAX_MSG];
    struct sockaddr_in clnt, srvr;
    time_t tmo, tlast, tleft;
    struct timeval tv;
    fd_set rfds;
    sig_t osigint;
    int sock, cc, n, recvcnt;
    volatile int notified;

    bzero(&wbp, sizeof(wbp));
    wbp.bp_op = BOOTREQUEST;
    wbp.bp_htype = HTYPE_ETHERNET;
    wbp.bp_hlen = 6;

    if (getethaddr (wbp.bp_chaddr) < 0) {
#ifdef DEBUG
	printf("bootp: getethaddr failed\n");
#endif
	return;
    }

#ifdef DEBUG
    printf("bootp: ethernet address is %s\n", 
	   ether_sprintf(wbp.bp_chaddr));
#endif
	    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	perror ("bootp socket");
	return;
    }

    n = 1;
    if (setsockopt (sock, SOL_SOCKET, SO_BROADCAST, &n, sizeof (n)) < 0) {
	perror ("bootp setsockopt(BROADCAST)");
	close (sock);
	return;
    }
    if (setsockopt (sock, SOL_SOCKET, SO_DONTROUTE, &n, sizeof (n)) < 0) {
	perror ("bootp setsockopt(DONTROUTE)");
	close (sock);
	return;
    }

    bzero ((char *)&clnt, sizeof (clnt));
    clnt.sin_family = AF_INET;
    clnt.sin_addr.s_addr = INADDR_ANY;
    clnt.sin_port = htons (IPPORT_BOOTPC);
    if (bind (sock, (struct sockaddr *)&clnt, sizeof(clnt)) < 0) {
	perror ("bootp bind");
	close (sock);
	return;
    }

    bzero ((char *)&srvr, sizeof (srvr));
    srvr.sin_family = AF_INET;
    srvr.sin_addr.s_addr = INADDR_BROADCAST;
    srvr.sin_port = htons (IPPORT_BOOTPS);

    notified = 0;
    
    osigint = signal(SIGINT, finish);
    if (setjmp (jmpb)) {
	if (notified)
	    fprintf (stderr, "interrupted\n");
	goto quit;
    }

    for (xid = time.tv_sec, n = 5; n >= 0; n--)
	xid = (xid << 5) + wbp.bp_chaddr[n];

    do {
	tlast = time.tv_sec * 1000000 + time.tv_usec;
	tmo = MINTMO;
	tleft = 0;
	recvcnt = 0;
	while (tmo < MAXTMO) {
	    if (tleft <= 0) {
		if ((nvend != 0 || tmo > MINTMO) && !notified) {
		    fprintf (stderr, "Waiting for bootp");
		    notified = 1;
		}
		if (notified)
		    fprintf (stderr, ".");

		wbp.bp_xid = htonl (++xid);
		wbp.bp_secs = htons (time.tv_sec - boottime.tv_sec);
		wbp.bp_unused |= htons (0x8000); /* request broadcast reply */
		bzero(wbp.bp_file, sizeof(wbp.bp_file));

		bcopy(vend[nvend], wbp.bp_vend, sizeof(long));
		wbp.bp_vend[sizeof(long)] = TAG_END;

		/* If we need vendor info, cycle vendor next time */
		if ((bootp->need & ~bootp->have) & ~BOOT_ADDR)
		    nvend = (nvend + 1) % (sizeof(vend) / sizeof(vend[0]));

#ifdef DEBUG
  printf ("bootp send\n");
#endif
		cc = sendto (sock, &wbp, sizeof(wbp), 0,
			     (struct sockaddr *)&srvr, sizeof(srvr));
		if (cc != sizeof(wbp)) {
		    perror ("bootp sendto");
		    goto quit;
		}
		tleft = tmo * 1000000;
	    }
	    FD_ZERO (&rfds);
	    FD_SET (sock, &rfds);
	    tv.tv_sec = tleft/1000000; tv.tv_usec = tleft%1000000;
	    n = select (sock + 1, &rfds, 0, 0, &tv);
	    if (n < 0) {
		perror ("bootp select");
		goto quit;
	    }

	    if (n == 0) {
#ifdef DEBUG
 printf ("select: no data\n");
#endif
		/* timeout: did we get anything useful */
		if (recvcnt > 0)
		    break;
		tmo <<= 1;
	    } else {
		cc = recv (sock, rbuf, sizeof(rbuf), 0);
#ifdef DEBUG
 printf ("recv: %d\n", cc);
#endif
		if (cc < 0) {
		    perror ("bootp recv");
		    goto quit;
		}

		/* Got a packet, process it */
		cc = bootprecv (bootp, (struct bootp *)rbuf, cc);
		if (cc >= 0)
		    ++recvcnt;
		/* don't wait any longer if we have got all we want */
		if ((bootp->have & bootp->need) == bootp->need)
		    break;
	    }

	    /* reduce time left until timeout */
	    tleft -= time.tv_sec * 1000000 + time.tv_usec - tlast;
	    tlast = time.tv_sec *1000000 + time.tv_usec;
	}
    } while (((bootp->have & bootp->need) != bootp->need) && tmo < MAXTMO);

    if (notified)
	fprintf (stderr, "\n");

quit:
    (void) signal (SIGINT, osigint);
    close (sock);
    sigsetmask (0);
}


/* Returns 0 if this is the packet we're waiting for else -1 */
static int
bootprecv(struct bootparams *bootp, struct bootp *bp, int rsize)
{
    static struct in_addr nmask;
    u_long ul;

    if (rsize < sizeof(*bp) || ntohl(bp->bp_xid) != xid) {
#ifdef DEBUG
	if (rsize < sizeof(*bp))
	    printf("bootprecv: expected %d bytes, got %d\n",
		   sizeof(*bp), rsize);
	else 
	    printf("bootprecv: expected xid 0x%x, got 0x%x\n",
		   xid, ntohl(bp->bp_xid));
#endif
	return (-1);
    }

#ifdef DEBUG
    printf("bootprecv: got one (len = %d)!\n", rsize);
#endif

    /* Pick up our ip address (and natural netmask) */
    if (bp->bp_yiaddr.s_addr != 0 && (bootp->have & BOOT_ADDR) == 0) {
	u_long addr;

	bootp->have |= BOOT_ADDR;
	bootp->addr = bp->bp_yiaddr;
#ifdef DEBUG
	printf("our ip address is %s\n", inet_ntoa(bootp->addr));
#endif
	
	addr = ntohl (bootp->addr.s_addr);
	if (IN_CLASSA(addr))
	    nmask.s_addr = htonl (IN_CLASSA_NET);
	else if (IN_CLASSB(addr))
	    nmask.s_addr = htonl (IN_CLASSB_NET);
	else
	    nmask.s_addr = htonl (IN_CLASSC_NET);
#ifdef DEBUG
	printf("'native netmask' is %s\n", inet_ntoa(nmask));
#endif
    }

    if (bp->bp_siaddr.s_addr != 0 && (bootp->have & BOOT_BOOTIP) == 0) {
	bootp->have |= BOOT_BOOTIP;
	bootp->bootip = bp->bp_siaddr;
    }

    if (bp->bp_file[0] != 0 && (bootp->have & BOOT_BOOTFILE) == 0) {
	bootp->have |= BOOT_BOOTFILE;
	strncpy (bootp->bootfile, bp->bp_file, sizeof (bootp->bootfile));
    }

    /* Suck out vendor info */
    if (bcmp(vm_cmu, bp->bp_vend, sizeof(vm_cmu)) == 0)
	vend_cmu(bootp, bp->bp_vend);
    else if (bcmp(vm_rfc1048, bp->bp_vend, sizeof(vm_rfc1048)) == 0)
	vend_rfc1048(bootp, bp->bp_vend, sizeof(bp->bp_vend));
    else if (bcmp(vm_zero, bp->bp_vend, sizeof(vm_zero)) != 0) {
	bcopy(bp->bp_vend, &ul, sizeof(ul));
	printf("bootprecv: unknown vendor 0x%x\n", ul);
    }

    /* Nothing more to do if we don't know our ip address yet */
    if ((bootp->have & BOOT_ADDR) == 0) {
	return (-1);
    }

    /* Check subnet mask against net mask; toss if bogus */
    if ((bootp->have & BOOT_MASK) != 0 &&
	(nmask.s_addr & bootp->mask.s_addr) != nmask.s_addr) {
#ifdef DEBUG
	printf("subnet mask (%s) bad\n", inet_ntoa(bootp->mask));
#endif
	bootp->have &= ~BOOT_MASK;
    }

    /* Get subnet (or natural net) mask */
    if ((bootp->have & BOOT_MASK) == 0)
	bootp->mask = nmask;

    /* Toss gateway if on a different net */
    if ((bootp->have & BOOT_GATEIP) != 0
	&& !SAMENET(bootp->addr, bootp->gateip, bootp->mask)) {
#ifdef DEBUG
	printf("gateway ip (%s) bad\n", inet_ntoa(bootp->gateip));
#endif
	bootp->gateip.s_addr = 0;
	bootp->have &= ~BOOT_GATEIP;
    }

#ifdef DEBUG
    if (bootp->have & BOOT_MASK)
	printf("smask: %s\n", inet_ntoa(bootp->mask));
    if (bootp->have & BOOT_GATEIP)
	printf("gateway ip: %s\n", inet_ntoa(bootp->gateip));
    if (bootp->have & BOOT_DNSIP)
	printf("nserver ip: %s\n", inet_ntoa(bootp->dnsip));
    if (bootp->have & BOOT_BOOTIP)
	printf("boot ip: %s\n", inet_ntoa(bootp->bootip));
    if (bootp->have & BOOT_HOSTNAME)
	printf ("hostname: %s\n", bootp->hostname);
    if (bootp->have & BOOT_DOMAINNAME)
	printf ("domainname: %s\n", bootp->domainname);
    if (bootp->have & BOOT_BOOTFILE)
	printf("boot file: %s\n", bootp->bootfile);
#endif

    return (0);
}


static void
vend_cmu(struct bootparams *bootp, u_char *cp)
{
    struct cmu_vend *vp;

    vp = (struct cmu_vend *)cp;

    if (vp->v_smask.s_addr != 0 && (bootp->have & BOOT_MASK) == 0) {
	bootp->have |= BOOT_MASK;
	bootp->mask = vp->v_smask;
    }
    if (vp->v_dgate.s_addr != 0 && (bootp->have & BOOT_GATEIP) == 0) {
	bootp->have |= BOOT_GATEIP;
	bootp->gateip = vp->v_dgate;
    }
    if (vp->v_dns1.s_addr != 0 && (bootp->have & BOOT_DNSIP) == 0) {
	bootp->have |= BOOT_DNSIP;
	bootp->dnsip = vp->v_dns1;
    }
}


static void
vend_rfc1048(struct bootparams *bootp, u_char *cp, u_int len)
{
    u_char *ep;
    int size;
    u_char tag;

    ep = cp + len;

    /* Step over magic cookie */
    cp += sizeof(long);

    while (cp < ep) {
	tag = *cp++;
	size = *cp++;
	if (tag == TAG_END)
	    break;
	switch (tag) {
	case TAG_SUBNET_MASK:
	    if (size == sizeof(long)
		&& (bootp->have & BOOT_MASK) == 0) {
		bootp->have |= BOOT_MASK;
		bcopy(cp, &bootp->mask, sizeof(long));
	    }
	    break;
	case TAG_GATEWAY:
	    if (size == sizeof(long)
		&& (bootp->have & BOOT_GATEIP) == 0) {
		bootp->have |= BOOT_GATEIP;
		bcopy(cp, &bootp->gateip, sizeof(long));
	    }
	    break;
	case TAG_DOMAIN_SERVER:
	    if (size == sizeof(long)
		&& (bootp->have & BOOT_DNSIP) == 0) {
		bootp->have |= BOOT_DNSIP;
		bcopy(cp, &bootp->dnsip, sizeof(long));
	    }
	    break;
	case TAG_HOST_NAME:
	    if (size < sizeof (bootp->hostname)
		&& (bootp->have & BOOT_HOSTNAME) == 0) {
		bootp->have |= BOOT_HOSTNAME;
		bcopy (cp, bootp->hostname, size);
		bootp->hostname[size] = '\0';
	    }
	    break;
	case TAG_DOMAIN_NAME:
	    if (size < sizeof (bootp->domainname)
		&& (bootp->have & BOOT_DOMAINNAME) == 0) {
		bootp->have |= BOOT_DOMAINNAME;
		bcopy (cp, bootp->domainname, size);
		bootp->domainname[size] = '\0';
	    }
	    break;
	}
	cp += size;
    }
}
