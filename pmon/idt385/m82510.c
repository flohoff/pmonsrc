/* $Id: m82510.c,v 1.2 1996/01/16 14:24:40 chris Exp $ */
#ifdef M82510

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <m82510.h>

#define CLK	4915200		/* clock freq: 4.9152 MHz */

static const unsigned short mpscbrtc[] = {
    -1,
    BRTC (CLK, 50),
    BRTC (CLK, 75),
    BRTC (CLK, 110),
    BRTC (CLK, 134),
    BRTC (CLK, 150),
    BRTC (CLK, 200),
    BRTC (CLK, 300),
    BRTC (CLK, 600),
    BRTC (CLK, 1200),
    BRTC (CLK, 1800),
    BRTC (CLK, 2400),
    BRTC (CLK, 4800),
    BRTC (CLK, 9600),
    BRTC (CLK, 19200),
    BRTC (CLK, 38400)};


static int
m8init (volatile m82510dev *dp)
{
    dp->bank = BANK_WRK; wbflush();	/* select work bank */
    dp->wrk.gsr_icm = ICM_SRST;		/* software reset */
    dp->wrk.msr_tcm = TCM_TXDI;		/* disable Tx */
    dp->wrk.rst_rcm = RCM_RXDI;		/* disable Rx */
    wbflush();

    dp->bank = BANK_MCFG; wbflush();	/* select modem config bank */
    dp->mcfg.clcf = CLCF_RXCS | CLCF_TXCS; 	/* 16x BRGA clock */
    dp->mcfg.bbcf = BBCF_BBCS_CLOCK | BBCF_BBM;	/* BRGB is BRG */
    dp->mcfg.bacf = BACF_BACS_CLOCK | BACF_BAM;	/* BRGA is BRG */
    dp->mcfg.pmd  = 0xfc & ~PMD_DTF;            /* BRGB output on pin,
                                                   0xfc is power-on value */
    dp->mcfg.mie = 0;				/* no modem interrupts */
    wbflush();

    dp->bank = BANK_GCFG; wbflush();	/* select work bank */
    dp->gcfg.imd = IMD_IAM | IMD_RFD_4; /* auto intack; 4-deep rx fifo */
    dp->gcfg.fmd = (3 << FMD_TFT_SHIFT) | (0 << FMD_RFT_SHIFT);
    dp->gcfg.rie = 0;				/* no rx interrupts */
    wbflush();

    dp->bank = BANK_WRK; wbflush();	/* select work bank */
    dp->wrk.msr_tcm = TCM_TXEN;		/* enable Tx */
    dp->wrk.rst_rcm = RCM_RXEN;		/* enable Rx */
    wbflush();

    return 0;
}


static int
m8program (volatile m82510dev *dp, int baudrate)
{
    unsigned short brtc;
    unsigned char lcr;

    baudrate &= CBAUD;
    if (baudrate == 0)
      return 1;
    brtc = mpscbrtc [baudrate];
    lcr = LCR_PARNONE | LCR_CL8;	/* no parity: 8 bits/char */

    dp->bank = BANK_NAS; wbflush();	/* select 8250 compat bank */
    dp->nas.lcr = LCR_DLAB | lcr; wbflush ();
    dp->nas.bal = brtc;
    dp->nas.bah = brtc >> 8; wbflush();

    dp->bank = BANK_MCFG; wbflush();	/* select modem config bank */
    dp->mcfg.bbl = CLK/1000;            /* Set BRGB to approx. 1 KHz. */ 
    dp->mcfg.bbh = (CLK/1000) >> 8; wbflush();

    dp->bank = BANK_NAS; wbflush();	/* select 8250 compat bank */
    dp->nas.lcr = lcr;		
    wbflush();

    dp->bank = BANK_WRK; wbflush();	/* select working bank */
    return 0;
}


m82510 (int op, char *dat, int chan, int data)
{
    volatile m82510dev *dp = (m82510dev *) dat;

    switch (op) {
    case OP_INIT:
	return m8init (dp);
    case OP_BAUD:
	return m8program (dp, data);
    case OP_TXRDY:
	return (dp->wrk.gsr_icm & GSR_TFIR);
    case OP_TX:
	dp->wrk.dat = data; wbflush ();
	break;
    case OP_RXRDY:
	return (dp->wrk.gsr_icm & GSR_RFIR);
    case OP_RX:
	return dp->wrk.dat;
    }
    return 0;
}

#endif /* M82510 */
