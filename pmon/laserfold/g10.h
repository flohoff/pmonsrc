/* $Id: g10.h,v 1.2 1997/09/12 16:26:05 nigel Exp $ */
/*
 * pmon/g10.h: IDT 79R3710 (aka Galileo G10) controller definitions
 */

#ifndef MHZ
# error Please define MHZ
#endif

#define CYCLETIME	((1000+MHZ/2)/MHZ)	/* cycle time in ns */
#define CACHECYCLE	CYCLETIME		/* cache cycle time in ns */

#ifdef __ASSEMBLER__
#define G10_REG(x)	PHYS_TO_K1(x)
#else
#define G10_REG(x)	(*(volatile unsigned int *)PHYS_TO_K1(x))
#endif

/*
 * Device Address Spaces
 */
#define IOCHAN0_BASE	0x08000000	/* IO 8-bit channel 0 base */
#define IOCHAN1_BASE	0x09000000	/* IO 8-bit channel 1 base */
#define CENDATA_BASE	0x0a000000	/* Centronics (IEEE1284) data */
#define ENGNCTL_BASE	0x0a800000	/* Engine control register */
#define IOGPCHAN0_BASE	0x0b000000	/* IO 16-bit channel 0 base */
#define IOGPCHAN1_BASE	0x0c000000	/* IO 16-bit channel 1 base */
#define TYPHOON_BASE	0x1c000000	/* Typhoon coprocessor base */
#define G10_BASE	0x1d000000	/* G10 local register base */

/*
 * G10 registers
 */
#define G10_ROMCONFIG	(G10_BASE+0x000) /* ROM configuration */
#define  ROMCONFIG_8MB	0x00000000 	 /* ROMCS[1:0] 8Mbyte */
#define  ROMCONFIG_4MB	0x00400000 	 /* ROMCS[1:0] 4Mbyte */
#define  ROMCONFIG_2MB	0x00800000 	 /* ROMCS[1:0] 2Mbyte */
#define  ROMCONFIG_1MB	0x00c00000 	 /* ROMCS[1:0] 1Mbyte */
#define  ROMCONFIG_FIRST_SHIFT	0
#define  ROMCONFIG_GAP1_SHIFT	4
#define  ROMCONFIG_GAP2_SHIFT	8
#define  ROMCONFIG_GAP3_SHIFT	12
#define  ROMCONFIG_ACK_SHIFT	16

#define G10_VIDCONFIG	(G10_BASE+0x020) /* Video timing and control */
#define  VCFG_LSB2MSB	0x40000000
#define  VCFG_MSB2LSB	0x00000000
#define  VCFG_INVERT	0x20000000
#define  VCFG_PLL	0x10000000
#define  VCFG_HSE	0x08000000
#define  VCFG_HSKIP_MASK 0x07ffc000
#define  VCFG_HSKIP_SHIFT 14
#define  VCFG_VSE	0x00002000
#define  VCFG_VSKIP_MASK 0x00001fff
#define  VCFG_VSKIP_SHIFT 0

#define G10_PIOOUT	(G10_BASE+0x040) /* PIO output bits */
#define G10_PIOCONFIG	(G10_BASE+0x044) /* PIO configurtion */
#define G10_COUNTER	(G10_BASE+0x048) /* Counter/Timer Value */
#define G10_CLKCONFIG	(G10_BASE+0x04c) /* Clock configuration */
#define  CLK_TIMER		0x2
#define  CLK_COUNTER		0x0
#define  CLK_ENABLE		0x1
#define G10_INTCAUSE	(G10_BASE+0x050) /* Interrupt cause / clear */
#define  INT_PIO_5		0x02000000
#define  INT_PIO_4		0x01000000
#define  INT_PIO_3		0x00800000
#define  INT_PIO_2		0x00400000
#define  INT_PIO_1		0x00200000
#define  INT_PIO_0		0x00100000
#define  INT_CENEQ2		0x00010000
#define  INT_CENEQ1		0x00008000
#define  INT_CENEQ0		0x00004000
#define  INT_CENRD		0x00002000
#define  INT_CENWR		0x00001000
#define  INT_CENINIT		0x00000800
#define  INT_DMACEN		0x00000080
#define  INT_DMAIO1		0x00000040
#define  INT_DMAIO0		0x00000020
#define  INT_TIMER		0x00000010
#define  INT_VIDPAGE		0x00000008
#define  INT_VIDBAND		0x00000004
#define  INT_VIDLINE		0x00000002
#define  INT_VIDDMA		0x00000001

#define G10_INTMASK	(G10_BASE+0x054) /* Interrupt mask */

#define G10_DRAMCONFIG	(G10_BASE+0x058) /* DRAM configuration */
#define  DRAMCONFIG_REF33	0x180 	 /* 33MHz refresh */
#define  DRAMCONFIG_REF25	0x100	 /* 25MHz refresh */
#define  DRAMCONFIG_REF20	0x080	 /* 20MHz refresh */
#define  DRAMCONFIG_REF16	0x000	 /* 16MHz refresh */
#define  DRAMCONFIG_TECAS	0x040	 /* extend CAS during Typhoon cycles */
#define  DRAMCONFIG_ECAS	0x020	 /* extend CAS */
#define  DRAMCONFIG_BANK0_MASK	0x018
#define  DRAMCONFIG_BANK0_SHIFT 3
#define  DRAMCONFIG_BANK12_MASK	0x007
#define  DRAMCONFIG_BANK12_SHIFT 0
#define  DRAMCONFIG_1Mb		0x0
#define  DRAMCONFIG_2Mb		0x1
#define  DRAMCONFIG_4Mb		0x2
#define  DRAMCONFIG_8Mb		0x3
#define  DRAMCONFIG_16Mb	0x4

#define G10_PIOIN	(G10_BASE+0x05c) /* PIO inputs */
#define G10_INTWRITE	(G10_BASE+0x060) /* Interrupt diag write */
#define G10_IODMA0ADDR	(G10_BASE+0x080) /* IO bus device 0 DMA address */
#define G10_IODMA1ADDR	(G10_BASE+0x084) /* IO bus device 1 DMA address */
#define G10_CENDMAADDR	(G10_BASE+0x088) /* Centronics DMA address */
#define G10_IODMA0COUNT	(G10_BASE+0x090) /* IO bus device 0 DMA count */
#define G10_IODMA1COUNT	(G10_BASE+0x094) /* IO bus device 1 DMA count */
#define G10_CENDMACOUNT (G10_BASE+0x098) /* Centronics DMA count */
#define G10_IOCONFIG	(G10_BASE+0x0a0) /* Miscellaneous controls */
#define  IOCONFIG_TOEN		0x10000000
#define  IOCONFIG_IOLE1		0x08000000
#define  IOCONFIG_IOLE0		0x04000000
#define  IOCONFIG_IOBE1		0x00000000
#define  IOCONFIG_IOBE0		0x00000000
#define  IOCONFIG_DEVTIME4_SHIFT 22
#define  IOCONFIG_DEVTIME3_SHIFT 18
#define  IOCONFIG_CENDMARD	0x00020000
#define  IOCONFIG_CENDMA	0x00010000
#define  IOCONFIG_CENTIME_SHIFT  12
#define  IOCONFIG_DMARD1	0x00000800
#define  IOCONFIG_DMA1		0x00000400
#define  IOCONFIG_DEVTIME1_SHIFT 6
#define  IOCONFIG_DMA0RD0	0x00000020
#define  IOCONFIG_DMA0		0x00000010
#define  IOCONFIG_DEVTIME0_SHIFT 0

#define  IOCONFIG_TIME(ns)	(((ns) - 1) / CYCLETIME)
#define  IOCONFIG_MAX_TIME	15

#define G10_CENMATCH0	(G10_BASE+0x0a4) /* Centronics data detect 0 */
#define G10_CENMATCH1	(G10_BASE+0x0a8) /* Centronics data detect 1 */
#define G10_CENMATCH2	(G10_BASE+0x0ac) /* Centronics data detect 2 */

#define G10_VIDCTRL	(G10_BASE+0x0c0) /* Video control */
#define  VID_BLANKBAND	 0x08
#define  VID_LASTBAND	 0x04
#define  VID_DMAEN	 0x02
#define  VID_CNTDOWN	 0x01
#define  VID_CNTUP	 0x00
#define G10_VIDLINELEN	(G10_BASE+0x0c4) /* Video words/line */
#define G10_VIDBANDLEN	(G10_BASE+0x0c8) /* Video lines/band */
#define G10_VIDDMAADDR	(G10_BASE+0x0cc) /* Video DMA address */
#define G10_VIDDMACOUNT	(G10_BASE+0x0d0) /* Video DMA count */

#define CENMODEREQ_NIBBLE	0x00
#define CENMODEREQ_BYTE		0x01
#define CENMODEREQ_DEVID	0x04
#define CENMODEREQ_ECP		0x10
#define CENMODEREQ_ECP_RLE	0x30
#define CENMODEREQ_EPP		0x40

#define G10_CENCTLOUT	(G10_BASE+0x100) /* Centronics status */
#define  CENOUT_PERROR		0x10
#define  CENOUT_SELECT		0x08
#define  CENOUT_NOTFAULT	0x04
#define  CENOUT_NOTACK		0x02
#define  CENOUT_BUSY		0x01
#define G10_CENIEEEMODE (G10_BASE+0x104) /* Centronics IEEE1284 mode and neg */
#define  CENMODE_ACCEPT		0x08
#define  CENMODE_MODEMASK	0x07
#define  CENMODE_COMPATIBLE	0
#define  CENMODE_NIBBLE		1
#define  CENMODE_BYTE		2
#define  CENMODE_ECP		3
#define  CENMODE_EPP		4
#define  CENMODE_CPU		5
#define  CENMODE_EXTEN		6
#define  CENMODE_TERMINATE	7
#define G10_CENNIBBLE	(G10_BASE+0x108) /* Centronics nibble data out */
#define G10_CENCTLIN	(G10_BASE+0x10c) /* Centronics host signal status */
#define  CENIN_NOTAUTOFD	0x08
#define  CENIN_NOTINIT		0x04
#define  CENIN_NOTSELECTIN	0x02
#define  CENIN_NOTSTROBE	0x01
#define G10_CENCONFIG	(G10_BASE+0x110) /* Centronics mode and DMA control */
#define  CENCFG_DMARD		0x08
#define  CENCFG_DMAEN		0x04
#define  CENCFG_APPLMASK	0x03
#define   CENCFG_APPL_STANDARD	 0x0
#define   CENCFG_APPL_IBMEPSON	 0x1
#define   CENCFG_APPL_GALILEO	 0x2
#define   CENCFG_APPL_CLASSIC	 0x3
#define G10_CENTIMING	(G10_BASE+0x114) /* Centronics cycle timing */
#define  CENTIMING_500NS_MASK	0x3f80
#define  CENTIMING_500NS_SHIFT	7
#define  CENTIMING_2500NS_MASK	0x007f
#define  CENTIMING_2500NS_SHIFT	0

#if MHZ>=33
#define DRAMCONFIG_REF	DRAMCONFIG_REF33
#elif MHZ>=25
#define DRAMCONFIG_REF	DRAMCONFIG_REF25
#elif MHZ>=20
#define DRAMCONFIG_REF	DRAMCONFIG_REF20
#else
#define DRAMCONFIG_REF	DRAMCONFIG_REF16
#endif

#define ROM_FIRST	((ROMCYCLE + CYCLETIME - 1) / CYCLETIME - 1)
#define ROM_GAP2	((ROMCYCLE - CYCLETIME / 2 - 1) / CYCLETIME)
#ifdef ROM_INTERLEAVED
#define ROM_GAP1	0
#define ROM_GAP3	0
#else
#define ROM_GAP1	ROM_GAP2
#define ROM_GAP3	ROM_GAP2
#endif
#define ROM_ACK		(ROM_FIRST + ROM_GAP1 + ROM_GAP2 + ROM_GAP3)


/*
 * rough scaling factors for 2 instruction DELAY loop to get 1ms and 1us delays
 */
#define ASMDELAY(ns,icycle)	\
	(((ns) + (icycle)) / ((icycle) * 2))

#define CACHEMISS	(CYCLETIME * 6)

#define CACHENS(us)	ASMDELAY((ns), CACHECYCLE)
#define RAMNS(us)	ASMDELAY((ns), CACHEMISS+RAMCYCLE)
#define ROMNS(us)	ASMDELAY((ns), CACHEMISS+ROMCYCLE)
#define CACHEUS(us)	ASMDELAY((us)*1000, CACHECYCLE)
#define RAMUS(us)	ASMDELAY((us)*1000, CACHEMISS+RAMCYCLE)
#define ROMUS(us)	ASMDELAY((us)*1000, CACHEMISS+ROMCYCLE)
#define CACHEMS(ms)	((ms) * ASMDELAY(1000000, CACHECYCLE))
#define RAMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+RAMCYCLE))
#define ROMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+ROMCYCLE))

