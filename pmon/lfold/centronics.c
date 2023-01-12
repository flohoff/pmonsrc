/* $Id: centronics.c,v 1.5 1996/12/09 20:11:25 nigel Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>

#define CENDATA	*(volatile unsigned short *)PHYS_TO_K1(CENDATA_BASE)
#define USEDMA

static int	cenopen;
static int	cenmode;
static int	cenrxstop;

#ifdef USEDMA

#define	NRING	64
#define	RMASK	(NRING-1)
#define	BSIZE	512

struct cenbuf {
    int		full;
    unsigned char *buf;
};

static struct cenbuf cenring[NRING];
static int	cenfirst;	/* first buffer with data */
static int	cencount;	/* number of buffers with data */
static int	cenoffset;	/* output pointer in current buffer */
static int	cendmabusy;	/* dma is currently active */

/* stats */
static int	cendmafull;
static int	cendmapartial;
static int	cendmawaiting;

static void
censtartdma ()
{
    static int led = 0;
    if (cencount < NRING) {
	struct cenbuf *cb = &cenring[(cenfirst + cencount++) & RMASK];
	cb->full = 0;
	if (IS_K0SEG (cb->buf))
	    dcache_clean (cb->buf, BSIZE);
	G10_REG (G10_CENDMAADDR) = K0_TO_PHYS (cb->buf);
	G10_REG (G10_CENDMACOUNT) = BSIZE - 1;
	G10_REG (G10_CENCONFIG) |= CENCFG_DMAEN;
	sbdled (led = !led);
	cendmabusy = 1;
    }
}
#endif


static int
ceninit (void)
{
    extern int memorysize;
    unsigned int ctlin;
    char *bp;
    int i;

    /* set timing based on CPU clock frequency */
    G10_REG(G10_CENTIMING) =
	(((2500 + CYCLETIME - 1) / CYCLETIME) << CENTIMING_2500NS_SHIFT) |
	(((500 + CYCLETIME - 1) / CYCLETIME) << CENTIMING_500NS_SHIFT);

    /* put into compatible (unidirectional) mode */
    G10_REG(G10_CENIEEEMODE) = cenmode = CENMODE_COMPATIBLE;

    /* with standard timings */
    G10_REG(G10_CENCONFIG) = CENCFG_APPL_STANDARD | CENCFG_DMARD;

    /* initialise output control signals */
    G10_REG(G10_CENCTLOUT) =
	CENOUT_SELECT | CENOUT_NOTFAULT | CENOUT_NOTACK; 

    ctlin = G10_REG(G10_CENCTLIN);
    if (!(ctlin & CENIN_NOTSELECTIN) && !(ctlin & CENIN_NOTSTROBE)) {
	/* SELECTed with STROBE active: send an ACK to clear it,
	   we'll never see a read interrupt otherwise. */
	G10_REG(G10_CENCTLOUT) &= ~CENOUT_NOTACK; 
	sbddelay (3);
	G10_REG(G10_CENCTLOUT) |= CENOUT_NOTACK; 
    }

    cenrxstop = 0;

#ifdef USEDMA
    G10_REG(G10_IOCONFIG) |= IOCONFIG_CENDMA | IOCONFIG_CENDMARD;
    G10_REG(G10_INTCAUSE) = ~INT_DMACEN;

    memorysize = (memorysize - NRING * BSIZE) & ~31;
    if (IS_K0SEG (cenring))
	bp = (char *) PHYS_TO_K0 (memorysize);
    else
	bp = (char *) PHYS_TO_K1 (memorysize);

    for (i = 0; i < NRING; i++, bp += BSIZE) {
	cenring[i].full = 0;
	cenring[i].buf = bp;
    }  

    cenfirst = cencount = cenoffset = cendmabusy = 0;
    censtartdma ();
#endif
    return 0;
}


static void
cenflush (void)
{
    G10_REG (G10_CENCONFIG) &= ~CENCFG_DMAEN;
    cenfirst = cencount = cenoffset = cendmabusy = 0;
    censtartdma ();
}


static int
cenreset (int chan)
{
    cenrxstop = 0;
    cenflush ();
    /* remove BUSY, FAULT & PERROR signals */
    G10_REG(G10_CENCTLOUT) &= ~(CENOUT_BUSY | CENOUT_PERROR);
    G10_REG(G10_CENCTLOUT) |= CENOUT_NOTFAULT;
    return 0;
}


static void
cennegotiate (void)
{
    unsigned char req;

    req = CENDATA;
    G10_REG(G10_INTCAUSE) = ~INT_CENWR;

    switch (req) {
    case CENMODEREQ_NIBBLE:
	cenmode = CENMODE_NIBBLE;
	break;
    case CENMODEREQ_BYTE:
	cenmode = CENMODE_BYTE;
	break;
    case CENMODEREQ_ECP:
	cenmode = CENMODE_ECP;
	break;
    case CENMODEREQ_EPP:
	cenmode = CENMODE_EPP;
	break;
    }

    if (cenmode != CENMODE_COMPATIBLE) {
	/* accept the proposal */
	if (cenmode == CENMODE_NIBBLE)
	    G10_REG(G10_CENIEEEMODE) = cenmode;
	else
	    G10_REG(G10_CENIEEEMODE) = CENMODE_ACCEPT | cenmode;
    } else {
	/* reject the proposal */
	G10_REG(G10_CENIEEEMODE) = CENMODE_TERMINATE;
    }
}



static unsigned int
cenpoll (void)
{
    unsigned int cause;

    if (cenopen == 0)
	return 0;

    cause = G10_REG(G10_INTCAUSE);
    
    if (cause & INT_CENINIT) {
	G10_REG(G10_INTCAUSE) = ~INT_CENINIT;
	cenmode = CENMODE_COMPATIBLE;
    }

    if ((cause & INT_CENWR) && cenmode == CENMODE_COMPATIBLE) {
	cause &= ~INT_CENWR;
	cennegotiate ();
    }

#ifdef USEDMA
    if (cause & INT_DMACEN) {
	G10_REG(G10_INTCAUSE) = ~INT_DMACEN;
	if (cendmabusy) {
	    /* current input buffer now full */
	    struct cenbuf *cb = &cenring[(cenfirst + cencount - 1) & RMASK];
	    cb->full = 1;
	    cendmabusy = 0;
	}
	censtartdma ();
    }

    cause &= ~INT_CENRD;
    if (cencount > 0 && !cenrxstop) {
	struct cenbuf *cb = &cenring[cenfirst];
	if (cb->full) {
	    /* dma buffer full (offset cannot be at end) */
	    cause |= INT_CENRD;
	    if (cendmabusy)
		++cendmafull;
	    else
		++cendmawaiting;
	}
	else if (cencount == 1 && !cb->full &&
		 G10_REG (G10_CENDMAADDR) > K0_TO_PHYS (&cb->buf[cenoffset])) {
	    /* some bytes in dma buffer above current read offset */
	    cause |= INT_CENRD;
	    ++cendmapartial;
	}
    }
#endif	

    return cause;
}


  
static unsigned char
cenrxdata (void)
{
#ifdef USEDMA
    struct cenbuf *cb = &cenring[cenfirst];
    volatile unsigned char *dp = &cb->buf[cenoffset];
    unsigned char data;

    if (!cb->full)
	/* buffer is not full, so we must use uncached address */
	dp = (unsigned char *) K0_TO_K1 (dp);

    data = *dp;

    if (++cenoffset == BSIZE) {
	/* this buffer is now empty, advance to next and restart dma */
	cenfirst = (cenfirst + 1) & RMASK;
	cencount--;
	cenoffset = 0;
	if (!cendmabusy)
	    censtartdma ();
    }

    return data;
#else
    G10_REG(G10_INTCAUSE) = ~INT_CENRD;
    return CENDATA & 0xff;
#endif
}


centronics (int op, char *dat, int chan, int data)
{
    switch (op) {
    case OP_INIT:
	return ceninit ();

    case OP_OPEN:
	if (++cenopen > 1)
	    break;
	/* first open: drop through */

    case OP_RESET:
	return cenreset (chan);

    case OP_CLOSE:
	if (cenopen > 0 && --cenopen == 0) {
	    /* stop DMA */
	    G10_REG (G10_CENCONFIG) &= ~CENCFG_DMAEN;
	    cendmabusy = 0;
	    /* set BUSY, FAULT & PERROR signals */
	    G10_REG(G10_CENCTLOUT) &= ~CENOUT_NOTFAULT;
	    G10_REG(G10_CENCTLOUT) |= (CENOUT_BUSY | CENOUT_PERROR);
	}
	break;

    case OP_TXRDY:
	/* if unidirectional then Tx is a wormhole */
	return (cenpoll() & INT_CENWR) || (cenmode == CENMODE_COMPATIBLE);

    case OP_TX:
	if (cenmode != CENMODE_COMPATIBLE && 
	    data != CNTRL('S') && data != CNTRL('Q')){
	    G10_REG(G10_INTCAUSE) = ~INT_CENWR;
	    if (cenmode == CENMODE_NIBBLE)
		G10_REG(G10_CENNIBBLE) = data;
	    else
		CENDATA = data;
	}
	break;

    case OP_RXRDY:
	/* if rx is stopped, just return !ready, we therefore won't read
	   the data, and no handshake will be sent to the host. */
	return (cenpoll () & INT_CENRD) && !cenrxstop;

    case OP_RX:
	return cenrxdata ();

    case OP_RXSTOP:
	cenrxstop = data;
	break;

#ifdef USEDMA
    case OP_FLUSH:
	if (data & 1)
	    cenflush ();
	break;
#endif
    }

    return 0;
}

