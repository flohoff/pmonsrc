#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>
#include <ns16550.h>

#define NSREAD(p)	sbd_ioread32(&(p))
#define NSWRITE(p,v)	sbd_iowrite32(&(p), v)

static const unsigned short nsbrtc[] = {
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

static int nsfifo;

static int
nsinit (volatile ns16550dev *dp, int chan)
{
    unsigned int x;

    /* force access to id reg */
    NSWRITE(dp->cfcr, 0);
    NSWRITE(dp->iir, 0);
    if ((NSREAD(dp->iir) & 0x38) != 0) {
	SBD_DISPLAY ("EIIR", CHKPNT_EIIR);
	return 1;
    }

    /* in case it really is a 16550, enable the fifos */
    dp->fifo = FIFO_ENABLE | FIFO_RCV_RST | FIFO_XMT_RST | FIFO_TRIGGER_4;
    sbddelay(100);

    if ((NSREAD(dp->iir) & IIR_FIFO_MASK) == IIR_FIFO_MASK) {
	if ((NSREAD(dp->fifo) & FIFO_TRIGGER_14) == FIFO_TRIGGER_4)
	    nsfifo = 1;
	else {
	    SBD_DISPLAY ("EFFO", CHKPNT_EFFO);
	    NSWRITE(dp->fifo, 0);
	}
    }
    else {
	SBD_DISPLAY ("NFFO", CHKPNT_NFFO);
	NSWRITE(dp->fifo, 0);
    }

    return 0;
}


static int
nsprogram (volatile ns16550dev *dp, int chan, int baudrate)
{
    unsigned short brtc;
    int timeout;

    /* wait for Tx fifo to completely drain */
    timeout = 10000;
    while (!(NSREAD(dp->lsr) & LSR_TSRE))
	if (--timeout == 0)
	    break;

    baudrate &= CBAUD;
    if (baudrate == 0)
      return 1;
    brtc = nsbrtc [baudrate];

    NSWRITE (dp->cfcr, CFCR_DLAB);
    NSWRITE (dp->data, brtc & 0xff);
    NSWRITE (dp->ier, brtc >> 8);
    NSWRITE (dp->cfcr, CFCR_8BITS);
    NSWRITE (dp->mcr, MCR_DTR | MCR_RTS | MCR_IENABLE);
    NSWRITE (dp->ier, 0);
    return 0;
}


ns16550 (int op, char *dat, int chan, int data)
{
    volatile ns16550dev *dp = (ns16550dev *) dat;

    if (chan > 0)
	return 1;

    switch (op) {
    case OP_INIT:
	return nsinit (dp, chan);

    case OP_BAUD:
	return nsprogram (dp, chan, data);

    case OP_TXRDY:
	return (NSREAD(dp->lsr) & LSR_TXRDY);

    case OP_TX:
	NSWRITE(dp->data, data); wbflush ();
	break;

    case OP_RXRDY:
	return (NSREAD(dp->lsr) & LSR_RXRDY);

    case OP_RX:
	return NSREAD(dp->data) & 0xff;

    case OP_RXSTOP:
	if (data)
	    NSWRITE(dp->mcr, NSREAD(dp->mcr) & ~MCR_RTS); /* disable Rx */
	else
	    NSWRITE(dp->mcr, NSREAD(dp->mcr) | MCR_RTS); /* enable Rx */
	break;
    }
    return 0;
}
