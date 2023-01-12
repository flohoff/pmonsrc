/* $Id: r3041.h,v 1.2 1996/01/16 14:17:23 chris Exp $ */
/*
 * r3041.h: IDT R3041 cp0 registers.
 */

#ifndef _R3041_H_
#define _R3041_H_

/* bus control (cp0 $2) */
#define BUSC_LOCK	0x80000000	/* write lock */
#define BUSC_SET	0x60170300	/* must be set (reserved high) */
#define BUSC_MEM_SH	26		/* MemStrobe Control field shift */
#define   BUSC_CTL_HIGH	  0x0		  /* remain high */
#define   BUSC_CTL_WR	  0x1		  /* on write cycles only */
#define   BUSC_CTL_RD	  0x2		  /* on read cycles only */
#define   BUSC_CTL_RDWR	  0x3		  /* on read and write cycles */
#define BUSC_ED_SH	24		/* ExtDataEn Control field shift */
#define BUSC_IO_SH	22		/* IOStrobe  Control field shift */
#define BUSC_BE16	0x00200000	/* enable BE16(1:0) */
#define BUSC_BE		0x00080000	/* enable BE(3:0) */
#define BUSC_BTA_0	0x00000000	/* bus turnaround (>= 0.5 cycles) */
#define BUSC_BTA_1	0x00004000	/* bus turnaround (>= 1.5 cycles) */
#define BUSC_BTA_2	0x00008000	/* bus turnaround (>= 2.5 cycles) */
#define BUSC_BTA_3	0x0000c000	/* bus turnaround (>= 3.5 cycles) */
#define BUSC_DMA	0x00002000	/* DMA protocol control */
#define BUSC_TC		0x00001000	/* TC control */
#define BUSC_BR		0x00000800	/* BR control */

/* cache configuration (cp0 $3) */
#define CCFG_LOCK	0x80000000	/* write lock */
#define CCFG_SET	0x40000000	/* must be set (reserved high) */
#define CCFG_DBR	0x20000000	/* data block refill enable */
#define CCFG_FDM	0x00080000	/* force dcache miss */

/* portsize (cp0 $10) */
#define PRTSZ_LOCK	0x80000000	/* write lock */

/* port width encodings */
#define PRTSZ_32	0x0		/* 32 bit */
#define PRTSZ_8		0x1		/* 8 bit */
#define PRTSZ_16	0x2		/* 16 bit */

/* memory region shift values */
#define PRTSZ_KSEG2B_SH 28
#define PRTSZ_KSEG2A_SH 26
#define PRTSZ_KUSEGD_SH	24
#define PRTSZ_KUSEGC_SH	22
#define PRTSZ_KUSEGB_SH	20
#define PRTSZ_KUSEGA_SH	18
#define PRTSZ_KSEG1H_SH	14
#define PRTSZ_KSEG1G_SH	12
#define PRTSZ_KSEG1F_SH	10
#define PRTSZ_KSEG1E_SH	8
#define PRTSZ_KSEG1D_SH	6
#define PRTSZ_KSEG1C_SH	4
#define PRTSZ_KSEG1B_SH	2
#define PRTSZ_KSEG1A_SH	0

#ifdef LANGUAGE_ASSEMBLY
#define C0_BUSCTRL	$2
#define C0_CACHECFG	$3
#define C0_COUNT	$9
#define C0_PORTSIZE	$10
#define C0_COMPARE	$11
#else
#define C0_BUSCTRL	2
#define C0_CACHECFG	3
#define C0_COUNT	9
#define C0_PORTSIZE	10
#define C0_COMPARE	11
#endif

#endif /* _R3041_H_ */
