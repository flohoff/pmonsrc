/* $Id: sbd.h,v 1.2 1996/01/16 14:24:10 chris Exp $ */
/* 
 * algvme/sbd.h: Algorithmics VME board information
 */

#ifndef _ALGVME_SBD_
#define _ALGVME_SBD_

#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x10000000 	/* Local memory size (256Mb max) */

#define VME_A24D16_BASE	0x10000000 	/* VME A24D16 accesses */
#define VME_A24D32_BASE	0x11000000 	/* VME A24D32 accesses */
#define VME_A24_SIZE	0x02000000	/* 32Mb dedicated A24 address space */

#define LOCAL_PROM	0x1f000000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00f00000	/* PROM decode size (15Mb max)*/

#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00080000	/* Boot Prom size (512k max)*/

#define VME_A16AM2D	0x1ffe0000 	/* VMEbus A16 AM=0x2d (supervisor) */
#define VME_A16_SIZE	0x00010000	/* VMEbus A16 64Kb */

/*
 * Device Address Spaces
 */
#define SCSI_BASE	0x1ff00000	/* SCSI controller */
#define SONIC_BASE	0x1ff20000	/* Ethernet controller */
#define BCRR		0x1ff40000 	/* Board Configuration/Reset Reg */
#define IACK_BASE	BCRR		/* Interrupt acknowledge */
#define MPSC_BASE	0x1ff44000	/* uPD72001 serial controller */
#define ALPHA_BASE	0x1ff48000 	/* Alphanumeric Display */
#define LED_BASE	0x1ff48000 	/* SL3000 LED Display */
#define ERRREG		0x1ff4c000	/* Error Register (VME3000 only) */
#define RTCLOCK_BASE	0x1ff60000	/* Mk48T02 NVRAM/RTC */
#define IACK_BASE2	0x1ffa0000	/* alternative interrupt ack */
#define VIC_BASE	0x1ffc0000	/* VIC device */
#define VAC_BASE	0x1ffd0000	/* VAC device */

/*
 * Configuration/reset register (write-only)
 */
#ifdef SL3000
#define BCRR_LBLK	0x01000000	/* SL3000 LED blank (active high) */
#else
#define BCRR_AUBLK	0x01000000	/* display blank (active low)*/
#define BCRR_AUCLR	0x02000000	/* display clear (active low) */
#define BCRR_ACSR	0x04000000	/* cursor enable */
#endif
#define BCRR_USART	0x08000000	/* enable uPD72001 usart */
#define BCRR_ETH	0x10000000	/* enable SONIC ethernet */
#define BCRR_SCSI	0x20000000	/* enable NCR SCSI */

/* Alphanumeric display */
#ifndef SL3000
/* offsets to display entries */
#define ALPHA_ENTRY(n)	(16 + ((3 - (n)) * 4))
#define ALPHA_DISP(n)	(ALPHA_BASE + ALPHA_ENTRY(n))
#endif

/* Error register (VME3000 only) */
#define ERR_CYC_MASK	0x38000000
#define ERR_CYC_SHIFT	27
#define ERR_CYC_NONE	0
#define ERR_CYC_CPU	1
#define ERR_CYC_IO	2
#define ERR_CYC_CPE	3
#define ERR_CYC_BRST4	4
#define ERR_CYC_BRST8	5
#define ERR_CYC_BRST16	6
#define ERR_CYC_BRST32	7
#define ERR_CYC_BMASK	0x20000000
#define ERR_ADDR_SHIFT	2
#define ERR_PAIR_SHIFT	5
#define ERR_B0A_MASK	0x07ffffe0
#define ERR_B4A_MASK	0x07ffffc0
#define ERR_B4P_MASK	0x00000020
#define ERR_B8A_MASK	0x07ffff80
#define ERR_B8P_MASK	0x00000060
#define ERR_B16A_MASK	0x07ffff00
#define ERR_B16P_MASK	0x000000e0
#define ERR_B32A_MASK	0x07fffe00
#define ERR_B32P_MASK	0x000001e0
#define ERR_BANK_MASK	0x00000010
#define ERR_BANKA	0x00000010
#define ERR_BANKB	0x00000000
#define ERR_BYTE3	0x00000008
#define ERR_BYTE2	0x00000004
#define ERR_BYTE1	0x00000002
#define ERR_BYTE0	0x00000001

#endif /* _ALGVME_SBD_ */

