/* 
 * NEC5074L/sbd.h: NEC DDB-Vrc5074 header file
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		200
#endif

#define DRAM_BASE	0x00000000
#define FLASH_BASE	0x04000000
#define FLASH_SIZE	0x00400000
#define PCI_IO_SPACE	0x06000000	/*  32MB: s/w configurable */
#define PCI_MEM_SPACE	0x08000000	/* 128MB: s/w configurable */
#define PCI_CONF_SPACE	0x10000000	/*  64MB: s/w configurable */
#define VRC5074_BASE	0x1fa00000
#define BOOTPROM_BASE	0x1fc00000

/* Local to PCI bus apertures sizes */
#define PCI_MEM_SPACE_SIZE	(128 * 1024 * 1024)
#define PCI_IO_SPACE_SIZE	( 32 * 1024 * 1024)
#define PCI_CONF_SPACE_SIZE	( 64 * 1024 * 1024)

/* ISA addresses */
#define ISAPORT_BASE(x)	(PCI_IO_SPACE + (x))
#define ISAMEM_BASE(x)	(PCI_MEM_SPACE + (x))

/* Super i/o chip - configuration */
#define M1543_PCFG_BASE	(PCI_CONF_SPACE + 0x40000)

/* Real-time clock/nvram (ISA mem bus) */
#define RTC_BASE	ISAMEM_BASE(0x000000)

/* Hex LED (ISA i/o bus) */
#define LED_BASE	ISAPORT_BASE(0x080)

/* External UART (ISA i/o bus) */
#define UART0_BASE	ISAPORT_BASE(0x3f8)
#define UART1_BASE	ISAPORT_BASE(0x2f8)
#define NS16550_HZ	(24000000/13)
#define NSREG(x)	(x)
#define nsreg(x)	unsigned char x

/* External parallel port (ISA i/o bus) */
#define ECP_BASE	ISAPORT_BASE(0x378)

/* Internal Vrc5074 UART */
#define UARTI_BASE	(VRC5074_BASE + N4_UART)

/* Guessed cycle timing */
#define RAMCYCLE	60			/* ~60ns dram cycle */
#define ROMCYCLE	1500			/* ~1500ns rom cycle */
#define CACHECYCLE	(1000/MHZ) 		/* pipeline clock */
#define CYCLETIME	CACHECYCLE
#define CACHEMISS	(CYCLETIME * 6)

/*
 * rough scaling factors for 2 instruction DELAY loop to get 1ms and 1us delays
 */
#define ASMDELAY(ns,icycle)	\
	(((ns) + (icycle)) / ((icycle) * 2))

#define CACHENS(ns)	ASMDELAY((ns), CACHECYCLE)
#define RAMNS(ns)	ASMDELAY((ns), CACHEMISS+RAMCYCLE)
#define ROMNS(ns)	ASMDELAY((ns), CACHEMISS+ROMCYCLE)
#define CACHEUS(us)	ASMDELAY((us)*1000, CACHECYCLE)
#define RAMUS(us)	ASMDELAY((us)*1000, CACHEMISS+RAMCYCLE)
#define ROMUS(us)	ASMDELAY((us)*1000, CACHEMISS+ROMCYCLE)
#define CACHEMS(ms)	((ms) * ASMDELAY(1000000, CACHECYCLE))
#define RAMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+RAMCYCLE))
#define ROMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+ROMCYCLE))

#ifndef __ASSEMBLER__
#define nsdelay(ns)	mips_cycle (ASMDELAY (ns, CACHECYCLE))
#define usdelay(us)	mips_cycle (ASMDELAY ((us)*1000, CACHECYCLE))
#endif

#if !defined(__ASSEMBLER__) && !defined(inb)
/* i/o port access ala 80x86 for ISA bus peripherals */
unsigned char	inb (unsigned int);
unsigned short	inw (unsigned int);
unsigned long	inl (unsigned int);
void		outb (unsigned int, unsigned char);
void		outw (unsigned int, unsigned short);
void		outl (unsigned int, unsigned long);
void *		ioport_map (unsigned int);
#endif
