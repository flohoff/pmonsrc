/* 
 * TEMIC/sbd.h: Algor/Temic module header file
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		120
#endif

#define DRAM_BASE	0x00000000	/*  64MB: sdram */
#define FLASH_BASE	0x04000000	/*  16MB: flash/eprom */
#define  FLASH_SIZE	 0x01000000
#define PCI_IO_SPACE	0x06000000	/*  32MB: s/w configurable */
#define PCI_MEM_SPACE	0x08000000	/* 128MB: s/w configurable */
#define PCI_CONF_SPACE	0x10000000	/*  64MB: s/w configurable */
#define DPRAM_BASE	0x14000000	/*   2MB: dual-port ram (128KB) */
#define SEM_BASE	0x14200000	/*   2MB: dual-port semaphores (8B)  */
#define VRC5074_BASE	0x1ee00000	/*   2MB: Vrc5074 (not default) */
#define BOOTPROM_BASE	0x1f000000	/*  16MB: eprom/flash */

/* Local to PCI bus apertures sizes */
#define PCI_MEM_SPACE_SIZE	(128 * 1024 * 1024)
#define PCI_IO_SPACE_SIZE	( 32 * 1024 * 1024)
#define PCI_CONF_SPACE_SIZE	( 64 * 1024 * 1024)

/* Internal UART */
#define UART0_BASE	(VRC5074_BASE + N4_UART)
#define NS16550_HZ	(MHZ*1000000/12)
#if #endian(big)
#define NSREG(x)	(((x)*8)+7)
#define nsreg(x)	unsigned long long :56; unsigned char x
#else
#define NSREG(x)	((x)*8)
#define nsreg(x)	unsigned char x; unsigned long long :56
#endif
#define xr16850(x) 	0

/* DCSIO (gpio) channel numbers */
#define	DCSIO_SCL	2	
#define	DCSIO_SDA	3
#define	DCSIO_SCS	4
#define	DCSIO_ICS	5

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

