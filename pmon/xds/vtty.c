/* $Id: vtty.c,v 1.2 1996/01/16 14:25:26 chris Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <vtty.h>

#include <sys/types.h>
#include <machine/endian.h>

#include <sbd.h>
#ifdef VTTY

struct vttysoftc {
    unsigned char 	*sc_sbuf;	/* pointer to sparc data buffer */
    unsigned char 	*sc_xbuf;	/* pointer to xds data buffer */
    unsigned int	sc_wptr;	/* soft copy of xbuf.vtty_buf */
    volatile unsigned int *sc_irq;	/* pointer to irq generator  */
    unsigned int	sc_vec;		/* vector to supply with interrupt */
} vttysoftc[NVTTY];

static volatile unsigned int *irqgen[8] = {
    NULL,
    VME_REG(VME_IRQ1GEN),
    VME_REG(VME_IRQ2GEN),
    VME_REG(VME_IRQ3GEN),
    VME_REG(VME_IRQ4GEN),
    VME_REG(VME_IRQ5GEN),
    VME_REG(VME_IRQ6GEN),
    VME_REG(VME_IRQ7GEN)
};

#define dcache_clean	r4k_dclean
#define dcache_inval	r4k_dinval

static void vttyinit (struct vttysoftc *sc, volatile vttydev *dp)
{
    unsigned int irq;

    
#ifdef STANDALONEVTTY
    dp->xbuf.vtty_rflags = htonl (VTTY_RDR_READY);
    dp->xbuf.vtty_wflags = htonl(0);
    dp->xbuf.vtty_wptr = dp->xbuf.vtty_rptr = htonl(0);
    dp->xbuf.vtty_irq = htonl(0);

    dp->sbuf.vtty_rflags = htonl (VTTY_RDR_READY);
    dp->sbuf.vtty_wflags = htonl(0);
    dp->sbuf.vtty_wptr = dp->xbuf.vtty_rptr = htonl(0);
    dp->sbuf.vtty_irq = htonl(0);
#else
    while ((VME_REG(VME_BNETSTAT) & 0xff) == 0)
	continue;
#endif    

    /* set up soft state */

#ifdef NOSRAMPARTIALACCESS
    sc->sc_sbuf = (unsigned char *)K1_TO_K0(dp->sbuf.vtty_buf);
    sc->sc_xbuf = (unsigned char *)K1_TO_K0(dp->xbuf.vtty_buf);
#else
    sc->sc_sbuf = dp->sbuf.vtty_buf;
    sc->sc_xbuf = dp->xbuf.vtty_buf;
#endif
    irq = ntohl(dp->xbuf.vtty_irq);
    if (0 < irq && irq <= 7) {
	sc->sc_irq = irqgen[irq];
	sc->sc_vec = ntohl(dp->xbuf.vtty_vec);
    }
    else
	sc->sc_irq = (unsigned int *)0;
    sc->sc_wptr = ntohl(dp->xbuf.vtty_wptr);
}

static void
vttyflush (volatile vttydev *dp)
{
    /* wait for Tx fifo to drain */
    while (!VTTYEMPTY(dp->xbuf))
	continue;
}


pvtty (int op, char *dat, int chan, int data)
{
    volatile vttydev *dp = (volatile vttydev *) dat;
    struct vttysoftc *sc = &vttysoftc[chan];
    unsigned int wptr, rptr;
    unsigned int rflags;
    unsigned char c;

    switch (op) {
    case OP_INIT:
	vttyinit (sc, dp);
	return 0; 
    case OP_TXRDY:
	/* Is there room in the buffer ? */
	rptr = ntohl (dp->xbuf.vtty_rptr);
	rflags = ntohl (dp->xbuf.vtty_rflags);
	wptr = sc->sc_wptr;
	return ((rflags & VTTY_RDR_READY) && VTTYSPACE(wptr, rptr) != 0);
    case OP_TX:
	wptr = sc->sc_wptr;
	sc->sc_xbuf[wptr] = data;
#ifdef NOSRAMPARTIALACCESS
	dcache_clean (&sc->sc_xbuf[wptr], 1);
#endif
	VTTYNEXT(wptr);
	sc->sc_wptr = wptr;
	dp->xbuf.vtty_wptr = htonl(wptr);
	if (sc->sc_irq)
	    *sc->sc_irq = sc->sc_vec;
#ifdef STANDALONEVTTY
	rptr = ntohl(dp->sbuf.vtty_rptr);
	VTTYNEXT(rptr);
	dp->xbuf.vtty_rptr = htonl(rptr);
#endif
	break;
    case OP_RXRDY:
	wptr = ntohl (dp->sbuf.vtty_wptr);
	rptr = ntohl (dp->sbuf.vtty_rptr);
	return (VTTYSPACE(rptr,wptr) != 0);

    case OP_RX:
	rptr = ntohl (dp->sbuf.vtty_rptr);
#if NOSRAMPARTIALACCESS
	dcache_inval(&sc->sc_sbuf[rptr]);
#endif
	c = sc->sc_sbuf[rptr];
	VTTYNEXT(rptr);
	dp->sbuf.vtty_rptr = htonl(rptr);
	return (c);

    case OP_FLUSH:
	vttyflush (dp);
	break;

    case OP_RXSTOP:
	/* rx flow control */
	if (data)
	    dp->sbuf.vtty_rflags |= VTTY_RDR_READY;
	else
	    dp->sbuf.vtty_rflags &= ~VTTY_RDR_READY;
	break;
    }
    return 0;
}


#endif
