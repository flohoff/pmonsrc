/* 
 * p5064/sbd.h: Algorithmics P5064 board definition header file
 * Copyright (c) 1997 Algorithmics Ltd.
 */

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		25
#endif

#define RAMCYCLE	60			/* FIXME dram cycle */
#define ROMCYCLE	840			/* rom cycle */
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


/* 
 * ROM/RAM and fixed memory address spaces etc.
 */
#define LOCAL_MEM	0x00000000 	/* Local on-board/private memory */
#define LOCAL_MEM_SIZE	0x04000000 	/* Local memory size (4Mb max) */


#define AVIAGTX_BASE	0x00000000
#define AVIA500_BASE	0x01000000
#define BCSR_BASE	0x02000000
#define MBREG_BASE	0x1b000000
#define BOOTPROM_BASE	0x1fc00000
#define UART_BASE	0x1c000000
#define DBGROM_BASE	0x1d000000
#define FLASH_BASE	0x1e000000

/* Define UART baud rate and register layout */
#define NS16550_HZ	18432000
#ifdef __ASSEMBLER__
#define NSREG(x)	((x)*2 + 1)
#else
#define nsreg(x)	unsigned char pad##x; unsigned char x
#endif
#define UART0_BASE	(UART_BASE + 0x00)
#define UART1_BASE	(UART_BASE + 0x10)
#define UART2_BASE	(UART_BASE + 0x20)
#if 0 /* this channel is not connected */
#define UART3_BASE	(UART_BASE + 0x30)
#endif

/* Board control/status registers */
#define BCSR_UART_IRQ	0x04
#define BCSR_UART_TXRDY	0x02
#define BCSR_UART_RXRDY	0x01
#define BCSR_PANIC_BUS	0x02
#define BCSR_PANIC_DBG	0x01
#define BCSR_BOOT_FRDY	0x02
#define BCSR_BOOT_ROM	0x01

#ifdef __ASSEMBLER__
#define BCSR_UART	(BCSR_BASE+0)
#define BCSR_PANIC	(BCSR_BASE+2)
#define BCSR_BOOT	(BCSR_BASE+4)
#else
typedef struct ccubebcsr {
    unsigned short	uart;
    unsigned short	panic;
    unsigned short	boot;
} ccubebcsr;
#endif


/* board attributes */

