/*
 * NEC41XX/sbd.h: board-specific definitions for NEC Vr41xx eval board
 * Copyright (c) 1998 Algorithmics Ltd
 */

#ifndef _NEC41XX_SBD_H_
#define _NEC41XX_SBD_H_

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		169
#endif

#ifdef IN_PMON
#include "vr41xxio.h"
#else
#include <nec/vr41xxio.h>
#endif

/* board device addresses */
#define DRAM_BASE	0x00000000
#define MEM_SIZE	(64*1024*1024)	/* max 64MB */
#define PCMCIA_MEM_BASE	(SYSBUS_MEM_BASE + 0x0000000)
#define ISA_MEM_BASE	(SYSBUS_MEM_BASE + 0x2000000)
#define PCMCIA_IO_BASE	(SYSBUS_IO_BASE + 0x1000000)
#define ISA_IO_BASE	(SYSBUS_IO_BASE + 0x2000000)
#define FLASH_BASE	0x1f000000
#define ROM_BASE	0x1fc00000

/* The board has three ns16550 uarts - two off-chip and one on */
#define ISA_INT		(ISA_IO_BASE + 0xfee0)
#define ISA_INTMSK	(ISA_IO_BASE + 0xfef0)
#define LEDWR_BASE	(ISA_IO_BASE + 0xffa0)
#define FLSHCNT_BASE	(ISA_IO_BASE + 0xffb0)
#define UART0_BASE	(ISA_IO_BASE + 0xffc0)	/* off-chip uart #0 */
#define DEBDSP_BASE	(ISA_IO_BASE + 0xffd0)
/*#define UART1_BASE	(ISA_IO_BASE + 0xffe0)	/* off-chip uart #1 */
#define UART1_BASE	(ISA_IO_BASE + 0x03f0)	/* off-chip uart #1 */
#define PPT_BASE	(ISA_IO_BASE + 0xfff0)
#define UART2_BASE	(SIU_BASE)		/* on-chip uart #2 */

/* How to access ns16550 registers (offsets) */
#define NSREG(x)	((x)*2)
#define nsreg(x)	unsigned char x; unsigned int :8

/* ISA_INT and ISA_MSK bits */
#define ISAINTSLOT2 0x0002
#define ISAINTSLOT3 0x0004

/* Flash memory control */
#define FLSHCNT_FRDY	0x80	/* flash RDY pin */
#define FLSHCNT_VPPEN	0x40	/* flash VPP enable */
#define FLSHCNT_DPSW1	0x20	/* dip switch 1 (0=flash boot, 1=rom boot) */
#define FLSHCNT_DPSW2	0x10	/* dip switch 2 (1=normal, 0=exchanged) */

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

#define RAMCYCLE	60			/* 60ns single sram cycle */
#define ROMCYCLE	165			/* 165ns single rom cycle */
#define CACHECYCLE	(1000/MHZ) 		/* pipeline clock */
#define CYCLETIME	CACHECYCLE
#define CACHEMISS	(CYCLETIME * 6)

#ifndef __ASSEMBLER__
extern void	sbdusdelay (unsigned int);
extern long	_sbd_tclkdiv (void);

extern void	isa_outb (unsigned int base, unsigned char v);
extern void	isa_outsb (unsigned int base, void *addr, int len);
extern unsigned	isa_inb (unsigned int base);
extern void	isa_insb (unsigned int base, void *addr, int len);
extern unsigned	isa_inw (unsigned int base);
extern void	isa_insw (unsigned int base, void *addr, int len);
extern void	isa_outw (unsigned int base, unsigned short v);
extern void	isa_outsw (unsigned int base, void *addr, int len);
#endif

#endif /* _NEC41XX_SBD_H_ */
