/*
 * centronics.c: centronics interface driver
 *
 * Currently only implements a simple compatible mode, unidirectional
 * "peripheral end" interface, to support high-speed download.  It
 * would be nice to support ECP and/or EPP mode to provide high-speed
 * remote debugging too, or maybe the BSDI PLIP protocol.
 */

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>
#include <ecp.h>
#include <z80pio.h>
#include <w83777f.h>

static volatile ecpdev * const ecp = PA_TO_KVA1 (ECP_BASE);
static volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);

static int		cenopen;
static int		cenrxstop;
static int		cenautoflush;
static int		cendebug;
static int		cenhost;
static int		latestrobe;
static int		earlystrobe;

static int
cenflag (const char *name)
{
    char *s = getenv (name);
    return s && (*s == 'y' || *s == 'Y' || *s == '1');
}


static int
ceninit (void)
{
    cenautoflush = cenflag ("cenign");
    cendebug = cenflag ("cendbg");
    cenhost = cenflag ("cenhost");

    {
	/* enable ECP mode */
	volatile unsigned int *efer = PA_TO_KVA1(EFER);
	volatile unsigned int *efir = PA_TO_KVA1(EFIR);
	volatile unsigned int *efdr = PA_TO_KVA1(EFDR);

	*efer = EFER_ENABLE;

	*efir = 0;
	*efdr = (*efdr & ~ (CR0_PRTMODS1 | CR0_PRTMODS0)) | CR0_PRTMODS1;

	*efir = 9;
	*efdr |= CR9_PRTMODS2;

	*efer = EFER_DISABLE;

    }
    /* bidirectional mode, no dma */
    ecp->ecr = ECR_MODE_BIDIR;

    if (cenhost) {
	/* disable auto-busy generation */
	bcr_bic (BCR_AUTO_BUSY);

	/* initial state */

	/* direction=output, i/u enable, selectin, !init, !autofd, !strobe */
	ecp->dcr = DCR_INTENB | DCR_H_SELECTIN;
	sbddelay (100);		/* 100us delay for printer startup */
	ecp->dcr |= DCR_H_NINIT;

	/* clear ack interrupt latch */
	icu->ctrl.clear = INTR_DEV_CENT;
    }
    else {
	/* enable auto-busy generation */
	bcr_bis (BCR_AUTO_BUSY);

	/* initial state */

	/* direction=input, i/u enable, !perror, select, !busy, !ack */
	ecp->dcr = DCR_INPUT | DCR_INTENB | DCR_P_NBUSY;

	/* & !fault */
	zpiob_bis (ZPIOB_NFAULT);

	/* clear strobe interrupt latch */
	icu->ctrl.clear = INTR_DEV_CENT;

	/* pulse ack: wakes up host and clears auto-busy output */
	ecp->dcr |= DCR_P_ACK;
	sbddelay (1);
	ecp->dcr &= ~DCR_P_ACK;

    }

    cenopen = cenrxstop = 0;

    return 0;
}


static void cenrdflush (void)
{
    if (icu->irr.dev & INTR_DEV_CENT) {
	/* clear strobe interrupt latch */
	icu->ctrl.clear = INTR_DEV_CENT;

	/* pulse ack/: wakes up host and clears busy output */
	ecp->dcr |= DCR_P_ACK;
	sbddelay (3);		/* 3us! */

	/* XXX bouncing IRQ workaround XXX */
	if (icu->irr.dev & INTR_DEV_CENT) {
	    /* unexpected strobe interrupt before busy released */
#ifdef DEBUG
	    *(volatile int *)0xbfc00004 = 0; /* analyser trigger */
#endif
	    ++earlystrobe;
	    /* clear strobe interrupt latch again! */
	    icu->ctrl.clear = INTR_DEV_CENT;
	}

	ecp->dcr &= ~DCR_P_ACK;
    }
}


centronics (int op, char *dat, int chan, int data)
{
    int rxdata;

    switch (op) {
    case OP_INIT:
	return ceninit ();

    case OP_OPEN:
	if (++cenopen > 1)
	    break;
	/* first open */
	latestrobe = earlystrobe = 0;
	/* drop through */

    case OP_RESET:
	cenrdflush();
	cenrxstop = 0;
	break;

    case OP_CLOSE:
	if (cenopen > 0) {
	    if (--cenopen == 0 && cendebug) {
		/* last close */
		if (latestrobe > 0)
		    printf ("cen: %d late strobes\n", latestrobe);
		if (earlystrobe > 0)
		    printf ("cen: %d early strobes\n", earlystrobe);
	    }
	}
	break;

    case OP_TXRDY:
	if (!cenhost)
	    return 1;
	return ecp->dsr & DSR_H_NBUSY;

    case OP_TX:
	if (cenhost) {
	    ecp->data = data;
	    wbflush ();
	    sbddelay (1);	/* data setup time 1us */
	    ecp->dcr |= DCR_H_STROBE;
	    wbflush ();
	    sbddelay (1);	/* strobe pulse width 1us */
	    ecp->dcr &= ~DCR_H_STROBE;
	    wbflush ();
	    sbddelay (2);	/* busy delay 1.5us, data hold time 1us */
	}
	break;

    case OP_RXRDY:
	if (cenhost)
	    return 0;

	/* look for latched strobe */
	if (icu->irr.dev & INTR_DEV_CENT) {
	    if (cenopen)
		return !cenrxstop;
	    if (cenautoflush)
		cenrdflush ();	/* quietly throw away the data */
	}
	return 0;

    case OP_RX:
	if (cenhost)
	    return 0;

	/* read data (XXX assumes it is held stable until ACKed) */
	rxdata = ecp->data & 0xff;

	/* XXX bouncing IRQ workaround XXX */
	/* make sure strobe has finished before sending ACK */
	if ((ecp->dsr & DSR_P_NSTROBE) == 0) {
	    ++latestrobe;
	    *(volatile int *)0xbfc00008 = 0; /* analyser trigger */
	    while ((ecp->dsr & DSR_P_NSTROBE) == 0)
		continue;
	}

	cenrdflush ();
	return rxdata;

    case OP_RXSTOP:
	cenrxstop = data;
	break;

    case OP_FLUSH:
	if (data & 1)
	    cenrdflush ();
	break;
    }

    return 0;
}

