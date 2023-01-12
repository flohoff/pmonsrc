/* $Id: sbd.h,v 1.2 1996/01/16 14:25:13 chris Exp $ */
/* 
 * mann/sbd.h: mannesman RIP-CPU 2.1 board information
 */

#ifndef _MANN_SBD_
#define _MANN_SBD_

#define IOSPACE		0x00000000 	/* IO space base address */
#define IOSPACE_SIZE	0x00800000 	/* IO space decode size */

#define LOCAL_MEM	0x00800000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x07800000 	/* Local memory size (120Mb max) */

#define SOFTROM		0x08000000 	/* SoftRom writeable window */
#define SOFTROM_SIZE	0x00800000	/* SoftRom window size */

#define ORING_MEM	0x08800000	/* Oring window onto DRAM */
#define ORING_MEM_SIZE	0x07800000	/* Oring window size */

#define LOCAL_PROM	0x1fc00000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00400000	/* PROM decode size */

#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00400000	/* Boot Prom size (4Mb max)*/

#define BORING_MEM	0x20800000	/* Burst-Oring window onto DRAM */
#define BORING_MEM_SIZE	0x07800000	/* Burst-Oring window onto DRAM */

/*
 * Device Address Spaces
 */
#define CNF_BASE	0x00700000	/* Configuration register */
#define DBG_BASE	0x00700020	/* Debug/FIFO register */
#define Z8530_BASE	0x00730000	/* Z8530 UART */
#define M82510_BASE	0x00770000	/* 82510 UART */
#define LED_BASE	0x00701000	/* seven segment hex display */
#define LED_BLNK_	0x00701020	/* display blank (active low) */

/* register definitions */
#define CNF_UART	0x00000080	/* enable usart (wo) */
#define CNF_CLKRATE	0x000000e0	/* board clock rate (ro) */
#define CNF_CLKSHIFT	5
#define CNF_PARITY	0x00000010	/* enable parity rams (wo) */
#define CNF_PARITYRAMS	0x00000010	/* parity ram supported (ro) */
#define CNF_MEM64	0x00000004	/* limit memory to 64Mbytes (rw) */
#define CNF_WRPRSROM	0x00000002	/* write protect softROM (rw)  */
#define CNF_SROM	0x00000001	/* enable softROM (rw) */

#define DBG_FIFOMIN	0x000000c0	/* FIFO mark bits in (rw) */
#define DBG_FIFOMOUT	0x00000030	/* FIFO mark bits out (ro) */
#define DBG_FIFOINTEN	0x00000008	/* FIFO interrupt enable (rw) */
#define DBG_DBGINTEN    0x00000004	/* Debug interrupt enable (rw) */
#define DBG_FIFOINT	0x00000002	/* FIFO interrupt state/clear (rw) */
#define DBG_DBGINT	0x00000001	/* Debug interrupt state/clear (rw) */

#define SR_IBITDBG	SR_IBIT7
#define CAUSE_IPDBG	CAUSE_IP7

#define RESET_ONE	(~0)
#define RESET_ZERO	0

#define LED_ON		RESET_ZERO
#define LED_OFF		RESET_ONE

#endif /* _MANN_SBD_ */
