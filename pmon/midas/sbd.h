/* 
 * sbd.h: Midas Arcade board definition header file
 */

#include "midas/arizona.h"

/* fastest possible pipeline clock */
#define MHZ		180

#define RAMCYCLE	60			/* FIXME 60ns dram cycle */
#define ROMCYCLE	750			/* FIXME ~750ns rom cycle */
#define CACHECYCLE	(1000/MHZ) 		/* 1 cache cycle */
#define CYCLETIME	CACHECYCLE
#define CACHEMISS	(CYCLETIME * 6)

/*
 * rough scaling factors for 2 instruction DELAY loop to get 1ms and 1us delays
 */
#define ASMDELAY(ns,icycle)	\
	(((ns) + (icycle)) / ((icycle) * 2))

#define CACHEUS		ASMDELAY(1000, CACHECYCLE)
#define RAMUS		ASMDELAY(1000, CACHEMISS+RAMCYCLE)
#define ROMUS		ASMDELAY(1000, CACHEMISS+ROMCYCLE)
#define CACHEMS		ASMDELAY(1000000, CACHECYCLE)
#define RAMMS		ASMDELAY(1000000, CACHEMISS+RAMCYCLE)
#define ROMMS		ASMDELAY(1000000, CACHEMISS+ROMCYCLE)

#ifndef __ASSEMBLER__
#define nsdelay(ns)	mips_cycle (ASMDELAY (ns, CACHECYCLE))
#define usdelay(us)	mips_cycle (ASMDELAY ((us)*1000, CACHECYCLE))
#endif


#define LOCAL_MEM_SIZE	0x03000000	/* 48Mb maximum */

/* I/O addresses */
#define MC_BASE		0x04000000
#define TSP_BASE	0x10000000
#define TSPIO_BASE	0x11000000
#define EEPROM_BASE	0x11080000
#define DAC_BASE	0x11100000
#define NIC_BASE	0x11180000
#define UART_BASE	0x11200000
#define GAME_BASE	0x11280000
#define DEBUG_BASE	0x11300000
#define VIDPLL_BASE	0x11380000
#define SMC1_BASE	0x12000000
#define SMC2_BASE	0x12800000
#define SMC3_BASE	0x13000000
#define ISP_BASE	0x14000000
#define SISP1_BASE	0x14800000
#define SISP2_BASE	0x15000000
#define SISP3_BASE	0x15800000

/* MC configuration */
#define MC_SD_CONF_A_INIT \
	((2 << MC_SD_CONF_A_ACT0_ACT1_SHIFT) |	\
	 (3 << MC_SD_CONF_A_PRE_ACT_SHIFT) |	\
	 (7 << MC_SD_CONF_A_REF_ACT_SHIFT) |	\
	 (7 << MC_SD_CONF_A_REF_REF_SHIFT) |	\
	 (2 << MC_SD_CONF_A_PRE_REF_SHIFT) |	\
	 (0x3d1 << MC_SD_CONF_A_MAXREF_SHIFT))

#if MC_SD_CONF_A_INIT != 0x3d127732
#error MC_SD_CONF_A_INIT!
#endif

#define MC_SD_CONF_B_INIT \
	((5 << MC_SD_CONF_B_ACT_PRE_SHIFT) |	\
	 (3 << MC_SD_CONF_B_ACT_CMD_SHIFT) |	\
	 (2 << MC_SD_CONF_B_WR_PRE_SHIFT) |	\
	 (2 << MC_SD_CONF_B_RD_PRE_SHIFT) |	\
	 (3 << MC_SD_CONF_B_BRST_WR_SHIFT) |	\
	 (2 << MC_SD_CONF_B_PRE_MRS_SHIFT) |	\
	 (2 << MC_SD_CONF_B_MRS_ACT_SHIFT) |	\
	 (8 << MC_SD_CONF_B_ACCESSHOLD_SHIFT))

#if MC_SD_CONF_B_INIT != 0x82232235
#error MC_SD_CONF_B_INIT!
#endif

#define MC_GEN_CTL_INIT0 \
	((0 << MC_GEN_CTL_MABORT_CNT_SHIFT) |	\
	 MC_GEN_CTL_RABYPASS_SCIO |		\
	 MC_GEN_CTL_RABYPASS_SCBRST |		\
	 MC_GEN_CTL_RABYPASS_REGBLK |		\
	 MC_GEN_CTL_SDRAM |			\
	 MC_GEN_CTL_SDRAM_MREFRESH)

#if MC_GEN_CTL_INIT0 != 0x00ec0000
#error MC_GEN_CTL_INIT0!
#endif

#if !defined(MIDASPABUG) || defined(MIDASRABUG)
#define MC_GEN_CTL_INIT1 \
	((0 << MC_GEN_CTL_MABORT_CNT_SHIFT) |	\
	 MC_GEN_CTL_RABYPASS_SCIO |		\
	 MC_GEN_CTL_RABYPASS_SCBRST |		\
	 MC_GEN_CTL_RABYPASS_REGBLK |		\
	 MC_GEN_CTL_RABYPASS_SDRAM |		\
	 MC_GEN_CTL_SDRAM |			\
	 MC_GEN_CTL_SDRAM_MREFRESH)

#if MC_GEN_CTL_INIT1 != 0x00fc0000
#error MC_GEN_CTL_INIT1!
#endif
#endif

#define	MC_SC_BURST_MAP_INIT		0x24DFF924
#define	MC_SC_IO_MAP_INIT		0x76FFF024
#define	MC_SC_IO_B_MAP_INIT		0xB6DBDFF1

/* NS16552 UART */
#define NS16550_HZ	24000000
#define UART0_BASE	(UART_BASE+0x20)
#define UART1_BASE	(UART_BASE+0x00)

/* debug */
#define DBGLED_BASE	(DEBUG_BASE+2)
#define DBGOUT_BASE	(DEBUG_BASE+3)
#define DBGSWT_BASE	(DEBUG_BASE+2)
#define DBGCNF_BASE	(DEBUG_BASE+3)

/* EEPROM */
#define EEPROM_CS	0x04000000
#define EEPROM_SK	0x02000000
#define EEPROM_DIN	0x01000000
#define EEPROM_DOUT	0x01000000

/* Additional checkpoint codes */
#define CHKPNT_EIIR	0x80	/* failed to read ns16550 IIR */
#define CHKPNT_EFFO	0x81	/* failed to detect ns16550 fifo */
#define CHKPNT_NFFO	0x82	/* no ns16550 fifo */

#ifndef __ASSEMBLER__
unsigned char	sbd_ioread8(volatile unsigned char *addr);
unsigned short	sbd_ioread16(volatile unsigned short *addr);
unsigned int	sbd_ioread32(volatile unsigned int *addr);
void sbd_iowrite8(volatile unsigned char *addr, unsigned char v);
void sbd_iowrite16(volatile unsigned short *addr, unsigned short v);
void sbd_iowrite32(volatile unsigned int *addr, unsigned int v);
#endif

/* configuration information for ethernet controller */
#define FE_DEBUG		3

/* disable loopback */
#define FE_D4_INIT		(FE_D4_CNTRL | FE_D4_LBC_DISABLE)

/* accept bad packets, promiscuous mode */
#define FE_D5_INIT		(FE_D5_BADPKT | FE_D5_AFM0 | FE_D5_AFM1)

/* force AUI interface, no link test */
#define FE_B13_INIT		(FE_B13_LNKTST | FE_B13_PORT_AUI)

/* since "accept bad packet" is set we mustn't enable error interrupts */
#define FE_RMASK 		(FE_D3_OVRFLO | FE_D3_BUSERR | FE_D3_PKTRDY)

/*
 * Program the 86964 as follows:
 *	SRAM: 8KB, 100ns, byte-wide access.
 *	Transmission buffer: 2KB x 2.
 *	System bus interface: 8 bits.
 */
#define FE_D6_INIT	(FE_D6_BUFSIZ_8KB | FE_D6_TXBSIZ_2x2KB | \
			 FE_D6_BBW_BYTE | FE_D6_SBW_BYTE | FE_D6_SRAM_100ns)
#ifdef MIPSEB
#define FE_D7_INIT		(FE_D7_BYTSWP_HL | FE_D7_IDENT_EC)
#else
#define FE_D7_INIT		(FE_D7_BYTSWP_LH | FE_D7_IDENT_EC)
#endif
