/*
 * z8530.h: AMD 85C30 / Zilog Z8530 UART definitions (async mode only)
 * Copyright (c) 1998 Algorithmics Ltd
 */

#ifndef _Z8530_H_
#define _Z8530_H_

/* Z8530 requires 1-2 us of recovery time between each register accesss.
   If this isn't enforced by h/w then we have to do it in software. */
#ifndef Z8530DELAY
#define Z8530DELAY 2
#endif

/* baud rate calculation assume 16X clock */
#define	BRTC(clock, baudrate)	((clock)/(16*2*(baudrate))-2)

#ifdef __ASSEMBLER__
#define UCMD		Z8530_CMD
#define UDATA		Z8530_DATA
#define CHA		Z8530_CHA
#define CHB		Z8530_CHB
#else
typedef struct {
  z8530reg0;
  z8530reg1;
} z8530dev;
#endif

/* Write Register names */
#define WR_COMMAND			0x00
#define WR_TXRXINT			0x01
#define WR_INTVEC			0x02
#define WR_RXBITS			0x03
#define WR_MODE				0x04
#define WR_TXBITS			0x05
#define WR_EXFEATURE			0x07
#define WR_DATA				0x08
#define	WR_INTCNTRL			0x09
#define WR_ENC				0x0a
#define WR_CLKMODE			0x0b
#define WR_BRGTC_LO			0x0c
#define WR_BRGTC_HI			0x0d
#define WR_BRGCNT			0x0e
#define WR_STATCNTRL			0x0f

/* Read Register names */
#define	RR_STATUS			0x00
#define RR_ERROR			0x01
#define RR_INTVEC			0x02
#define RR_IPEND			0x03
#define RR_MODE				0x04
#define RR_TXBITS			0x05
#define RR_DATA				0x08
#define RR_RXBITS			0x09
#define RR_MISCSTAT			0x0a
#define RR_ENC				0x0b
#define RR_BRGTC_LO			0x0c
#define RR_BRGTC_HI			0x0d
#define RR_EXFEATURE			0x0e
#define RR_STATCNTRL			0x0f

/* 
 * Command bits 
 */

	/* WR_COMMAND (WR0) */
#define UC_RESIST			0x10
#define UC_RESITX			0x28
#define UC_RESERR			0x30
#define UC_RESIUS			0x38

	/* WR_INTCNTRL (WR9) */
#define UC_HWRESET			0xc0
#define UC_RESETA			0x80
#define UC_RESETB			0x40


/* 
 * Mode bits 
 */

	/* WR_TXRXINT (WR1) */
#define UM_RXINT			0x10
#define UM_TXINT			0x02
#define UM_EXTINT			0x01

	/* WR_RXBITS (WR3) */
#define UM_8BITS_RX			0xc0
#define UM_6BITS_RX			0x60
#define UM_7BITS_RX			0x40
#define UM_5BITS_RX			0x00
#define UM_AUTOEN			0x20
#define UM_RXENABLE			0x01

	/* WR_MODE (WR4) */
#define UM_1X_CLK			0x00
#define UM_16X_CLK			0x40
#define UM_2STOPBITS			0x0c
#define UM_15STOPBITS			0x08
#define UM_1STOPBIT			0x04
#define UM_PARITYEVEN			0x02
#define UM_PARITYODD			0x00
#define UM_PARITYON			0x01
#define UM_PARITYOFF			0x00

	/* WR_TXBITS (WR5) */
#define UM_DTR				0x80
#define UM_8BITS_TX			0x60
#define UM_6BITS_TX			0x40
#define UM_7BITS_TX			0x20
#define UM_5BITS_TX			0x00
#define UM_SETBREAK			0x10
#define UM_TXENABLE			0x08
#define UM_RTS				0x02

	/* WR_INTCNTRL (WR9) */
#define UM_MIE				0x08
#define UM_DLC				0x04
#define UM_NOVEC			0x02
#define UM_VIS				0x01

	/* WR_ENC (WR10) */

	/* WR_CLKMODE (WR11) */
#define UM_RXCISBRG			0x40
#define UM_TXCISBRG			0x10
#define UM_TRXISOP			0x04
#define UM_TRXOPISBRG			0x02

	/* WR_BRGCNT (WR14) */
#define UM_LOOPBACK			0x10
#define UM_BRGPCLK			0x02
#define UM_BRGENABLE			0x01

	/* WR_STATCNTRL (WR15) */
#define UM_BREAKIE			0x80
#define UM_CTSIE			0x20
#define UM_DCDIE			0x08
#define UM_ZEROCIE			0x02

/*
 * Status bits
 */

	/* RR_STATUS (RR0) */
#define US_RXRDY			0x01
#define US_TXRDY			0x04
#define US_DCD				0x08
#define US_CTS				0x20
#define US_BREAK			0x80

	/* RR_ERROR (RR1) */
#define US_ALLSENT			0x01
#define US_PARITY			0x10
#define US_OVERRUN			0x20
#define US_FRAMING			0x40

	/* RR_IPEND (RR3) */
#define US_EXTSTBIP			0x01
#define US_TXBIP			0x02
#define US_RXBIP			0x04
#define US_EXTSTAIP			0x08
#define US_TXAIP			0x10
#define US_RXAIP			0x20


#endif /* _Z8530_H_ */
