/* $Id: sbd.h,v 1.2 1996/01/16 14:24:26 chris Exp $ */
#define SIO0BASE	0xbf200000	/* uart #0 base */
#define SIO1BASE	0xbf400000	/* uart #1 base */
#define SIO2BASE	0xbf600000	/* uart #2 base */
#define IRR		0xbf800000	/* interrupt request reguster */
#define IACKPANIC	0xbf880000	/* acknowledge panic interrupt */
#define IACKSYNC	0xbf8c0000	/* acknowledge sync interrupt */
#define PROMBASE	0xbfc00000	/* Boot PROM base (on or off board) */
#define E2BASE		0xbfe00000	/* Onboard E2PROM */
#define E2SIZE		0x80000		/* E2PROM size (512K) */

#define MEMSIZE		0x200000	/* Max memory (2Mb) */

/* Interrupt request bits */
#define IRR_WBUSERR	0x20
#define IRR_DEBUG	0x10
#define IRR_WATCHDOG	0x08
#define IRR_SIO2	0x04
#define IRR_SIO1	0x02
#define IRR_SIO0	0x01
