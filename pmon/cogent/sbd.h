/* $Id: sbd.h,v 1.3 1996/05/29 22:28:13 chris Exp $ */
/* 
 * sbd.h: COGENT board information
 */

#ifndef _COGENT_SBD_
#define _COGENT_SBD_

#ifndef MHZ
# error Please define MHZ
#endif

#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x02000000 	/* Local memory size (32Mb max) */

#define EXP1		0x02000000	/* I/O Expansion Slot 1 */
#define EXP1_SIZE	0x02000000	/* Expansion area size */

#define EXP2		0x04000000	/* I/O Expansion Slot 2 */
#define EXP2_SIZE	0x02000000	/* Expansion area size */

#define EXP3		0x06000000	/* I/O Expansion Slot 3 */
#define EXP3_SIZE	0x02000000	/* Expansion area size */

#define LOCAL_PROM	0x1fc00000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00080000	/* PROM decode size */

#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00080000	/* Boot Prom size (512k max)*/

/*
 * Device Address Spaces
 */
#define EPROM1_BASE	0x0e000000 	/* EPROM */
#define RTCLOCK_BASE	0x0e800000	/* Mk48T02 NVRAM/RTC */
#define UART_BASE	0x0e900000	/* NS16C552 DUART */
#define NIC_BASE	0x0ea00000	/* Ethernet controller */
#define LCD_BASE	0x0eb00000 	/* Alphanumeric display */
#define DIP_BASE	0x0ec00000 	/* DIP Switch */
#define IDROM1_BASE	0x0f100000 	/* Slot 1 IDROM */
#define IDROM2_BASE	0x0f100000 	/* Slot 2 IDROM */
#define IDROM3_BASE	0x0f100000 	/* Slot 3 IDROM */
#define ENDIAN_BASE	0x0f700000 	/* Set littleendian byte enables */
#define EPROM2_BASE	0x0f800000 	/* EPROM */

#define IOCHAN0_BASE	(UART_BASE+0x40)
#define IOCHAN1_BASE	(UART_BASE+0x00)

#define IP_UART		CR_HINT0	/* UART Interrupt pending  */
#define IP_NET		CR_HINT1	/* Network Interrupt pending  */
#define IP_SLOT1	CR_HINT2	/* Slot 1 Interrupt pending  */
#define IP_SLOT2_3	CR_HINT3	/* Slot 2/3 Interrupt pending  */

/* configuration information for ethernet controller */
#define FE_D4_INIT	(FE_D4_LBC_DISABLE | FE_D4_CNTRL)
#define FE_D5_INIT	0
/*
 * Program the 86960 as follows:
 *	SRAM: 32KB, 100ns, byte-wide access.
 *	Transmission buffer: 4KB x 2.
 *	System bus interface: 8 bits.
 * We cannot change these values but TXBSIZE, because they
 * are hard-wired on the board.  Modifying TXBSIZE will affect
 * the driver performance.
 */
#define FE_D6_INIT	(FE_D6_BUFSIZ_32KB | FE_D6_TXBSIZ_2x4KB | FE_D6_BBW_BYTE | FE_D6_SBW_BYTE | FE_D6_SRAM_100ns)
#define FE_D7_INIT	(FE_D7_BYTSWP_LH | FE_D7_IDENT_EC)

#define CYCLETIME	((1000+MHZ/2)/MHZ)	/* cycle time in ns */
#define CACHECYCLE	CYCLETIME		/* cache cycle time in ns */

#define RAMCYCLE	80			/* 80ns dram */
#define ROMCYCLE	150			/* 150ns rom */

#define ASMDELAY(ns,icycle)	(((ns) + (icycle)) / ((icycle) * 2))
#define CACHEMISS		(CYCLETIME * 6)
#define CACHEUS			ASMDELAY(1000, CACHECYCLE)
#define RAMUS			ASMDELAY(1000, CACHEMISS+RAMCYCLE)
#define ROMUS			ASMDELAY(1000, CACHEMISS+ROMCYCLE)
#define CACHEMS			ASMDELAY(1000000, CACHECYCLE)
#define RAMMS			ASMDELAY(1000000, CACHEMISS+RAMCYCLE)
#define ROMMS			ASMDELAY(1000000, CACHEMISS+ROMCYCLE)
#endif /* _COGENT_SBD_ */
