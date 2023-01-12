/* 
 * sbd.h: Control Techniques board definition header file
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		133
#endif

#define RAMCYCLE	60			/* 60ns dram cycle */
#define ROMCYCLE	750			/* ~750ns rom cycle */
#define CACHECYCLE	(1000/(MHZ*2)) 		/* external clk * 2 */
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

#define BOOTPROM_BASE	0x1fc00000
#define EPROM_BASE	0x1fd00000
#define FLASH_BASE	0x1fe00000
#define NIC_BASE	0x1ff00000 	/* Ethernet */
#define LED_BASE	0x1ff20010 	/* Siemens 4 character LED */
#define  LED(n)	((3-(n))*4)
#define UART0_BASE	0x1ff30000 	/* TL16552 com1 */
#define UART1_BASE	0x1ff40000 	/* TL16552 com2 */
#define ECP_BASE	0x1ff50000 	/* TL16552 lpt1 */
#define FNC6_BASE	0x1ff80000
#define BCR_BASE	0x1ffb0000	/* board configuration */
#define DCR_BASE	0x1ffc0000	/* dram configuration */
#define OPTION_BASE	0x1ffd0000	/* switch input */

/* soft copy of write-only registers and other machine state
   lives just above XTLB exception vector, just under cache
   exception */
#define SBD_SOFTC	PA_TO_KVA1 (0x000000f0)

#ifdef __ASSEMBLER__
    .struct 0
S_DCR:		.byte	0
S_BCR:		.byte	0
S_MEMSZ:	.byte	0
S_SIMM0SZ:	.byte	0
S_SIMM1SZ:	.byte	0
    .previous
#else
/* soft copy of write-only registers and other machine state */
struct sbd_softc {
    unsigned char	s_dcr; 		/* DCR soft copy */
    unsigned char	s_bcr; 		/* BCR soft copy */
    unsigned char	s_memsz; 	/* total memory size in MB */
    unsigned char	s_simm0sz;	/* SIMM0 size */
    unsigned char	s_simm1sz;	/* SIMM1 size */
};
#endif

/* Interrupt mappings */
#define SR_FNC6		SR_IBIT7	/* h/w int 4 = FNC-6 I/O */
#define SR_SPARE1	SR_IBIT6	/* h/w int 3 */
#define SR_DEBUG	SR_IBIT5	/* h/w int 2 = debug switch */
#define SR_ETHER	SR_IBIT4	/* h/w int 1 = ethernet */
#define SR_DUART	SR_IBIT3	/* h/w int 0 = DUART */

#define CAUSE_FNC6	CAUSE_IP7
#define CAUSE_BUSERR	CAUSE_IP6
#define CAUSE_DEBUG	CAUSE_IP5
#define CAUSE_ETHER	CAUSE_IP4
#define CAUSE_DUART	CAUSE_IP3

/* simple 8-bit local devices are connected to bits D7:0 */
#define ioaddrw(base,o)	((base)+(o)*4)
#define ioaddrb(base,o)	ioaddrw(base, o)
#define ioaddrh(base,o)	ioaddrw(base, o)

/* UART input clock 3.6864 MHz */
#define NS16550_HZ	3686400

/* DRAM control register (8 bits, 1 word per bit) */
#define DCR_SIMM1_DRAM	0x80	/* simm1 is DRAM */
#define DCR_TYPE	0x60	/* simm0/1 type */
#define  DCR_BURSTEDO	 0x60	  /* burst edo */
#define  DCR_EDO	 0x40	  /* normal edo */
#define  DCR_FASTPAGE1	 0x20	  /* fast page mode */
#define  DCR_FASTPAGE2	 0x00	  /* fast page mode */
#define DCR_DRAMFAST	0x10	/* select fast(60ns)/slow(70ns) DRAM timings */
#define DCR_SIMM1_SGL	0x08	/* simm1 is single-sided */
#define DCR_SIMM0_SGL	0x04	/* simm0 is single-sided */
#define DCR_SIMM0_SIZE	0x03	/* simm0 size (x2 if double-sided) */
#define  DCR_4MB	 0x1
#define  DCR_8MB	 0x2
#define  DCR_16MB	 0x3

/* Board control register (8 bits, 1 word per bit) */
#define BCR_FLASH_WE	0x80 	/* flash prom write enable */
#define BCR_IO_LE	0x40	/* little-endian i/o system */
#define BCR_FPGA_NCFG	0x20 	/* ?? */
#define BCR_GRFPGA_D0	0x10 	/* ?? */
#define BCR_IOFPGA_D0	0x08 	/* ?? */
#define BCR_LED_ON	0x04	/* led on (!blank) */
#define BCR_FPGA_DCLK	0x01	/* ?? */
#define BCR_FNC6_IO_ENB	0x01	/* FNC6 enable */

/* Ethernet */
#define FE_SYS_BUS_WIDTH	8
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
 *	SRAM: 32KB, 100ns, byte-wide access.
 *	Transmission buffer: 4KB x 2.
 *	System bus interface: 8 bits.
 */
#define FE_D6_INIT		(FE_D6_BUFSIZ_32KB | FE_D6_TXBSIZ_2x4KB | \
				 FE_D6_BBW_BYTE | FE_D6_SBW_BYTE | \
				 FE_D6_SRAM_100ns)
#ifdef MIPSEB
#define FE_D7_INIT		(FE_D7_BYTSWP_HL | FE_D7_IDENT_EC)
#else
#define FE_D7_INIT		(FE_D7_BYTSWP_LH | FE_D7_IDENT_EC)
#endif
