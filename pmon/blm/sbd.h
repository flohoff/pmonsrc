/*
 * sbd.h: cpu board definitions for Siemens Atea BLM-RISC
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		180
#endif

#define DRAM_BASE		0x00000000
#define GT64011_BASE		0x14000000

/* CS0 - 1c000000:1c7fffff, 8-bit (even bus) */
#define CS0_BASE		0x1c000000
#define CS0_LIMIT		0x1c7fffff
#define DUART_BASE		0x1c000000

/* CS1 - 1c800000:1cffffff, 32-bit (even bus) */
#define CS1_BASE		0x1c800000
#define CS1_LIMIT		0x1cffffff
#define SEMAPHORE_BASE		0x1c800000
#define DPRAMCS_BASE		0x1c808000

/* CS2 - 1d000000:1dffffff, 32-bit (even bus) */
#define CS2_BASE		0x1d000000
#define CS2_LIMIT		0x1dffffff
#define FLASH_BASE		0x1d000000
#define FLASH_SIZE		0x00100000

/* CS3 - 1f000000:1fbfffff, 32-bit (even bus) */
#define CS3_BASE		0x1f000000
#define CS3_LIMIT		0x1fbfffff
#define DPRAM_BASE		0x1f000000
#define CAM_BASE		0x1f040000
#define UREQ_BASE		0x1f080000
#define LOADASS_BASE		0x1f0c0000
#define LOADANA_BASE		0x1f100000
#define CPLANE_BASE		0x1f140000

/* BOOTCS - 1f800000:1ffffff, 8-bit (even bus) */
#define BOOTCS_BASE		0x1f800000
#define BOOTCS_LIMIT		0x1fffffff
#define PROM_BASE		0x1fc00000

#define LOCAL_MEM		DRAM_BASE
#define LOCAL_MEM_SIZE		(32*1024*1024)		/* DRAM size (32Mb) */
#define BOOTPROM_BASE		PROM_BASE

/* 
 * Define the Z8530 duart parameters
 */

#define Z8530CLOCK		3686400		/* baud rate clock */
#define Z8530DELAY		2		/* 2us recovery time */

/* uart register layout offsets; data is on 31:24 */
#define Z8530_CHB	0
#define Z8530_CHA	8
#if #endian(big)
# define Z8530_CMD	0
# define Z8530_DATA	4
# define z8530reg0	unsigned cmd:8; unsigned :24
# define z8530reg1	unsigned data:8; unsigned :24
#else
# define Z8530_CMD	3
# define Z8530_DATA	7
# define z8530reg0	unsigned :24; unsigned cmd:8
# define z8530reg1	unsigned :24; unsigned data:8
#endif


/* FIXME These timings are completely fictional */
#define RAMCYCLE	60			/* 60ns dram cycle */
#define ROMCYCLE	750			/* ~750ns rom cycle */
#define CACHECYCLE	((1000+MHZ-1)/MHZ)	/* internal clock */
#define CYCLETIME	CACHECYCLE
#define CACHEMISS	(CYCLETIME * 6)

/*
 * rough scaling factors for 2 instruction DELAY loop to get 1ms and 1us delays
 */
#define ASMDELAY(ns,icycle)	\
	(((ns) + (icycle)) / ((icycle) * 2))

#define CACHENS(us)	ASMDELAY((ns), CACHECYCLE)
#define RAMNS(us)	ASMDELAY((ns), CACHEMISS+RAMCYCLE)
#define ROMNS(us)	ASMDELAY((ns), CACHEMISS+ROMCYCLE)
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

