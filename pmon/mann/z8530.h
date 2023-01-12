#ident "$Header: /vol/cvsroot/pmon/pmon/mann/z8530.h,v 1.1.1.1 1996/01/10 16:35:21 chris Exp $"

/*
 * Copyright 1991 Algorithmics Ltd.
 */

/*
 * Templates onto the IO space of the cpu board z8530 dusart.
 */

#define Z8530_DELAY		2		/* requires ~2usec delays */

#define	BRTC(clock, baudrate)	(clock/(16*2*(baudrate))-2)

#ifndef __ASSEMBLER__
typedef struct {
	unsigned int	ucmd;
	unsigned int	udata;
} z8530dev;
#endif

#define CMD	0
#define DATA	4


/* Register names */
#define WR_COMMAND			0x00
#define WR_TXRXINT			0x01
#define WR_RXBITS			0x03
#define WR_MODE				0x04
#define WR_TXBITS			0x05
#define WR_DATA				0x08
#define	WR_INTCNTRL			0x09
#define WR_ENC				0x0a
#define WR_CLKMODE			0x0b
#define WR_BRGTC_LO			0x0c
#define WR_BRGTC_HI			0x0d
#define WR_BRGCNT			0x0e
#define WR_STATCNTRL			0x0f

#define	RR_STATUS			0x00
#define RR_ERROR			0x01
#define RR_IPEND			0x03
#define RR_DATA				0x08
#define RR_BRGTC_LO			0x0c
#define RR_BRGTC_HI			0x0d

/* Mode bits */
						/* WR_COMMAND */

#define UM_TXINT			0x02	/* WR_TXRXINT */
#define UM_EXTINT			0x01

#define UM_AUTOEN			0x20	/* WR_RXBITS */
#define UM_RXENABLE			0x01

#define UM_PARITYEVEN			0x02	/* WR_MODE */
#define UM_PARITYODD			0x00
#define UM_PARITYON			0x01

#define UM_DTR				0x80	/* WR_TXBITS */
#define UM_SETBREAK			0x10
#define UM_TXENABLE			0x08
#define UM_RTS				0x02

#define UM_MIE				0x08	/* WR_INTCNTRL */


						/* WR_ENC */

#define UM_TRXISOP			0x04	/* WR_CLKMODE */


#define UM_LOOPBACK			0x10	/* WR_BRGCNT */
#define UM_BRGPCLK			0x02
#define UM_BRGENABLE			0x01


#define UM_BREAKIE			0x80	/* WR_STATCNTRL */
#define UM_CTSIE			0x20
#define UM_DCDIE			0x08

/* Command bits */
#define UC_RESIST			0x10	/* WR_COMMAND */
#define UC_RESITX			0x28
#define UC_RESERR			0x30
#define UC_RESIUS			0x38

#define UC_RXINT			0x10	/* WR_TXRXINT */

#define UC_8BITS_RX			0xc0	/* WR_RXBITS */
#define UC_6BITS_RX			0x60
#define UC_7BITS_RX			0x40
#define UC_5BITS_RX			0x00

#define UC_1X_CLK			0x00	/* WR_MODE */
#define UC_16X_CLK			0x40
#define UC_2STOPBITS			0x0c
#define UC_15STOPBITS			0x08
#define UC_1STOPBIT			0x04

#define UC_8BITS_TX			0x60	/* WR_TXBITS */
#define UC_6BITS_TX			0x40
#define UC_7BITS_TX			0x20
#define UC_5BITS_TX			0x00

#define UC_HWRESET			0xc0	/* WR_INTCNTRL */
#define UC_RESETA			0x80
#define UC_RESETB			0x40

						/* WR_ENC */

#define UC_RXCISBRG			0x40	/* WR_CLKMODE */
#define UC_TXCISBRG			0x10
#define UC_TRXOPISBRG			0x02

						/* WR_BRGCNT */

						/* WR_STATCNTRL */

/* Status bits */
#define US_RXRDY			0x01	/* RR_STATUS */
#define US_TXRDY			0x04
#define US_DCD				0x08
#define US_CTS				0x20
#define US_BREAK			0x80

#define US_ALLSENT			0x01	/* RR_ERROR */
#define US_PARITY			0x10
#define US_OVERRUN			0x20
#define US_FRAMING			0x40

