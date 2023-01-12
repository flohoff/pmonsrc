/* $Id: z8530.c,v 1.2 1999/04/02 18:21:43 nigel Exp $ */
#ifdef Z8530

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>
#include <z8530.h>

#define z8530delay(x) sbddelay(x)

static void
z8530putreg (volatile z8530dev *dp, int reg, int val)
{
    if (reg != WR_COMMAND) {
	dp->cmd = reg;
	wbflush (); z8530delay (Z8530DELAY);
    }
    dp->cmd = val;
    wbflush ();
    z8530delay (Z8530DELAY);
}


static unsigned int
z8530getreg (volatile z8530dev *dp, int reg)
{
    unsigned char ucmd;
    if (reg != RR_STATUS) {
	dp->cmd = reg;
	wbflush ();
	z8530delay (Z8530DELAY);
    }
    ucmd = dp->cmd;
    z8530delay (Z8530DELAY);
    return (ucmd);
}


static int
z8init (volatile z8530dev *dp)
{

    volatile z8530dev *dpa, *dpb;

    /* strange but true */
    dpa = dp;
    dpb = (z8530dev *)((char *)dp + Z8530_CHB - Z8530_CHA);	

    /* single read to get in known state */
    (void) z8530getreg(dpa, RR_STATUS);
    (void) z8530getreg(dpb, RR_STATUS);

#ifndef DBGSBD
    z8530putreg(dpa, WR_INTCNTRL, UC_HWRESET);
    z8530putreg(dpa, WR_INTCNTRL, UC_RESETA);
    z8530putreg(dpb, WR_INTCNTRL, UC_RESETB);
#endif

    return 0;
}



static int
z8program (volatile z8530dev *dp, int baudrate)
{
    unsigned short brtc;
    unsigned int ucmd;
    int txwait;

    baudrate = getbaudval (baudrate);
    if (baudrate == 0)
      return 1;

    brtc = BRTC(Z8530CLOCK,baudrate);

    /* wait for output to drain */
    txwait = 10000;
    while ((z8530getreg (dp, RR_ERROR) & US_ALLSENT) == 0)
	if (--txwait == 0)
	    break;

    z8530putreg(dp, WR_TXRXINT, 0);		/*  no interrupts*/
    z8530putreg(dp, WR_RXBITS, UM_8BITS_RX | UM_RXENABLE);
    z8530putreg(dp, WR_MODE, UM_16X_CLK | UM_1STOPBIT);
    z8530putreg(dp, WR_TXBITS, UM_8BITS_TX | UM_TXENABLE | UM_RTS);
    z8530putreg(dp, WR_INTCNTRL, UM_MIE | UM_NOVEC);
    z8530putreg(dp, WR_CLKMODE, UM_RXCISBRG | UM_TXCISBRG);
    z8530putreg(dp, WR_COMMAND, UC_RESIST);
    z8530putreg(dp, WR_COMMAND, UC_RESIST);
    z8530putreg(dp, WR_BRGCNT, UM_BRGENABLE);
    z8530putreg(dp, WR_STATCNTRL, 0);
	
    z8530putreg(dp, WR_BRGCNT, 0);
    z8530putreg(dp, WR_BRGTC_LO, brtc & 0xff);
    z8530putreg(dp, WR_BRGTC_HI, (brtc >> 8) & 0xff);
    z8530putreg(dp, WR_BRGCNT, UM_BRGENABLE);

    return 0;
}


z8530 (int op, char *dat, int chan, int data)
{
    volatile z8530dev *dp = (z8530dev *) dat;
    unsigned char ucmd, udata;

    switch (op) {
    case OP_INIT:
	return z8init (dp);
    case OP_BAUD:
	return z8program (dp, data);
    case OP_TXRDY:
	ucmd = dp->cmd;
	z8530delay (Z8530DELAY);
	return (ucmd & US_TXRDY);
    case OP_TX:
	dp->data = data;
	wbflush ();
	z8530delay (Z8530DELAY);
	break;
    case OP_RXRDY:
	ucmd = dp->cmd;
	z8530delay (Z8530DELAY);
	return (ucmd & US_RXRDY);
    case OP_RX:
	udata = dp->data;
	z8530delay (Z8530DELAY);
	return (udata);
    }

    return 0;
}


/* Force environment reset if UART B's DTR & DCD are looped together
 * In theory this should work even if the chip hasn't been programmed
 * yet, because RESET defaults to simple ASYNC mode.
 *
 * But omit if ITROM is present, because it will have already
 * handled this, and may have placed other status values in
 * the environment which we must read.  */

#if defined(FLASH) && !defined(ITBASE)

int
_sbd_env_force_reset ()
{
    volatile z8530dev *dp = PA_TO_KVA1(DUART_BASE + Z8530_CHB);
    unsigned int ostat, otxb;
    int doreset = 1;
    int i;

    /* get in sync */
    (void) z8530getreg (dp, RR_STATUS);

    /* clear DCD i/u enable so that we can read the real state of DCD */
    ostat = z8530getreg (dp, RR_STATCNTRL);
    z8530putreg (dp, WR_STATCNTRL, ostat & ~UM_DCDIE);

    /* save old DTR setting */
    otxb = z8530getreg (dp, WR_TXBITS);

    /* wiggle DTR four times */
    for (i = 0; i < 4 && doreset; i++) {

	/* check that we can force DCD off by clearing DTR */
	z8530putreg (dp, WR_TXBITS, otxb & ~UM_DTR);
	sbddelay (10000);
	if ((z8530getreg (dp, RR_STATUS) & US_DCD) != 0)
	    doreset = 0;

	/* check that we can force DCD on by setting DTR */
	z8530putreg (dp, WR_TXBITS, otxb | UM_DTR);
	sbddelay (10000);
	if ((z8530getreg (dp, RR_STATUS) & US_DCD) == 0)
	    doreset = 0;
    }

    z8530putreg (dp, WR_TXBITS, otxb);
    z8530putreg (dp, WR_STATCNTRL, ostat);

    return (doreset);
}

#endif /* FLASH && !ITBASE */

#endif /* Z8530 */
