/* 
 * sbd.h: Glacier board definition header file
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		133
#endif

#ifndef PCI_HZ
#define PCI_HZ		33000000
#endif

#define RAMCYCLE	60			/* FIXME 60ns dram cycle */
#define ROMCYCLE	750			/* FIXME ~750ns rom cycle */
#define CACHECYCLE	(1000/MHZ)
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
#define PCI_MEM_SPACE	0x10000000	/* 256MB */
#define PCI_MEM_SPACE_SIZE	(256 * 1024 * 1024)
#define PCI_IO_SPACE	0x20000000	/* FIXME! */
#define PCI_IO_SPACE_SIZE	(0 * 1024 * 1024)


#define LOCAL_MEM_SIZE	(128 * 1024 * 1024) /* maximum bank memory size */

#define BOOTPROM_BASE	0x1fc00000

#define LIFESAVER_BASE	0x0ff00000
