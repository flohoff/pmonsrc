/* $Id: if_sonic.c,v 1.6 1999/03/29 12:23:24 chris Exp $ */
/*
 *	if_sonic.c: National Semiconductor SONIC Driver
 *	
 *	Copyright (c) 1992 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */

#ifndef INET
#define INET
#endif

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/mbuf.h"
#include "sys/protosw.h"
#include "sys/socket.h"
#include "sys/errno.h"
#include "sys/uio.h"
#include "sys/ioctl.h"
#include "sys/syslog.h"

#include "net/if.h"
#include "net/if_types.h"
#include "net/netisr.h"
#include "net/route.h"

#ifdef INET
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#include "netinet/if_ether.h"
#endif

#include "mips.h"
#include "sbd.h"

#include "if_sonic.h"

#define NSONIC		1	/* number of sonics */
#define TINYFRAG	12	/* tx frags < this size count as "tiny" */
#define MAXTINIES	3	/* max "tiny" frags, after which we compress */
#define SNRETRIES	5	/* max retries on fatal tx errors */

#define DBG_TXPKT	0x02
#define DBG_RXPKT	0x04
int T_sn;
#define T_txp		(T_sn & DBG_TXPKT)
#define T_rxp		(T_sn & DBG_RXPKT)

#ifdef R4000
#define clean_dcache	mips_clean_dcache
#else
#define clean_dcache	r3k_dclean
#endif


/* 
 * Statistics collected over time 
 * (same order as enpstats)
 */
struct sn_stats {
    int ss_tpacks;		/* transmit ok */
    int ss_tmore;		/* transmit more than one retry */
    int ss_tone;		/* transmit one retry */
    int ss_cerr;		/* transmit failed after retries */
    int ss_tdef;		/* transmit deferred */
    int ss_tbuff;		/* transmit buffer errors */
    int ss_tuflo;		/* transmit silo underflow */
    int ss_tlcol;   		/* transmit late collisions */
    int ss_tlcar;		/* transmit lost carriers */
    int ss_babl;		/* transmit babbling */
    int	ss_noheart;		/* transmit no heartbeat */
    int ss_tmerr;		/* transmit memory error */

    int ss_rpacks;		/* receive ok */
    int ss_miss;		/* receive missed packets */
    int ss_rcrc;		/* receive crc errors */
    int ss_rfram;		/* receive framing errors */
    int ss_rbuff;		/* receive buffer errors */
    int ss_roflo;		/* receive silo overflow */
    int ss_rmerr;		/* receive memory error */
};


static struct sn_softc {
    struct arpcom	sn_ac;
#define	sn_if		sn_ac.ac_if	/* network visible interface */
#define	sn_enaddr	sn_ac.ac_enaddr	/* hardware ethernet address */

    volatile struct sn_reg *sn_csr;	/* hardware pointer */

    int 		sn_rxmark;	/* index in rx ring */
    int			sn_rramark;	/* index into rra of wp */
    volatile struct RXpkt *sn_lrxp;	/* last RDA available to chip */
    int			sn_retries;	/* transmit retries */
    int			sn_rbe;		/* had an RBE signal */

    struct sn_stats	sn_sum;		/* software maintained stats */
    u_int		sn_crct;	/* crc tally from chip */
    u_int		sn_faet;	/* frame error tally from chip */
    u_int		sn_mpt;		/* missed packet tally from chip */
    int			sn_rev;		/* chip revision number */
} sn_softc[NSONIC];


/* Public functions from outside this module */
static int	snifinit (int unit);
static int	snioctl (struct ifnet *ifp, int cmd, caddr_t data);
static int	snifstart (struct ifnet *ifp);
static int	snwatch (int unit);
static int	snreset (int unit);
static void	snintr (int unit);

/* Local functions */
static int	snrestart (struct sn_softc *sn);
static int	snstartup (struct sn_softc *sn);
static int	snclosedown (struct sn_softc *sn);
static int	snput (struct sn_softc *sn, struct mbuf *m0);
static void	sntxint (struct sn_softc *);
static void	sntxdump (volatile struct TXpkt *txp);
static void	snrxint (struct sn_softc *);
static int	snerrint (struct sn_softc *, u_int isr);
static struct mbuf *snget (struct sn_softc *sn, caddr_t addr, 
			   int dlen, int toff, int tlen);
static int	snread (struct sn_softc *, volatile struct RXpkt *);
static struct mbuf *sn_fillup (struct mbuf *m0, int minlen);

static void	caminitialise (void);
static void	camentry (int, u_char *ea);
static int	camprogram (struct sn_softc *);

static int	allocatebuffers (void);
static void	initialise_tda (struct sn_softc *);
static void	initialise_rda (struct sn_softc *);
static void	initialise_rra (struct sn_softc *);

#ifdef MIPSEL
static void	snswapb (u_char *s, u_char *d, int len);
static struct mbuf *snswapm (struct mbuf *m);
#endif

#if defined(XDSSONICBUG)
/*
 * SONIC buffers need to be aligned 64 bit aligned.
 * These macros calculate and verify alignment.
 */
#define SONICDW		64
#define SONICALIGN	(SONICDW/16)
#else
/*
 * SONIC buffers need to be aligned 16 or 32 bit aligned.
 * These macros calculate and verify alignment.
 */
#define SONICDW		32
#define SONICALIGN	(SONICDW/8)
#endif

#define UPPER(x)	(K0_TO_PHYS((x)) >> 16)
#define LOWER(x)	(((unsigned int)(x)) & 0xffff)

/*
 * buffer sizes in 32 bit mode
 * 1 TXpkt is 4 hdr words + (3 * FRAGMAX) + 1 link word 
 * FRAGMAX == 16 => 54 words == 216 bytes
 *
 * 1 RxPkt is 7 words == 28 bytes
 * 1 Rda   is 4 words == 16 bytes
 */

#define NRRA	32		/* # receive resource descriptors */
#define RRAMASK	(NRRA-1)	/* why it must be power of two */

#define NRBA	16		/* # receive buffers < NRRA */
#define NRDA	NRBA		/* # receive descriptors */
#define NTDA	4		/* # transmit descriptors */

#define CDASIZE sizeof(struct CDA)
#define RRASIZE (NRRA*sizeof(struct RXrsrc))
#define RDASIZE (NRDA*sizeof(struct RXpkt))
#define TDASIZE (NTDA*sizeof(struct TXpkt))

/* size of FCS (CRC) appended to received packets */
#define FCSSIZE	4		

/* buffer size (enough for 1 max packet 1520 up to cache line boundary) */
#ifdef XDSCONICBUG
#define RBASIZE	(1536*2)
#else
#define RBASIZE	1536
#endif

#ifdef XDS
/*
 * transmit data must be copied into SRAM
 */
#define TBASIZE	RBASIZE
#endif

/* eobc set for only one packet per buffer */
#define EOBC	1520		/* Assumes 32 bit operation */

/* total buffer size needed for all descriptors */
#define SONICBUFSIZE	((RRASIZE+CDASIZE+RDASIZE+TDASIZE)*2 + SONICALIGN - 1)

/*
 * aligned pointers into sonic buffers
 */
static volatile struct RXrsrc	*rra;	/* receiver resource descriptors */
static volatile struct RXpkt	*rda;	/* receiver desriptors */
static volatile struct TXpkt	*tda;	/* transmitter descriptors */
static volatile struct CDA	*cda;	/* CAM descriptors */

/*
 * receive buffers for sonic accessed by SONIC 
 * each buffer will hold one ethernet packet
 */
static char	*rba;
#ifdef XDS
static char	*tba;
#endif


/* Meta transmit descriptors */
static struct mtd {
  struct mtd		*mtd_link;
  volatile struct TXpkt	*mtd_txp;
  struct mbuf		*mtd_mbuf;
#ifdef XDS
  char			*mtd_tba;
#endif
} mtda[NTDA];

static struct mtd *mtdfree;	/* list of free meta transmit descriptors */
static struct mtd *mtdhead;	/* head of descriptors assigned to chip */
static struct mtd *mtdtail;	/* tail of descriptors assigned to chip */
static struct mtd *mtdnext;	/* next descriptor to give to chip */

static struct mtd *mtd_alloc (void);
static void mtd_free (struct mtd *);
static void sntxdone (struct sn_softc *, struct mtd *);

/*
 * eninit(): initialise ethernet
 * check to see if sonic chip is on the machine 
 */
int
eninit ()
{
    int unit = 0;
    struct sn_softc *sn = &sn_softc[unit];
    struct ifnet *ifp = &sn->sn_if;
    volatile struct sn_reg *csr;
    int timeout;
    u_short dcr;

    csr = (struct sn_reg *) PHYS_TO_K1 (SONIC_BASE);
    
    /* reset Sonic chip */
#if defined(XDS)
    *(u_int *) PHYS_TO_K1 (VME_SONIC_RES) = RESET_ACTIVE; wbflush ();
    DELAY(1000);
    *(u_int *) PHYS_TO_K1 (VME_SONIC_RES) = RESET_INACTIVE; wbflush ();
#elif defined(P4000)
    *(u_int *) PHYS_TO_K1 (NET_RESET_) = RESET_ZERO; wbflush();
    DELAY(1000);
    *(u_int *) PHYS_TO_K1 (NET_RESET_) = RESET_ONE; wbflush();
#else
    /* other Algorithmics boards */
    *(u_int *) PHYS_TO_K1 (BCRR) &= ~BCRR_ETH; wbflush();
    DELAY(1000);
    *(u_int *) PHYS_TO_K1 (BCRR) |= BCRR_ETH; wbflush();
#endif
    DELAY(1000);

    if (!(csr->s_cr & CR_RST)) {
	log (LOG_ERR, "sonic: did not reset\n");
	return;
    }

    /* config it */
#if defined(XDS)
    dcr = DCR_ASYNC|DCR_WAIT1|DCR_DW32|DCR_DMABLOCK|DCR_RFT24|DCR_TFT24;
#else
    dcr = DCR_ASYNC|DCR_WAIT0|DCR_DW32|DCR_DMABLOCK|DCR_RFT24|DCR_TFT24;
#endif
    csr->s_dcr = dcr; wbflush();
    if ((csr->s_dcr & ~(DCR_USR1|DCR_USR0)) != dcr) {
	log (LOG_ERR, "sonic: cannot configure\n");
	return;
    }

    csr->s_imr = 0; wbflush();
    sn->sn_rev = csr->s_sr;

    ifp = &sn->sn_if;
    ifp->if_name	= "en";
    ifp->if_unit	= unit;

    if (sbdethaddr (sn->sn_enaddr) < 0)
      return;

    log (LOG_INFO, "%s%d: rev %d, ethernet address: %s\n", 
	 sn->sn_if.if_name, sn->sn_if.if_unit, sn->sn_rev, 
	 ether_sprintf (sn->sn_enaddr));

    /* network management */
    ifp->if_type	= IFT_ETHER;
    ifp->if_addrlen	= 6;
    ifp->if_hdrlen	= 14;
    ifp->if_mtu		= ETHERMTU;
    ifp->if_flags	= IFF_BROADCAST | IFF_SIMPLEX | IFF_NOTRAILERS;
#ifdef DEBUG    
    ifp->if_flags	|= IFF_DEBUG;
#endif

    ifp->if_init	= snifinit;
    ifp->if_output	= ether_output;		
    ifp->if_start	= snifstart;		
    ifp->if_ioctl	= snioctl;
    ifp->if_watchdog	= snwatch;
    ifp->if_reset	= snreset;
    ifp->if_timer	= 0;

    sn->sn_csr = csr;
    if_attach (ifp);
#ifdef PROM
    if_newaddr(ifp, IFT_ETHER, (caddr_t)((struct arpcom *)ifp)->ac_enaddr);
#else
#if defined(P4000) && defined(USEINTS)
    sbd_setvec (CR_HINT0, IRR(0,IRR0_NET), snintr, unit);
#endif
#endif
}


/*
 * snifinit: initialise interface.
 */
static int
snifinit (unit)
    int unit;
{
#ifdef notdef
    /* let SIOCSIFADDR/FLAGS do the job */
    if (unit < NSONIC) {
	register struct sn_softc *sn = &sn_softc[unit];
	int s = splimp ();
	if (snstartup (sn) == 0)
	  sn->sn_if.if_flags |= IFF_UP;
	(void) splx (s);
    }
#endif
}

/*
 * snioctl: process an ioctl request.
 */
static int
snioctl (struct ifnet *ifp, int cmd, caddr_t data)
{
    struct ifaddr *ifa = (struct ifaddr *)data;
    struct ifreq  *ifr = (struct ifreq *)data;
    struct sn_softc *sn = &sn_softc[ifp->if_unit];
    char *bp;
    int error = 0;
    int s = splimp();

    switch (cmd) {

    case SIOCPOLL:
	if (ifp->if_flags & IFF_RUNNING)
	  snintr (ifp->if_unit);
	break;

    case SIOCSIFADDR: 
	ifp->if_flags |= IFF_UP;
	switch (ifa->ifa_addr->sa_family) {
#ifdef INET
	case AF_INET:
	    if (!(error = snstartup (sn))) {
		((struct arpcom *)ifp)->ac_ipaddr = IA_SIN(ifa)->sin_addr;
#ifdef PROM
		if (IA_SIN(ifa)->sin_addr.s_addr != INADDR_ANY &&
		    (IA_SIN(ifa)->sin_addr.s_addr >> IN_CLASSA_NSHIFT)
		    != IN_LOOPBACKNET)
#endif
		    arpwhohas((struct arpcom *)ifp, &IA_SIN(ifa)->sin_addr);
	    }
	    break;
#endif INET
#ifdef NS
	case AF_NS:
	    {
		register struct ns_addr *ina = &(IA_SNS(ifa)->sns_addr);
		
		if (ns_nullhost(*ina))
		  ina->x_host = *(union ns_host *)(ns->ns_addr);
		else {
		    /* force reset of controller for new address */
		    snclosedown (sn);
		    bcopy((caddr_t)ina->x_host.c_host,
			  (caddr_t)ns->ns_addr, sizeof(ns->ns_addr));
		}
		error = snstartup (sn);
		break;
	    }
#endif
	default:
	    error = snstartup (sn);
	    break;
	}
	break;

  case SIOCSIFFLAGS:
	if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) == IFF_UP) 
	  /* UP switched on, but not RUNNING: start up interface */
	  error = snstartup (sn);
	else if ((ifp->if_flags & (IFF_UP|IFF_RUNNING)) == IFF_RUNNING)
	  /* UP switched off, but still RUNNING: close down interface */
	  snclosedown (sn);
	break;

    default: 
	error = EINVAL;
  }
  splx (s);
  return (error);
}


static int
snreset (int unit)
{
    struct sn_softc *sn = &sn_softc[unit];

    if (sn->sn_if.if_flags & IFF_UP)
      snrestart (sn);
}

/*
 * snrestart(): reset and restart the SONIC 
 *
 * Called in case of fatal hardware/software errors.
 */
static int
snrestart (struct sn_softc *sn)
{
    int isup = sn->sn_if.if_flags & IFF_UP;
    int error;

    snclosedown(sn);
    error = snstartup(sn);
    if (error == 0 && isup) {
	/* restart dequeing of tx packets */
	sn->sn_if.if_flags |= IFF_UP;
	snifstart (&sn->sn_if);
    }
    return error;
}


static int
snstartup (struct sn_softc *sn)
{
    volatile struct sn_reg *csr = sn->sn_csr;
    u_short rcr;
    int error, s;
    
    if (!csr)
      /* no such device */
      return ENXIO;

    if (!sn->sn_if.if_addrlist)
      /* no addresses */
      return EINVAL;

    if (sn->sn_if.if_flags & IFF_RUNNING)
      /* already running */
      return (0);
    
    s = splhigh ();

#if 0
    /* unreset it */
#if defined(XDS)
    *(u_int *) PHYS_TO_K1 (VME_SONIC_RES) = RESET_INACTIVE; wbflush ();
#elif defined(P4000)
    *(u_int *) PHYS_TO_K1 (NET_RESET_) = RESET_ONE; wbflush();
#else
    *(u_int *) PHYS_TO_K1 (BCRR) |= BCRR_ETH; wbflush();
#endif
    DELAY(1000);
#endif

    if (!(csr->s_cr & CR_RST)) {
	error = EIO;
	goto bad;
    }

    log (LOG_INFO, "%s%d: starting interface\n",
	 sn->sn_if.if_name, sn->sn_if.if_unit);

    /* config it */
#if defined(XDS)
    csr->s_dcr = 
	DCR_ASYNC|DCR_WAIT1|DCR_DW32|DCR_DMABLOCK|DCR_RFT24|DCR_TFT16;
#elif defined(P4000)
    csr->s_dcr = 
      DCR_EXBUS|DCR_LBR|
	DCR_ASYNC|DCR_WAIT0|DCR_DW32|DCR_DMABLOCK|DCR_RFT24|DCR_TFT16;
#else
    csr->s_dcr = 
	DCR_ASYNC|DCR_WAIT0|DCR_DW32|DCR_DMABLOCK|DCR_RFT24|DCR_TFT16;
#endif
    csr->s_dcr2 = 0;
    wbflush ();

    rcr = RCR_BRD|RCR_LBNONE;
    if (sn->sn_if.if_flags & IFF_PROMISC)
      rcr |= RCR_PRO;
    csr->s_rcr = rcr;

    /* set interrupt mask */
    csr->s_imr =
      IMR_BREN | IMR_PTXEN | IMR_TXEREN | IMR_HBLEN |
	IMR_PRXEN | IMR_RDEEN | IMR_RBAEEN | IMR_RFOEN |
	  IMR_CRCEN | IMR_FAEEN | IMR_MPEN;
    
    /* clear pending interrupts */
    csr->s_isr = ~0;
    
    /* clear tally counters */
    csr->s_crct = ~0;
    csr->s_faet = ~0;
    csr->s_mpt = ~0;
    
    /* initialise memory descriptors */
    if (rba) {
	bzero (rba, NRBA * RBASIZE);
	if (IS_K0SEG (rba))
	  clean_dcache (rba, NRBA * RBASIZE);
    } else {
	error = allocatebuffers ();
	if (error)
	  goto bad;
    }

    caminitialise ();
    initialise_tda (sn);
    initialise_rda (sn);
    initialise_rra (sn);
    
    /* enable (unreset) the chip */
    csr->s_cr = 0; wbflush();

    /* program the cam with our address (m/c addresses already present) */
    camentry (0, sn->sn_enaddr);
    if (!camprogram (sn))
      goto bad;
    
    /* get it to read initial resource descriptors */
    csr->s_cr = CR_RRRA; wbflush ();
    while (csr->s_cr & CR_RRRA)
      DELAY(100);

    /* enable receiver */
    csr->s_cr = CR_RXEN; wbflush ();
    
    /* flag interface as "running" */
    sn->sn_if.if_flags |= IFF_RUNNING;
    sn->sn_retries = 0;
    sn->sn_rbe = 0;

    splx(s);
    return 0;

 bad:
    snclosedown (sn);
    return error;
}

/*
 * snclosedown(): close down an interface and free its buffers
 * Called on final close of device, or if snstartup() fails
 * part way through.
 */
static int
snclosedown (struct sn_softc *sn)
{
    register volatile struct sn_reg *csr = sn->sn_csr;
    int s = splhigh();

    sn->sn_if.if_flags &= ~(IFF_RUNNING | IFF_UP | IFF_OACTIVE);
    sn->sn_if.if_timer = 0;

    log (LOG_INFO, "%s%d: stopping interface\n",
	 sn->sn_if.if_name, sn->sn_if.if_unit);

    /* update tally counters */
    sn->sn_crct += csr->s_crct;
    sn->sn_faet += csr->s_faet;
    sn->sn_mpt += csr->s_mpt;

    /* forcibly shut the chip up */
    csr->s_cr = CR_RST; wbflush();
    DELAY(1000);
    
    /* free all receive buffers (currently static so nowt to do) */
    
    /* free all transmit mbufs still pending */
    {
	struct mtd *mtd;
	for (mtd = mtdhead; mtd; mtd = mtd->mtd_link) {
	    if (mtd->mtd_mbuf) {
		m_freem (mtd->mtd_mbuf);
		mtd->mtd_mbuf = 0;
	    }
	}
    }
    
    splx(s);
    return 0;
}


#ifdef PROM
static int
sncheckclient (struct sn_softc *sn)
{
    register volatile struct sn_reg *csr = sn->sn_csr;

    if (csr->s_urra != UPPER (rra) || csr->s_rsa != LOWER (rra)) {
	/* client has reprogrammed sonic, we must now ignore it */
	sn->sn_if.if_flags &= ~IFF_RUNNING;
	log (LOG_DEBUG, "%s%d: reprogrammed by client, PMON ignoring\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit);
	return 0;
    }

    return 1;
}
#endif


static int
snifstart (struct ifnet *ifp)
{
    register struct sn_softc *sn = &sn_softc[ifp->if_unit];
    int s;

#ifdef PROM
    if (!sncheckclient (sn))
	return ENETDOWN;
#endif

    /*
     * start feeding any queued packets to chip
     */
    s = splimp ();

    for (;;) {
	struct mbuf *m;
	
	IF_DEQUEUE (&ifp->if_snd, m);
	if (m == 0)			/* nothing left to send */
	  break;
	
	if (!snput (sn, m)) {
	    /* not enough space */
	    IF_PREPEND (&ifp->if_snd, m);
	    ifp->if_flags |= IFF_OACTIVE;
	    ifp->if_oerrors++;
	    break;
	}
    }
    (void) splx (s);
    return 0;
}

/*
 * stuff packet into sonic (at splimp)
 */
#if defined(XDS)

static int
snput (struct sn_softc *sn, struct mbuf *m0)
{
    volatile struct sn_reg *csr = sn->sn_csr;
    register volatile union frag *fr;
    register struct mtd *mtdnew;
    register volatile struct TXpkt *txp;
    register struct mbuf *m;
    int len, i;
    unsigned char *ptba;
    unsigned int pa;

    if (!csr) 
      return 0;

    /* grab the replacement mtd */
    if ((mtdnew = mtd_alloc()) == 0)
      return 0;
    
    /* this packet goes in mtdnext */
    txp = mtdnext->mtd_txp;
    txp->config = 0;
    txp->status = 0;

#ifdef MIPSEL
    /* Byte-swap the mbuf chain */
    m0 = snswapm (m0);
#endif

    /* copy the data into the static RAM buffer */
    fr = &txp->frags[0];
    ptba = mtdnext->mtd_tba;
    /* ASSERT (IS_K0SEG (ptba)); */
    len = 0;
    for (m = m0; m; m = m->m_next) {
	bcopy (mtod (m, char *), ptba, m->m_len);
	ptba += m->m_len;
	len += m->m_len;
    }

    /* pad out last fragment for minimum size */
    if (len < ETHERMIN + sizeof(struct ether_header))
	len = ETHERMIN + sizeof(struct ether_header);

#ifdef XDSSONICBUG
    xds_expand_buffer (mtdnext->mtf_tba, len);
#else
#if defined(R4000)
    /* flush data to static RAM */
    clean_dcache(mtdnext->mtd_tba, len);
#endif
#endif

    pa = K0_TO_PHYS(mtdnext->mtd_tba);
    txp->frag_count = 1;
    txp->pkt_size = len;
    fr->frag_ptrlo = pa;
    fr->frag_ptrhi = pa >> 16;
    fr->frag_size = len;

    /* link onto the next mtd that will be used */
    fr->tlink = LOWER(mtdnew->mtd_txp) | EOL;
    
    if (!mtdhead) {
	/* no current transmit list, so start with this one */
	mtdhead = mtdnext;
    } else {
	/* have an existing transmit list, so append it to end of list
	 * note mtdnext is already physically linked to mtdtail in
	 * mtdtail->mtd_txp->frags[mtdtail->mtd_txp->frag_count].tlink
	 */
	mtdtail->mtd_txp->frags[mtdtail->mtd_txp->frag_count].tlink &= ~EOL;
	wbflush ();
    }

    /* kick transmitter */
    csr->s_cr = CR_TXP; wbflush ();

    /* update s/w linkage */
    mtdnext->mtd_mbuf = m0;	/* to be freed on txint */
    mtdnext->mtd_link = mtdnew;
    mtdtail = mtdnext;
    mtdnext = mtdnew;
    
    sn->sn_if.if_timer = 5;	/* 5 second watchdog */
    return 1;
}

#else

static int
snput (struct sn_softc *sn, struct mbuf *m0)
{
    volatile struct sn_reg *csr = sn->sn_csr;
    register volatile union frag *fr;
    register struct mtd *mtdnew;
    register volatile struct TXpkt *txp;
    register struct mbuf *m;
    int firstattempt = 1;
    int len, nfr, i, tinyfrags, needcompress;
    
    if (!csr) 
      return 0;

    /* grab the replacement mtd */
    if ((mtdnew = mtd_alloc()) == 0)
      return 0;
    
    /* this packet goes in mtdnext */
    txp = mtdnext->mtd_txp;
    txp->config = 0;
    txp->status = 0;

 retry:
    /* see whether it will fit in the TDA */
    tinyfrags = needcompress = len = nfr = 0;
    fr = &txp->frags[0];
    for (m = m0; m; m = m->m_next) {
	unsigned va = mtod (m, unsigned);
	int resid = m->m_len;
	
	len += resid;
	while (resid) {
	    unsigned n = resid;
#ifndef PROM
	    if (IS_KSEG2 (va)) {
		/* this could cross a page boundary */
		n = NBPC - (va & (NBPC - 1));
		if (n > resid)
		  n = resid;
	    }
#endif

	    /* heuristic to prevent transmit underflow */
	    if (n < TINYFRAG) {
		if (++tinyfrags > MAXTINIES && tinyfrags > needcompress)
		  needcompress = tinyfrags; /* remember maximum */
	    }
	    else 
	      tinyfrags = 0;

	    nfr++; va += n; resid -= n;
	}
    }

    if (needcompress != 0 || nfr > FRAGMAX) {
	if (firstattempt) {
	    /* try to compress chain */
	    m0 = sn_fillup (m0, ETHERMTU / FRAGMAX + 1);
	    firstattempt = 0;
	    goto retry;
	}
	mtd_free (mtdnew);
	m_freem (m0);
	log (LOG_WARNING, "%s%d: frag compress failed (%d frags, %d tiny)\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, nfr, needcompress);
	sn->sn_if.if_oerrors++;
	return 1;
    }

#ifdef MIPSEL
    /* Byte-swap the mbuf chain */
    m0 = snswapm (m0);
#endif

#ifdef R4000
    /* hand mbuf data over to Sonic */
    if (IS_K0SEG (m0))
      for (m = m0; m; m = m->m_next)
	if (m->m_len)
	  clean_dcache (mtod(m, caddr_t), m->m_len);
#endif

    /* Now fill up the txd fragment list */
    fr = &txp->frags[0];
    for (m = m0; m; m = m->m_next) {
	unsigned va = mtod (m, unsigned);
	int resid = m->m_len;
	
	while (resid) {
	    u_long pa;
	    unsigned n = resid;

#ifndef PROM
	    pa = kvtophys (va);
	    if (IS_KSEG2 (va)) {
		/* this could cross a page boundary */
		n = NBPC - (pa & (NBPC - 1));
		if (n > resid)
		  n = resid;
	    }
#else
	    pa = K1_TO_PHYS (va);
#endif

	    fr->frag_ptrlo = pa;
	    fr->frag_ptrhi = pa >> 16;
	    fr->frag_size = n;
	    fr++;

	    va += n; resid -= n;
	}
    }

    /* pad out last fragment for minimum size */
    if (len < ETHERMIN + sizeof(struct ether_header)) {
	int pad = (ETHERMIN + sizeof(struct ether_header)) - len;
	(fr-1)->frag_size += pad;
	len = ETHERMIN + sizeof(struct ether_header);
    }

    txp->frag_count = nfr;
    txp->pkt_size = len;

    /* link onto the next mtd that will be used */
    fr->tlink = LOWER(mtdnew->mtd_txp) | EOL;
    
    if (!mtdhead) {
	/* no current transmit list, so start with this one */
	mtdhead = mtdnext;
    } else {
	/* have an existing transmit list, so append it to end of list
	 * note mtdnext is already physically linked to mtdtail in
	 * mtdtail->mtd_txp->frags[mtdtail->mtd_txp->frag_count].tlink
	 */
	mtdtail->mtd_txp->frags[mtdtail->mtd_txp->frag_count].tlink &= ~EOL;
	wbflush ();
    }

    /* kick transmitter */
    csr->s_cr = CR_TXP; wbflush ();

    /* update s/w linkage */
    mtdnext->mtd_mbuf = m0;	/* to be freed on txint */
    mtdnext->mtd_link = mtdnew;
    mtdtail = mtdnext;
    mtdnext = mtdnew;
    
    sn->sn_if.if_timer = 5;	/* 5 second watchdog */
    return 1;
}
#endif

void
snintr (int unit)
{
    struct sn_softc *sn = &sn_softc[unit];
    volatile struct sn_reg *csr = sn->sn_csr;
    register u_int isr;

    if (unit >= NSONIC || !csr)
      return;
    
#ifdef PROM
    /* Timer may be used by programmers, ignore TC interrupt */
    while (isr = (csr->s_isr & ~ISR_TC)) {
#else
    while (isr = csr->s_isr) {
#endif

#ifdef PROM
	if (!sncheckclient (sn))
	    return;
#endif

	/* scrub the interrupts that we are going to service */
	csr->s_isr = isr & ~ISR_RBE; wbflush ();

	if (isr & ISR_BR) {
	    log (LOG_WARNING, "%s%d: bus error\n",
		 sn->sn_if.if_name, sn->sn_if.if_unit);
	    snrestart (sn);
	    break;
	}

	if (isr & (ISR_LCD|ISR_PINT|ISR_TC))
	  log (LOG_WARNING, "%s%d: unexpected interrupt status 0x%x\n",
	       sn->sn_if.if_name, sn->sn_if.if_unit, isr);
	
	if (isr & (ISR_TXDN|ISR_TXER))
	  sntxint (sn);
	
	if (isr & ISR_PKTRX)
	  snrxint (sn);

	if (isr & ISR_ERRS) {
	    if (snerrint (sn, isr))
	      break;
	} else {
	    sn->sn_rbe = 0;
	}
    }
}


/*
 * Error interrupt routine
 */
static int
snerrint (struct sn_softc *sn, u_int isr)
{
    if (isr & ISR_RBE) {
	sn->sn_if.if_ierrors++;
	sn->sn_sum.ss_rbuff++;
	log (LOG_INFO, "%s%d: receive buffer exhausted, isr=0x%x\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, isr);
	if (!sn->sn_rbe || (isr & ISR_PKTRX)) {
	    /* probably managed to free some rx space, try again */
	    sn->sn_csr->s_isr = ISR_RBE; wbflush ();
	    sn->sn_rbe = 1;
	} else if (sn->sn_rbe) {
	    log (LOG_INFO, "%s%d: and no rx data to free\n",
		 sn->sn_if.if_name, sn->sn_if.if_unit);
	    snrestart (sn);
	    return (1);
	}
    }

    if (isr & ISR_RDE) {
	sn->sn_if.if_ierrors++;
	sn->sn_sum.ss_rbuff++;
	log (LOG_INFO, "%s%d: rx descriptors exhausted, isr=0x%x\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, isr);
    }
    else if (isr & ISR_RBAE) {
	sn->sn_if.if_ierrors++;
	sn->sn_sum.ss_rbuff++;
	log (LOG_INFO, "%s%d: rx buffer area exceeded, isr=0x%x\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, isr);
    }
    else if (isr & ISR_RFO) {
	sn->sn_if.if_ierrors++;
	sn->sn_sum.ss_roflo++;
	log (LOG_INFO, "%s%d: rx fifo overflow, isr=0x%x\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, isr);
    }

    if (isr & ISR_HBL)
      sn->sn_sum.ss_noheart++;
    
    if (isr & ISR_CRC)
      sn->sn_crct += 0x10000;
    
    if (isr & ISR_FAE)
      sn->sn_faet += 0x10000;
    
    if (isr & ISR_MP)
      sn->sn_mpt += 0x10000;

    return (0);
}

/*
 * Transmit interrupt routine
 */
static void
sntxint (struct sn_softc *sn)
{
    struct mtd *mtd;
    register volatile struct TXpkt *txp;
    register u_int status, ncol;

    while (mtd = mtdhead) {

	txp = mtd->mtd_txp;
	if ((status = txp->status) == 0) {	
	    /* this packet isn't sent yet */
	    break;
	}
	txp->status = 0;

#ifdef DEBUG
	if (T_txp) {
	    struct ether_header *eh=mtod(mtd->mtd_mbuf,struct ether_header *);
	    printf ("xmit status=0x%x len=%d type=0x%x from %s",
			status, txp->pkt_size, 
			ntohs(eh->ether_type), ether_sprintf(eh->ether_shost));
	    printf(" (to %s)\n", ether_sprintf (eh->ether_dhost));
	}
#endif

	if (status & TSR_PTX) {
	    /* tx packet has flown, free the packet mbufs */
	    sntxdone (sn, mtd);

	    /* update statistics for this packet */
	    sn->sn_sum.ss_tpacks++;
	    sn->sn_if.if_opackets++;
	    if (status & TSR_DEF) {
		sn->sn_sum.ss_tdef++;
	    }
	    if (status & TSR_OWC) {
		sn->sn_sum.ss_tlcol++;
		sn->sn_if.if_oerrors++;
		log (LOG_NOTICE, "%s%d: late collision (noisy network?)\n", 
		     sn->sn_if.if_name, sn->sn_if.if_unit);
	    }
	    if (status & (TSR_NCRS|TSR_CRSL)) {
		sn->sn_sum.ss_tlcar++;
		sn->sn_if.if_oerrors++;
		log (LOG_NOTICE, "%s%d: no carrier (check cables?)\n", 
		     sn->sn_if.if_name, sn->sn_if.if_unit);
	    }
	    if (ncol = (status & TSR_NC) >> TSR_NCSHFT) {
		sn->sn_if.if_collisions += ncol;
		if (ncol == 1)
		  sn->sn_sum.ss_tone++;
		else
		  sn->sn_sum.ss_tmore++;
	    }
	}
	else {
	    /* tx failed: CTDA still points at failing packet */
	    sn->sn_if.if_oerrors++;
	    if (status & (TSR_EXD|TSR_EXC)) {
		sn->sn_sum.ss_cerr++;
		log (LOG_NOTICE, "%s%d: excessive %s (network jammed?)\n", 
		     sn->sn_if.if_name, sn->sn_if.if_unit,
		     (status & TSR_EXD) ? "deferrals" : "collisions");
		status &= ~TSR_BCM; /* ignore bcm error */
	    }
	    if (status & TSR_FU) {
		sn->sn_sum.ss_tuflo++;
		log (LOG_INFO, "%s%d: tx fifo underflow\n",
		     sn->sn_if.if_name, sn->sn_if.if_unit);
		if (sn->sn_if.if_flags & IFF_DEBUG)
		  sntxdump (txp);
	    }
	    if (status & TSR_BCM) {
		int fr, frag_total;
		sn->sn_sum.ss_tbuff++;
		for (fr = 0, frag_total = 0; fr < txp->frag_count; fr++)
		  frag_total += txp->frags[fr].frag_size;
		if ((sn->sn_if.if_flags & IFF_DEBUG) ||
		    frag_total != txp->pkt_size) {
		    log (LOG_WARNING, "%s%d: tx byte count mismatch, psz=%d, %d in %d frags\n",
			     sn->sn_if.if_name, sn->sn_if.if_unit,
			     txp->pkt_size, frag_total, txp->frag_count);
		    sntxdump (txp);
		}
		/* pointless attempt to fix error! */
		txp->pkt_size = frag_total;
	    }

	    if (sn->sn_retries++ >= SNRETRIES) {
		/* give up on this packet */
		log (LOG_INFO, "%s%d: tx retry count exceeded\n",
		     sn->sn_if.if_name, sn->sn_if.if_unit);
		sntxdone (sn, mtd);
		/* manually advance to next packet:
		 * strictly we only need to set CTDA here, but we are
		 * paranoid and reset it for all errors, below. */
	    }

	    if (mtdhead) {
		/* restart transmission */
		register volatile struct sn_reg *csr = sn->sn_csr;
		csr->s_ctda = LOWER(mtdhead->mtd_txp); wbflush();
		csr->s_cr = CR_TXP; wbflush();
		break;
	    }
	}
    }

    /* dequeue some new packets, if possible */
    if (!(sn->sn_if.if_flags & IFF_OACTIVE))
      snifstart (&sn->sn_if);
}


static void
sntxdone (struct sn_softc *sn, struct mtd *mtd)
{
    sn->sn_if.if_flags &= ~IFF_OACTIVE;
    sn->sn_retries = 0;

    if (mtd->mtd_mbuf) {
	m_freem (mtd->mtd_mbuf);
	mtd->mtd_mbuf = 0;
    }
    mtdhead = mtd->mtd_link;
    if (mtdhead == mtdnext)
      mtdhead = 0;
    mtd_free (mtd);
}


static void
sntxdump (volatile struct TXpkt *txp)
{
    int fr;
    printf ("     frags:");
    for (fr = 0; fr < txp->frag_count; fr++)
      printf (" %d", txp->frags[fr].frag_size);
    printf ("\n");
}

/*
 * Receive interrupt routine
 */
static void
snrxint (struct sn_softc *sn)
{
    volatile struct sn_reg *csr = sn->sn_csr;
    volatile struct RXpkt *rxp;

    rxp = &rda[sn->sn_rxmark];
    while (rxp->in_use == 0) {

	if ((rxp->status & RCR_LPKT) == 0)
	  log (LOG_WARNING, "%s%d: more than one packet in RBA\n",
	       sn->sn_if.if_name, sn->sn_if.if_unit);
	
#ifdef DEBUG
	if (SEQNO_PKT (rxp->seqno) != 0)
	  log (LOG_WARNING, "%s%d: bad psn sequence no. %d\n",
	       sn->sn_if.if_name, sn->sn_if.if_unit, SEQNO_PKT (rxp->seqno));
#endif
	
	if (rxp->status & RCR_PRX) {
	    if (snread (sn, rxp)) {
		sn->sn_if.if_ipackets++;
		sn->sn_sum.ss_rpacks++;
	    }
	} else {
	    log (LOG_INFO, "%s%d: rx packet error 0x%x\n", rxp->status);
	    sn->sn_if.if_ierrors++;
	}

	/* give receive buffer area back to chip 
	 * XXX what buffer did the sonic use for this descriptor
	 * answer look at the rba sequence number !!
	 */
	{
	    int orra = SEQNO_RBA (rxp->seqno) & RRAMASK;
	    volatile struct RXrsrc *orr = &rra[orra];
	    
#ifdef DEBUG
	    if (rxp->pkt_ptrhi != orr->buff_ptrhi ||
		rxp->pkt_ptrlo != orr->buff_ptrlo ||
		orr->buff_wclo == 0)
	      log (LOG_WARNING, "%s%d: bad rx pkt pointers\n",
		   sn->sn_if.if_name, sn->sn_if.if_unit);
#endif
	    
	    /* orra is now empty of packets and can be freed
	     * (if snread didnt copy it out, but instead passed
	     * pointers around then we would have to wait for
	     * higher levels to free it up)
	     *
	     * (dont bother add it back in again straight away)
	     */

	    if (IS_K0SEG (rba))
		/* hand data buffer over to Sonic */
		clean_dcache (PHYS_TO_K0 ((orr->buff_ptrhi << 16) |
					  orr->buff_ptrlo), RBASIZE);

	    rra[sn->sn_rramark] = *orr;

	    /* zap old rra */
	    orr->buff_wchi = orr->buff_wclo = 0;
	    
	    sn->sn_rramark = (sn->sn_rramark + 1) & RRAMASK;
	    csr->s_rwp = LOWER(&rra[sn->sn_rramark]); wbflush ();
	}

	/*
	 * give receive descriptor back to chip
	 * simple list is circular 
	 */
	rxp->in_use = ~0;
	rxp->rlink |= EOL;
	sn->sn_lrxp->rlink &= ~EOL;
	sn->sn_lrxp = rxp;
	
	if (++sn->sn_rxmark >= NRDA)
	  sn->sn_rxmark = 0;
	rxp = &rda[sn->sn_rxmark];
    }
}

/*
 * snread -- pull packet off interface and forward to appropriate
 * protocol handler
 */
static int
snread (struct sn_softc *sn, volatile struct RXpkt *rxp)
{
    struct ifqueue *inq;
    extern char *ether_sprintf();
    struct ether_header *eh;
    caddr_t addr;
    struct mbuf *m;
    int len, i; 
    int toff, tlen;

    /*
     * Get input data length.
     * Get pointer to ethernet header (in input buffer).
     * Deal with trailer protocol: if type is PUP trailer
     * get true type from first 16-bit word past data.
     * Remember that type was trailer by setting off.
     */
    len = rxp->byte_count - FCSSIZE;
    if (IS_K0SEG (rba))
	addr = (caddr_t) PHYS_TO_K0 ((rxp->pkt_ptrhi << 16) | rxp->pkt_ptrlo);
    else
	addr = (caddr_t) PHYS_TO_K1 ((rxp->pkt_ptrhi << 16) | rxp->pkt_ptrlo);

#ifdef MIPSEL
    snswapb (addr, addr, len);
#endif

#ifdef XDSSONICBUG
    xds_compress_buffer (addr, len);
#endif

    eh = (struct ether_header  *)addr;
    eh->ether_type = ntohs((u_short)eh->ether_type);

    addr += sizeof (struct ether_header);
    len -= sizeof (struct ether_header);

    if (len < ETHERMIN || len > ETHERMTU) {
	log (LOG_WARNING, "%s%d: bad packet length received: %d bytes\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit, len);
	return 0;
    }
    
    if (eh->ether_type >= ETHERTYPE_TRAIL &&
	eh->ether_type < ETHERTYPE_TRAIL+ETHERTYPE_NTRAILER) {
	toff = (eh->ether_type - ETHERTYPE_TRAIL) * 512;
	if (toff >= ETHERMTU) {
	    log (LOG_WARNING, "%s%d: trailer offset %d >= ETHERMTU\n",
		 sn->sn_if.if_name, sn->sn_if.if_unit, toff);
	    return 0;
	}

	eh->ether_type = ntohs(*(u_short *)(addr + toff));
	tlen = ntohs(*(u_short *)(addr + toff + sizeof(u_short)));
	if (toff + tlen > len) {
	    log (LOG_WARNING, "%s%d: bad trailer toff=%d tlen=%d plen=%d\n",
		 sn->sn_if.if_name, sn->sn_if.if_unit, toff, tlen, len);
	    return 0;
	}
	len = toff;		/* sizeof data only */
	toff += 2*sizeof(u_short);
	tlen -= 2*sizeof(u_short);
    }
    else {
	toff = tlen = 0;
    }
    
    if (T_rxp) {
	printf("rcvd 0x%x status=0x%x len=%d type=0x%x from %s",
	       addr, rxp->status, len, eh->ether_type,
	       ether_sprintf (eh->ether_shost));
	printf(" (to %s)\n", ether_sprintf (eh->ether_dhost));
    }

    /*
     * Pull packet off interface.  Off is nonzero if packet
     * has trailing header; sonic_get will then force this header
     * information to be at the front, but we still have to drop
     * the type and length which are at the front of any trailer data.
     */
    m = snget (sn, addr, len, toff, tlen);
    if (!m)
      return 0;

    ether_input (&sn->sn_if, eh, m);
    return 1;
}

/*
 * munge the received packet into an mbuf chain
 * because we are using stupid buffer management this 
 * is slow.
 */
static struct mbuf *
snget (struct sn_softc *sn, caddr_t addr, int dlen, int toff, int tlen)
{
    struct mbuf *top = 0;
    struct mbuf **mp = &top;
    register struct mbuf *m;
    register int len;
    caddr_t sp;

    if (dlen + tlen == 0)
      return 0;

    MGETHDR (m, M_DONTWAIT, MT_DATA);
    if (!m)
      return 0;

    m->m_pkthdr.rcvif = &sn->sn_if;
    m->m_pkthdr.len = dlen + tlen;
    m->m_len = MHLEN;

    sp = addr + toff;
    len = tlen;
    while (1) {
	if (len == 0) {
	    if (dlen == 0)
	      return top;	/* all done */
	    sp = addr;
	    len = dlen;
	    dlen = 0;
	}

	if (top) {
	    /* get next mbuf */
	    MGET (m, M_DONTWAIT, MT_DATA);
	    if (!m) {
		m_freem (top);
		return 0;
	    }
	    m->m_len = MLEN;
	}

	if (len >= MINCLSIZE) {
	    MCLGET (m, M_DONTWAIT);
	    if (m->m_flags & M_EXT)
	      m->m_len = MCLBYTES;
	} 
	else if (top == 0 && len + max_linkhdr <= m->m_len) {
	    /* place initial small packet/header at end of mbuf */
	    m->m_data += max_linkhdr;
	}
		
	if (m->m_len > len)
	  m->m_len = len;
	bcopy (sp, mtod(m, caddr_t), m->m_len);
	sp += m->m_len;
	len -= m->m_len;
	*mp = m;
	mp = &m->m_next;
    }
}

static void
mtd_free (struct mtd *mtd)
{
  mtd->mtd_mbuf = (struct mbuf *)0;
  mtd->mtd_link = mtdfree;
  mtdfree = mtd;
}

static struct mtd *
mtd_alloc ()
{
  struct mtd *mtd;
  mtd = mtdfree;
  if (mtd) {
    mtdfree = mtd->mtd_link;
    mtd->mtd_link = 0;
  }
  return (mtd);
}

/*
 * CAM support
 */
static void
caminitialise ()
{
  int i;

  bzero ((char *)cda, CDASIZE);
  for (i = 0; i < MAXCAM; i++)
    cda->desc[i].cam_ep = i;
  cda->enable = 0;
}

static void
camentry (int entry, u_char *ea)
{
  cda->desc[entry].cam_ep = entry;
  cda->desc[entry].cam_ap2 = (ea[5]<<8) | ea[4];
  cda->desc[entry].cam_ap1 = (ea[3]<<8) | ea[2];
  cda->desc[entry].cam_ap0 = (ea[1]<<8) | ea[0];
  cda->enable |= (1 << entry);
}

static int
camprogram(struct sn_softc *sn)
{
  volatile struct sn_reg *csr;
  int timeout;
  int i;

  csr = sn->sn_csr;
  csr->s_cdp = LOWER(cda);
  csr->s_cdc = MAXCAM; wbflush ();
  csr->s_cr = CR_LCAM; wbflush ();

  timeout = 1000;
  while ((csr->s_cr & CR_LCAM) && --timeout > 0)
    DELAY (100);		/* let it get at the bus */

  if (timeout <= 0) {
      log (LOG_ERR, "sonic: CAM init failed\n");
      return 0;
  }

  timeout = 1000;
  while ((csr->s_isr & ISR_LCD) == 0 && --timeout > 0)
    DELAY (100);
  csr->s_isr = ISR_LCD; wbflush();

  if (timeout <= 0) {
      log (LOG_ERR, "sonic: CAM init didn't interrupt\n");
      return 0;
  }

  return 1;
}

#ifdef notdef
static void
camdump()
{
  printf ("CAM entries:\n");
  csr->s_cr = CR_RST;
  wbflush ();
   
  for (i = 0; i < 16; i++) {
    u_short ap2, ap1, ap0;
    csr->s_cep = i;
    wbflush ();
    ap2 = csr->s_cap2;
    ap1 = csr->s_cap1;
    ap0 = csr->s_cap0;
    printf ("%d: ap2=0x%x ap1=0x%x ap0=0x%x\n", i, ap2, ap1, ap0);
  }
  printf ("CAM enable 0x%x\n", csr->s_cep);
   
  csr->s_cr = 0;
  wbflush ();
}
#endif

/*
 * because the sonic is basically a 16 bit device it 'concatenates'
 * a higher buffer address to a 16 bit offset this can cause wrap
 * around problems near 64k boundaries !!
 */
static int
allocatebuffers ()
{
    void *buf;
    u_long p, lo, hi;

#if defined(XDS)
    rba = (char *)PHYS_TO_K0(SRAM_NET);
    tba = rba + NRBA * RBASIZE;
    buf = tba + NTDA * TBASIZE;
#else
    buf = malloc (SONICBUFSIZE, M_DEVBUF, 0);
    if (!buf)
      return (ENOBUFS);

    rba = (char *) malloc (NRBA * RBASIZE, M_DEVBUF, 0);
    if (!rba) {
	free (buf, M_DEVBUF);
	return (ENOBUFS);
    }
#endif

#ifdef R4000
    /* flush dirty data first */
    if (IS_K0SEG (buf)) {
	clean_dcache (buf, SONICBUFSIZE);
	clean_dcache (rba, NRBA * RBASIZE);
#ifdef XDS
	clean_dcache (tba, NTDA * TBASIZE);
#endif
    }
#endif

    /* force into kseg1, and align correctly */
    p = ((u_long) K0_TO_K1 (buf) + SONICALIGN - 1) & ~(SONICALIGN - 1);

    /* RRA and CDA must fit inside 64k region */
    if ((p ^ (p+RRASIZE+CDASIZE)) & ~0xffff)
      p = (p+0x10000) & ~0xffff;
    rra = (struct RXrsrc *)p; p += RRASIZE;
    cda = (struct CDA *)p; p += CDASIZE;
    
    /* RDA must fit inside 64k region */
    if ((p ^ (p+RDASIZE)) & ~0xffff)
      p = (p+0x10000) & ~0xffff;
    rda = (struct RXpkt *)p; p += RDASIZE;
    
    /* TDA must fit inside 64k region */
    if ((p ^ (p+TDASIZE)) & ~0xffff)
      p = (p+0x10000) & ~0xffff;
    tda = (struct TXpkt *)p; p += TDASIZE;
    
    /* check sanity of buffer addresese */
    lo = (u_long) K0_TO_K1 (buf); 
    hi = lo + SONICBUFSIZE;

    if ((unsigned)rra < lo || (unsigned)rra >= hi ||
	(unsigned)cda < lo || (unsigned)cda >= hi ||
	(unsigned)rda < lo || (unsigned)rda >= hi ||
	(unsigned)tda < lo || (unsigned)tda >= hi) {
	log (LOG_ERR, "sonic descriptors out of range\n");
	return ENOMEM;
    }

    /* return errno */
    return 0;
}


static void
initialise_tda (struct sn_softc *sn)
{
  volatile struct sn_reg *csr = sn->sn_csr;
  struct mtd *mtd;
  int i;

  bzero ((char *)tda, TDASIZE);

  mtdfree = mtdhead = mtdtail = (struct mtd *)0;
  for (i = 0; i < NTDA; i++) {
    mtd = &mtda[i];
    mtd->mtd_txp = &tda[i];
    mtd_free(mtd);
#ifdef XDS
    mtd->mtd_tba = tba + i*TBASIZE;
#endif   
  }
  mtdnext = mtd_alloc ();
  
  csr->s_utda = UPPER(tda); wbflush();
  csr->s_ctda = LOWER(mtdnext->mtd_txp); wbflush();
}

static void
initialise_rda (struct sn_softc *sn)
{
  volatile struct sn_reg *csr = sn->sn_csr;
  int i;
    
  /* link the RDA's together into a circular list
   */
  bzero ((char *)rda, RDASIZE);
  for (i = 0; i < (NRDA-1); i++) {
    rda[i].rlink = LOWER(&rda[i+1]);
    rda[i].in_use = ~0;
  }
  rda[NRDA-1].in_use = ~0;
  rda[NRDA-1].rlink =  LOWER(&rda[0]) | EOL;

  /* mark end of receive descriptor list */
  sn->sn_lrxp = &rda[NRDA-1];
  sn->sn_rxmark = 0;

  csr->s_urda = UPPER(rda);
  csr->s_crda = LOWER(rda);
  wbflush();
}

static void
initialise_rra (struct sn_softc *sn)
{
  volatile struct sn_reg *csr = sn->sn_csr;
  char *rb;
  int i;

  bzero ((char *)rra, RRASIZE);
  csr->s_eobc = EOBC / 2;
  csr->s_urra = UPPER(rra);
  csr->s_rsa = LOWER(rra);
  csr->s_rea = LOWER(&rra[NRRA]);
    
  /*
   * fill up SOME of the rra with buffers 
   */
  for (i = 0, rb = rba; i < NRBA; i++, rb += RBASIZE) {
    rra[i].buff_ptrhi = UPPER(rb);
    rra[i].buff_ptrlo = LOWER(rb);
#ifdef XDSSONICBUG
    rra[i].buff_wchi = (RBASIZE / 4) >> 16;
    rra[i].buff_wclo = (RBASIZE / 4);
#else
    rra[i].buff_wchi = (RBASIZE / 2) >> 16;
    rra[i].buff_wclo = (RBASIZE / 2);
#endif
  }
  sn->sn_rramark = NRBA;
  csr->s_rrp = LOWER(rra);
  csr->s_rwp = LOWER(&rra[sn->sn_rramark]);
  wbflush();
}

/*
 * snwatch(): interface watchdog timer
 * 
 * Called if any Tx packets remain unsent after 5 seconds,
 * In all cases we just reset the chip, and any retransmission 
 * will be handled by higher level protocol timeouts.
 */
static int
snwatch (int unit)
{
    struct sn_softc *sn = &sn_softc[unit];
    int s = splimp();
    
    if (mtdhead && mtdhead != mtdnext) {
	/* something still pending for transmit */
	log (LOG_WARNING, "%s%d: Tx - %s\n",
	     sn->sn_if.if_name, sn->sn_if.if_unit,
	     (mtdhead->mtd_txp->status == 0) ? "timeout" : "lost interrupt");
	snrestart (sn);
    }
    (void) splx (s);
    return 0;
}

/*
 * sn_pullup(): reorganise part of an mbuf chain to satisfy SONIC.
 * 
 * It doesn't free mbufs that reduce to zero length.  
 * This is in case they are NFS mbufs which point to disk buffers.
 * and the buffers must not be released until after the packet has flown.
 */
static struct mbuf *
sn_pullup (n, len)
    register struct mbuf *n;
    register int len;
{
    register struct mbuf *m;
    register int count;
    
    /* check it will fit in an ordinary mbuf */
    if (len > MLEN)
      goto bad;

    if ((n->m_flags & M_EXT) == 0) {
	/* ordinary mbuf */
	if (n->m_data + len > &n->m_dat[MLEN]) {
	    /* not enough space: must realign buffer start */
	    bcopy (n->m_data, n->m_dat, n->m_len);
	    n->m_data = n->m_dat;
	}
    }
    else {
	/* external cluster (must be only user) */
	if (mclrefcnt[mtocl(n->m_data)] != 1) {
	    log (LOG_NOTICE, "snpullup: shared ext cluster\n");
	    goto bad;
	}
	if (n->m_data + len > n->m_ext.ext_buf + n->m_ext.ext_size) {
	    /* not enough space: must realign cluster start */
#ifdef DEBUG
	    if (len <= n->m_ext.ext_size) {
		log (LOG_NOTICE, "snpullup: ext buf too small (%d < %d)\n",
		     n->m_ext.ext_size, len);
		goto bad;
	    }
#endif
	    bcopy (n->m_data, n->m_ext.ext_buf, n->m_len);
	    n->m_data = n->m_ext.ext_buf;
	}
    }

    /* now ok to append in place */
    m = n;
    n = n->m_next;
    len -= m->m_len;

    while (len > 0 && n) {
	count = MIN(len, n->m_len);
	bcopy(mtod(n, caddr_t), mtod(m, caddr_t)+m->m_len, (unsigned)count);
	len -= count;
	m->m_len += count;
	n->m_len -= count;
	if (n->m_len)
	  n->m_data += count;
	else
	  n = n->m_next;
    }
    return (m);

 bad:
    m_freem(n);
    return (0);
}


/*
 * Fillup a whole chain, so that each mbuf has a sensible minimum
 * length, to avoid transmit underflows.
 */
static struct mbuf *
sn_fillup (m, minlen)
    register struct mbuf *m;
    register int minlen;
{
    struct mbuf *m0 = 0;
    register struct mbuf *mprev = (struct mbuf *)&m0;

    while (m) {
	if (m->m_len && m->m_len < minlen && m->m_next)
	  m = sn_pullup (m, minlen);
	mprev->m_next = m;
	mprev = m;
	m = m->m_next;
    }
    return m0;
}



#ifdef MIPSEL
static void
snswapb (s, d, len)
    u_char *s, *d;
    int len;
{
    while (len > 0) {
	*(unsigned int *)d = (s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3];
	d += 4;
	s += 4;
	len -= 4;
    }
}


static struct mbuf *
snswapm (m0)
    struct mbuf *m0;
{
    struct mbuf *m;
    caddr_t s;

    for (m = m0; m; m = m->m_next) {
	if ((m->m_flags & M_EXT) && mclrefcnt[mtocl(m->m_data)] != 1)
	    /* external cluster (must be only user) */
	    panic ("snswapm: shared ext cluster");

	s = m->m_data;
	if ((unsigned)s & 3) {
	    /* buffer is unaligned: must align data in buffer */
	    caddr_t dat, d;
	    unsigned int msz;
	    
	    /* determine data buffer base and size */
	    if (m->m_flags & M_EXT) {
		dat = m->m_ext.ext_buf;
		msz = m->m_ext.ext_size;
	    } else if (m->m_flags & M_PKTHDR) {
		dat = m->m_pktdat;
		msz = MHLEN;
	    } else {
		dat = m->m_dat;
		msz = MLEN;
	    }

	    /* round buffer base up to word boundary */
	    d = (caddr_t) (((unsigned long)dat + 3) & ~3);

	    /* can we shuffle the data down to start of buffer? */
	    if (m->m_data > d)
		m->m_data = d;
	    else {
		/* is there room after the current data? */
		d = (caddr_t) (((unsigned long)m->m_data + m->m_len + 3) & ~3);
		if (d + ((m->m_len + 3) & ~3) <= dat + msz)
		    m->m_data = d;
		else
		    /* this shouldn't happen!! */
		    panic ("snswapm");
	    }
	}

	snswapb (s, m->m_data, m->m_len);
    }

    return m0;
}
#endif

#ifdef XDSSONICBUG
/*
 * compress buffer in SRAM after it has been received
 */

void
xds_compress_buffer (addr, len)
caddr_t addr;
int len;
{
    int words = (len + 3) >> 2;
    unsigned int *sp, *dp;
    /* ASSERT(IS_K0SEG(addr)); */
    sp = dp = (unsigned int *)addr;
    while (words-- > 0) {
	*sp = *dp;
	sp++;
	dp += 2;
    }
}

/*
 * expand buffer in SRAM before it is sent
 */

void
xds_expand_buffer (addr, len)
caddr_t addr;
int len;
{
    int words = (len + 3) >> 2;
    unsigned int *sp, *dp;
    /* ASSERT(IS_K0SEG(addr)); */
    sp = (unsigned int *)addr + words;
    dp = sp + words;
    while (words-- > 0) {
	*(dp+1) = 0xdeaddead;
	*dp = *sp;
	sp--;
	dp -= 2;
    }
    clean_dcache (addr, len * 2);
}

#endif
