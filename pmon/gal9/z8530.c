/* $Id: z8530.c,v 1.3 1997/06/24 15:24:59 chris Exp $ */
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
	dp->ucmd = reg;
	wbflush ();
	z8530delay (Z8530_DELAY);
    }
    dp->ucmd = val;
    wbflush ();
    z8530delay (Z8530_DELAY);
}


static unsigned int
z8530getreg (volatile z8530dev *dp, int reg)
{
    unsigned char ucmd;
    if (reg != RR_STATUS) {
	dp->ucmd = reg;
	wbflush ();
	z8530delay (Z8530_DELAY);
    }
    ucmd = dp->ucmd;
    z8530delay (Z8530_DELAY);
    return (ucmd);
}


static int
z8init (volatile z8530dev *dp)
{

    volatile z8530dev *dpa, *dpb;

    dpa = dp;
    dpb = dp - 1;		/* strange but true */

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

    baudrate = getbaudval (baudrate);
    if (baudrate == 0)
      return 1;

    brtc = BRTC(Z8530_CLOCK,baudrate);

    /* wait for output to drain */
    do {
	ucmd = dp->ucmd;
	z8530delay (Z8530_DELAY);
    } while ((ucmd & US_TXRDY) == 0);

    z8530putreg(dp, WR_TXRXINT, 0);		/*  no interrupts*/
    z8530putreg(dp, WR_RXBITS, UC_8BITS_RX | UM_RXENABLE);
    z8530putreg(dp, WR_MODE, UC_16X_CLK | UC_1STOPBIT);
    z8530putreg(dp, WR_TXBITS, UC_8BITS_TX | UM_TXENABLE | UM_RTS);
    z8530putreg(dp, WR_INTCNTRL, UM_MIE | UM_NOVEC);
    z8530putreg(dp, WR_CLKMODE, UC_RXCISBRG | UC_TXCISBRG);
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
	ucmd = dp->ucmd;
	z8530delay (Z8530_DELAY);
	return (ucmd & US_TXRDY);
    case OP_TX:
	dp->udata = data;
	wbflush ();
	z8530delay (Z8530_DELAY);
	break;
    case OP_RXRDY:
	ucmd = dp->ucmd;
	z8530delay (Z8530_DELAY);
	return (ucmd & US_RXRDY);
    case OP_RX:
	udata = dp->udata;
	z8530delay (Z8530_DELAY);
	return (udata);
    }

    return 0;
}
 
#endif /* Z8530 */
