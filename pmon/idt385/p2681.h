/* $Id: p2681.h,v 1.2 1996/01/16 14:24:44 chris Exp $ */
#ifndef _P2681_
#define _P2681_

#ifdef __ASSEMBLER__

/* offsets for externally defined devinfo data structure */
#define	SIOBASE	0
#define	BRATE	4
#define	CURACR	6

#else

struct p2681dev {
    struct {
	unsigned int	mr;
	unsigned int	sr;
	unsigned int	cr;
	unsigned int	dat;
	unsigned int	reg[4];
    } channel[2];
};

#define ipcr	channel[0].reg[0]
#define acr	channel[0].reg[0]
#define isr	channel[0].reg[1]
#define imr	channel[0].reg[1]
#define ctu	channel[0].reg[2]
#define ctl	channel[0].reg[3]

#define iport	channel[1].reg[1]
#define opcr	channel[1].reg[1]
#define setop	channel[1].reg[2]
#define rstop	channel[1].reg[3]

#define SR_RXRDY	0x01
#define SR_TXRDY	0x04

#define DELAY		30

struct p2681info {
    volatile struct p2681dev *dev;
    unsigned char	brate[2];
    unsigned char	curacr;
};
#endif

#endif /* _P2681_ */
