/* $Id: ns16550.c,v 1.3 1996/06/28 12:29:40 nigel Exp $ */

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>
#include <ns16550.h>

static const unsigned short mpscbrtc[] = {
    -1,
    BRTC (50),
    BRTC (75),
    BRTC (110),
    BRTC (134),
    BRTC (150),
    BRTC (200),
    BRTC (300),
    BRTC (600),
    BRTC (1200),
    BRTC (1800),
    BRTC (2400),
    BRTC (4800),
    BRTC (9600),
    BRTC (19200),
    BRTC (38400)};

#define NCHAN	2

static int
nsinit (volatile ns16550dev *dp, int chan)
{
    /* force access to id reg */
    dp->cfcr = 0;
    dp->iir = 0;
    if ((dp->iir & 0x38) != 0)
	return 1;

    /* in case it really is a 16550, enable the fifos */
    dp->fifo = FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_4;
    sbddelay(100);

    return 0;
}


static int
nsprogram (volatile ns16550dev *dp, int chan, int baudrate)
{
    unsigned short brtc;
    int timeout;

    /* wait for Tx fifo to completely drain */
    timeout = 10000;
    while (!(dp->lsr & LSR_TSRE))
	if (--timeout == 0)
	    break;

    baudrate &= CBAUD;
    if (baudrate == 0)
      return 1;
    brtc = mpscbrtc [baudrate];

    dp->cfcr = CFCR_DLAB;
    dp->data = brtc & 0xff;
    dp->ier = brtc >> 8;
    dp->cfcr = CFCR_8BITS;
    dp->mcr = MCR_DTR | MCR_RTS | MCR_IENABLE;
    dp->ier = 0;
    return 0;
}


ns16550 (int op, char *dat, int chan, int data)
{
    volatile ns16550dev *dp = (ns16550dev *) dat;

    if (chan >= NCHAN)
	return 1;

    switch (op) {
    case OP_INIT:
	return nsinit (dp, chan);

    case OP_BAUD:
	return nsprogram (dp, chan, data);

    case OP_TXRDY:
	return (dp->lsr & LSR_TXRDY);

    case OP_TX:
	dp->data = data; wbflush ();
	break;

    case OP_RXRDY:
	return (dp->lsr & LSR_RXRDY);

    case OP_RX:
	return dp->data & 0xff;

    case OP_RXSTOP:
	if (data)
	    dp->mcr &= ~MCR_RTS;	    /* disable Rx */
	else
	    dp->mcr |= MCR_RTS;	    	    /* enable Rx */
	break;
    }
    return 0;
}
