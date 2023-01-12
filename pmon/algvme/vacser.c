/* $Id: vacser.c,v 1.2 1996/01/16 14:24:13 chris Exp $ */
#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <algvme/sbd.h>
#include <algvme/vac068.h>

typedef volatile unsigned int vint;

#ifdef VACBERRBUG
extern unsigned int vac_uint (vint *reg);
#else
#define vac_uint(r) (*(r))
#endif


static const struct vacserdev {
  vint		*mr;
  vint		*txdat;
  vint		*rxdat;
  vint		*imr;
  vint		*isr;
} vacserdev[2] = {
    {VACMRA, VACTXA, VACRXA, VACIMA, VACISA},
    {VACMRB, VACTXB, VACRXB, VACIMB, VACISB},
};

/* 
 *  Array indexed by the baud value stored in the cflag word:
 *  translates to the appropriate time-constant value for that rate.
 */
static const int vsiobconv[] = {
  0,				/* B0 */
  -1,				/* B50 */
  VAC_UM_B75,			/* B75 */
  -1,				/* B110 */
  -1,				/* B134 */
  VAC_UM_B150,			/* B150 */
  -1,				/* B200 */
  VAC_UM_B300,			/* B300 */
  VAC_UM_B600,			/* B600 */
  VAC_UM_B1200,			/* B1200 */
  -1,				/* B1800 */
  VAC_UM_B2400,			/* B2400 */
  -1,				/* B4800 */
  VAC_UM_B9600,			/* B9600 */
  -1,				/* B19200 */
  -1				/* B38400 */
};


static int
vsinit (const struct vacserdev *dp)
{
    *VACCLKDIV = VAC_CLK_16MHZ; 
    wbflush();
    return 0;
}


static int
vsprogram (const struct vacserdev *dp, int baudrate)
{
    unsigned int mode;
 
    baudrate &= CBAUD;
    if (vsiobconv[baudrate] <= 0)
      return 1;

    /* setup mode from cflag */
    mode = VAC_UM_RXCEN | VAC_UM_TXCEN | VAC_UM_RXEN | VAC_UM_TXEN;
    mode |= VAC_UM_8BITS;
    mode |= vsiobconv[baudrate];
    
    /* drop any pending input */
    while (*dp->isr & (VAC_UIS_RXC|VAC_UIS_FERR|VAC_UIS_BRK))
      (void) vac_uint(dp->rxdat);

    *dp->mr = mode; wbflush();
    return 0;
}


pvacser (int op, char *dat, int chan, int data)
{
    const struct vacserdev *dp = &vacserdev[chan];

    switch (op) {
    case OP_INIT:
	return vsinit (dp);
    case OP_BAUD:
	return vsprogram (dp, data);
    case OP_TXRDY:
	return (*dp->isr & VAC_UIS_TXR);
    case OP_TX:
	*dp->txdat = VACCTOTX (data); wbflush ();
	return 0;
    case OP_RXRDY:
	return (*dp->isr & (VAC_UIS_RXC | VAC_UIS_FIFOF));
    case OP_RX:
	return VACRXTOC (vac_uint (dp->rxdat));
    }
}
