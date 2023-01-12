#ident "$Header: /vol/cvsroot/pmon/pmon/libraio/sbd.h,v 1.1 1996/06/28 12:29:54 nigel Exp $"

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
 * sbd.h: Libra board information
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

#define G10_PIO_UARTINT0	0x08 /* input:  uart #0 interrupt */
#define G10_PIO_UARTINT1	0x10 /* input:  uart #1 interrupt */
#define INT_UART0		INT_PIO_3
#define INT_UART1		INT_PIO_4


#ifdef __ASSEMBLER__
#define WBFLUSH \
	.set nomove; \
        lw zero, PHYS_TO_K1(BOOTPROM); \
	.set move
#endif
