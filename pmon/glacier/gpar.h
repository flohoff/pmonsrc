/*
 * gpar.h: Grin Serial Function
 */

#ifdef __ASSEMBLER__

/* offsets from base register */
#define GPARW(x)		(x)

#else /* !__ASSEMBLER */

/* offsets from base pointer, this construct allows optimisation */
#define pGPAR(v) void *const _gparp = (void * const)(v)

#define GPARW(x)		*(volatile unsigned long *)(_gparp + (x))

#endif /* __ASSEMBLER__ */

#ifndef GPAR_HZ
#define GPAR_HZ	PCI_HZ		/* FIXME what is the real input clock? */
#endif

/* FIXME: This is formula is correct (I think) but gives the wrong answer */
#define GPARPRESCALE (((GPAR_HZ+1650000)/3300000)-1)
#define GPAR500NS    (GPAR_HZ/2000000)

#define	GPAR_DATAIN		GPARW(0x00000)	/* Data In */
#define	GPAR_STATUS		GPARW(0x00004)	/* Status */
#define	GPAR_DATA		GPARW(0x00008)	/* Data */
#define	GPAR_DATAOUT		GPARW(0x0000c)	/* Data Out */
#define	GPAR_CONTROL		GPARW(0x00010)	/* Control */
#define	GPAR_500NS		GPARW(0x00014)	/* 500ns Timer */
#define	GPAR_RLECOUNT		GPARW(0x00018)	/* RLE Count */
#define	GPAR_ECPTHROTTLE	GPARW(0x0001c)	/* ECP Throttle */
#define	GPAR_ISTATUS		GPARW(0x00020)	/* Interrupt Status */
#define	GPAR_IMASK		GPARW(0x00024)	/* Interrupt Mask */
#define	GPAR_IDMABCOUNT		GPARW(0x00028)	/* Input DMA Byte Count */
#define	GPAR_IDMATIMEOUT	GPARW(0x0002c)	/* Input DMA Timeout Count */
#define	GPAR_IDMACSR		GPARW(0x00030)	/* Input DMA Control/Status */
#define	GPAR_IDMAADDR		GPARW(0x00034)	/* Input DMA Address */
#define	GPAR_ODMABCOUNT		GPARW(0x00038)	/* Output DMA Byte Count */
#define	GPAR_ODMACSR		GPARW(0x0003c)	/* Output DMA Control/Status */
#define	GPAR_ODMAADDR		GPARW(0x00040)	/* Output DMA Address */
#define	GPAR_PRESCALE		GPARW(0x00044)	/* Prescaler */

#define	GPAR_PIENABLE		GPARW(0x00020)	/* Port Interrupt Enable */
#define	GPAR_PIPEND		GPARW(0x00020)	/* Port Interrupt Pending */
#define	GPAR_PIPOST		GPARW(0x00020)	/* Port Interrupt Post */
#define	GPAR_PIACK		GPARW(0x00020)	/* Port Interrupt Acknowledge */
#define	GPAR_PIMASK		GPARW(0x00020)	/* Port Interrupt Mask */


#define GPAR_STATUS_COMPAT	0x00008000
#define GPAR_STATUS_RXEMPTY	0x00004000
#define GPAR_STATUS_TXEMPTY	0x00002000
#define GPAR_STATUS_RXREADY	0x00001000
#define GPAR_STATUS_nAUTOFD	0x00000800
#define GPAR_STATUS_nSELECTIN	0x00000400
#define GPAR_STATUS_nINIT	0x00000200
#define GPAR_STATUS_nSTROBE	0x00000100

#define GPAR_CONTROL_EXTINT	0x00400000
#define GPAR_CONTROL_HOSTCLK	0x00200000
#define GPAR_CONTROL_ECPSPECIAL	0x00100000
#define GPAR_CONTROL_PERIPH	0x00080000
#define GPAR_CONTROL_REVBLOCK	0x00040000
#define GPAR_CONTROL_NOHIGHDRV	0x00010000
#define GPAR_CONTROL_ECPBSYQUAL	0x00008000
#define GPAR_CONTROL_PLH	0x00004000
#define GPAR_CONTROL_EXTRESP	0x00002000
#define GPAR_CONTROL_CLRDATAIN	0x00001000
#define GPAR_CONTROL_CLRDATAOUT	0x00000800
#define GPAR_CONTROL_REVAVAIL	0x00000400
#define GPAR_CONTROL_RCPDATA	0x00000200
#define GPAR_CONTROL_STRBSEL	0x00000100
#define GPAR_CONTROL_STRBINIT	0x00000080
#define GPAR_CONTROL_IEEE1284	0x00000040
#define GPAR_CONTROL_FASTEN	0x00000020
#define GPAR_CONTROL_BUSY	0x00000010
#define GPAR_CONTROL_nACK	0x00000008
#define GPAR_CONTROL_SELECT	0x00000004
#define GPAR_CONTROL_PERROR	0x00000002
#define GPAR_CONTROL_nFAULT	0x00000001

#define GPAR_500NS_SHIFT	24

#define GPAR_PRESCALE_ENABLE	0x00000010

