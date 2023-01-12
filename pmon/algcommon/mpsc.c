/* $Id: mpsc.c,v 1.3 1997/04/10 19:55:34 chris Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <mpsc.h>

typedef struct mpscdev *mpscdp;

#define MPSCCLK	6144000	 /* clock rate 6.144MHz */


/*
 * internal routines to access mpsc registers
 * (we dont need to worry about interrupts do we?)
 */
static void
mpscputreg (volatile mpscdp dp, unsigned int reg, unsigned int val)
{
  if (reg != mCR0) {
      dp->ucmd = mCR0_REG | (reg << 24); 
      wbflush ();
  }
  dp->ucmd = val;
  wbflush ();
}

static unsigned int
mpscgetreg (volatile mpscdp dp, unsigned int reg)
{
  if (reg != mSR0) {
    dp->ucmd = mCR0_REG | (reg << 24);
    wbflush ();
  }
  return (dp->ucmd);
}

static void
mpscputbaud (volatile mpscdp dp, int regval, int baud)
{
  mpscputreg (dp, mCR12, regval);
  dp->ucmd = baud << 24; wbflush();
  dp->ucmd = baud << 16; wbflush ();
  (void)dp->ucmd;
}


static int
mpscinit (volatile mpscdp dp)
{
    volatile mpscdp dpa, dpb;

#ifndef P4000
    /* dp points to channel 0 */
    dpa = dp; dpb = dpa + 1;
#else
    /* dp points to channel 1, because P4000 connectors are swapped! */
    dpb = dp; dpa = dpb - 1;
#endif

    /* single read to get in known state */
    (void) dpa->ucmd;
    (void) dpb->ucmd;

    /* chip remains in standby mode until first write to register 0 */
    dpa->ucmd = mCR0_NOP; wbflush();

    /* global initialisation */
    dpa->ucmd = mCR0_RESETCHN; wbflush();
    dpb->ucmd = mCR0_RESETCHN; wbflush();

    /* select interrupt operation and make receivers have priority */
    mpscputreg (dpa, mCR2A, mCR2A_PRIRX|mCR2A_SVEC|mCR2A_B1|mCR2A_INTAINTB);
    
    /* interrupt vector */
    mpscputreg (dpb, mCR2B, 0);

    return 0;
}


static void
mpscflush (volatile mpscdp dp)
{
    /* wait for Tx fifo to drain */
    int timeout = 10000;
    while (!(mpscgetreg (dp, mSR1) & mSR1_ALLSENT))
	if (--timeout == 0)
	    break;
}


static int
mpscprogram (volatile mpscdp dp, int baudrate)
{
    unsigned int cr3, cr4, cr5, brtc;

    mpscflush (dp);

    baudrate = getbaudval (baudrate);
    if (baudrate == 0)
      return 1;

    brtc = BRTC (MPSCCLK, baudrate);

    /*
     * See the mpsc manual for details.
     */
    cr4 = mCR4_CLKX | mCR4_STOP1;
    cr3 = mCR3_AUTO | mCR3_RXEN | mCR3_8BIT;
    cr5 = mCR5_DTR | mCR5_RTS | mCR5_TXEN | mCR5_8BIT;

    mpscputreg (dp, mCR1, 0);		/* disable interrupts */
    mpscputreg (dp, mCR4, cr4);
    mpscputreg (dp, mCR5, cr5);
    mpscputreg (dp, mCR3, cr3);
    mpscputreg (dp, mCR14, mCR14_NOP); /* stop BRG */
    mpscputbaud (dp, mCR12_TXBRGSET, brtc);
    mpscputbaud (dp, mCR12_RXBRGSET, brtc);
    mpscputreg (dp, mCR15, mCR15_RXRXBRG|mCR15_TXTXBRG|mCR15_TRXCO|mCR15_TRXCTXCLK);
    mpscputreg (dp, mCR10, mCR10_NRZ);
    mpscputreg (dp, mCR14, mCR14_NOP|mCR14_BRGSYS|mCR14_BRGRX|mCR14_BRGTX);
    mpscputreg (dp, mCR12, mCR12_TXTRXC);
    mpscputreg (dp, mCR11, 0);
    return 0;
}


pmpsc (int op, char *dat, int chan, int data)
{
    volatile mpscdp dp = (mpscdp) dat;

    switch (op) {
    case OP_INIT:
	return mpscinit (dp);
    case OP_XBAUD:
    case OP_BAUD:
	return mpscprogram (dp, data);
    case OP_TXRDY:
	return (dp->ucmd & mSR0_TXEMPTY);
    case OP_TX:
	dp->udata = data << 24; wbflush ();
	break;
    case OP_RXRDY:
	return (dp->ucmd & mSR0_RXRDY);
    case OP_RX:
	return (dp->udata >> 24) & 0xff;
    case OP_FLUSH:
	mpscflush (dp);
	break;
    case OP_RXSTOP:
	/* rx flow control */
	mpscputreg (dp, mCR5, mCR5_DTR | mCR5_TXEN | mCR5_8BIT | 
		    (data ? 0 : mCR5_RTS));
	break;
    }
    return 0;
}
