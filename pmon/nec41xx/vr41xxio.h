/* 
 * vr41xxio.h: onchip registers for Vr41xx family
 * Copyright (c) 1998 Algorithmics Ltd
 */

#ifndef _VR41XXIO_H_
#define _VR41XXIO_H_

/*
 * Base addresses for onchip devices
 */


#define LCD_MEM_BASE	0x0a000000	/* lcd frame buffer */

#define BCU_BASE	0x0b000000	/* bus control unit */
#define DMAA_BASE	0x0b000020	/* dma address unit */
#define DCU_BASE	0x0b000040	/* dma control unit */
#define CMU_BASE	0x0b000060	/* clock mask unit */
#define ICU1_BASE	0x0b000080	/* interrupt control unit #1 */
#define PMU1_BASE	0x0b0000a0	/* power management unit #1 */
#define RTC1_BASE	0x0b0000c0	/* real-time clock #1 */
#define DSU_BASE	0x0b0000e0	/* deadman's switch unit */
#define GIU1_BASE	0x0b000100	/* general purpose i/o unit #1 */
#define PIU1_BASE	0x0b000120	/* touch-panel interface unit #1 */
#define AIU_BASE	0x0b000160	/* audio interface unit */
#define KIU_BASE	0x0b000180	/* keyboard interface unit */
#define DSIU_BASE	0x0b0001a0	/* debug serial i/o unit */
#define RTC2_BASE	0x0b0001c0	/* real-time clock #2 */
#define ICU2_BASE	0x0b000200	/* interrupt control unit #2 */
#define LED_BASE	0x0b000240	/* led unit */
#define ADTEST_BASE	0x0b000260	/* ??? */
#define PIU2_BASE	0x0b0002a0	/* touch-panel interface unit #1 */
#define PMU2_BASE	0x0b0002c0	/* power management unit #2 */
#define GIU2_BASE	0x0b0002e0	/* general purpose i/o unit #2 */

#define SIU_BASE	0x0c000000	/* serial i/o unit */
#define HSP_BASE	0x0c000020	/* high-speed modem interface */
#define FIR_BASE	0x0c000040	/* fast infrared interface */

#define SYSBUS_MEM_BASE	0x10000000	/* system bus memory space */

#define SYSBUS_IO_BASE	0x14000000	/* system bus i/o space */


/* 
 * BCU - bus control unit
 */

#define BCU_CNT1	0x00
#define BCU_CNT2	0x02
#define BCU_ROMSIZE	0x04	/* 4121 */
#define BCU_RAMSIZE	0x06	/* 4121 */
#define BCU_SPEED	0x0A
#define BCU_ERRST	0x0C
#define BCU_RFCNT	0x0E
#define BCU_REVID	0x10
#define BCU_RFCOUNT	0x12
#define BCU_CLKSPEED	0x14
#define BCU_CNT3	0x16
#define BCU_SDRAMMODE	0x1A	/* 4121 */
#define BCU_SROMMODE	0x1C	/* 4121 */
#define BCU_SDRAMCNT	0x1E	/* 4121 */

#ifndef __ASSEMBLER__
struct vr41xxbcu {
    unsigned short	bcu_cnt1;
    unsigned short	bcu_cnt2;
    unsigned short	bcu_unused04;
    unsigned short	bcu_unused06;
    unsigned short	bcu_unused08;
    unsigned short	bcu_speed;
    unsigned short	bcu_errst;
    unsigned short	bcu_rfcnt;
    unsigned short	bcu_revid;
    unsigned short	bcu_rfcount;
    unsigned short	bcu_clkspeed;
    unsigned short	bcu_cnt3;
};
#endif

#define BCUCNT1_RSTOUT		0x0001
#define BCUCNT1_BUSHERREN	0x0002
#define BCUCNT1_ROMWEN0		0x0010
#define BCUCNT1_ROMWEN2		0x0040
#define BCUCNT1_PAGEROM0	0x0100
#define BCUCNT1_PAGEROM2	0x0400
#define BCUCNT1_PAGE128		0x1000
#define BCUCNT1_ISAM		0x2000
#define BCUCNT1_DRAM64		0x4000
#define BCUCNT1_ROM64		0x8000

#define BCUCNT2_GMODE		0x0001

#define BCUCNT3_LCD32		0x0080
#define BCUCNT3_EXTMEM		0x0800
#define BCUCNT3_ROMCS2		0x1000
#define BCUCNT3_ROMCS3		0x2000
#define BCUCNT3_EXTDRAM64	0x4000
#define BCUCNT3_EXTROM64	0x8000

#define BCUERRST_BERRST		0x0001

#define BCUCLKSPEED_CLKSP_MASK	0x001f
#define BCUCLKSPEED_CLKSP_SHIFT	0
#define BCUCLKSPEED_DIVVT_MASK	0x0f00	/* vr4121 */
#define BCUCLKSPEED_DIVVT_SHIFT	8
#define BCUCLKSPEED_DIV4B	0x2000	/* vr4111 */
#define BCUCLKSPEED_DIV3B	0x4000	/* vr4111 */
#define BCUCLKSPEED_DIV2B	0x8000	/* vr4111 */
#define BCUCLKSPEED_DIVT_MASK	0xf000	/* vr4121 */
#define BCUCLKSPEED_DIVT_SHIFT	12

#define BCUREVID_RID_MASK	0xf000
#define BCUREVID_RID_SHIFT	12
#define BCUREVID_MJREV_MASK	0x0f00
#define BCUREVID_MJREV_SHIFT	8
#define BCUREVID_MNREV_MASK	0x000f
#define BCUREVID_MNREV_SHIFT	0

#define RID_VR4102		1
#define RID_VR4111		2
#define RID_VR4121		3
#define RID_VR4122		4

/* 
 * DMAA - dma address unit
 */

#define DMAA_AIUIBAL	0x00
#define DMAA_AIUIBAH	0x02
#define DMAA_AIUIAL	0x04
#define DMAA_AIUIAH	0x06
#define DMAA_AIUOBAL	0x08
#define DMAA_AIUOBAH	0x0A
#define DMAA_AIUOAL	0x0C
#define DMAA_AIUOAH	0x0E
#define DMAA_FIRBAL	0x10
#define DMAA_FIRBAH	0x12
#define DMAA_FIRAL	0x14
#define DMAA_FIRAH	0x16

#ifndef __ASSEMBLER__
struct vr41xxdmaa {
    unsigned short	dmaa_aiuibal;
    unsigned short	dmaa_aiuibah;
    unsigned short	dmaa_aiuial;
    unsigned short	dmaa_aiuiah;
    unsigned short	dmaa_aiuobal;
    unsigned short	dmaa_aiuobah;
    unsigned short	dmaa_aiuoal;
    unsigned short	dmaa_aiuoah;
    unsigned short	dmaa_firbal;
    unsigned short	dmaa_firbah;
    unsigned short	dmaa_firal;
    unsigned short	dmaa_firah;
};
#endif


/*
 * DCU - dma control unit
 */

/* register offsets */
#define DCU_DMARST	0x00
#define DCU_DMAIDLE	0x02
#define DCU_DMASEN	0x04
#define DCU_DMAMSK	0x06
#define DCU_DMAREQ	0x08
#define DCU_TD		0x0A

#ifndef __ASSEMBLER__
struct vr41xxdcu {
    unsigned short	dcu_dmarst;
    unsigned short	dcu_dmaidle;
    unsigned short	dcu_dmasen;
    unsigned short	dcu_dmamsk;
    unsigned short	dcu_dmareq;
    unsigned short	dcu_td;
};
#endif

#define DMARST_DMARST		0x0001

#define DMAIDLE_DMAISTAT	0x0001

#define DMASEN_DMASEN		0x0001

#define DMAMSK_FOUT		0x0001
#define DMAMSK_AOUT		0x0004
#define DMAMSK_AIN		0x0008

#define DMAREQ_FOUT		0x0001
#define DMAREQ_AOUT		0x0004
#define DMAREQ_AIN		0x0008

#define DMATD_FIR_IO2MEM	0x0001
#define DMATD_FIR_MEM2IO	0x0000


/*
 * CMU - clock mask unit
 */

#define CMU_CLKMSK	0x00

#ifndef __ASSEMBLER__
struct vr41xxcmu {
    unsigned short	cmu_clkmsk;
};
#endif

#define CLKMSK_PIU		0x0001
#define CLKMSK_SIU		0x0002
#define CLKMSK_AIU		0x0004
#define CLKMSK_KIU		0x0008
#define CLKMSK_FIR		0x0010
#define CLKMSK_DSIU		0x0020
#define CLKMSK_SSIU		0x0100
#define CLKMSK_SHSP		0x0200
#define CLKMSK_FFIR		0x0400


/*
 * ICU1 - interrupt control unit (part 1)
 */

/* register offsets */
#define ICU1_SYSINT1		0x00
#define ICU1_PIUINT		0x02
#define ICU1_AIUINT		0x04
#define ICU1_KIUINT		0x06
#define ICU1_GIUINTL		0x08
#define ICU1_DSIUINT		0x0A
#define ICU1_MSYSINT1		0x0C
#define ICU1_MPIUINT		0x0E
#define ICU1_MAIUINT		0x10
#define ICU1_MKIUINT		0x12
#define ICU1_MGIUINTL		0x14
#define ICU1_MDSIUINT		0x16
#define ICU1_NMI		0x18
#define ICU1_SOFTINT		0x1A

#ifndef __ASSEMBLER__
struct vr41xxicu {
    unsigned short	icu1_sysint1;
    unsigned short	icu1_piuint;
    unsigned short	icu1_aiuint;
    unsigned short	icu1_kiuint;
    unsigned short	icu1_giuintl;
    unsigned short	icu1_dsiuint;

    unsigned short	icu1_msysint1;
    unsigned short	icu1_mpiuint;
    unsigned short	icu1_maiuint;
    unsigned short	icu1_mkiuint;
    unsigned short	icu1_mgiuintl;
    unsigned short	icu1_mdsiuint;

    unsigned short	icu1_nmi;
    unsigned short	icu1_softint;
};
#endif


#define SYSINT1_BATINTR		0x0001
#define SYSINT1_POWERINTR	0x0002
#define SYSINT1_RTCL1INTR	0x0004
#define SYSINT1_ETIMERINTR	0x0008
#define SYSINT1_PIUINTR		0x0020
#define SYSINT1_AIUINTR		0x0040
#define SYSINT1_KIUINTR		0x0080
#define SYSINT1_GIUINTR		0x0100
#define SYSINT1_SIUINTR		0x0200
#define SYSINT1_WRBERRINTR	0x0400
#define SYSINT1_SOFTINTR	0x0800
#define SYSINT1_DOZEPIUINTR	0x2000

#define PIUINT_PENCHGINTR	0x0001
#define PIUINT_PADDLOSTINTR	0x0004
#define PIUINT_PADPAGE0INTR	0x0008
#define PIUINT_PADPAGE1INTR	0x0010
#define PIUINT_PADADPINTR	0x0020
#define PIUINT_PADCMDINTR	0x0040

#define AIUINT_INTSIDLE		0x0002
#define AIUINT_INTS		0x0004
#define AIUINT_INTSEND		0x0008
#define AIUINT_INTMST		0x0100
#define AIUINT_INTMIDLE		0x0200
#define AIUINT_INTM		0x0400
#define AIUINT_INTMEND		0x0800

#define KIUINT_SCANINT		0x0001
#define KIUINT_KDATRDY		0x0002
#define KIUINT_KDATLOST		0x0004

#define DSIUINT_INTST0		0x0100
#define DSIUINT_INTSR0		0x0200
#define DSIUINT_INTSER0		0x0400
#define DSIUINT_INTDCTS		0x0800

#define NMI_NMIORINT		0x0001


/* 
 * ICU2 - interrupt control unit (part 2)
 */

#define ICU2_SYSINT2		0x00
#define ICU2_GIUINTH		0x02
#define ICU2_FIRINT		0x04
#define ICU2_MSYSINT2		0x06
#define ICU2_MGIUINTH		0x08
#define ICU2_MFIRINT		0x0A

#ifndef __ASSEMBLER__
struct vr41xxicu2 {
    unsigned short	icu2_sysint2;
    unsigned short	icu2_giuinth;
    unsigned short	icu2_firint;
    unsigned short	icu2_msysint2;
    unsigned short	icu2_mgiuinth;
    unsigned short	icu2_mfirint;
};
#endif

#define SYSINT2_RTCL2INTR	0x0001
#define SYSINT2_LEDINTR		0x0002
#define SYSINT2_HSPINTR		0x0004
#define SYSINT2_TCLKINTR	0x0008
#define SYSINT2_FIRINTR		0x0010
#define SYSINT2_DSIUINTR	0x0020


/* 
 * PMU1 - power management unit (part 1)
 */

/* register offsets */
#define PMU1_INT	0x00
#define PMU1_CNT	0x02
#define PMU1_INT2	0x04
#define PMU1_CNT2	0x06
#define PMU1_WAIT	0x08

#ifndef __ASSEMBLER__
struct vr41xxpmu1 {
    unsigned short	pmu1_int;
    unsigned short	pmu1_cnt;
    unsigned short	pmu1_int2;
    unsigned short	pmu1_cnt2;
    unsigned short	pmu1_wait;
};
#endif


#define PMUINT_POWERSWINTR	0x0001
#define PMUINT_BATTINTR		0x0002
#define PMUINT_DMSRST		0x0004
#define PMUINT_RSTSW		0x0008
#define PMUINT_RTCRST		0x0010
#define PMUINT_TIMOUTRST	0x0020
#define PMUINT_MEMO0		0x0040
#define PMUINT_MEMO1		0x0080
#define PMUINT_BATTINH		0x0100
#define PMUINT_RTCINTR		0x0200
#define PMUINT_DCDST		0x0400
#define PMUINT_GPIO0INTR	0x1000
#define PMUINT_GPIO1INTR	0x2000
#define PMUINT_GPIO2INTR	0x4000
#define PMUINT_GPIO3INTR	0x8000

#define PMUCNT_SET		0x0002
#define PMUCNT_HALTIMERRST	0x0004
#define PMUCNT_STANDBY		0x0080
#define PMUCNT_GPIO0TRG		0x0100
#define PMUCNT_GPIO1TRG		0x0200
#define PMUCNT_GPIO2TRG		0x0400
#define PMUCNT_GPIO3TRG		0x0800
#define PMUCNT_GPIO0MSK		0x1000
#define PMUCNT_GPIO1MSK		0x2000
#define PMUCNT_GPIO2MSK		0x4000
#define PMUCNT_GPIO3MSK		0x8000

#define PMUINT2_GPIO9INTR	0x1000
#define PMUINT2_GPIO10INTR	0x2000
#define PMUINT2_GPIO11INTR	0x4000
#define PMUINT2_GPIO12INTR	0x8000

#define PMUCNT2_GPIO9TRG	0x0100
#define PMUCNT2_GPIO10TRG	0x0200
#define PMUCNT2_GPIO11TRG	0x0400
#define PMUCNT2_GPIO12TRG	0x0800
#define PMUCNT2_GPIO9MSK	0x1000
#define PMUCNT2_GPIO10MSK	0x2000
#define PMUCNT2_GPIO11MSK	0x4000
#define PMUCNT2_GPIO12MSK	0x8000


/*
 * RTC1 - real-time clock (part 1) 
 */

#define RTC1_ETIMEL		0x00
#define RTC1_ETIMEM		0x02
#define RTC1_ETIMEH		0x04
#define RTC1_ECMPL		0x08
#define RTC1_ECMPM		0x0A
#define RTC1_ECMPH		0x0C
#define RTC1_L1L		0x10
#define RTC1_L1H		0x12
#define RTC1_L1CNTL		0x14
#define RTC1_L1CNTH		0x16
#define RTC1_L2L		0x18
#define RTC1_L2H		0x1A
#define RTC1_L2CNTL		0x1C
#define RTC1_L2CNTH		0x1E

#ifndef __ASSEMBLER__
struct vr41xxrtc1 {
    unsigned short	rtc1_etimel;
    unsigned short	rtc1_etimem;
    unsigned short	rtc1_etimeh;
    unsigned short	rtc1_unused06;
    unsigned short	rtc1_ecmpl;
    unsigned short	rtc1_ecmpm;
    unsigned short	rtc1_ecmph;
    unsigned short	rtc1_unused0e;
    unsigned short	rtc1_l1l;
    unsigned short	rtc1_l1h;
    unsigned short	rtc1_l1cntl;
    unsigned short	rtc1_l1cnth;
    unsigned short	rtc1_l2l;
    unsigned short	rtc1_l2h;
    unsigned short	rtc1_l2cntl;
    unsigned short	rtc1_l2cnth;
};
#endif


/*
 * RTC2 - real-time clock (part 2) 
 */

#define RTC2_TCLKL		0x00
#define RTC2_TCLKH		0x02
#define RTC2_TCLKCNTL		0x04
#define RTC2_TCLKCNTH		0x06
#define RTC2_INT		0x1E

#ifndef __ASSEMBLER__
struct vr41xxrtc2 {
    unsigned short	rtc2_tclkl;
    unsigned short	rtc2_tclkh;
    unsigned short	rtc2_tclkcntl;
    unsigned short	rtc2_tclkcnth;
    unsigned short	rtc2_unused[11];
    unsigned short	rtc2_int;
};
#endif

#define RTCINT_INTR0	0x0001		/* Elapsed time*/
#define RTCINT_INTR1	0x0002		/* RTCLong1 */
#define RTCINT_INTR2	0x0004		/* RTCLong2 */
#define RTCINT_INTR3	0x0008		/* TClock */


/*
 * DSU - deadman's switch unit
 */

#define DSU_CNT	0x00
#define DSU_SET	0x02
#define DSU_CLR	0x04
#define DSU_TIM	0x06

#ifndef __ASSEMBLER__
struct vr41xxdsu {
    unsigned short	dsu_cnt;
    unsigned short	dsu_set;
    unsigned short	dsu_clr;
    unsigned short	dsu_tim;
};
#endif

#define DSUCNT_DSWEN	0x0001
#define DSUCLR_DSWCLR	0x0001


/*
 * GIU1 - general purpose i/o (part 1)
 */

#define GIU1_IOSELL	0x00
#define GIU1_IOSELH	0x02
#define GIU1_PIODL	0x04
#define GIU1_PIODH	0x06
#define GIU1_INTSTATL	0x08
#define GIU1_INTSTATH	0x0A
#define GIU1_INTENL	0x0C
#define GIU1_INTENH	0x0E
#define GIU1_INTTYPL	0x10
#define GIU1_INTTYPH	0x12
#define GIU1_INTALSELL	0x14
#define GIU1_INTALSELH	0x16
#define GIU1_INTHTSELL	0x18
#define GIU1_INTHTSELH	0x1A
#define GIU1_PODATL	0x1C
#define GIU1_PODATH	0x1E

#ifndef __ASSEMBLER__
struct vr41xxgiu1 {
	unsigned short	giu1_iosell;
	unsigned short	giu1_ioselh;
	unsigned short	giu1_piodl;
	unsigned short	giu1_piodh;
	unsigned short	giu1_intstatl;
	unsigned short	giu1_intstath;
	unsigned short	giu1_intenl;
	unsigned short	giu1_intenh;
	unsigned short	giu1_inttypl;
	unsigned short	giu1_inttyph;
	unsigned short	giu1_intalsell;
	unsigned short	giu1_intalselh;
	unsigned short	giu1_inthtsell;
	unsigned short	giu1_inthtselh;
	unsigned short	giu1_podatl;
	unsigned short	giu1_podath;
};
#endif


/*
 * GIU2 - general purpose i/o (part 2)
 */

#define GIU2_USEUPDN	0x00
#define GIU2_TERMUPDN	0x02

#ifndef __ASSEMBLER__
struct vr41xxgiu2 {
	unsigned short	giu2_useupdn;
	unsigned short	giu2_termupdn;
};
#endif


/*
 * PIU1 - touch panel interface (part 1)
 */

#define PIU1_CNT	0x02
#define PIU1_INT	0x04
#define PIU1_SIVL	0x06
#define PIU1_STBL	0x08
#define PIU1_CMD	0x0A
#define PIU1_ASCN	0x10
#define PIU1_AMSK	0x12
#define PIU1_CIVL	0x1E

#ifndef __ASSEMBLER__
struct vr41xxpiu1 {
    unsigned short	piu1_unused00;
    unsigned short	piu1_cnt;
    unsigned short	piu1_int;
    unsigned short	piu1_sivl;
    unsigned short	piu1_stbl;
    unsigned short	piu1_cmd;
    unsigned short	piu1_unused0c;
    unsigned short	piu1_unused0e;
    unsigned short	piu1_ascn;
    unsigned short	piu1_amsk;
    unsigned short	piu1_unused14;
    unsigned short	piu1_unused16;
    unsigned short	piu1_unused18;
    unsigned short	piu1_unused1a;
    unsigned short	piu1_unused1c;
    unsigned short	piu1_civl;
};
#endif

#define PIUCNT_PADRST		0x0001
#define PIUCNT_PIUPWR		0x0002
#define PIUCNT_PIUSEQEN		0x0004
#define PIUCNT_PIUMODE_MASK	0x0018
# define PIUCNT_PIUMODE_SAMPLE		(0 << 3)
# define PIUCNT_PIUMODE_OPERATEAD	(1 << 3)
#define PIUCNT_PADSCANTYPE	0x0020
#define PIUCNT_PADSCANSTART	0x0040
#define PIUCNT_PADSCANSTOP	0x0080
#define PIUCNT_PADATSTART	0x0100
#define PIUCNT_PADATSTOP	0x0200
#define PIUCNT_PADSTATEMASK	0x1c00
# define PIUCNT_PADSTATE_DISABLE	(0 << 10)
# define PIUCNT_PADSTATE_STANDBY	(1 << 10)
# define PIUCNT_PADSTATE_ADPORTSCAN	(2 << 10)
# define PIUCNT_PADSTATE_WAITPENTOUCH	(4 << 10)
# define PIUCNT_PADSTATE_PENDATASCAN	(5 << 10)
# define PIUCNT_PADSTATE_INTVLNEXTSCAN	(6 << 10)
# define PIUCNT_PADSTATE_CMDSCAN	(7 << 10)
#define PIUCNT_PENSTC		0x2000

#define PIUINT_PENCHGINTR	0x0001
#define PIUINT_PADLOSTINTR	0x0004
#define PIUINT_PADPAGE0INTR	0x0008
#define PIUINT_PADPAGE1INTR	0x0010
#define PIUINT_PADADPINTR	0x0020
#define PIUINT_PADCMDINTR	0x0040
#define PIUINT_OVP		0x8000

/* XXX define more PIU bitfields */


/*
 * PIU2 - touch panel interface (part 2)
 */

#define PIU2_PB00	0x00
#define PIU2_PB01	0x02
#define PIU2_PB02	0x04
#define PIU2_PB03	0x06
#define PIU2_PB10	0x08
#define PIU2_PB11	0x0A
#define PIU2_PB12	0x0C
#define PIU2_PB13	0x0E
#define PIU2_AB0	0x10
#define PIU2_AB1	0x12
#define PIU2_AB2	0x14
#define PIU2_AB3	0x16
#define PIU2_PB04	0x1C
#define PIU2_PB14	0x1E

#ifndef __ASSEMBLER__
struct vr41xxpiu2 {
    unsigned short	piu2_pb00;
    unsigned short	piu2_pb01;
    unsigned short	piu2_pb02;
    unsigned short	piu2_pb03;
    unsigned short	piu2_pb10;
    unsigned short	piu2_pb11;
    unsigned short	piu2_pb12;
    unsigned short	piu2_pb13;
    unsigned short	piu2_ab0;
    unsigned short	piu2_ab1;
    unsigned short	piu2_ab2;
    unsigned short	piu2_ab3;
    unsigned short	piu2_unused18;
    unsigned short	piu2_unused1a;
    unsigned short	piu2_pb04;
    unsigned short	piu2_pb14;
};
#endif

/* XXX define PIU2 bitfields */

/*
 * AIU - audio interface unit 
 */

#define AIU_MDMADAT	0x00
#define AIU_SDMADAT	0x02
#define AIU_SODAT	0x06
#define AIU_SCNT	0x08
#define AIU_SCNVR	0x0A
#define AIU_MIDAT	0x10
#define AIU_MCNT	0x12
#define AIU_MCNVR	0x14
#define AIU_DVALID	0x18
#define AIU_SEQ		0x1A
#define AIU_INT		0x1C

#ifndef __ASSEMBLER__
struct vr41xxaiu {
    unsigned short	aiu_mdmadat;
    unsigned short	aiu_sdmadat;
    unsigned short	aiu_unused04;
    unsigned short	aiu_sodat;
    unsigned short	aiu_scnt;
    unsigned short	aiu_scnvr;
    unsigned short	aiu_unused0c;
    unsigned short	aiu_unused0e;
    unsigned short	aiu_midat;
    unsigned short	aiu_mcnt;
    unsigned short	aiu_mcnvr;
    unsigned short	aiu_unused16;
    unsigned short	aiu_dvalid;
    unsigned short	aiu_seq;
    unsigned short	aiu_int;
};
#endif

#define AIUSCNT_SSTOPEN		0x0002
#define AIUSCNT_SSTATE		0x0008
#define AIUSCNT_DAENAIU		0x8000

#define AIUMCNT_ADREQAIU	0x0001
#define AIUMCNT_MSTOPEN		0x0002
#define AIUMCNT_MSTATE		0x0008
#define AIUMCNT_ADENAIU		0x8000

#define AIUCNV_CNVR_11K		0x0000
#define AIUCNV_CNVR_22K		0x0001
#define AIUCNV_CNVR_44K		0x0002
#define AIUCNV_CNVR_8K		0x0004

#define AIUDVALID_MDMAV		0x0001
#define AIUDVALID_MIDATV	0x0002
#define AIUDVALID_SDMAV		0x0004
#define AIUDVALID_SODATV	0x0008

#define AIUSEQ_AIUSEN		0x0001
#define AIUSEQ_AIUMEN		0x0010
#define AIUSEQ_AIURST		0x8000

#define AIUINT_SIDLEINTR	0x0002
#define AIUINT_SINTR		0x0004
#define AIUINT_SENDINTR		0x0008
#define AIUINT_MSTINTR		0x0100
#define AIUINT_MIDLEINTR	0x0200
#define AIUINT_MINTR		0x0400
#define AIUINT_MENDINTR		0x0800



/* 
 * KIU - keyboard interface unit
 */

#define KIU_DAT0	0x00
#define KIU_DAT1	0x02
#define KIU_DAT2	0x04
#define KIU_DAT3	0x06
#define KIU_DAT4	0x08
#define KIU_DAT5	0x0A
#define KIU_SCANREP	0x10
#define KIU_SCANS	0x12
#define KIU_WKS		0x14
#define KIU_WKI		0x16
#define KIU_INT		0x18
#define KIU_RST		0x1A
#define KIU_GPEN	0x1C
#define KIU_SCANLINE	0x1E

#ifndef __ASSEMBLER__
struct vr41xxkiu {
    unsigned short	kiu_dat0;
    unsigned short	kiu_dat1;
    unsigned short	kiu_dat2;
    unsigned short	kiu_dat3;
    unsigned short	kiu_dat4;
    unsigned short	kiu_dat5;
    unsigned short	kiu_unused0c;
    unsigned short	kiu_unused0e;
    unsigned short	kiu_scanrep;
    unsigned short	kiu_scans;
    unsigned short	kiu_wks;
    unsigned short	kiu_wki;
    unsigned short	kiu_int;
    unsigned short	kiu_rst;
    unsigned short	kiu_gpen;
    unsigned short	kiu_scanline;
};
#endif

#define KIUSCANREP_ATSCAN      0x0001
#define KIUSCANREP_ATSTP       0x0002
#define KIUSCANREP_SCANSTART   0x0004
#define KIUSCANREP_SCANSTP     0x0008
#define KIUSCANREP_STPREP_MASK 0x03f0
#define KIUSCANREP_STPREP_SHFT 4
#define KIUSCANREP_KEYEN       0x8000

#define KIUSCANS_SSTAT_MASK	0x0003
#define	 KIUSCANS_SSTAT_STOPPED	    0x0
#define	 KIUSCANS_SSTAT_WAITSCAN    0x1
#define	 KIUSCANS_SSTAT_WAITKEY	    0x2
#define	 KIUSCANS_SSTAT_SCANNING    0x3

#define KIUINT_SCANINT		0x0001
#define KIUINT_KDATRDY		0x0002
#define KIUINT_KDATLOST		0x0004

#define KIURST_KIURST	   0x0001

#define KIUSCANL_LINE_MASK	0x0003
#define	 KIUSCANL_LINE_12PINS	    0x0
#define	 KIUSCANL_LINE_10PINS	    0x1
#define	 KIUSCANL_LINE_8PINS	    0x2
#define	 KIUSCANL_LINE_NOPINS	    0x3


/*
 * DSIU - debug serial interface unit
 */

#define DSIU_PORT	0x00
#define DSIU_MODEM	0x02
#define DSIU_ASIM00	0x04
#define DSIU_ASIM01	0x06
#define DSIU_RXB0R	0x08
#define DSIU_RXB0L	0x0A
#define DSIU_TXS0R	0x0C
#define DSIU_TXS0L	0x0E
#define DSIU_ASIS0	0x10
#define DSIU_INTR0	0x12
#define DSIU_BPRM0	0x16
#define DSIU_RESET	0x18

#ifndef __ASSEMBLER__
struct vr41xxdsiu {
    unsigned short	dsiu_port;
    unsigned short	dsiu_modem;
    unsigned short	dsiu_asim00;
    unsigned short	dsiu_asim01;
    unsigned short	dsiu_rxb0r;
    unsigned short	dsiu_rxb0l;
    unsigned short	dsiu_txs0r;
    unsigned short	dsiu_txs0l;
    unsigned short	dsiu_asis0;
    unsigned short	dsiu_intr0;
    unsigned short	dsiu_unused14;
    unsigned short	dsiu_bprm0;
    unsigned short	dsiu_reset;
};
#endif

#define DSIUPORT_CDCTS		0x0001
#define DSIUPORT_CDRTS		0x0002
#define DSIUPORT_CDDOUT		0x0004
#define DSIUPORT_CDDIN		0x0008

#define DSIUMODEM_DCTS		0x0001
#define DSIUMODEM_DRTS		0x0002

#define DSIUASIM00_STP1		0x0000
#define DSIUASIM00_STP2		0x0004
#define DSIUASIM00_CS7		0x0000
#define DSIUASIM00_CS8		0x0008
#define DSIUASIM00_PNONE	0x0000
#define DSIUASIM00_PZERO	0x0010
#define DSIUASIM00_PODD		0x0020
#define DSIUASIM00_PEVEN	0x0030
#define DSIUASIM00_RXE0		0x0040

#define DSIUASIM01_EBS0		0x0001

#define DSIUASIS0_OVE0		0x0001
#define DSIUASIS0_FE0		0x0002
#define DSIUASIS0_PE0		0x0004
#define DSIUASIS0_SOT0		0x0080

#define DSIUINTR0_INTST0	0x0001
#define DSIUINTR0_INTSR0	0x0002
#define DSIUINTR0_INTSER0	0x0004
#define DSIUINTR0_INTDCD	0x0008

#define DSIUBPRM0_BPR_MASK	0x0007
#define	 DSIUBPRM0_BPR_B1200	    0x0
#define	 DSIUBPRM0_BPR_B2400	    0x1
#define	 DSIUBPRM0_BPR_B4800	    0x2
#define	 DSIUBPRM0_BPR_B9600	    0x3
#define	 DSIUBPRM0_BPR_B19200	    0x4
#define	 DSIUBPRM0_BPR_B38400	    0x5
#define	 DSIUBPRM0_BPR_B57600	    0x6
#define	 DSIUBPRM0_BPR_B115200	    0x7
#define DSIUBPRM0_BRCE0		0x0080

#define DSIURST_DSIURST		0x0001


/*
 * LED - LED control unit
 */

#define LED_HTS		0x00
#define LED_LTS		0x02
#define LED_CNT		0x08
#define LED_ASTC	0x0A
#define LED_INT		0x0C

#ifndef __ASSEMBLER__
struct vr41xxled {
    unsigned short	led_hts;
    unsigned short	led_lts;
    unsigned short	led_unused04;
    unsigned short	led_unused06;
    unsigned short	led_cnt;
    unsigned short	led_astc;
    unsigned short	led_int;
};
#endif

#define LEDCNT_LEDENABLE	0x0001
#define LEDCNT_LEDSTOP		0x0002

#define LEDINT_LEDINT		0x0001


/*
 * HSP - high-speed modem interface unit
 */

#define HSP_INIT		0x00
#define HSP_DATA		0x02
#define HSP_INDEX		0x04
#define HSP_ID			0x08
#define HSP_PCS			0x09
#define HSP_PCTEL		0x09

#ifndef __ASSEMBLER__
struct vr41xxhsp {
    unsigned short	init;
    unsigned short	data;
    unsigned short	index;
    unsigned short	unused06;
    unsigned char	id;
    unsigned char	pcs;
};
#endif

/* XXX define HSP bitfields */


/*
 * FIR - fast infrared interface unit
 */

#define FIR_FRST	0x00
#define FIR_DPINT	0x02
#define FIR_DPCNT	0x04
#define FIR_TD		0x10
#define FIR_RD		0x12
#define FIR_IM		0x14
#define FIR_FS		0x16
#define FIR_IRS		0x18
#define FIR_CRCS	0x1c
#define FIR_FIR_C	0x1e
#define FIR_MIRC	0x20
#define FIR_DMAC	0x22
#define FIR_DMAE	0x24
#define FIR_TXI		0x26
#define FIR_RXI		0x28
#define FIR_IF		0x2a
#define FIR_RXSTS	0x2c
#define FIR_TXFL	0x2e
#define FIR_MRXF	0x30
#define FIR_RXF		0x34

#ifndef __ASSEMBLER__
struct vr41xxfir {
    unsigned short	fir_frst;
    unsigned short	fir_dpint;
    unsigned short	fir_dpcnt;
    unsigned short	fir_unused06;
    unsigned short	fir_unused08;
    unsigned short	fir_unused0a;
    unsigned short	fir_unused0c;
    unsigned short	fir_unused0e;
    unsigned short	fir_td;
    unsigned short	fir_rd;
    unsigned short	fir_im;
    unsigned short	fir_fs;
    unsigned short	fir_irs;
    unsigned short	fir_unused1a;
    unsigned short	fir_crcs;
    unsigned short	fir_fir_c;
    unsigned short	fir_mirc;
    unsigned short	fir_dmac;
    unsigned short	fir_dmae;
    unsigned short	fir_txi;
    unsigned short	fir_rxi;
    unsigned short	fir_if;
    unsigned short	fir_rxsts;
    unsigned short	fir_txfl;
    unsigned short	fir_mrxf;
    unsigned short	fir_unused32;
    unsigned short	fir_rxf;
};
#endif

/* XXX define FIR bitfields */

#endif /* _VR41XXIO_H_ */

