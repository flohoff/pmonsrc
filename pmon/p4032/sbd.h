/* 
 * sbd.h: Algorithmics P4032 board definition header file
 */

#ifndef MHZ
/* fastest possible pipeline clock (75MHz * 3, 62MHz * 4) */
#define MHZ		250
#endif

#define RAMCYCLE	60			/* 60ns dram cycle */
#define ROMCYCLE	750			/* ~750ns rom cycle */
#define CACHECYCLE	(1000/MHZ) 		/* internal clock */
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

/* Our local to PCI bus apertures are set up with the following sizes */
#define PCI_MEM_SPACE_SIZE	(128 * 1024 * 1024)
#define PCI_IO_SPACE_SIZE_NEW	( 16 * 1024 * 1024)
#define PCI_IO_SPACE_SIZE_OLD	(  1 * 1024 * 1024)
#define PCI_CONF_SPACE_SIZE	(  1 * 1024 * 1024)

#define PCI_MEM_SPACE	0x10000000	/* 128MB: s/w configurable */
#define PCI_IO_SPACE_NEW 0x1d000000	/*  16MB: s/w configurable */
#define PCI_IO_SPACE_OLD 0x1ed00000	/*   1MB: s/w configurable */
#define PCI_CONF_SPACE	0x1ee00000	/*   1MB: s/w configurable */
#define V96XPBC_BASE	0x1ef00000	/*  64KB: s/w configurable */
#define BOOTPROM_BASE	0x1fc00000
#define EPROM_BASE	0x1fd00000
#define FLASH_BASE	0x1fe00000
#define RTC_BASE	0x1ff00000
#define  RTC_ADDR	 0
#define  RTC_DATA	 4
#define KEYBD_BASE	0x1ff10000
#define LED_BASE	0x1ff20010
#define  LED(n)	((3-(n))*4)
#define LCD_BASE	0x1ff30000
#define ZPIO_BASE	0x1ff40000
#define ZPIO_IACK	0x1ff50000
#define ZPIO_RETI	0x1ff50000
#define COMBI_BASE	0x1ff80000
#define ICU_BASE	0x1ff90000
#define FDC_DACK	0x1ffa0000
#define BCR_BASE	0x1ffb0000
#define DCR_BASE	0x1ffc0000
#define OPTION_BASE	0x1ffd0000

#ifndef __ASSEMBLER__
typedef union {
    struct {
	unsigned int dev;
	unsigned int err;
	unsigned int pci;
    } irr;
    struct {
	unsigned int mask;
	unsigned int clear;
	unsigned int pcimask;
	unsigned int devxbar0;
	unsigned int devxbar1;
	unsigned int pcixbar;
    } ctrl;
} p4032icu;
#else
#define ICU_IRR_DEV		0
#define ICU_IRR_ERR		4
#define ICU_IRR_PCI		8
#define ICU_CTRL_MASK		0
#define ICU_CTRL_CLEAR		4
#define ICU_CTRL_PCIMASK	8
#define ICU_CTRL_DEVXBAR0	12
#define ICU_CTRL_DEVXBAR1	16
#define ICU_CTRL_PCIXBAR	20
#endif

/* device interrupt request and mask bits */
#define INTR_DEV_RTC	0x80
#define INTR_DEV_GPIO	0x40
#define INTR_DEV_CENT	0x20	/* also the "clear" bit */
#define INTR_DEV_UART1	0x10
#define INTR_DEV_UART0	0x08
#define INTR_DEV_KBD	0x04
#define INTR_DEV_FDC	0x02
#define INTR_DEV_V96	0x01

/* error interrupt request and clear bits */
#define INTR_ERR_BUSERR	0x04
#define INTR_ERR_ACFAIL	0x02
#define INTR_ERR_DEBUG	0x01

/* PCI request and mask bits */
#define INTR_PCI_INT3	0x80
#define INTR_PCI_INT2	0x40
#define INTR_PCI_INT1	0x20
#define INTR_PCI_INT0	0x10
#define INTR_PCI_FDCDMA	0x08	/* write only to enable DMA IRQ on HWINT3 */

/* devxbar0 shifts */
#define INTR_DEVX0_V96X	0
#define INTR_DEVX0_FDC	2
#define INTR_DEVX0_KBD	4
#define INTR_DEVX0_UART0 6

/* devxbar1 shifts */
#define INTR_DEVX1_UART1 0
#define INTR_DEVX1_CENT	2
#define INTR_DEVX1_GPIO	4
#define INTR_DEVX1_RTC	6

/* pcixbar shifts */
#define INTR_PCIX_0	0
#define INTR_PCIX_1	2
#define INTR_PCIX_2	4
#define INTR_PCIX_3	6

/* interrupt numbers */
/* FIXME - this is a software covention and should go somewhere else */
#define INTR_BUSERR	0
#define INTR_ACFAIL	1
#define INTR_DEBUG	2
#define INTR_FDCDMA	3
#define INTR_RTC	4
#define INTR_GPIO	5
#define INTR_CENT	6
#define INTR_UART1	7
#define INTR_UART0	8
#define INTR_KBD	9
#define INTR_FDC	10
#define INTR_PCI	11
#define INTR_PCI0	12
#define INTR_PCI1	13
#define INTR_PCI2	14
#define INTR_PCI3	15
#define INTR_MAX	16

/* interrupt selects */
#define INTR_HWINT0	0
#define INTR_HWINT1	1
#define INTR_HWINT2	2
#define INTR_HWINT3	3
#define INTR_HWNONE	3	/* if INTR_PCI_FDCDMA is set */
#define INTR_HWMASK	3

/* simple 8-bit local devices are connected to bits D7:0 */

#define ioaddrw(base,o)	((base)+(o)*4)
#define ioaddrb(base,o)	ioaddrw(base, o)
#define ioaddrh(base,o)	ioaddrw(base, o)

/* Winbond chip subsystems */
#define NS16550_HZ	(24000000/13)
#define UART0_BASE	ioaddrb(COMBI_BASE, 0x3f8)
#define UART1_BASE	ioaddrb(COMBI_BASE, 0x2f8)
#define ECP_BASE	ioaddrw(COMBI_BASE, 0x378)
#define FDC_BASE	ioaddrw(COMBI_BASE, 0x3f0)
#define IDE_BASE	ioaddrw(COMBI_BASE, 0x1f0)

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
#define BCR_ETH_ENABLE	0x20 	/* ethernet chip enable (!reset) */
#define BCR_FDC_TC	0x10 	/* floppy dma terminal count */
#define BCR_SCSI_ENABLE	0x08	/* scsi chip enable (!reset) */
#define BCR_LED_ON	0x04	/* led on (!blank) */
#define BCR_AUTO_BUSY	0x02	/* enable auto busy generation */
#define BCR_V96X_ENABLE	0x01	/* enable V96x pci chip (!reset) */

