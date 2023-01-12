/* $Id: lr33020.h,v 1.2 1996/01/16 14:17:19 chris Exp $ */
/*
** lr33020.h -- lr33020 definitions
*/

#ifndef _LR33020_
#define _LR33020_

#ifdef LANGUAGE_C
#define C2_SRCSKEW	0	/* graphics source skew */
#define C2_SRCSHIFT	1	/* graphics source shift */
#define C2_COLOR0	2	/* COLOR0 data */
#define C2_COLOR1	3	/* COLOR1 data */
#define C2_GCPCNTRL	4	/* graphics control */
#define C2_RASTEROP	5	/* RasterOp logical operation */
#define C2_PLANEMASK	6	/* Plane mask */
#define C2_CONFIG	9	/* video control */
#define C2_PSSTAT	24	/* I/O port status */
#define C2_PSCOMM	25	/* I/O port command */
#define C2_PSCOMMWE	27	/* I/O port write enable */

/* cp2 data registers */
#define C2_SCRSTART	0	/* Screen start address */
#define C2_SCRPITCH	1	/* Screen pitch */
#define C2_HWCRSRSTART	2	/* Hardware cursor start addr */
#define C2_HWCRSRCURR	3	/* Current hardware cursor addr */
#define C2_SAMEXTENT	4	/* VRAM serial-access-mode extent */
#define C2_NXTDISP	5	/* Next display addr */
#define C2_CURDISP	6	/* Current display address */
#define C2_LINECOUNT	7	/* Video line count */
#define C2_VBLKSIZE	8	/* Video block size */
#define C2_SRCLINE	9	/* Src start of next line addr */
#define C2_SRCCURR	10	/* Source current addr */
#define C2_SRCPITCH	11	/* Source pitch */
#define C2_DESTLINE	12	/* Dest start of next line addr */
#define C2_DESTCURR	13	/* Destination current addr */
#define C2_DESTPITCH	14	/* Destination pitch */
#define C2_GBLKSIZE	15	/* Graphics src read block size */
#define C2_SERPULS	16	/* Comp sync serration pulse width */
#define C2_HLINTR	17	/* Horizontal interrupt line */
#define C2_BLANKS	18	/* Blank start */
#define C2_SYNCS	19	/* SYNC start */
#define C2_SYNCRESET	20	/* SYNC reset */
#define C2_BLANKE	21	/* Blank end */
#define C2_HWCRSR	22	/* Hardware cursor location */
#define C2_VHWCONFIG	23	/* Video hardware config bits */
#define C2_PSTXB	24	/* PS/2 serial port Tx buffer */
#define C2_PSRCVB	25	/* PS/2 serial port Rx buffer */
#define C2_PSTXBWE	27	/* PS/2 serial port Tx buffer write enable */
#define C2_SRCDATA	28	/* Source data */
#define C2_DESTDATA	29	/* Destination data */
#define C2_LEFTMASK	30	/* Left edge mask */
#define C2_RIGHTMASK	31	/* Right edge mask */
#else
/* cp2 control registers */
#define C2_SRCSKEW	$0	/* graphics source skew */
#define C2_SRCSHIFT	$1	/* graphics source shift */
#define C2_COLOR0	$2	/* COLOR0 data */
#define C2_COLOR1	$3	/* COLOR1 data */
#define C2_GCPCNTRL	$4	/* graphics control */
#define C2_RASTEROP	$5	/* RasterOp logical operation */
#define C2_PLANEMASK	$6	/* Plane mask */
#define C2_CONFIG	$9	/* video control */
#define C2_PSSTAT	$24	/* I/O port status */
#define C2_PSCOMM	$25	/* I/O port command */
#define C2_PSCOMMWE	$27	/* I/O port write enable */

/* cp2 data registers */
#define C2_SCRSTART	$0	/* Screen start address */
#define C2_SCRPITCH	$1	/* Screen pitch */
#define C2_HWCRSRSTART	$2	/* Hardware cursor start addr */
#define C2_HWCRSRCURR	$3	/* Current hardware cursor addr */
#define C2_SAMEXTENT	$4	/* VRAM serial-access-mode extent */
#define C2_NXTDISP	$5	/* Next display addr */
#define C2_CURDISP	$6	/* Current display address */
#define C2_LINECOUNT	$7	/* Video line count */
#define C2_VBLKSIZE	$8	/* Video block size */
#define C2_SRCLINE	$9	/* Src start of next line addr */
#define C2_SRCCURR	$10	/* Source current addr */
#define C2_SRCPITCH	$11	/* Source pitch */
#define C2_DESTLINE	$12	/* Dest start of next line addr */
#define C2_DESTCURR	$13	/* Destination current addr */
#define C2_DESTPITCH	$14	/* Destination pitch */
#define C2_GBLKSIZE	$15	/* Graphics src read block size */
#define C2_SERPULS	$16	/* Comp sync serration pulse width */
#define C2_HLINTR	$17	/* Horizontal interrupt line */
#define C2_BLANKS	$18	/* Blank start */
#define C2_SYNCS	$19	/* SYNC start */
#define C2_SYNCRESET	$20	/* SYNC reset */
#define C2_BLANKE	$21	/* Blank end */
#define C2_HWCRSR	$22	/* Hardware cursor location */
#define C2_VHWCONFIG	$23	/* Video hardware config bits */
#define C2_PSTXB	$24	/* PS/2 serial port Tx buffer */
#define C2_PSRCVB	$25	/* PS/2 serial port Rx buffer */
#define C2_PSTXBWE	$27	/* PS/2 serial port Tx buffer write enable */
#define C2_SRCDATA	$28	/* Source data */
#define C2_DESTDATA	$29	/* Destination data */
#define C2_LEFTMASK	$30	/* Left edge mask */
#define C2_RIGHTMASK	$31	/* Right edge mask */

#define sstep		.word	0x4a00ffff
#define sbstep		.word	0x4a40ffff
#define wstep		.word	0x4a80ffff
#define wstep_l		.word	0x4a88ffff
#define wstep_r		.word	0x4a84ffff
#define wstep_l_r	.word	0x4a8cffff
#define wstep_s		.word	0x4aa0ffff
#define wstep_s_l	.word	0x4aa8ffff
#define wstep_s_r	.word	0x4aa4ffff
#define wstep_s_l_r	.word	0x4aacffff
#define wstep_sb	.word	0x4ab0ffff
#define wstep_sb_l	.word	0x4ab8ffff
#define wstep_sb_r	.word	0x4ab4ffff
#define wstep_sb_l_r	.word	0x4abcffff
#define wstep_four	.word	0x4a82ffff
#define wstep_bfour	.word	0x4a81ffff
#define bstep_bfour	.word	0x4ac1ffff
#define bstep		.word	0x4ac0ffff
#define bstep_l		.word	0x4ac8ffff
#define bstep_r		.word	0x4ac4ffff
#define bstep_l_r	.word	0x4accffff
#define bstep_s		.word	0x4ae0ffff
#define bstep_s_l	.word	0x4ae8ffff
#define bstep_s_r	.word	0x4ae4ffff
#define bstep_s_l_r	.word	0x4aecffff
#define bstep_sb	.word	0x4af0ffff
#define bstep_sb_l	.word	0x4af8ffff
#define bstep_sb_r	.word	0x4af4ffff
#define bstep_sb_l_r	.word	0x4afcffff
#endif

/* register bit assignments */
/* M_CFGREG */
#define CR_BANKMASK	(7<<5)
#define CR_BANKSHFT	5

/* C2_GCPCNTRL creg4 */
#define GCPC_SPCLWEMASK	0x00006000
#define GCPC_YDIR	(1<<12)
#define GCPC_XDIR	(1<<11)
#define GCPC_WO		(1<<9)
#define GCPC_MW		(1<<8)
#define GCPC_MASK	(1<<6)
#define GCPC_EXPND	(1<<4)
#define GCPC_TRAN	(1<<3)
#define GCPC_PIXSIZMASK	0x00000007

/* C2_VHWCNTRL creg9 */
#define VHWCNTRL_HCUR		(1<<19)
#define VHWCNTRL_DRAM		(1<<18)
#define VHWCNTRL_SAM		(1<<17)
#define VHWCNTRL_VRAM		(1<<16)
#define VHWCNTRL_D3		(1<<15)
#define VHWCNTRL_IORV3MASK 	(7<<12)
#define VHWCNTRL_IOWAIT3MASK 	(0xf<<8)
#define VHWCNTRL_D2		(1<<7)
#define VHWCNTRL_IORV2MASK 	(7<<4)
#define VHWCNTRL_IOWAIT2MASK 	(0xf<<0)

/* C2_PSSTAT creg24 */
#define PSSTAT_FERR	(1<<7)
#define PSSTAT_PAR	(1<<6)
#define PSSTAT_RXIN	(1<<5)
#define PSSTAT_RXBF	(1<<4)
#define PSSTAT_TXBE	(1<<3)
#define PSSTAT_TXIN	(1<<2)
#define PSSTAT_CLKX	(1<<1)
#define PSSTAT_SI1	(1<<1)
#define PSSTAT_CLK	(1<<0)
#define PSSTAT_SI0	(1<<0)

/* C2_PSCOMM creg25 */
#define PSCOMM_IO	(1<<7)
#define PSCOMM_CLKINH	(1<<4)
#define PSCOMM_RCVINT	(1<<3)
#define PSCOMM_TXINT	(1<<2)
#define PSCOMM_TXEN	(1<<1)
#define PSCOMM_SO1	(1<<1)
#define PSCOMM_INTHTX	(1<<0)
#define PSCOMM_SO0	(1<<0)

/* C2_VHWCONFIG dreg23 */
#define VHWCONFIG_EN		(1<<31)
#define VHWCONFIG_CROSSINV	(1<<18)
#define VHWCONFIG_CROSSDAT	(1<<17)
#define VHWCONFIG_CROSSEN	(1<<16)
#define VHWCONFIG_CSYNCEN	(1<<15)
#define VHWCONFIG_OVRSCN	(1<<14)
#define VHWCONFIG_VEND		(1<<13)
#define VHWCONFIG_VSYNCOUTEN	(1<<12)
#define VHWCONFIG_VSYNCINEN	(1<<11)
#define VHWCONFIG_HWCRSR	(1<<10)
#define VHWCONFIG_SHFTMASK	(3<<8)
#define VHWCONFIG_VSPOS		(1<<7)
#define VHWCONFIG_HSPOS		(1<<6)
#define VHWCONFIG_VSINT		(1<<5)
#define VHWCONFIG_HLINT		(1<<4)
#define VHWCONFIG_VLINTEN	(1<<1)
#define VHWCONFIG_HLINTEN	(1<<0)

#endif /* _LR33020_ */
