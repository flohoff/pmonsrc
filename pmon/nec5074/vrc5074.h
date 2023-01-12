/*
 * nec/vr5074.h: NEC Vrc5074 (Nile4) system controller definitions
 * Copyright (c) 1999 Algorithmics Ltd
 */

#ifndef __VRC5074_H__
#define __VRC5074_H__

/*
 * Vrc5074 register offsets 
 */
#define N4_SDRAM0	0x0000
#define N4_SDRAM1	0x0008
#define N4_DCS2		0x0010
#define N4_DCS3		0x0018
#define N4_DCS4		0x0020
#define N4_DCS5		0x0028
#define N4_DCS6		0x0030
#define N4_DCS7		0x0038
#define N4_DCS8		0x0040
#define N4_0048		0x0048
#define N4_0050		0x0050
#define N4_0058		0x0058
#define N4_PCIW0	0x0060
#define N4_PCIW1	0x0068
#define N4_INTCS	0x0070
#define N4_BOOTCS	0x0078
#define N4_CPUSTAT	0x0080
#define N4_INTCTRL	0x0088
#define N4_INTSTAT0	0x0090
#define N4_INTSTAT1	0x0098
#define N4_INTCLR	0x00a0
#define N4_INTPPES	0x00a8
#define N4_00b0       	0x00b0
#define N4_PCIERR	0x00b8
#define N4_MEMCTRL	0x00c0
#define N4_ACSTIME	0x00c8
#define N4_CHKERR	0x00d0
#define N4_00d8		0x00d8
#define N4_PCICTRL	0x00e0
#define N4_PCIARB	0x00e8
#define N4_PCIINIT0	0x00f0
#define N4_PCIINIT1	0x00f8
#define N4_LCNFG	0x0100
#define N4_0108		0x0108
#define N4_LCST2	0x0110
#define N4_LCST3	0x0118
#define N4_LCST4	0x0120
#define N4_LCST5	0x0128
#define N4_LCST6	0x0130
#define N4_LCST7	0x0138
#define N4_LCST8	0x0140
#define N4_0148		0x0148
#define N4_DCSFN	0x0150
#define N4_DCSIO	0x0158
#define N4_0160		0x0160
#define N4_0168		0x0168
#define N4_0170		0x0170
#define N4_BCST		0x0178
#define N4_DMACTRL0	0x0180
#define N4_DMASRCA0	0x0188
#define N4_DMADESA0	0x0190
#define N4_DMACTRL1	0x0198
#define N4_DMASRCA1	0x01a0
#define N4_DMADESA1	0x01a8
#define N4_01b0		0x01b0
#define N4_01b8		0x01b8
#define N4_T0CTRL	0x01c0
#define N4_T0CNTR	0x01c8
#define N4_T1CTRL	0x01d0
#define N4_T1CNTR	0x01d8
#define N4_T2CTRL	0x01e0
#define N4_T2CNTR	0x01e8
#define N4_T3CTRL	0x01f0
#define N4_T3CNTR	0x01f8
#define N4_PCICFG	0x0200
#define N4_UART		0x0300

/*
 * Vrc5074 local PCI config space offsets (or add to N4_PCICFG)
 */
#define N4P_VID		0x00	/* vendor id */
#define N4P_DID		0x02	/* device id */
#define N4P_CMD		0x04	/* command */
#define N4P_STS		0x06	/* status */
#define N4P_CLASS	0x08	/* class + revid */
#define N4P_CLSIZ	0x0c	/* cache line size */
#define N4P_MLTIM	0x0d 	/* master latency timer */
#define N4P_HTYPE	0x0e	/* header type */
#define N4P_BIST	0x0f	/* self test */
#define N4P_BARC	0x10	/* controller base addr */
#define N4P_BAR0	0x18	/* sdram bank 0 addr */
#define N4P_BAR1	0x20	/* sdram bank 1 addr */
#define N4P_CIS		0x28	/* cardbus cis pointer */
#define N4P_SSVID	0x2c 	/* sub-system vendor id */
#define N4P_SSID	0x2e	/* sub-system id */
#define N4P_EXPROMBAR	0x30	/* expansion rom addr */
#define N4P_RESVD34	0x34
#define N4P_RESVD38	0x38
#define N4P_INTLIN	0x3c	/* interrupt line */
#define N4P_INTPIN	0x3d	/* interrupt pin */
#define N4P_MINGNT	0x3e	/* min grant */
#define N4P_MAXLAT	0x3f	/* max latency */
#define N4P_BAR2	0x40	/* dcs2 addr */
#define N4P_BAR3	0x48	/* dcs3 addr */
#define N4P_BAR4	0x50	/* dcs4 addr */
#define N4P_BAR5	0x58	/* dcs5 addr */
#define N4P_BAR6	0x60	/* dcs6 addr */
#define N4P_BAR7	0x68	/* dcs7 addr */
#define N4P_BAR8	0x70	/* dcs8 addr */
#define N4P_BARB	0x78	/* bootcs addr */

#ifndef __ASSEMBLER__

/*
 * Vrc5074 control registers
 */
struct vrc5074 {
	unsigned long long	n4_sdram0;
	unsigned long long	n4_sdram1;
	unsigned long long	n4_dcs2;
	unsigned long long	n4_dcs3;
	unsigned long long	n4_dcs4;
	unsigned long long	n4_dcs5;
	unsigned long long	n4_dcs6;
	unsigned long long	n4_dcs7;
	unsigned long long	n4_dcs8;
	unsigned long long	n4_0048;
	unsigned long long	n4_0050;
	unsigned long long	n4_0058;
	unsigned long long	n4_pciw0;
	unsigned long long	n4_pciw1;
	unsigned long long	n4_intcs;
	unsigned long long	n4_bootcs;
	unsigned long long	n4_cpustat;
	unsigned long long	n4_intctrl;
	unsigned long long	n4_intstat0;
	unsigned long long	n4_intstat1;
	unsigned long long	n4_intclr;
	unsigned long long	n4_intppes;
	unsigned long long	n4_00b0       ;
	unsigned long long	n4_pcierr;
	unsigned long long	n4_memctrl;
	unsigned long long	n4_acstime;
	unsigned long long	n4_chkerr;
	unsigned long long	n4_00d8;
	unsigned long long	n4_pcictrl;
	unsigned long long	n4_pciarb;
	unsigned long long	n4_pciinit0;
	unsigned long long	n4_pciinit1;
	unsigned long long	n4_lcnfg;
	unsigned long long	n4_0108;
	unsigned long long	n4_lcst2;
	unsigned long long	n4_lcst3;
	unsigned long long	n4_lcst4;
	unsigned long long	n4_lcst5;
	unsigned long long	n4_lcst6;
	unsigned long long	n4_lcst7;
	unsigned long long	n4_lcst8;
	unsigned long long	n4_0148;
	unsigned long long	n4_dcsfn;
	unsigned long long	n4_dcsio;
	unsigned long long	n4_0160;
	unsigned long long	n4_0168;
	unsigned long long	n4_0170;
	unsigned long long	n4_bcst;
	unsigned long long	n4_dmactrl0;
	unsigned long long	n4_dmasrca0;
	unsigned long long	n4_dmadesa0;
	unsigned long long	n4_dmactrl1;
	unsigned long long	n4_dmasrca1;
	unsigned long long	n4_dmadesa1;
	unsigned long long	n4_01b0;
	unsigned long long	n4_01b8;
	unsigned long long	n4_t0ctrl;
	unsigned long long	n4_t0cntr;
	unsigned long long	n4_t1ctrl;
	unsigned long long	n4_t1cntr;
	unsigned long long	n4_t2ctrl;
	unsigned long long	n4_t2cntr;
	unsigned long long	n4_t3ctrl;
	unsigned long long	n4_t3cntr;
	char			n4_pcicfg[0x100];
	unsigned long long 	n4_uart[8];
};


/*
 * Vrc5074 local PCI config space (or overlay over n4_pcicfg)
 */
struct vrc5074_pcicfg {
    	unsigned short		n4p_vid;	/* 00 - vendor id */
    	unsigned short		n4p_did;	/* 02 - device id */
    	unsigned short		n4p_cmd;	/* 04 - command */
    	unsigned short		n4p_sts;	/* 06 - status */
    	unsigned long		n4p_class;	/* 08 - class + revid */
    	unsigned char		n4p_clsiz;	/* 0c - cache line size */
    	unsigned char		n4p_mltim;	/* 0d - master latency timer */
    	unsigned char		n4p_htype;	/* 0e - header type */
    	unsigned char		n4p_bist;	/* 0f - self test */
    	unsigned long long	n4p_barc;	/* 10 - controller base addr */
    	unsigned long long	n4p_bar0;	/* 18 - sdram bank 0 addr */
    	unsigned long long	n4p_bar1;	/* 20 - sdram bank 1 addr */
    	unsigned long 		n4p_cis;	/* 28 - cardbus cis pointer */
    	unsigned short 		n4p_ssvid;	/* 2c - sub-system vendor id */
    	unsigned short 		n4p_ssid;	/* 2e - sub-system id */
    	unsigned long 		n4p_exprombar;	/* 30 - expansion rom addr */
    	unsigned long 		n4p_resvd34;
    	unsigned long 		n4p_resvd38;
    	unsigned char		n4p_intlin;	/* 3c - interrupt line */
    	unsigned char		n4p_intpin;	/* 3d - interrupt pin */
    	unsigned char		n4p_mingnt;	/* 3e - min grant */
    	unsigned char		n4p_maxlat;	/* 3f - max latency */
    	unsigned long long	n4p_bar2;	/* 40 - dcs2 addr */
    	unsigned long long	n4p_bar3;	/* 48 - dcs3 addr */
    	unsigned long long	n4p_bar4;	/* 50 - dcs4 addr */
    	unsigned long long	n4p_bar5;	/* 58 - dcs5 addr */
    	unsigned long long	n4p_bar6;	/* 60 - dcs6 addr */
    	unsigned long long	n4p_bar7;	/* 68 - dcs7 addr */
    	unsigned long long	n4p_bar8;	/* 70 - dcs8 addr */
    	unsigned long long	n4p_barb;	/* 78 - bootcs addr */
};

/* long long constant for C */
#define N4CONST(x)	x ## ULL
#define N4CAST(x)	((unsigned long long)(x))

#else

/* long long constant for assembler */
#define N4CONST(x)	x
#define N4CAST(x)	(x)

#endif


/* 
 * Physical Device Address Registers (PDARs) 
 */
#define N4_PDAR_SIZE_MASK	0x000000000f
# define N4_PDAR_OFF	 	 0x0
# define N4_PDAR_4GB	 	 0x4
# define N4_PDAR_2GB	 	 0x5
# define N4_PDAR_1GB	 	 0x6
# define N4_PDAR_512MB	 	 0x7
# define N4_PDAR_256MB	 	 0x8
# define N4_PDAR_128MB	 	 0x9
# define N4_PDAR_64MB	 	 0xa
# define N4_PDAR_32MB	 	 0xb
# define N4_PDAR_16MB	 	 0xc
# define N4_PDAR_8MB	 	 0xd
# define N4_PDAR_4MB	 	 0xe
# define N4_PDAR_2MB	 	 0xf
#define N4_PDAR_MEM		0x000000010
#define N4_PDAR_LOC		0x000000000
#define N4_PDAR_VISPCI		0x000000020
#define N4_PDAR_WIDTH_MASK	0x0000000c0
# define N4_PDAR_8BIT		 (0x0<<6)
# define N4_PDAR_16BIT		 (0x1<<6)
# define N4_PDAR_32BIT		 (0x2<<6)
# define N4_PDAR_64BIT		 (0x3<<6)
#define N4_PDAR_ADDR_MASK	N4CONST(0xfffe00000)


/*
 * CPU Interface Registers 
 */
#define N4_CPUSTAT_CLDRST	0x00000001
#define N4_CPUSTAT_WARMRST	0x00000002
#define N4_CPUSTAT_DISPC	0x00000004
#define N4_CPUSTAT_DISCPUPC	0x00000008
#define N4_CPUSTAT_TMODE_MASK	0x00000030
# define N4_CPUSTAT_TMODE_SBAD	 (0x0<<4)
# define N4_CPUSTAT_TMODE_SGOOD	 (0x1<<4)
# define N4_CPUSTAT_TMODE_MBAD	 (0x2<<4)
# define N4_CPUSTAT_TMODE_MGOOD	 (0x3<<4)
#define N4_CPUSTAT_CTRLNUM_MASK	0x00000300
#define N4_CPUSTAT_CTRLNUM_SHFT	8
#define N4_CPUSTAT_MAINCTRL	0x00000400

/* interrupt sources */
#define N4_INT_CPCE		0
#define N4_INT_CNTD		1
#define N4_INT_MCE		2
#define N4_INT_DMA		3
#define N4_INT_UART		4
#define N4_INT_WDOG		5
#define N4_INT_GPT		6
#define N4_INT_LBRT		7
#define N4_INT_INTA		8
#define N4_INT_INTB		9
#define N4_INT_INTC		10
#define N4_INT_INTD		11
#define N4_INT_INTE		12
#define N4_INT_13		13
#define N4_INT_PCIS		14
#define N4_INT_PCIE		15

#define N4_INTCTRL_MASK(dev)	(N4CAST(0xf)<<((dev)*4))
#define N4_INTCTRL_PRI(dev,pri)	(N4CAST(pri)<<((dev)*4))
#define N4_INTCTRL_PRI_NMI(dev)	(N4CONST(0x6)<<((dev)*4))
#define N4_INTCTRL_EN(dev)	(N4CONST(0x8)<<((dev)*4))

#define N4_INTSTAT0_IL0_SHIFT	0
#define N4_INTSTAT0_IL0_MASK	(N4CONST(0xffff) << 0)
#define N4_INTSTAT0_IL0(dev)	(N4CONST(1) << ((dev)+0))
#define N4_INTSTAT0_IL1_SHIFT	16
#define N4_INTSTAT0_IL1_MASK	(N4CONST(0xffff) << 16)
#define N4_INTSTAT0_IL1(dev)	(N4CONST(1) << ((dev)+16))
#define N4_INTSTAT0_IL2_SHIFT	32
#define N4_INTSTAT0_IL2_MASK	(N4CONST(0xffff) << 32)
#define N4_INTSTAT0_IL2(dev)	(N4CONST(1) << ((dev)+32))
#define N4_INTSTAT0_IL3_SHIFT	48
#define N4_INTSTAT0_IL3_MASK	(N4CONST(0xffff) << 48)
#define N4_INTSTAT0_IL3(dev)	(N4CONST(1) << ((dev)+48))

#define N4_INTSTAT1_IL4_SHIFT	0
#define N4_INTSTAT1_IL4_MASK	(N4CONST(0xffff) << 0)
#define N4_INTSTAT1_IL4(dev)	(N4CONST(1) << ((dev)+0))
#define N4_INTSTAT1_IL5_SHIFT	16
#define N4_INTSTAT1_IL5_MASK	(N4CONST(0xffff) << 16)
#define N4_INTSTAT1_IL5(dev)	(N4CONST(1) << ((dev)+16))
#define N4_INTSTAT1_NMI(dev)	(N4CONST(1) << ((dev)+32))
#define N4_INTSTAT1_OE(pri)	(N4CONST(1) << ((pri)+48))
#define N4_INTSTAT1_OE_NMI	(N4CONST(1) << (6+48))
#define N4_INTSTAT1_OEMASK	(N4CONST(0x7f) << 48)

#define N4_INTCLR_DEV(dev)	(N4CONST(1) << (dev))

/* PCI Interrupt Sources */
#define N4_INTP_A		0
#define N4_INTP_B		1
#define N4_INTP_C		2
#define N4_INTP_D		3
#define N4_INTP_E		4

#define N4_INTPPES_HIGH(src)	(0x0 << ((src) * 2))
#define N4_INTPPES_LOW(src)	(0x1 << ((src) * 2))
#define N4_INTPPES_EDGE(src)	(0x0 << ((src) * 2))
#define N4_INTPPES_LEVEL(src)	(0x2 << ((src) * 2))


/*
 * Timer Registers 
 */
#define N4_TnCTRL_EN		N4CONST(0x0000000100000000)
#define N4_TnCTRL_PREN		N4CONST(0x0000000200000000)
#define N4_TnCTRL_PRSRC_MASK	N4CONST(0x0000000c00000000)
# define N4_TnCTRL_PRSRC_0	 N4CONST(0x0000000000000000)
# define N4_TnCTRL_PRSRC_1	 N4CONST(0x0000000400000000)
# define N4_TnCTRL_PRSRC_2	 N4CONST(0x0000000800000000)
# define N4_TnCTRL_PRSRC_3	 N4CONST(0x0000000c00000000)


/*
 * Memory Control Registers 
 */
#define N4_MEMCTRL_DRAMTYP_MASK	0x00000003
# define N4_MEMCTRL_DRAMTYP_16MB_2BANK	0x00000000
# define N4_MEMCTRL_DRAMTYP_64MB_2BANK	0x00000001
# define N4_MEMCTRL_DRAMTYP_64MB_4BANK	0x00000002
# define N4_MEMCTRL_DRAMTYP_256MB_4BANK	0x00000003
#define N4_MEMCTRL_CHKMODE	0x00000004
# define N4_MEMCTRL_CHKMODE_ECC	 0x00000004
# define N4_MEMCTRL_CHKMODE_PAR  0x00000000
#define N4_MEMCTRL_CHKDIS	0x00000008
#define N4_MEMCTRL_CHKENB	0x00000000
#define N4_MEMCTRL_ENABLE	0x00000010
#define N4_MEMCTRL_ILEAVD	0x00000040
#define N4_MEMCTRL_HOLDLD	0x00000080

#define N4_ACSTIME_ACCT_MASK	0x0000001f
#define N4_ACSTIME_ACCT_SHFT	0
#define N4_ACSTIME_ACCT(n)	(n)
#define N4_ACSTIME_DISMRDY	0x00000100

#define N4_CHKERR_CEADDR_MASK	N4CONST(0x0000000fffffffff)
#define N4_CHKERR_CESYN_MASK    N4CONST(0x00ff000000000000)
#define N4_CHKERR_CESYN_SHFT	48
#define N4_CHKERR_PCHKERR	N4CONST(0x0100000000000000)
#define N4_CHKERR_ECHKERR	N4CONST(0x0200000000000000)
#define N4_CHKERR_MCHKERR	N4CONST(0x0400000000000000)


/* 
 * PCI Control Registers 
 */
#define N4_PCICTRL_PCISYNC	N4CONST(0x0000000000000001)
#define N4_PCICTRL_CLKSEL_MASK	N4CONST(0x000000000000000e)
#define N4_PCICTRL_CLKSEL_SHFT	1
#define  N4_PCICTRL_CLKSEL_1_3	 (0<<1)
#define  N4_PCICTRL_CLKSEL_2_3	 (1<<1)
#define  N4_PCICTRL_CLKSEL_1_2	 (2<<1)
#define  N4_PCICTRL_CLKSEL_1_1	 (3<<1)
#define  N4_PCICTRL_CLKSEL_PCLK	 (4<<1)
#define N4_PCICTRL_CPUHOG_MASK	N4CONST(0x00000000000000f0)
#define N4_PCICTRL_CPUHOG_SHFT	4
#define N4_PCICTRL_CPUHOG(x)	(((x) & 0xf) << 4)
#define N4_PCICTRL_DMAHOG_MASK	N4CONST(0x0000000000000f00)
#define N4_PCICTRL_DMAHOG_SHFT	8
#define N4_PCICTRL_DMAHOG(x)	(((x) & 0xf) << 8)
#define N4_PCICTRL_FAPER	N4CONST(0x0000000000002000)
#define N4_PCICTRL_FDPER	N4CONST(0x0000000000004000)
#define N4_PCICTRL_FIFOSTALL	N4CONST(0x0000000000008000)
#define N4_PCICTRL_RTYLIM_MASK	N4CONST(0x0000000000ff0000)
#define N4_PCICTRL_RTYLIM_SHFT	16
#define N4_PCICTRL_RTYLIM(x)	(((x) >> 8) << 16)
#define N4_PCICTRL_DISCTIM_MASK	N4CONST(0x00000000ff000000)
#define N4_PCICTRL_DISCTIM_SHFT	24
#define N4_PCICTRL_DISCTIM(x)	(N4CAST((x) >> 8) << 24)
#define N4_PCICTRL_TACH		N4CONST(0x0000000100000000)
#define N4_PCICTRL_MACH		N4CONST(0x0000000200000000)
#define N4_PCICTRL_RTYCH	N4CONST(0x0000000400000000)
#define N4_PCICTRL_PERCH	N4CONST(0x0000000800000000)
#define N4_PCICTRL_DTIMCH	N4CONST(0x0000001000000000)
#define N4_PCICTRL_ERRTYPE_MASK	N4CONST(0x000000e000000000)
#define N4_PCICTRL_ERRTYPE_SHFT	37
# define N4_PCICTRL_ERRTYPE_TABORT	(N4CONST(1)<<37)
# define N4_PCICTRL_ERRTYPE_MABORT	(N4CONST(2)<<37)
# define N4_PCICTRL_ERRTYPE_RETRYLIM	(N4CONST(3)<<37)
# define N4_PCICTRL_ERRTYPE_RDPERR	(N4CONST(4)<<37)
# define N4_PCICTRL_ERRTYPE_WRPERR	(N4CONST(5)<<37)
# define N4_PCICTRL_ERRTYPE_DTIMEX	(N4CONST(6)<<37)
#define N4_PCICTRL_TASE		N4CONST(0x0000010000000000)
#define N4_PCICTRL_MASE		N4CONST(0x0000020000000000)
#define N4_PCICTRL_RTYSE	N4CONST(0x0000040000000000)
#define N4_PCICTRL_PERSE	N4CONST(0x0000080000000000)
#define N4_PCICTRL_DTIMSE	N4CONST(0x0000100000000000)
#define N4_PCICTRL_AERSE	N4CONST(0x0000200000000000)
#define N4_PCICTRL_INT1SE	N4CONST(0x0000400000000000)
#define N4_PCICTRL_TAIN		N4CONST(0x0001000000000000)
#define N4_PCICTRL_MAIN		N4CONST(0x0002000000000000)
#define N4_PCICTRL_RTYIN	N4CONST(0x0004000000000000)
#define N4_PCICTRL_PERIN	N4CONST(0x0008000000000000)
#define N4_PCICTRL_DTIMIN	N4CONST(0x0010000000000000)
#define N4_PCICTRL_AERIN	N4CONST(0x0020000000000000)
#define N4_PCICTRL_INTAEN	N4CONST(0x0100000000000000)
#define N4_PCICTRL_LATDIS	N4CONST(0x0800000000000000)
#define N4_PCICTRL_PLL_SYNC	N4CONST(0x1000000000000000)
#define N4_PCICTRL_PLL_STBY	N4CONST(0x2000000000000000)
#define N4_PCICTRL_PCIWRST	N4CONST(0x4000000000000000)
#define N4_PCICTRL_PCICRST	N4CONST(0x8000000000000000)

#define N4_PCIARB_GROUP0_MASK	N4CONST(0x000000000000003f)
#define N4_PCIARB_GROUP0_SHFT	0
# define N4_PCIARB_GROUP_REQ0	 N4CONST(0x01)
# define N4_PCIARB_GROUP_REQ1	 N4CONST(0x02)
# define N4_PCIARB_GROUP_REQ2	 N4CONST(0x04)
# define N4_PCIARB_GROUP_REQ3	 N4CONST(0x08)
# define N4_PCIARB_GROUP_REQ4	 N4CONST(0x10)
# define N4_PCIARB_GROUP_CTLR	 N4CONST(0x20)
#define N4_PCIARB_GROUP1_MASK	N4CONST(0x0000000000003f00)
#define N4_PCIARB_GROUP1_SHFT	8
#define N4_PCIARB_GROUP2_MASK	N4CONST(0x00000000003f0000)
#define N4_PCIARB_GROUP2_SHFT	16
#define N4_PCIARB_CONS0_MASK	N4CONST(0x000000000f000000)
#define N4_PCIARB_CONS0_SHFT	24
#define N4_PCIARB_CONS0(n)	(N4CAST((n) & 0xf) << 24)
#define N4_PCIARB_CONS0N_MASK	N4CONST(0x00000000f0000000)
#define N4_PCIARB_CONS0N_SHFT	28
#define N4_PCIARB_CONS0N(n)	(N4CAST((n) & 0xf) << 28)
#define N4_PCIARB_CONS1_MASK	N4CONST(0x0000000f00000000)
#define N4_PCIARB_CONS1_SHFT	32
#define N4_PCIARB_CONS1(n)	(N4CAST((n) & 0xf) << 32)
#define N4_PCIARB_CONS2_MASK	N4CONST(0x000000f000000000)
#define N4_PCIARB_CONS2_SHFT	36
#define N4_PCIARB_CONS2(n)	(N4CAST((n) & 0xf) << 36)
#define N4_PCIARB_PARK0_MASK	N4CONST(0x00000f0000000000)
#define N4_PCIARB_PARK0_SHFT	40
#define N4_PCIARB_PARK0(n)	(N4CAST((n) & 0xf) << 40)
#define N4_PCIARB_PARK1_MASK	N4CONST(0x0000f00000000000)
#define N4_PCIARB_PARK1_SHFT	44
#define N4_PCIARB_PARK1(n)	(N4CAST((n) & 0xf) << 44)
#define N4_PCIARB_PARK2_MASK	N4CONST(0x000f000000000000)
#define N4_PCIARB_PARK2_SHFT	48
#define N4_PCIARB_PARK2(n)	(N4CAST((n) & 0xf) << 48)
#define N4_PCIARB_DEFGNT_MASK	N4CONST(0x0070000000000000)
#define N4_PCIARB_DEFGNT_SHFT	52
# define N4_PCIARB_DEFGNT_GNT0	 (N4CONST(0x0)<<52)
# define N4_PCIARB_DEFGNT_GNT1	 (N4CONST(0x1)<<52)
# define N4_PCIARB_DEFGNT_GNT2	 (N4CONST(0x2)<<52)
# define N4_PCIARB_DEFGNT_GNT3	 (N4CONST(0x3)<<52)
# define N4_PCIARB_DEFGNT_GNT4	 (N4CONST(0x4)<<52)
# define N4_PCIARB_DEFGNT_CTLR	 (N4CONST(0x5)<<52)
#define N4_PCIARB_ARBDISABLE	N4CONST(0x8000000000000000)

#define N4_PCIINIT_TYPE_MASK	N4CONST(0x000000000000000e)
#define N4_PCIINIT_TYPE_SHFT	0
# define N4_PCIINIT_TYPE_SPEC	 N4CONST(0x0)
# define N4_PCIINIT_TYPE_IO	 N4CONST(0x2)
# define N4_PCIINIT_TYPE_MEM	 N4CONST(0x6)
# define N4_PCIINIT_TYPE_CONF	 N4CONST(0xa)
# define N4_PCIINIT_TYPE_MULT	 N4CONST(0xc)
#define N4_PCIINIT_ACCESS	N4CONST(0x0000000000000010)
# define N4_PCIINIT_32BIT	 N4CONST(0x0000000000000010)
# define N4_PCIINIT_64BIT	 N4CONST(0x0000000000000000)
#define N4_PCIINIT_LOCK		N4CONST(0x0000000000000020)
#define N4_PCIINIT_COMBINING	N4CONST(0x0000000000000040)
#define N4_PCIINIT_MERGING	N4CONST(0x0000000000000080)
#define N4_PCIINIT_PREFETCHABLE	N4CONST(0x0000000000000100)
#define N4_PCIINIT_CONFIGTYPE1	N4CONST(0x0000000000000200)
#define N4_PCIINIT_CONFIGTYPE0	N4CONST(0x0000000000000000)
#define N4_PCIINIT_SINGLE_PFB_MASK N4CONST(0x0000000000007c00)
#define N4_PCIINIT_SINGLE_PFB_SHFT 10
#define N4_PCIINIT_SINGLE_PFB(n)   (N4CAST((n) & 0x1f) << 10)
#define N4_PCIINIT_BLOCK_PFB_MASK  N4CONST(0x00000000001f8000)
#define N4_PCIINIT_BLOCK_PFB_SHFT  15
#define N4_PCIINIT_BLOCK_PFB(n)    (N4CAST((n) & 0x3f) << 15)
#define N4_PCIINIT_PCIADD_MASK	N4CONST(0xffffffffffe00000)

#define N4_PCIERR_IS_CPU	N4CONST(0x0000000000000001)
#define N4_PCIERR_ADDR_MASK	N4CONST(0xfffffffffffffffc)


/*
 * Local Bus Registers 
 */
#define N4_LCNFG_ARBMODE	N4CONST(0x0000000000000001)
# define N4_LCNFG_ARBMODE_M68K	 N4CONST(0x0000000000000001)
# define N4_LCNFG_ARBMODE_INTEL	 N4CONST(0x0000000000000000)
#define N4_LCNFG_ARBENB		N4CONST(0x0000000000000002)
#define N4_LCNFG_FLCLCLK	N4CONST(0x0000000000000010)
# define N4_LCNFG_FLCLCLK_DIV2	 N4CONST(0x0000000000000010)
# define N4_LCNFG_FLCLCLK_DIV4	 N4CONST(0x0000000000000000)
#define N4_LCNFG_DMAHOG_MASK	N4CONST(0x00000000000f0000)
#define N4_LCNFG_DMAHOG_SHFT	16
#define N4_LCNFG_DMAHOG(n)	(N4CAST((n) - 1) << 16)
#define N4_LCNFG_PCIHOG_MASK	N4CONST(0x0000000000f00000)
#define N4_LCNFG_PCIHOG_SHFT	20
#define N4_LCNFG_PCIHOG(n)	(N4CAST((n) - 1) << 20)
#define N4_LCNFG_CPUHOG_MASK	N4CONST(0x00000000000f0000)
#define N4_LCNFG_CPUHOG_SHFT	24
#define N4_LCNFG_CPUHOG(n)	(N4CAST((n) - 1) << 24)

#define N4_LCST_CSON_MASK	N4CONST(0x0000000000000001)
#define N4_LCST_CSON_SHFT	0
#define N4_LCST_CSON(n)		(N4CAST(n))
#define N4_LCST_CONSET_MASK	N4CONST(0x0000000000000006)
#define N4_LCST_CONSET_SHFT	1
#define N4_LCST_CONSET(n)	(N4CAST(n) << 1)
#define N4_LCST_CONWID_MASK	N4CONST(0x00000000000001f8)
#define N4_LCST_CONWID_SHFT	3
#define N4_LCST_CONWID(n)	(N4CAST((n) - 1) << 3)
#define N4_LCST_SUBSCWID_MASK	N4CONST(0x0000000000007e00)
#define N4_LCST_SUBSCWID_SHFT	9
#define N4_LCST_SUBSCWID(n)	(N4CAST((n) - 1) << 9)
#define N4_LCST_CSOFF_MASK	N4CONST(0x0000000000018000)
#define N4_LCST_CSOFF_SHFT	15
#define N4_LCST_CSOFF(n)	(N4CAST(n) << 15)
#define N4_LCST_COFHOLD_MASK	N4CONST(0x0000000000060000)
#define N4_LCST_COFHOLD_SHFT	17
#define N4_LCST_COFHOLD(n)	(N4CAST(n) << 17)
#define N4_LCST_BUSIDLE_MASK	N4CONST(0x0000000000380000)
#define N4_LCST_BUSIDLE_SHFT	19
#define N4_LCST_BUSIDLE(n)	(N4CAST(n) << 19)
#define N4_LCST_RDYMODE		N4CONST(0x0000000000400000)
#define N4_LCST_RDYMODE_LOCRDY	N4CONST(0x0000000000400000)
#define N4_LCST_RDYMODE_FIXED	N4CONST(0x0000000000000000)
#define N4_LCST_RDYSYN		N4CONST(0x0000000000800000)
#define N4_LCST_CONOFF_MASK	N4CONST(0x0000000003000000)
#define N4_LCST_CONOFF_SHFT	24
#define N4_LCST_CONOFF(n)	(N4CAST(n) << 24)
#define N4_LCST_CS_POL		N4CONST(0x0000000004000000)
# define N4_LCST_CS_POL_HI	 N4CONST(0x0000000004000000)
# define N4_LCST_CS_POL_LO	 N4CONST(0x0000000000000000)
#define N4_LCST_CON_POL		N4CONST(0x0000000008000000)
# define N4_LCST_CON_POL_HI	 N4CONST(0x0000000008000000)
# define N4_LCST_CON_POL_LO	 N4CONST(0x0000000000000000)

#define N4_DCSFN_DCSFN2_MASK	N4CONST(0x0000000000000007)
#define N4_DCSFN_DCSFN2_SHFT	0
# define N4_DCSFN_DCSFN2_GPI	 (N4CONST(0x0)<<0)
# define N4_DCSFN_DCSFN2_BUS	 (N4CONST(0x1)<<0)
# define N4_DCSFN_DCSFN2_GPO	 (N4CONST(0x3)<<0)
# define N4_DCSFN_DCSFN2_RTS	 (N4CONST(0x5)<<0)
#define N4_DCSFN_DCSFN3_MASK	N4CONST(0x0000000000000070)
#define N4_DCSFN_DCSFN3_SHFT	4
# define N4_DCSFN_DCSFN3_GPI	 (N4CONST(0x0)<<4)
# define N4_DCSFN_DCSFN3_BUS	 (N4CONST(0x1)<<4)
# define N4_DCSFN_DCSFN3_GPO	 (N4CONST(0x3)<<4)
# define N4_DCSFN_DCSFN3_CTS	 (N4CONST(0x6)<<4)
#define N4_DCSFN_DCSFN4_MASK	N4CONST(0x0000000000000700)
#define N4_DCSFN_DCSFN4_SHFT	8
# define N4_DCSFN_DCSFN4_GPI	 (N4CONST(0x0)<<8)
# define N4_DCSFN_DCSFN4_BUS	 (N4CONST(0x1)<<8)
# define N4_DCSFN_DCSFN4_GPO	 (N4CONST(0x3)<<8)
# define N4_DCSFN_DCSFN4_DCD	 (N4CONST(0x6)<<8)
#define N4_DCSFN_DCSFN5_MASK	N4CONST(0x0000000000007000)
#define N4_DCSFN_DCSFN5_SHFT	12
# define N4_DCSFN_DCSFN5_GPI	 (N4CONST(0x0)<<12)
# define N4_DCSFN_DCSFN5_BUS	 (N4CONST(0x1)<<12)
# define N4_DCSFN_DCSFN5_GPO	 (N4CONST(0x3)<<12)
# define N4_DCSFN_DCSFN5_XIN	 (N4CONST(0x6)<<12)
#define N4_DCSFN_DCSFN6_MASK	N4CONST(0x0000000000070000)
#define N4_DCSFN_DCSFN6_SHFT	16
# define N4_DCSFN_DCSFN6_GPI	 (N4CONST(0x0)<<16)
# define N4_DCSFN_DCSFN6_BUS	 (N4CONST(0x1)<<16)
# define N4_DCSFN_DCSFN6_GPO	 (N4CONST(0x3)<<16)
# define N4_DCSFN_DCSFN6_DACK	 (N4CONST(0x5)<<16)
#define N4_DCSFN_DCSFN7_MASK	N4CONST(0x0000000000700000)
#define N4_DCSFN_DCSFN7_SHFT	20
# define N4_DCSFN_DCSFN7_GPI	 (N4CONST(0x0)<<20)
# define N4_DCSFN_DCSFN7_BUS	 (N4CONST(0x1)<<20)
# define N4_DCSFN_DCSFN7_GPO	 (N4CONST(0x3)<<20)
# define N4_DCSFN_DCSFN7_DREQ	 (N4CONST(0x6)<<20)
#define N4_DCSFN_DCSFN8_MASK	N4CONST(0x0000000007000000)
#define N4_DCSFN_DCSFN8_SHFT	24
# define N4_DCSFN_DCSFN8_GPI	 (N4CONST(0x0)<<24)
# define N4_DCSFN_DCSFN8_BUS	 (N4CONST(0x1)<<24)
# define N4_DCSFN_DCSFN8_GPO	 (N4CONST(0x3)<<24)
# define N4_DCSFN_DCSFN8_DEOT	 (N4CONST(0x6)<<24)

#define N4_DCSIO_DCSL2IN	N4CONST(0x00000001)
#define N4_DCSIO_DCSL3IN	N4CONST(0x00000002)
#define N4_DCSIO_DCSL4IN	N4CONST(0x00000004)
#define N4_DCSIO_DCSL5IN	N4CONST(0x00000008)
#define N4_DCSIO_DCSL6IN	N4CONST(0x00000010)
#define N4_DCSIO_DCSL7IN	N4CONST(0x00000020)
#define N4_DCSIO_DCSL8IN	N4CONST(0x00000040)
#define N4_DCSIO_DCSL2OUT	N4CONST(0x00000100)
#define N4_DCSIO_DCSL3OUT	N4CONST(0x00000200)
#define N4_DCSIO_DCSL4OUT	N4CONST(0x00000400)
#define N4_DCSIO_DCSL5OUT	N4CONST(0x00000800)
#define N4_DCSIO_DCSL6OUT	N4CONST(0x00001000)
#define N4_DCSIO_DCSL7OUT	N4CONST(0x00002000)
#define N4_DCSIO_DCSL8OUT	N4CONST(0x00004000)


/*
 * DMA Registers
 */

#define N4_DMACTRL_BLKSIZE_MASK	N4CONST(0x00000000000fffff)
#define N4_DMACTRL_HHSDEST	N4CONST(0x0000000000400000)
#define N4_DMACTRL_HHSSEN	N4CONST(0x0000000000800000)
#define N4_DMACTRL_DRST		N4CONST(0x0000000001000000)
#define N4_DMACTRL_SRCINC	N4CONST(0x0000000002000000)
#define N4_DMACTRL_DESINC	N4CONST(0x0000000004000000)
#define N4_DMACTRL_SU		N4CONST(0x0000000008000000)
#define N4_DMACTRL_GO		N4CONST(0x0000000010000000)
#define N4_DMACTRL_IVLD		N4CONST(0x0000000020000000)
#define N4_DMACTRL_IE		N4CONST(0x0000000040000000)
#define N4_DMACTRL_BZ		N4CONST(0x0000000080000000)
#define N4_DMACTRL_MRDERR 	N4CONST(0x0000000100000000)
#define N4_DMACTRL_PRDERR 	N4CONST(0x0000000200000000)
#define N4_DMACTRL_UDRDERR 	N4CONST(0x0000000400000000)
#define N4_DMACTRL_HHSEOT 	N4CONST(0x0000000800000000)

#endif /* __VRC5074_H__ */

