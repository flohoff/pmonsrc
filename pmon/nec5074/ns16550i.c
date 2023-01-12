/*
 * ns16550i.c: internal NS16550 in NEC DDB-Vrc5074 chip
 */

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>

/*
 * Redefine NS16550 layout to match internal device with
 * one register every 8 bytes 
 */

#undef NS16550_HZ
#undef NSREG
#undef nsreg

#define NS16550_HZ	(sbdcpufreq()/12)
#if #endian(big)
#define NSREG(x)	(((x)*8)+7)
#define nsreg(x)	unsigned long long :56; unsigned char x
#else
#define NSREG(x)	((x)*8)
#define nsreg(x)	unsigned char x; unsigned long long :56
#endif
#define xr16850(dp) 	0

#include <ns16550.h>

static int
nsinit (volatile ns16550dev *dp, int chan)
{
    unsigned int x;

    /* force access to id reg */
    dp->cfcr = 0;
    dp->iir = 0;
    if ((dp->iir & 0x38) != 0) {
	SBD_DISPLAY ("EIIR", 0);
	return 1;
    }

    /* in case it really is a 16550, enable the fifos */
    dp->fifo = FIFO_ENABLE;
    sbddelay(100);
    dp->fifo = FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_1;
    sbddelay(100);

    if ((dp->iir & IIR_FIFO_MASK) != IIR_FIFO_MASK) {
	SBD_DISPLAY ("NFFO", 0);
	dp->fifo = 0;
    }
    dp->scr = 0;

    (void) xr16850 (dp);
    return 0;
}


static int
nsbrtc (unsigned int tick, unsigned int baud)
{
    int brtc, err;

#define divrnd(n, q)	(((n) * 2 / (q) + 1) / 2) /* divide and round off */
    brtc = divrnd(COMTICK, baud);
    if (brtc <= 0 || brtc > 0xffff)
	return 0;

    err = divrnd(1000 * COMTICK, brtc * baud) - 1000;
#undef divrnd

    if (err < 0)
	err = -err;
    if (err > SPEED_TOLERANCE)
	return 0;
    return brtc;
}


static int
nsbaud (volatile ns16550dev *dp, int chan, int baudrate)
{
    unsigned short brtc;
    unsigned char ocfcr;
    int timeout;
    int div4;

    /* wait for Tx fifo to completely drain */
    timeout = 10000;
    while (!(dp->lsr & LSR_TSRE)) {
	(void) dp->scr;		/* Vrc5074 bug workaround */
	if (--timeout == 0)
	    break;
    }
    dp->scr = 0;

    baudrate = getbaudval (baudrate);
    if (baudrate == 0)
      return 1;
    
    div4 = 0; brtc = nsbrtc (COMTICK, baudrate);
    if (brtc == 0 && xr16850 (dp)) {
	/* try divide by four */
	div4 = 1; brtc = nsbrtc (COMTICK/4, baudrate);
    }
    if (brtc == 0)
	return 1;

    ocfcr = dp->cfcr;
    dp->cfcr = CFCR_DLAB;
    dp->dll = brtc & 0xff;
    dp->dlm = brtc >> 8;
    dp->cfcr = ocfcr;
    dp->mcr = MCR_DTR | MCR_RTS | MCR_IENABLE | (div4 ? MCR_CLKDIV4 : 0);
    dp->ier = 0;
    return 0;
}


static int
nsmode (volatile ns16550dev *dp, int chan, struct termio *t)
{
    unsigned int cfcr;

    cfcr = 0;
    switch (t->c_lflag & CSIZE) {
    case CS5: cfcr = CFCR_5BITS; break;
    case CS6: cfcr = CFCR_6BITS; break;
    case CS7: cfcr = CFCR_7BITS; break;
    case CS8: cfcr = CFCR_8BITS; break;
    }

    if (t->c_lflag & CSTOPB)
	cfcr |= CFCR_STOPB;

    if (t->c_lflag & PARENB) {
	cfcr |= CFCR_PENAB;
	cfcr |= (t->c_lflag & PARODD) ? CFCR_PODD : CFCR_PEVEN;
    }

    if (xr16850 (dp)) {
	volatile xr16850efr *xp = (xr16850efr *)dp;
	unsigned char efr = EFR_ENABLE;

	if (t->c_iflag & ICTS_OFLOW)
	    efr |= EFR_AUTO_CTS;	/* h/w output flow control */
	if (t->c_iflag & IRTS_IFLOW)
	    efr |= EFR_AUTO_RTS;	/* h/w input flow control */
	if (t->c_iflag & IXON)
	    efr |= EFR_OFLOW_1;		/* s/w output flow control */
	if (t->c_iflag & IXOFF)
	    efr |= EFR_IFLOW_1;		/* s/w input flow control */

#if 0
	/* XXX ambiguous xr16850 manual on this MCR bit */
	if (t->c_iflag & IXANY)
	    dp->mcr |= MCR_XON_ANY;
	else
	    dp->mcr &= ~MCR_XON_ANY;
#endif

	dp->cfcr = CFCR_EFR;
	xp->xoff1 = t->c_cc[V_STOP];
	xp->xon1 = t->c_cc[V_START];
	xp->efr = efr;
    }

    dp->cfcr = cfcr;
    return 0;
}


ns16550i (int op, char *dat, int chan, int data)
{
    volatile ns16550dev *dp = (ns16550dev *) dat;
    unsigned int txrdy;

    if (chan > 0)
	return 1;

    switch (op) {
    case OP_INIT:
	return nsinit (dp, chan);

    case OP_XBAUD:
    case OP_BAUD:
	return nsbaud (dp, chan, data);

    case OP_TXRDY:
	txrdy = dp->scr;
	if (txrdy == 0 && (dp->lsr & LSR_TXRDY)) {
	    /* fifo empty - compute free space */
	    if (xr16850(dp))
		txrdy = 128;
	    else if ((dp->iir & IIR_FIFO_MASK) == IIR_FIFO_MASK)
		txrdy = 16;
	    else 
		txrdy = 1;
	    dp->scr = txrdy;
	}
	return txrdy;

    case OP_TX:
	dp->data = data;
	dp->scr--;
	wbflush ();
	break;

    case OP_RXRDY:
	(void) dp->scr;		/* Vrc5074 bug workaround */
	return (dp->lsr & LSR_RXRDY);

    case OP_RX:
	return dp->data & 0xff;

    case OP_RXSTOP:
	if (data)
	    dp->mcr &= ~MCR_RTS;	    /* disable Rx */
	else
	    dp->mcr |= MCR_RTS;	    	    /* enable Rx */
	break;

    case OP_TCSET:
	return nsmode (dp, chan, (struct termio *)data);

    default:
	return 1;
    }

    return 0;
}
