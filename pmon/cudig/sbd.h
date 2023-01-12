/*
 * CUDIG/sbd.h: cpu board definitions for Telegate CUDIG
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		67
#endif

#define DRAM_BASE		0x00000000
#define GT64011_BASE		0x14000000
/* CS0 - 1c000000:1c7fffff, 16-bit (odd bus) */
#define CS0_BASE		0x1c000000
#define CS0_LIMIT		0x1c7fffff
#define HPI_BASE		0x1c000000
/* CS1 - 1c800000:1cffffff, 16-bit (odd bus) */
#define CS1_BASE		0x1c800000
#define CS1_LIMIT		0x1cffffff
#define RF_FFT_BASE		0x1c800000
#define RF_WOLA_BASE		0x1cc00000
/* CS2 - 1d000000:1dffffff, 16-bit (odd bus) */
#define CS2_BASE		0x1d000000
#define CS2_LIMIT		0x1dffffff
#define RF_RFCONT_BASE		0x1d400000
#define RF_STATUS_BASE		0x1d500000
#define RF_RESET_BASE		0x1d600000
#define RF_SPI_BASE		0x1d700000
#define RF_FIELD_BASE		0x1d800000
#define LED0_BASE		0x1d900000
#define DISPLAY0_BASE		0x1db00000
#define DISPLAY1_BASE		0x1dc00000
#define VME0_BASE		0x1dd00000
#define VME1_BASE		0x1de00000
#define USFPDR_BASE		0x1df00000
/* CS3 - 1e000000:1f7fffff, 32-bit (even bus) */
#define CS3_BASE		0x1e000000
#define CS3_LIMIT		0x1f7fffff
#define USCDPR_BASE		0x1e400000
#define CONTROL0_BASE		0x1e600000
#define CONTROL1_BASE		0x1e800000
#define CONTROL2_BASE		0x1ea00000
#define CONTROL3_BASE		0x1ec00000
#define DSBE_BASE		0x1ee00000
#define FLASH_BASE		0x1f000000
#define FLASH_SIZE		0x00800000
/* BOOTCS - 1f800000:1ffffff, 8-bit (even bus) */
#define BOOTCS_BASE		0x1f800000
#define BOOTCS_LIMIT		0x1fffffff
#define UART0_BASE		0x1f800000
#define UART1_BASE		0x1fa00000
#define PROM_BASE		0x1fc00000
#define FLEX_BASE		0x1fe00000

#define LOCAL_MEM		DRAM_BASE
#define LOCAL_MEM_SIZE		(32*1024*1024)		/* DRAM size (32Mb) */
#define BOOTPROM_BASE		PROM_BASE
#define BOOTPROM_SIZE		(512*1024)

/* NS16550 configuration */
#define NSREG(x)	((x)*1)			/* byte addressable */
#define nsreg(x)	unsigned char x
#define NS16550_HZ	7372800			/* fast input clock */
#define XR16850					/* could be Exar XR16850 */

/* 4 character display - first register */
#define DISPLAY0_CHAR	 	0x007f
#define DISPLAY0_NCLR	 	0x0080
#define DISPLAY0_IDLE		(DISPLAY0_NCLR)

/* 4 character display - second register */
#define DISPLAY1_A0	 	0x0100
#define DISPLAY1_A1	 	0x0200
#define DISPLAY1_NCE1	 	0x0400
#define DISPLAY1_NCE2	 	0x0800
#define DISPLAY1_NCU	 	0x1000
#define DISPLAY1_CUE	 	0x2000
#define DISPLAY1_NBL		0x4000
#define DISPLAY1_NWR		0x8000
#define DISPLAY1_IDLE		(DISPLAY1_NWR | DISPLAY1_NBL | DISPLAY1_NCU \
				 | DISPLAY1_NCE1 | DISPLAY1_NCE2)

/* control register 0 (32-bits, bits 7:0) */
/* xxx */

/* control register 1 (32-bits, bits 15:8) */
#define CONTROL1_M_WDI		0x00000100
#define CONTROL1_RFCARD_RST_N	0x00000200
#define CONTROL1_DSP_RST_N	0x00000400
#define CONTROL1_FLASH_RST_N	0x00000800
#define CONTROL1_DS_RST_N	0x00001000
#define CONTROL1_VME_RST_N	0x00002000
#define CONTROL1_SER2_RST	0x00004000
#define CONTROL1_SER1_RST	0x00008000

/* control register 2 (32-bits, bits 23:16) */
/* xxx */

/* control register 3 (32-bits, bits 31:24) */
/* xxx */

/* FIXME These timings are completely fictional */
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

