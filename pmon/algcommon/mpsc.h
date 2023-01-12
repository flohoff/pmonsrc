/* $Id: mpsc.h,v 1.2 1996/01/16 14:24:07 chris Exp $ */
/*
 * Templates onto the IO space of the cpu board NEC MPSC dusart.
 * See the NEC data book for information.
 */

#ifndef __ASSEMBLER__
struct mpscdev {
  unsigned int		udata;
  unsigned 		:32;
  unsigned int		ucmd;
  unsigned 		:32;
};
#endif

#ifndef MPSCX
#define MPSCX 16		/* default to 16X clocking */
#define mCR4_CLKX	mCR4_CLKX16
#endif

/* convert baudrate into a value for the BRG */
#define BRTC(clk, brate) (((clk) / (2*(brate)*MPSCX)) - 2)

#ifdef __ASSEMBLER__
/* offsets from usart base for usart registers */
#define CMD	8
#define DATA	0
#endif



/*
 * Register descriptions from UMCOMMUNICO88V30 B-1-116
 */
#define mCR0	0		/* register selection and triggers */
#define mCR1	1		/* interrupts/dma/receive data mode */
#define mCR2A	2		/* CPU interface */
#define mCR2B	2		/* interrupt vectors */
#define mCR3	3		/* receive data control */
#define mCR4	4		/* protocol selection */
#define mCR5	5		/* transmit data control */
#define mCR6	6		/* sync char [COP], secondary address [BOP] */
#define mCR7	7		/* sync char [COP], flag [BOP] */
#define mCR8	8		/* transmit data length lo [BOP] */
#define mCR9	9		/* transmit data length hi [BOP] */
#define mCR10	10		/* data format, loop control, sync bits */
#define mCR11	11		/* E/S interrupt mask */
#define mCR12	12		/* BRG control */
#define mCR13	13		/* transmit data counter control */
#define mCR14	14		/* DPLL, BRG, test mode */
#define mCR15	15		/* transmit/receive clock source */

#define mSR0	0		/* transmit/receive operation status */
#define mSR1	1		/* E/S status */
#define mSR2B	2		/* interrupt source */
#define mSR3	3		/* BRG zero count and residue count */
#define mSR4A	4		/* interrupt pending source */
#define mSR8	8		/* transmit data length count lo */
#define mSR9	9		/* transmit data length count hi */
#define mSR10	10		/* DPLL status and loop status */
#define mSR11	11		/* E/S interrupt mask bit status */
#define mSR12	12		/* receive BRG value lo */
#define mSR13	13		/* receive BRG value hi */
#define mSR14	14		/* transmit BRG value lo */
#define mSR15	15		/* transmit BRG value hi */

#define mCR0_NOP	0x00000000	/* CRC control - no operation */
#define mCR0_CRCRXINI	0x40000000	/* CRC control - initialise Rx CRC */
#define mCR0_CRCTXINI	0x80000000	/* CRC control - initialise Tx CRC */
#define mCR0_RESETEOM	0xc0000000	/* CRC control - reset Tx EOM */
#define mCR0_NOP	0x00000000	/* Command - no operation */
#define mCR0_REG	0x00000000	/* Command - noop alias for register access */
#define mCR0_HIGH	0x08000000	/* Command - high register pointer */
#define mCR0_RESETES	0x10000000	/* Command - reset E/S bit latch */
#define mCR0_RESETCHN	0x18000000	/* Command - reset channel */
#define mCR0_NEXTRX	0x20000000	/* Command - enable next Rx char interrupt */
#define mCR0_RESETTX	0x28000000	/* Command - reset Tx interrupt */
#define mCR0_RESETERR	0x30000000	/* Command - reset errors */
#define mCR0_EOI	0x38000000	/* Command - interrupt service completed */

#define mCR1_SFD	0x80000000	/* enable short frame detect [BOP] */
#define mCR1_IIOVER	0x40000000	/* interrupt immediately on overrun */
#define mCR1_RXFMASK	0x20000000	/* mask first Rx character interrupt */
#define mCR1_IRXDIS	0x00000000	/* disable Rx interrupts */
#define mCR1_IRXFIRST	0x08000000	/* interrupt on first Rx character */
#define mCR1_IRX	0x10000000	/* interrupt on Rx character */
#define mCR1_IRXERR	0x18000000	/* interrupt on Rx character and errors */
#define mCR1_ITXFIRST	0x04000000	/* interrupt when Tx enabled */
#define mCR1_ITX	0x02000000	/* interrupt on Tx buffer empty */
#define mCR1_IES	0x01000000	/* interrupt on external status change */

#define mCR2A_VEC	0x80000000	/* use vectored interrupts */
#define mCR2A_SVEC	0x40000000	/* use variable vectors */
#define mCR2A_A1	0x00000000	/* use vector scheme A-1 */
#define mCR2A_A2	0x08000000	/* use vector scheme A-2 */
#define mCR2A_A3	0x10000000	/* use vector scheme A-3 */
#define mCR2A_B1	0x18000000	/* use vector scheme B-1 */
#define mCR2A_B2	0x20000000	/* use vector scheme B-2 */
#define mCR2A_PRIRX	0x04000000	/* pri: RxA > RxB > TxA > TxB > E/SA > E/SB */
#define mCR2A_PRICHNA	0x00000000	/* pri: RxA > TxA > RxB > TxB > E/SA > E/SB */
#define mCR2A_INTAINTB	0x00000000	/* channel A interrupt, channel B interrupt */
#define mCR2A_DMAAINTB	0x01000000	/* channel A DMA, channel B interrupt */
#define mCR2A_DMAADMAB	0x02000000	/* channel A DMA, channel B DMA */

#define mCR3_5BIT	0x00000000	/* 5 bit data */
#define mCR3_7BIT	0x40000000	/* 7 bit data */
#define mCR3_6BIT	0x80000000	/* 6 bit data */
#define mCR3_8BIT	0xc0000000	/* 8 bit data */
#define mCR3_AUTO	0x20000000	/* auto CTS/DCD operation */
#define mCR3_HUNT	0x10000000	/* hunt for sync [COP, BOP] */
#define mCR3_RXCRC	0x08000000	/* CRC include Rx data [COP, BOP] */
#define mCR3_ASRCH	0x04000000	/* enter address search mode [BOP] */
#define mCR3_SYNCINH	0x02000000	/* inhibit sync character load [COP] */
#define mCR3_MULTICAST	0x02000000	/* enter multicast operation [BOP] */
#define mCR3_RXEN	0x01000000	/* enable receiver */

#define mCR4_CLKX1	0x00000000	/* select x1 asynchronous clock */
#define mCR4_CLKX16	0x40000000	/* select x16 asynchronous clock */
#define mCR4_CLKX32	0x80000000	/* select x32 asynchronous clock */
#define mCR4_CLKX64	0xc0000000	/* select x64 asynchronous clock */
#define mCR4_MONOSYNC	0x00000000	/* mono-sync mode [COP] */
#define mCR4_BISYNC	0x10000000	/* bi-sync mode [COP] */
#define mCR4_EXTSYNC	0x30000000	/* external sync mode [COP] */
#define mCR4_STOP1	0x04000000	/* use 1 stop bit */
#define mCR4_STOP1_5	0x08000000	/* use 1.5 stop bits */
#define mCR4_STOP2	0x0c000000	/* use 2 stop bits */
#define mCR4_PARODD	0x00000000	/* use odd parity */
#define mCR4_PAREVEN	0x02000000	/* use even parity */
#define mCR4_PAREN	0x01000000	/* enable parity */

#define mCR5_DTR	0x80000000	/* assert DTR output */
#define mCR5_5BIT	0x00000000	/* 5 bit data */
#define mCR5_7BIT	0x20000000	/* 7 bit data */
#define mCR5_6BIT	0x40000000	/* 6 bit data */
#define mCR5_8BIT	0x60000000	/* 8 bit data */
#define mCR5_BREAK	0x10000000	/* send break */
#define mCR5_ABORT	0x10000000	/* send abort [BOP] */
#define mCR5_TXEN	0x08000000	/* enable transmitter */
#define mCR5_CRCCCITT	0x00000000	/* use CRC-CCITT polynomial [COP,BOP] */
#define mCR5_CRC16	0x04000000	/* use CRC-16 polynomial [COP] */
#define mCR5_RTS	0x02000000	/* assert RTS output */
#define mCR5_TXCRC	0x01000000	/* include Tx data in CRC [COP,BOP]*/

#define mCR10_CRC0	0x00000000	/* initialise CRC to all 0's [COP,BOP] */
#define mCR10_CRC1	0x80000000	/* initialise CRC to all 1's [COP,BOP] */
#define mCR10_NRZ	0x00000000	/* use NRZ encoding */
#define mCR10_NRZI	0x20000000	/* use NRZI encoding */
#define mCR10_FM1	0x40000000	/* use FM1 encoding */
#define mCR10_FM0	0x60000000	/* use FM0 encoding */
#define mCR10_AUTOTX	0x10000000	/* auto Tx on sync [COP] */
#define mCR10_LOOPTX	0x10000000	/* enable TX on loop [BOP] */
#define mCR10_IDLEFLAG	0x00000000	/* transmit flag when Tx completed [BOP] */
#define mCR10_IDLEMARK	0x08000000	/* mark transmit state on completion [BOP] */
#define mCR10_UNORMAL	0x00000000	/* send CRC flags on underrun [BOP] */
#define mCR10_UABORT	0x04000000	/* send abort on underrun [BOP] */
#define mCR10_AUTOTXEN	0x02000000	/* enable auto Tx on sync [COP] */
#define mCR10_TXLOOPEN	0x02000000	/* enable Tx loop [BOP] */
#define mCR10_SYNC8	0x00000000	/* use 8 bit sync characters [COP] */
#define mCR10_SYNC6	0x01000000	/* use 6 bit sync characters [COP]*/

#define mCR11_IBREAK	0x80000000	/* enable break interrupt [ASYNC] */
#define mCR11_IABORT	0x80000000	/* enable abort/GA interrupt [BOP] */
#define mCR11_IUNDER	0x40000000	/* enable underrun/EOM interrupt [COP,BOP] */
#define mCR11_ICTS	0x20000000	/* */
#define mCR11_ISYNCHUNT	0x10000000	/* enable sync/hunt interrupt */
#define mCR11_IDCD	0x08000000	/* enable DCD change interrpt */
#define mCR11_IALLSENT	0x04000000	/* enable all sent interrupt */
#define mCR11_IIDLE	0x02000000	/* enable idle interrupt [COP,BOP] */
#define mCR11_IBRG	0x01000000	/* enable BRG interrupt */

#define mCR12_RXTRXC	0x00000000	/* output RxBRG on TRxC pin */
#define mCR12_TXTRXC	0x80000000	/* output TxBRG on TRxC pin */
#define mCR12_RXDPLL	0x00000000	/* use RxBRG as DPLL source */
#define mCR12_TXDPLL	0x40000000	/* use TxBRG as DPLL source */
#define mCR12_ITXBRG	0x08000000	/* enable TxBRG interrupt */
#define mCR12_IRXBRG	0x04000000	/* enable RxBRG interrupt */
#define mCR12_TXBRGSET	0x02000000	/* write TxBRG counter */
#define mCR12_RXBRGSET	0x01000000	/* write RxBRG counter */

#define mCR13_TXDPL	0x02000000	/* enable Tx data length comparison [BOP] */
#define mCR13_STANDBY	0x01000000	/* enter standby operation */

#define mCR14_NOP	0x00000000	/* DPLL command: no operation */
#define mCR14_ESRCH	0x20000000	/* DPLL command: enter search */
#define mCR14_RESETMISS	0x40000000	/* DPLL command: reset missing clock */
#define mCR14_DISABLE	0x60000000	/* DPLL command: disable DPLL */
#define mCR14_BRG	0x80000000	/* DPLL command: DPLL source BRG */
#define mCR14_XS	0xa0000000	/* DPLL command: DPLL source XTAL/STRxC */
#define mCR14_FM	0xc0000000	/* DPLL command: use FM Rx data format */
#define mCR14_NRZI	0xe0000000	/* DPLL command: use NRZI Rx data format */
#define mCR14_LLOOP	0x10000000	/* local loopback */
#define mCR14_ECHO	0x08000000	/* external echo */
#define mCR14_BRGXS	0x00000000	/* use XTAL/STRxC as BRG source */
#define mCR14_BRGSYS	0x04000000	/* use system clock as BRG source */
#define mCR14_BRGRX	0x02000000	/* enable RxBRG */
#define mCR14_BRGTX	0x01000000	/* enable TxBRG */

#define mCR15_XTAL	0x80000000	/* use external crystal oscillator */
#define mCR15_RXSTRXC	0x00000000	/* receive clock: STRxC input */
#define mCR15_RXTRXC	0x20000000	/* receive clock: TRxC input */
#define mCR15_RXRXBRG	0x40000000	/* receive clock: RxBRG */
#define mCR15_RXDPLL	0x60000000	/* receive clock: DPLL */
#define mCR15_TXSTRXC	0x00000000	/* transmit clock: STRxC input */
#define mCR15_TXTRXC	0x08000000	/* transmit clock: TRxC input */
#define mCR15_TXTXBRG	0x10000000	/* transmit clock: TxBRG */
#define mCR15_TXDPLL	0x18000000	/* transmit clock: DPLL */
#define mCR15_TRXCI	0x00000000	/* define TRxC as an input */
#define mCR15_TRXCO	0x04000000	/* define TRxC as an output */
#define mCR15_TRXCXTAL	0x00000000	/* TRxC output: XTAL */
#define mCR15_TRXCTXCLK	0x01000000	/* TRxC output: TxCLK */
#define mCR15_TRXCBRG	0x02000000	/* TRxC output: BRG */
#define mCR15_TRXCDPLL	0x03000000	/* TRxC output: DPLL */

#define mSR0_EOF	0x80000000	/* end of frame [BOP] */
#define mSR0_FRAME	0x40000000	/* framing error [ASYNC] */
#define mSR0_CRC	0x40000000	/* CRC error [COP,BOP] */
#define mSR0_OVER	0x20000000	/* overrun error */
#define mSR0_PARITY	0x10000000	/* parity error [ASYNC,COP] */
#define mSR0_SFD	0x08000000	/* short frame detected [BOP] */
#define mSR0_TXEMPTY	0x04000000	/* transmit buffer empty */
#define mSR0_ABORT	0x02000000	/* sending abort [BOP] */
#define mSR0_RXRDY	0x01000000	/* Rx data available */

#define mSR1_BREAK	0x80000000	/* break detected [ASYNC] */
#define mSR1_ABORT	0x80000000	/* abort/GA detected [COP,BOP] */
#define mSR1_UNDER	0x40000000	/* Tx underrun/EOM detected [COP,BOP] */
#define mSR1_CTS	0x20000000	/* CTS status */
#define mSR1_SYNC	0x10000000	/* sync/hunt detected [ASYNC] */
#define mSR1_HUNT	0x10000000	/* in hunt mode [COP,BOP] */
#define mSR1_DCD	0x08000000	/* DCD status */
#define mSR1_ALLSENT	0x04000000	/* all data sent [ASYNC,BOP] */
#define mSR1_IDLE	0x02000000	/* idle detect [BOP] */
#define mSR1_BRGZERO	0x01000000	/* BRG reached zero */

#define mSR3_TXBRGZERO	0x10000000	/* TxBRG count has reached zero */
#define mSR3_RXBRGZERO	0x08000000	/* RxBRG count has reached zero */
#define mSR3_RMASK	0x07000000	/* residual count mask */

#define mSR4A_SRXA	0x80000000	/* special RxA interrupt */
#define mSR4A_SRXB	0x40000000	/* special RxB interrupt */
#define mSR4A_RXA	0x20000000	/* RxA interrupt */
#define mSR4A_TXA	0x10000000	/* TxA interrupt */
#define mSR4A_ESA	0x08000000	/* E/S A interrupt */
#define mSR4A_RXB	0x04000000	/* RxB interrupt */
#define mSR4A_TXB	0x02000000	/* TxB interrupt */
#define mSR4A_ESB	0x01000000	/* E/S B interrupt */

#define mSR10_MISSONE	0x80000000	/* one clock missing */
#define mSR10_MISSTWO	0x40000000	/* two clocks missing */
#define mSR10_LOOPING	0x10000000	/* transmitting data in loop mode [BOP] */
#define mSR10_TXSYNC	0x02000000	/* transmitter synchronised [COP] */
#define mSR10_LOOP	0x02000000	/* SDLC loop [BOP] */
