/* $Id: pport.c,v 1.2 1996/01/16 14:25:10 chris Exp $ */
#include <mips.h>
#include <termio.h>
#include <pmon.h>

typedef struct ppdev {
    volatile unsigned int *p_ctl;
    volatile unsigned int *p_data;
} ppdev_t;
static ppdev_t ppdev;

static int
ppinit (volatile ppdev_t *dp)
{
    dp->p_ctl = PHYS_TO_K1(0);
    dp->p_data = PHYS_TO_K1(0);
    return 0;
}

pp (int op, char *dat, int chan, int data)
{
    ppdev_t *dp = &ppdev;

    switch (op) {
    case OP_INIT:
	return ppinit (dp);
    case OP_BAUD:
	return (0);
    case OP_TXRDY:
	return (1);		/* always ready */
    case OP_TX:
	break;
    case OP_RXRDY:
	return (0);
    case OP_RX:
	return (0);
    }
    return 0;
}
