/* $Id: m82510.h,v 1.2 1996/01/16 14:24:40 chris Exp $ */
/*
 * cirrms/m82510.h: definitions for Intel M82510 serial controller
 */

#ifndef __ASSEMBLER__
typedef union {
    /*
     * Bank 0: 8250 compatible 
     */
    struct {
	unsigned char	dat;	unsigned :24;	/* rx data (r) / tx data (w) */
	unsigned char	ger;	unsigned :24;	/* gen. (i/u) enable (r/w) */
#define bal	dat				/* BRGA low (when DLAB=1) */
#define bah	ger				/* BRGA high */
	unsigned char	gir;	unsigned :24;	/* gen. i/u (r) / bank select (w) */
#define bank	nas.gir
	unsigned char	lcr;	unsigned :24;	/* line control */
	unsigned char	mcr;	unsigned :24;	/* modem control (r/w) */
	unsigned char	lsr;	unsigned :24;	/* line status (r/w) */
	unsigned char	msr;	unsigned :24;	/* modem status (r/w) */
	unsigned char	acr0;	unsigned :24;	/* addr/ctrl char #0 */
    } nas;


    /*
     * Bank 1: Working registers
     */
    struct {
	unsigned char	dat;	unsigned :24;	/* rx data (r) / tx data (w) */
	unsigned char	flg;	unsigned :24;	/* rx flags (r) / tx flags (w) */
	unsigned char	gir;	unsigned :24;	/* gen. i/u (r) / bank select (w) */
	unsigned char	tmst_cr; unsigned :24;	/* timr stat (r) / timr ctrl (w) */
	unsigned char	flr_mcr; unsigned :24;	/* fifo level (r) / mdm ctrl (w) */
	unsigned char	rst_rcm; unsigned :24;	/* rx stat (r) / rx cmd (w) */
	unsigned char	msr_tcm; unsigned :24;	/* mdm stat (r) / tx cmd (w) */
	unsigned char	gsr_icm; unsigned :24;	/* gen. stat (r) / int. cmd (w) */
    } wrk;

    /*
     * Bank 2: General configuration
     */
    struct {
	unsigned char	xxx;	unsigned :24;	/* unused */
	unsigned char	fmd;	unsigned :24;	/* fifo mode (r/w) */
	unsigned char	gir;	unsigned :24;	/* gen. i/u (r) / bank select (w) */
	unsigned char	tmd;	unsigned :24;	/* tx mode (r/w) */
	unsigned char	imd;	unsigned :24;	/* internal mode (r/w) */ 
	unsigned char	acr1;	unsigned :24;	/* addr/ctrl char #1 */
	unsigned char	rie;	unsigned :24;	/* rx i/u enable (r/w) */
	unsigned char	rmd;	unsigned :24;	/* rx mode (r/w) */
    } gcfg;

    /*
     * Bank 3: Modem configuration
     */
    struct {
	unsigned char	clcf;	unsigned :24;	/* clock config (r/w) */
	unsigned char	bacf;	unsigned :24;	/* BRGA config (r/w) */
#define bbl	clcf				/* BRGB low (when DLAB=1) */
#define bbh	bacf				/* BRGB high */
	unsigned char	gir;	unsigned :24;	/* gen. i/u (r) / bank select (w) */
	unsigned char	bbcf;	unsigned :24;	/* BRGB config (r/w) */
	unsigned char	pmd;	unsigned :24;	/* i/o pin mode (r/w) */
	unsigned char	mie;	unsigned :24;	/* modem i/u enable (r/w) */
	unsigned char	tmie;	unsigned :24;	/* timer i/u enable (r/w) */
	unsigned char	xxx;	unsigned :24;	/* unused */
    } mcfg;

} m82510dev;

#else 

    /*
     * Bank 0: 8250 compatible 
     */
#define	NAS_DAT		0		/* Tx/Rx data */
#define	NAS_GER		4		/* gen. (i/u) enable (r/w) */ 
#define NAS_BAL		NAS_DAT		/* BRGA low (when DLAB=1) */
#define NAS_BAH		NAS_GER		/* BRGA high */
#define	NAS_GIR		8		/* gen. i/u (r) / bank select (w) */
#define BANK		NAS_GIR
#define	NAS_LCR		12		/* line control */
#define	NAS_MCR		16		/* modem control (r/w) */
#define	NAS_LSR		20		/* line status (r/w) */
#define	NAS_MSR		24		/* modem status (r/w) */
#define	NAS_ACR0	28		/* addr/ctrl char #0 */


    /*
     * Bank 1: Working registers
     */
#define WRK_DAT		0		/* rx/tx data */
#define WRK_FLG		4		/* rx/tx flags */
#define WRK_GIR		8		/* gen. i/u (r) / bank select (w) */
#define WRK_TMST 	12		/* timr stat (r) */
#define WRK_TMCR 	12		/* timr control (w) */
#define WRK_FLR 	16		/* fifo level (r) */
#define WRK_MCR 	16		/* mdm ctrl (w) */
#define WRK_RST 	20		/* rx stat (r) */
#define WRK_RCM 	20		/* rx cmd (w) */
#define WRK_MSR 	24		/* mdm stat (r) */
#define WRK_TCM 	24		/* tx cmd (w) */
#define WRK_GSR 	28		/* gen. stat (r) */
#define WRK_ICM 	28		/* int. cmd (w) */

    /*
     * Bank 2: General configuration
     */
#define GCFG_XXX	0		/* unused */
#define GCFG_FMD	4		/* fifo mode (r/w) */
#define GCFG_GIR	8		/* gen. i/u (r) / bank select (w) */
#define GCFG_TMD	12		/* tx mode (r/w) */
#define GCFG_IMD	16		/* internal mode (r/w) */ 
#define GCFG_ACR1	20		/* addr/ctrl char #1 */
#define GCFG_RIE	24		/* rx i/u enable (r/w) */
#define GCFG_RMD	28		/* rx mode (r/w) */

    /*
     * Bank 3: Modem configuration
     */
#define MCFG_CLCF	0		/* clock config (r/w) */
#define MCFG_BACF	4		/* BRGA config (r/w) */
#define MCFG_BBL	MCFG_CLCF	/* BRGB low (when DLAB=1) */
#define MCFG_BBH	MCFG_BACF	/* BRGB high */
#define MCFG_GIR	8		/* gen. i/u (r) / bank select (w) */
#define MCFG_BBCF	12		/* BRGB config (r/w) */
#define MCFG_PMD	16		/* i/o pin mode (r/w) */
#define MCFG_MIE	20		/* modem i/u enable (r/w) */
#define MCFG_TMIE	24		/* timer i/u enable (r/w) */
#define MCFG_XXX	28		/* unused */

#endif /* __ASSEMBLER__ */

#define BRTC(xtal, rate)	((xtal) / 16 / rate)

/* GIR - General Interrupt / Bank Register */
#define GIR_BANK	0x60		/* Bank select */
#define  GIR_BANK_0	 (0<<5)
#define  GIR_BANK_1	 (1<<5)
#define  GIR_BANK_2	 (2<<5)
#define  GIR_BANK_3	 (3<<5)
#define GIR_VEC		0x0e		/* Highest prio int type */
#define GIR_VEC_SHIFT	1
#define GIR_IPN		0x01		/* Interrupt Pending */

#define  BANK_NAS	 GIR_BANK_0
#define  BANK_WRK	 GIR_BANK_1
#define  BANK_GCFG	 GIR_BANK_2
#define  BANK_MCFG	 GIR_BANK_3

/*
 * BANK 0: 8250 COMPATIBILITY
 */

/* GER - General (Interrupt) Enable Register */
#define GER_TIE		0x20		/* Timers i/u enable */
#define GER_TXIE	0x10		/* Tx i/u enable */
#define GER_MIE		0x08		/* Modem i/u enable */
#define GER_RXIE	0x04		/* Rx i/u enable */
#define GER_TFIE	0x02		/* Tx fifo i/u enable */
#define GER_RFIE	0x01		/* Rx fifo i/u enable */

/* LCR - Line Control Register */
#define LCR_DLAB	0x80		/* Divisor latch */
#define LCR_SBK		0x40		/* Set break */
#define LCR_PARFORCE	0x20		/* Force parity bit */
#define LCR_PAREVEN	0x10		/* Even or low parity */
#define LCR_PARENB	0x08		/* Parity enable */
#define LCR_PARNONE	0x00		/* Parity enable */
#define LCR_SBL0	0x04		/* Stop bit length (bit 0) */
#define LCR_CL		0x03		/* Character length */
#define  LCR_CL5	 0x00
#define  LCR_CL6	 0x01
#define  LCR_CL7	 0x02
#define  LCR_CL8	 0x03

/* MCR - Modem Control Register */
#define MCR_OUT0	0x20		/* OUT0 output bit */
#define MCR_LC		0x10		/* Loopback control */
#define MCR_OUT2	0x08		/* OUT2 output bit */
#define MCR_OUT1	0x04		/* OUT1 output bit */
#define MCR_RTS		0x02		/* RTS output */
#define MCR_DTR		0x01		/* DTR output */

/* LSR - Line Status Register */
#define LSR_TXST	0x40		/* Tx status */
#define LSR_TFST	0x20		/* Tx fifo status */
#define LSR_BKD		0x10		/* Break detected */
#define LSR_FE		0x08		/* Framing error */
#define LSR_PE		0x04		/* Parity error */
#define LSR_OE		0x02		/* Overrun error */
#define LSR_RFIR	0x01		/* Rx fifo interrupt */

/* MSR - Modem Status Register */
#define MSR_DCD		0x80            /* DCD active */
#define MSR_RI		0x40            /* RI active */ 
#define MSR_DSR		0x20            /* DSR active */
#define MSR_CTS		0x10            /* CTS active */
#define MSR_DDCD	0x08            /* DCD changed */
#define MSR_DDRI	0x04            /* RI changed */ 
#define MSR_DDSR	0x02            /* DSR changed */
#define MSR_DCTS	0x01            /* CTS changed */

/*
 * BANK 1: WORKING REGISTERS 
 */

/* RXF - Received Character Flags (read only) */
#define RXF_ROK		0x40		/* Received char OK */
#define RXF_RXN		0x20		/* Received char noisy */
#define RXF_RPE		0x10		/* Received char parity error */
#define RXF_ACR		0x08		/* Address/control char marker */
#define RXF_BKF		0x04		/* Break flag */
#define RXF_RFE		0x02		/* Received char framing error */
#define RXF_RND		0x01		/* Ninth bit of received char */

/* TXF - Transmit Flags Register (write only) */
#define TXF_ULAN	0x80		/* Address marker bit */
#define TXF_SP		0x40		/* Software parity bit */
#define TXF_D8		0x20		/* Ninth data bit */

/* TMST - Timer Status Register (read only) */
#define TMST_GBS	0x20		/* Gate B counting enabled */
#define TMST_GAS	0x10		/* Gate A counting enabled */
#define TMST_TBEX	0x02		/* Timer B expired */
#define TMST_TAEX	0x01		/* Timer A expired */

/* TMCR - Timer Control Register (write only) */
#define TMCR_TGB	0x20		/* Timer B Gate enable */
#define TMCR_TGA	0x10		/* Timer A Gate enable */
#define TMCR_STB	0x02		/* Start Timer B */
#define TMCR_STA	0x01		/* Start Timer A */

/* FLR - FIFO Level Register (read only) */
#define FLR_RFL		0x70
#define FLR_RFL_SHIFT	4
#define FLR_TFL		0x07
#define FLR_TFL_SHIFT	0

/* MCR - Modem Control Register (write only) */
/* see bank 0 above */

/* RST - Receive Status Register (read only) */
#define RST_CRF		0x80		/* Address char received */
#define RST_PCRF	0x40		/* Programmed address char received */
#define RST_BKT		0x20		/* Break terminated */
#define RST_BKD		0x10		/* Break detected */
#define RST_FE		0x08		/* Framing error */
#define RST_PE		0x04		/* Parity error */
#define RST_OE		0x02		/* Overrun error */
#define RST_RFIR	0x01		/* Rx fifo interrupt */

/* RCM - Receive Command Register (write only) */
#define RCM_RXEN	0x80		/* Rx enable */
#define RCM_RXDI	0x40		/* Rx disable */
#define RCM_FRM		0x20		/* Flush receiver */
#define RCM_FRF		0x10		/* Flush Rx fifo */
#define RCM_LRF		0x08		/* Lock Rx fifo */
#define RCM_ORF		0x04		/* Open Rx fifo */

/* MSR - Modem Status Register (read only) */
/* see bank 0 above */

/* TCM - Transmit Command Register (write only) */
#define TCM_FTM		0x08		/* Flush transmitter */
#define TCM_FTF		0x04		/* Flush Tx fifo */
#define TCM_TXEN	0x02		/* Tx enable */
#define TCM_TXDI	0x01		/* Tx disable */

/* GSR - General Status Register (read only) */
#define GSR_TIR		0x20		/* Timers interrupt request */
#define GSR_TXIR	0x10		/* Tx interrupt request */
#define GSR_MIR		0x08		/* Modem interrupt request */
#define GSR_RXIR	0x04		/* Rx interrupt request */
#define GSR_TFIR	0x02		/* Tx fifo interrupt request */
#define GSR_RFIR	0x01		/* Rx fifo interrupt request */

/* ICM - Internal Command Register (write only) */
#define ICM_SRST	0x10		/* Software reset */
#define ICM_INTA	0x08		/* Interrupt acknowledge */
#define ICM_STC		0x04		/* Status clear */
#define ICM_PDM		0x02		/* Power down */

/* 
 * BANK 2: GENERAL CONFIGURATION 
 */

/* FMD - FIFO Mode Register (read/write) */
#define FMD_RFT		0x30
#define FMD_RFT_SHIFT	4
#define FMD_TFT		0x03
#define FMD_TFT_SHIFT	0

/* TMD - Transmit Mode Register (read/write) */
#define TMD_EED		0x80		/* Error echo disable */
#define TMD_CED		0x40		/* Control character echo disable */
#define TMD_NBCL	0x20		/* Nine-bit length */
#define TMD_TM		0x18		/* Transmit mode: */
#define  TMD_TM_MANUAL	 0x00		   /* manual */
#define  TMD_TM_SEMI	 0x10		   /* semi-automatic */
#define  TMD_TM_AUTO	 0x18		   /* automatic */
#define TMD_SPF		0x04		/* Software parity force */
#define TMD_SBL2	0x02		/* Stop bit length (bit 2) */
#define TMD_SBL1	0x01		/* Stop bit length (bit 1) */

/* IMD - Internal Mode Register (read/write) */
#define IMD_IAM		0x08		/* Auto acknowledge on interrupt */
#define IMD_RFD		0x04		/* Fifo depth */
#define  IMD_RFD_1	 0x04		  /* 1 byte */
#define  IMD_RFD_4	 0x00		  /* 4 bytes */
#define IMD_ULM		0x02		/* uLAN mode: use MCS-51 protocol */
#define IMD_LEM		0x01		/* Loopback/echo mode */

/* RIE - Receive Interrupt Enable Register (read/write) */
#define RIE_CRE		0x80		/* Address char recognition enable */
#define RIE_PCRE	0x40		/* Address char match enable */
#define RIE_BKTE	0x20		/* Break termination enable */
#define RIE_BKDE	0x10		/* Break detect enable */
#define RIE_FEE		0x08		/* Framing error enable */
#define RIE_PEE		0x04		/* Parity error enable */
#define RIE_OEE		0x02		/* Overrun error enable */

/* RMD - Receive Mode Register (read/write) */
#define RMD_UCM		0xc0		/* uLAN control character recognition: */
#define  RMD_UCM_MANUAL	 0x00		   /* manual */
#define  RMD_UCM_SEMI	 0x40		   /* semi-automatic */
#define  RMD_UCM_AUTO	 0x80		   /* automatic */
#define RMD_DPD		0x20		/* Disable digital phase lock loop */
#define RMD_SWM		0x10		/* Large sampling window */
#define RMD_SSM		0x08		/* Normal start bit sampling */

/*
 * BANK 3: MODEM CONFIGURATION
 */

/* CLCF - Clocks Configuration Register (read/write) */
#define CLCF_RXCM	0x80		/* Rx clock: set=1x clear=16x */
#define CLCF_RXCS	0x40		/* Rx clock: set=BRGA clear=BRGB */
#define CLCF_TXCM	0x20		/* Tx clock: set=1x clear=16x */
#define CLCF_TXCS	0x10		/* Tx clock: set=BRGA clear=BRGB */

/* BACF - BRGA Configuration Register (read/write) */
#define BACF_BACS	0x40		/* BRGA source: */
#define  BACF_BACS_CLOCK 0x00		   /* System clock */
#define  BACF_BACS_SCLK	 0x40		   /* SCLK pin */
#define BACF_BAM	0x04		/* BRGA mode: set=BRG clear=timer */

/* BBCF - BRGB Configuration Register (read/write) */
#define BBCF_BBCS	0xc0		/* BRGB source: */
#define  BBCF_BBCS_CLOCK 0x00		   /* System clock */
#define  BBCF_BBCS_SCLK	 0x40		   /* SCLK pin */
#define  BBCF_BBCS_BRGA	 0x80		   /* BRGA output */
#define BBCF_BBM	0x04		/* BRGB mode: set=BRG clear=timer */

/* PMD - I/O Pin Mode Register (read/write) */
#define PMD_DIOD	0x80		/* DCD/ICLK/OUT1 direction: set=input */
#define PMD_DIOF	0x40		/* DCD/ICLK/OUT1 function: set=general */
#define PMD_DTAD	0x20		/* DSR/TA/OUT0 direction */
#define PMD_DTAF	0x10		/* DSR/TA/OUT0 function */
#define PMD_RRF		0x08		/* RI/SCLK function */
#define PMD_DTF		0x04		/* DTR/TB function */

/* MIE - Modem Interrupt Enable Register (read/write) */
#define MIE_DCDE	0x08		/* DCD interrupt enable */
#define MIE_RIE		0x04		/* RI interrupt enable */
#define MIE_DSRE	0x02		/* DSR interrupt enable */
#define MIE_CTSE	0x01		/* CTS interrupt enable */

/* TMIE - Timer Interrupt Enable Register (read/write) */
#define TMIE_TBIE	0x02		/* Timer B interrupt enable */
#define TMIE_TAIE	0x01		/* Timer A interrupt enable */
