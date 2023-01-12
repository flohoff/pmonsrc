/* $Id: sbd.h,v 1.2 1996/01/16 14:25:17 chris Exp $ */
/* 
 * p4000/sbd.h: Algorithmics P4000 board information
 */

#ifndef _P4000_SBD_
#define _P4000_SBD_

#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x10000000 	/* Local memory size (256Mb max) */

#define EXP1		0x10000000	/* Daughterboard expansion area */
#define EXP1_SIZE	0x0f000000	/* Daughterboard expansion area */

#define LOCAL_PROM	0x1fc00000	/* PROM decode address */
#define LOCAL_PROM_SIZE	0x00080000	/* PROM decode size */

#define BOOTPROM	0x1fc00000	/* Boot Prom address */
#define BOOTPROM_SIZE	0x00080000	/* Boot Prom size (512k max)*/

#define EXP2		0x20000000	/* Daughterboard expansion area */
#define EXP2_SIZE	0xdf000000	/* Daughterboard expansion area */

/*
 * Device Address Spaces
 */
#define RESET_BASE	0x1f100000	/* Reset/Configuration */
#define ALPHN_BASE	0x1f200000 	/* Alphanumeric display */
#define MPSC_BASE	0x1f300000	/* NEC uPD72001 */
#define RTCLOCK_BASE	0x1f400000	/* Mk48T02 NVRAM/RTC */
#define SONIC_BASE	0x1f600000	/* Ethernet controller */
#define INTREG_BASE	0x1f700000	/* Interrupt registers */

#define ALPHN_CLR_	(RESET_BASE+0)  /* display clear (active low) */
#define ALPHN_CSR	(RESET_BASE+4)  /* display cursor enable */
#define ALPHN_BLNK_	(RESET_BASE+8)  /* display blank (active low) */
#define SIO_RESET_	(RESET_BASE+12) /* uPD72001 reset (active low) */
#define NET_RESET_	(RESET_BASE+16) /* SONIC reset (active low) */

#define RESET_ZERO	0
#define RESET_ONE	(~0)

/* Alphanumeric display */
#define ALPHA_BASE	(ALPHN_BASE + 16)
#define CURSOR_BASE	ALPHN_BASE
#define ALPHN_ENTRY(n)	((3 - (n)) << 2)
#define ALPHN_CURS(n)	(CURSOR_BASE + ALPHN_ENTRY(n))
#define ALPHN_CHAR(n)	(ALPHA_BASE + ALPHN_ENTRY(n))

/* interrupt registers */
#define INT_IRR0	(INTREG_BASE+0x00)	/* interrupt request lo (read) */
#define INT_IRR1	(INTREG_BASE+0x20)	/* interrupt request hi (read) */
#define INT_IMR0	(INTREG_BASE+0x00)	/* interrupt mask (write) */
#define INT_IMR1	(INTREG_BASE+0x04)	/* interrupt mask (write) */
#define INT_IMR2	(INTREG_BASE+0x08)	/* interrupt mask (write) */
#define INT_IMR3	(INTREG_BASE+0x0c)	/* interrupt mask (write) */
#define INT_ICG0	(INTREG_BASE+0x20)	/* interrupt group (write) */
#define INT_ICG1	(INTREG_BASE+0x24)	/* interrupt group (write) */
#define INT_ICG2	(INTREG_BASE+0x28)	/* interrupt group (write) */
#define INT_ICG3	(INTREG_BASE+0x2c)	/* interrupt group (write) */
#define INT_ACKPANIC	(INTREG_BASE+0x40)	/* interrupt acknowledge (read) */

#define IRR0_IOIRQ1	0x80000000		/* Daughterboard IRQ1 */
#define IRR0_IOIRQ0	0x40000000		/* Daughterboard IRQ0 */
#define IRR0_SIO	0x20000000		/* uPD72001 */
#define IRR0_NET	0x10000000		/* SONIC */
#define IRR0_BUS	0x08000000		/* bus timeout */
#define IRR0_ETHPAR	0x04000000		/* ethernet parity error */
#define IRR0_ACFAIL	0x02000000		/* AC fail */
#define IRR0_DEBUG	0x01000000		/* Debug button */
#define IRR1_IOIRQ3	0x02000000		/* Daughterboard IRQ3 */
#define IRR1_IOIRQ2	0x01000000		/* Daughterboard IRQ2 */

#define IRR(hi,lo)	(((unsigned int)(hi) & 0xff000000) | \
			 ((((unsigned int)(lo))>>8)&0x00ff0000))

#define IMR_ENABLE	0			/* enable interrupt */
#define IMR_DISABLE	(~0)			/* disable interrupt */

#define ICG_CLR		0			/* clear ICG bit */
#define ICG_SET		(~0)			/* set ICG bit */

#endif /* _P4000_SBD_ */
