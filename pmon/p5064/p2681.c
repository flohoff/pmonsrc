/* $Id: p2681.c,v 1.1 1997/09/01 14:46:41 nigel Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <p5064/p2681.h>

struct baudconv {
    unsigned char	csrval;
    unsigned char	acr7;
};

const struct baudconv btab[] = {
    /* if CSR == -1, then baud rate is not supported */
    /* if ACR7 == 2, then that bit is don't care */
    {0xff,	2},	/* B0 */
    {0x00,	0},	/* B50  */
    {0x00,	1},	/* B75 */
    {0x11,	2},	/* B110 */
    {0x22,	2},	/* B134 */
    {0x33,	1},	/* B150 */
    {0x33,	0},	/* B200 */
    {0x44,	2},	/* B300 */
    {0x55,	2},	/* B600 */
    {0x66,	2},	/* B1200 */
    {0xaa,	1},	/* B1800 */
    {0x88,	2},	/* B2400 */
    {0x99,	2},	/* B4800 */
    {0xbb,	2},	/* B9600 */
    {0xcc,	1},	/* B19200 */
    {0xcc,	0}	/* B38400 */
};


#define WDELAY() \
{\
   register int i; \
   wbflush(); \
   for (i = DELAY; i != 0; i--) continue; \
}

static int 
p2681init (struct p2681info *info)
{
    volatile struct p2681dev *dp = info->dev;
    int ch;

    dp->imr = 0x00; WDELAY();			/* disable all i/us */
    dp->acr = 0x00; WDELAY();			/* initial ACR */
    info->curacr = 0x00;
    for (ch = 0; ch <= 1; ch++) {
	dp->channel[ch].cr = 0x0a; WDELAY();	/* disable tx & rx */
	dp->channel[ch].cr = 0x10; WDELAY();	/* reset mr ptr */
	dp->channel[ch].cr = 0x20; WDELAY();	/* reset rx */
	dp->channel[ch].cr = 0x30; WDELAY();	/* reset tx */
	dp->channel[ch].cr = 0x70; WDELAY();	/* stop break */
	dp->channel[ch].mr = 0x13; WDELAY();	/* no parity, 8 bits data */
	dp->channel[ch].mr = 0x0f; WDELAY();	/* 1 stop bit */
	dp->channel[ch].cr = 0x05; WDELAY();	/* enable tx & rx */
	info->brate[ch] = 0;
    }
    return 0;
}


static int 
p2681baud (struct p2681info *info, int chan, unsigned int brate)
{
    volatile struct p2681dev *dp = info->dev;
    const struct baudconv *this, *other;

    if (brate == 0 || brate > B38400)
      return 1;

    if (brate == info->brate[chan])
      return 0;

    this = &btab[brate];
    other = &btab[info->brate[chan ^ 1]];
    
    /* valid baud rate? */
    if (this->csrval == 0xff)
      return 1;

    /* both channels' ACR values must not add up to one */
    if (this->acr7 + other->acr7 == 1)
      return 1;

    /* disable rx & tx */
    dp->channel[chan].cr = 0x0a;
    WDELAY();

    /* reprogram ACR if ACR7 isn't "don't care", and isn't already programmed */
    if (this->acr7 != 2 && other->acr7 != this->acr7) {
	info->curacr = (info->curacr & 0x7f) | (this->acr7 << 7);
	dp->acr = info->curacr;
	WDELAY();
    }

    /* write to clock select register */
    dp->channel[chan].sr = this->csrval;
    WDELAY();
    
    /* enable rx & tx */
    dp->channel[chan].cr = 0x05;
    WDELAY();

    /* remember this channel's baudrate and return */
    info->brate[chan] = brate;
    return 0;
}



p2681 (int op, char *dat, int chan, int data)
{
    struct p2681info *info = (struct p2681info *) dat;
    volatile struct p2681dev *dp = info->dev;

    switch (op) {
    case OP_INIT:
	return p2681init (info);
    case OP_BAUD:
	return p2681baud (info, chan, data);
    case OP_TXRDY:
	return (dp->channel[chan].sr & SR_TXRDY);
    case OP_TX:
	dp->channel[chan].dat = data; wbflush ();
	break;
    case OP_RXRDY:
	return (dp->channel[chan].sr & SR_RXRDY);
    case OP_RX:
	return dp->channel[chan].dat & 0xff;
    }
    return 0;
}
