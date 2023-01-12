/* $Id: sbd.h,v 1.3 1996/03/11 12:39:04 chris Exp $ */
/*
 * sbd.h: XDS board information
 *
 *	Copyright (c) 1995 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *
 */

#ifndef _SBD_
#define _SBD_

#ifndef MHZ
#define MHZ		33			/* 33MHz sysclk */
#endif

/* 
 * ROM/RAM and fixed memory address spaces etc.
 */
#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x04000000 	/* Local memory size (64Mb max) */

#define LOCAL_PROM	0x1fc00000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00080000	/* PROM decode size */

#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00100000	/* Boot Prom size (1Mb)*/

/*
 * Device Address Spaces
 */

#define P9100N_BASE	0x04060000	/* P9100 'native' */
#define FRAME_BASE	0x04800000	/* Frame buffer */
#define SONIC_BASE	0x05000000	/* SONIC */
#define P9100C_BASE	0x05800000	/* P9100 config */
#define VME_BASE	0x06000000	/* VME */
#define KBDMSE_BASE	0x06800000	/* Keyboard/mouse */
#define SRAM_BASE	0x07000000
#define FLASH_BASE	0x07800000 	/* FLASH PROM */

#define SRAM_VME	(SRAM_BASE+0x80000) /* 256Kb VME shared mem buffers */
#define SRAM_VME_SIZE	0x40000 
#define SRAM_NET	(SRAM_BASE+0xC0000) /* 252Kb network buffers */
#define SRAM_NET_SIZE	0x3C000
#define SRAM_ENV	(SRAM_BASE+0xFC000) /* 4Kb environment */
#define SRAM_ENV_SIZE	0x04000

#define PTTY_BASE	(SRAM_VME-0x10)
#define IOCHAN0_BASE	(SRAM_VME+0x00000)
#define IOCHAN1_BASE	(SRAM_VME+0x10000)
#define IOCHAN2_BASE	(SRAM_VME+0x20000)
#define IOCHAN3_BASE	(SRAM_VME+0x30000)

/* VME registers */
#define VME_IRQ1GEN	(VME_BASE+0x0c)
#define VME_IRQ2GEN	(VME_BASE+0x14)
#define VME_IRQ3GEN	(VME_BASE+0x1c)
#define VME_IRQ4GEN	(VME_BASE+0x24)
#define VME_IRQ5GEN	(VME_BASE+0x2c)
#define VME_IRQ6GEN	(VME_BASE+0x34)
#define VME_IRQ7GEN	(VME_BASE+0x3c)
#define VME_CONF	(VME_BASE+0x4c)
#define VME_STAT	(VME_BASE+0x54)
#define VME_BITESTAT	(VME_BASE+0x64)
#define VME_BNETSTAT	(VME_BASE+0x6c)
#define VME_IPEND	(VME_BASE+0x74)
#define VME_IMASK	(VME_BASE+0x7c)
#define VME_SEM0	(VME_BASE+0x84)
#define VME_SEM1	(VME_BASE+0x8c)
#define VME_SEM2	(VME_BASE+0x94)
#define VME_SEM3	(VME_BASE+0x9c)
#define VME_SEM4	(VME_BASE+0xa4)
#define VME_SEM5	(VME_BASE+0xac)
#define VME_SEM6	(VME_BASE+0xb4)
#define VME_SEM7	(VME_BASE+0xbc)
#define VME_P9100_RES	(VME_BASE+0xcc)
#define VME_SONIC_RES	(VME_BASE+0xd4)
#define VME_RAMDAC_RES	(VME_BASE+0xdc)
#define VME_KEYMSE_RES	(VME_BASE+0xe4)
#define VME_XDSREV	(VME_BASE+0xfc)

#define VME_REG(r)	((volatile unsigned int *)r)

#define RESET_ACTIVE	0xff
#define RESET_INACTIVE	0x00

#define SWITCH_BOOTFLASH	0x08
#define SWITCH_FLASHWRITE	0x04
#define SWITCH_DEBUG		0x02
#define SWITCH_PWRONRESET	0x01

/* SONIC */
#define SN_REG(x) \
	unsigned int :32; unsigned short :16; unsigned short x
#if XDSSONICBUG
#define SN_MEM(x) unsigned int x; unsigned int :32
#else
#define SM_MEM(X) unsigned int x
#endif

/* FIXME - all of these are pure guesswork */
#define CYCLETIME	((1000+MHZ/2)/MHZ)	/* cycle time in ns */
#define CACHECYCLE	CYCLETIME		/* cache cycle time in ns */

#define RAMCYCLE	70			/* 70ns dram */
#define ROMCYCLE	150			/* 150ns rom */

#define ASMDELAY(ns,icycle)	(((ns) + (icycle)) / ((icycle) * 2))
#define CACHEMISS		(CYCLETIME * 6)
#define CACHEUS			ASMDELAY(1000, CACHECYCLE)
#define RAMUS			ASMDELAY(1000, CACHEMISS+RAMCYCLE)
#define ROMUS			ASMDELAY(1000, CACHEMISS+ROMCYCLE)
#define CACHEMS			ASMDELAY(1000000, CACHECYCLE)
#define RAMMS			ASMDELAY(1000000, CACHEMISS+RAMCYCLE)
#define ROMMS			ASMDELAY(1000000, CACHEMISS+ROMCYCLE)
#endif /* _SBD_ */
