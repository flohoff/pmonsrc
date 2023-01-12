#ident "$Header: /vol/cvsroot/pmon/pmon/laserfold/sbd.h,v 1.2 1997/02/20 15:02:26 nigel Exp $"

/*
 *	
 *	Copyright (c) 1994 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */

/*
 * sbd.h: Citadel / Europatech laser printer rip - board information
 */

#define RAMCYCLE	80			/* 80ns dram */
#define ROMCYCLE	150			/* 150ns rom */

/* 
 * ROM/RAM and fixed memory address spaces etc.
 */
#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x02400000 	/* Local memory size (36Mb max) */
#define LOCAL_PROM	0x1f400000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00800000	/* PROM decode size (12Mb max)*/
#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00400000	/* Boot Prom size (4Mb max)*/

#include <g10.h>

/* 2 x 16450 uarts on 8-bit bus D7:D0 */
#define UART0_BASE	(IOCHAN0_BASE)
#define UART1_BASE	(IOCHAN1_BASE)

#define NS16550_HZ	1843200

/* big-endian cpu with uart on 8-bit bus D7:D0 */
#define nsreg(x)	unsigned :24; unsigned char x;
#define NSREG(x) 	(((x)<<2)+3)

/* i8255A parallel i/o on 16-bit bus */
#define PIO_BASE	(IOGPCHAN0_BASE)

/* Font ROM in ROM1 space (512KB = 2MB used space) */
#define FONTROM_BASE	0x1fa00000

#define G10_PIO_LSYNC		0x01 /* output: wire-ored with printer lsync */
#define G10_PIO_VSYNCOUT	0x02 /* output: to g10 pagintr pin */
#define G10_PIO_VSYNCIN		0x04 /* input:  printer vsreq signal */
#define G10_PIO_UARTINT0	0x08 /* input:  uart #0 interrupt */
#define G10_PIO_UARTINT1	0x10 /* input:  uart #1 interrupt */
#define G10_PIO_CENBUSY		0x20 /* output: centronics busy */

#define INT_VSYNCIN		INT_PIO_2
#define INT_UART0		INT_PIO_3
#define INT_UART1		INT_PIO_4

/*
 * ENGINE CONTROL register (r/o)
 */
#define	ENGN_IN_NSTS	0x08	/* status bit */
#define ENGN_IN_NREADY	0x04	/* !printer ready */
#define ENGN_IN_PPRDY	0x02	/* printer power ready */
#define ENGN_IN_NSTBSY	0x01	/* !status busy */

/*
 * ENGINE CONTROL register (w/o)
 */
#define ENGN_OUT_NSTLED	0x40	/* l.e.d */
#define ENGN_OUT_NCMD	0x20	/* command bit */
#define ENGN_OUT_NSCLK	0x10	/* !serial clock */
#define ENGN_OUT_NCMBSY	0x08	/* !command busy */
#define ENGN_OUT_NPRINT	0x04	/* !print request */
#define ENGN_OUT_NVSYNC	0x02	/* !vertical synch */
#define ENGN_OUT_CPRDY	0x01	/* controller power ready */

#ifdef __ASSEMBLER__
#define WBFLUSH \
	.set nomove; \
        lw zero, PHYS_TO_K1(BOOTPROM); \
	.set move
#endif
