/* $Id: ptty.c,v 1.2 1996/01/16 14:25:20 chris Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <vtty.h>

#include <sys/types.h>
#include <machine/endian.h>

#include <sbd.h>

#ifdef PTTY
typedef struct {
    unsigned int rsem;
    unsigned int rdata;
    unsigned int wsem;
    unsigned int wdata;
} pttydev;

static void
pttyflush (volatile pttydev *dp)
{
    /* wait for Tx fifo to drain */
    while (dp->wsem != 0)
	continue;
}

pptty (int op, char *dat, int chan, int data)
{
    volatile pttydev *dp = (volatile pttydev *) dat;
    int c;

    switch (op) {
    case OP_INIT:
	dp->wsem = 0;
	return 0; 
    case OP_TXRDY:
	/* Is there room in the buffer ? */
	return (dp->wsem == 0);
    case OP_TX:
	dp->wdata = data;
	dp->wsem = ~0;
	break;

    case OP_RXRDY:
	return (dp->rsem != 0);
	break;
    case OP_RX:
	c = dp->rdata;
	dp->rsem = 0;
	return (c & 0xff);

    case OP_FLUSH:
	pttyflush (dp);
	break;

    case OP_RXSTOP:
	break;
    }
    return 0;
}


#endif
