/*
 * gser.h: Grin Serial Function
 */

#ifdef __ASSEMBLER__

/* offsets from base register */
#define GSERW(x)		(x)

#else /* !__ASSEMBLER */

/* offsets from base pointer, this construct allows optimisation */
#define pGSER(v) void *const _gserp = (void * const)(v)

#define GSERW(x)		*(volatile unsigned long *)(_gserp + (x))

#endif /* __ASSEMBLER__ */

#ifndef GSER_HZ
#define GSER_HZ	PCI_HZ
#endif

#define GSERPRESCALE(baud) ((GSER_HZ+((baud)/2)*16)/((baud)*16))

#define GSER_CFG		GSERW(0x00000)
#define GSER_PRESCALE		GSERW(0x00004)
#define GSER_RXCSR		GSERW(0x00008)
#define GSER_TXCSR		GSERW(0x0000c)
#define GSER_DATA		GSERW(0x00010)
#define GSER_DMAADDR		GSERW(0x00020)
#define GSER_DMALENGTH		GSERW(0x00024)
#define GSER_DMACSR		GSERW(0x00028)
#define GSER_DMATIMEOUT		GSERW(0x0002c)
#define GSER_ADDRCNT		GSERW(0x00030)
#define GSER_LENGTHCNT		GSERW(0x00034)
#define GSER_PRESCALECNT	GSERW(0x00040)
#define GSER_DMATIMEOUTCNT	GSERW(0x00060)
#define GSER_IENABLE		GSERW(0x10000)
#define GSER_IPEND		GSERW(0x10004)
#define GSER_IPOST		GSERW(0x10008)
#define GSER_IACK		GSERW(0x1000c)
#define GSER_IMASK		GSERW(0x10010)

#define GSER_CFG_8BIT		0x0001
#define GSER_CFG_PE		0x0002
#define GSER_CFG_EP		0x0004
#define GSER_CFG_5WIRE		0x0008
#define GSER_CFG_IDTR		0x0010

#define GSER_RXCSR_SIE		0x0001
#define GSER_RXCSR_DMAE		0x0002
#define GSER_RXCSR_FULL		0x0004
#define GSER_RXCSR_PARITY	0x0008
#define GSER_RXCSR_FRAME	0x0010
#define GSER_RXCSR_OVERRUN	0x0020

#define GSER_TXCSR_SOE		0x0001
#define GSER_TXCSR_XOFF		0x0002
#define GSER_TXCSR_XON		0x0004
#define GSER_TXCSR_EMPTY	0x0008
#define GSER_TXCSR_DSR		0x0010
#define GSER_TXCSR_DTR		0x0020

