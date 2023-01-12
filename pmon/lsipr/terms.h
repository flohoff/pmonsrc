/* $Id: terms.h,v 1.2 1996/01/16 14:25:07 chris Exp $ */
#ifndef _TERMS_
#define _TERMS_

#define OP_INIT 1
#define OP_TX 2
#define OP_RX 3
#define OP_RXRDY 4
#define OP_TXRDY 5
#define OP_BAUD 6

typedef unsigned char byte;

struct p8530info {
	byte *siobase;
	byte wr3[2];
	byte wr5[2];
	} ;

struct sableinfo {
	byte *siobase;
	} ;

struct p2681info {
	byte *siobase;
	byte brate[2];
	byte curacr;
	} ;

#endif /* _TERMS_ */
