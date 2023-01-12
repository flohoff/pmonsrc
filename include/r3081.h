/* $Id: r3081.h,v 1.2 1996/01/16 14:17:24 chris Exp $ */
/*
 * r3081.h: IDT R3081 configuration register
 */

#ifndef _R3081_H_
#define _R3081_H_

#define CFG_LOCK	0x80000000	/* lock register */
#define CFG_SLOWBUS	0x40000000	/* slow external bus */
#define CFG_DBREFILL	0x20000000	/* data bus refill: 1/4 words */
#define CFG_FPINT_MASK	0x1c000000 	/* select fpa interrupt (0-5) */
#define CFG_FPINT_SHIFT	26
#define CFG_HALT	0x02000000 	/* halt until interrupt */
#define CFG_RF		0x01000000	/* reduce frequency */
#define CFG_AC		0x00800000	/* alt cache: 8kb+8kb */

#ifdef LANGUAGE_ASSEMBLY
#define C0_CONFIG	$3
#else
#define C0_CONFIG	3
#endif

#endif /* _R3081_H_ */
