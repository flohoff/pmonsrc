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
static volatile p5064icu * const icu = PA_TO_KVA1 (ICU_BASE);
static volatile p5064bcr0 * const bcr0 = PA_TO_KVA1 (BCR0_BASE);

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
    int chip;

    cenautoflush = cenflag ("cenign");
    cendebug = cenflag ("cendbg");
    cenhost = cenflag ("cenhost");

    /* reprogram WinBond chip (if it is there) */
    outb (EFER_PORT, EFER_ENABLE);
    outb (EFIR_PORT, 0x9);
    chip = inb(EFDR_PORT) & CR9_CHIPID;
    if (chip == CHIPID_W83877F) {
	/* enable ECP mode on Winbond chip, so that bidir mode works */
	outb (EFIR_PORT, 0x0);
	outb (EFDR_PORT, (inb (EFDR_PORT) & ~(CR0_PRTMODS1 | CR0_PRTMODS0)) 
	      | CR0_PRTMODS1);
	outb (EFIR_PORT, 0x9);
	outb (EFDR_PORT, inb (EFDR_PORT) | CR9_PRTMODS2);
	/* enable totem-poll (i.e. not open-drain) IRQ output */
	outb (EFIR_PORT, 0x17);
	outb (EFDR_PORT, inb (EFDR_PORT) | CR17_PRIOQOD);
	outb (EFER_PORT, EFER_DISABLE);
    }

    if (cenhost) {
	/* disable auto-busy generation */
	bcr0->cen_busy = BCR0_CEN_BUSY_MANUAL;

	/* set fifo output mode */
	ecp->ecr = ECR_MODE_CFIFO;

	/* direction=output, i/u enable, selectin, !init, !autofd, !strobe */
	ecp->dcr = DCR_INTENB | DCR_H_SELECTIN;
	sbddelay (100);		/* 100us delay for printer startup */
	ecp->dcr = DCR_INTENB | DCR_H_SELECTIN | DCR_H_NINIT;

	/* clear interrupt latch */
	icu->ctrl.clear = ICU_DEV_CENT;
    }
    else {
	/* set bidirectional mode, no dma */
	ecp->ecr = ECR_MODE_BIDIR;

	/* enable auto-busy generation */
	bcr0->cen_busy = BCR0_CEN_BUSY_AUTO;

	/* initial state */

	/* direction=input, i/u enable, !perror, select, !busy, !ack */
	ecp->dcr = DCR_INPUT | DCR_INTENB | DCR_P_NBUSY;

	/* & !fault */
	zpiob_bis (ZPIOB_NFAULT);

	/* clear strobe interrupt latch */
	icu->ctrl.clear = ICU_DEV_CENT;

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
    if (icu->irr.dev & ICU_DEV_CENT) {
	/* clear strobe interrupt latch */
	icu->ctrl.clear = ICU_DEV_CENT;

	/* pulse ack/: wakes up host and clears busy output */
	ecp->dcr |= DCR_P_ACK;
	sbddelay (3);		/* 3us! */

	/* XXX bouncing IRQ workaround XXX */
	if (icu->irr.dev & ICU_DEV_CENT) {
	    /* unexpected strobe interrupt before busy released */
#ifdef DEBUG
	    *(volatile int *)0xbfc00004 = 0; /* analyser trigger */
#endif
	    ++earlystrobe;
	    /* clear strobe interrupt latch again! */
	    icu->ctrl.clear = ICU_DEV_CENT;
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
	    /* Tx is a bit bucket when not in host mode */
	    return 1;
	/* keep sending until output fifo is full */
	return !(ecp->ecr & ECR_FIFO_FULL);

    case OP_TX:
	if (cenhost)
	    /* stuff into output fifo */
	    ecp->cFifo = data;
	break;

    case OP_RXRDY:
	if (cenhost)
	    return 0;
#if 0
	if (ecp->dsr & DSR_P_NSELECTIN)
	    /* no SELECTIN signal */
	    return 0;
#endif
	/* look for latched strobe */
	if (icu->irr.dev & ICU_DEV_CENT) {
	    if (cenopen)
		return !cenrxstop;
	    if (cenautoflush)
		cenrdflush ();	/* quietly throw away the data */
	}
	return 0;

    case OP_RX:
	if (cenhost)
	    return 0;

#ifdef P5064
	/* read from onboard data latch */
	rxdata = ecp->latch & 0xff;
#else
	/* read data (XXX this assumes it is held stable until ACKed,
	   which is not necessarily true, e.g. Win3.1 driver) */
	rxdata = ecp->data & 0xff;
#endif

	/* make sure strobe has finished before sending ACK */
	if ((ecp->dsr & DSR_P_NSTROBE) == 0) {
	    ++latestrobe;
#ifdef DEBUG
	    *(volatile int *)0xbfc00008 = 0; /* analyser trigger */
#endif
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

