/*
 * NEC5074L/sbdreset.S: low level reset code for NEC DDB-Vrc5074 
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#if !__mips64
# error Must be compiled with 64-bit support
#endif
	
/* Enable Parity or ECC (or neither) for memory */
#define NEC5074_PARITY
/*#define NEC5074_ECC*/
		
#ifdef IN_PMON	
#include <mips.h>
#include <pmon.h>
#include <sbd.h>
#define _sbd_reset	sbdreset
#else	
#if !#cpu(r5000)
# error Wrong CPU type selected
#endif
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>
#include <mips/endian.h>
#endif
	
#include "sbd.h"
#include "vrc5074.h"
#include "../pci/pcireg.h"
	
#define WBFLUSH 


#define MEG	0x100000		
	
#define LED(x) \
	li	t8,(x)*0x11111111; \
	sw	t8,PA_TO_KVA1(LED_BASE)
	
/*
 * Basic board initialisation, called straight from RESET.
 * It is only called if this program is built for ROM.  
 * It can use any registers except s8, k0 and k1.
 *
 * Note that s8 holds a "relocation factor" (see ../share/romlow.sx)
 * which must be added to any address before it is used.  This
 * is to support relocatable roms.
 */
	
#define tmpsize		s0
#define msize		s5
#define	rasave		s6
#define	RELOC		s8

LEAF(_sbd_reset)
	move	rasave,ra
	
#ifndef ITBASE	
	/* ensure KSEG0 is cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~CFG_K0MASK
	or	t1,CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
#endif
		
	mfc0	t1,C0_STATUS		# get Status reg
	mtc0	zero,C0_CAUSE		# clear s/w interrupts
	and	t1,SR_SR		# leave the SoftReset bit
	or	t1,SR_BEV		# force Boot Exception Vec
	mtc0	t1,C0_STATUS
	
	
	/* Initialise other low-level I/O devices */
	la	a0,reginittab
	addu	a0,RELOC
	bal	reginit

	/* 
	 * Now determine DRAM configuration
	 * XXX Fixed on this board.
	 */
	li	msize,0x4000000
	
#if defined(NEC5074_ECC) || defined(NEC5074_PARITY)
	/* Clear memory to initialise ECC/parity */
	
	/* Clear bottom 64K running uncached */
	li	a0,PA_TO_KVA1(0)
	addu	a1,a0,65536
	.set noreorder
1:	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	addu	a0,32
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder
	
	/* we can now initialise the caches for a fast clear_mem */
#ifdef IN_PMON	
	jal	mips_init_cache	
#else	
	bal	mips_init_cache	
#endif
		
	/* Finally clear rest of memory running cached */
	li	a0,PA_TO_KVA1(65536)
	subu	a1,msize,65536
	blez	a1,.noclear
	addu	a1,a0
	
	/* clear all of memory (to set correct parity bits) */
	
	/* run memory clear loop cached */
	la	t0,1f
	addu	t0,RELOC
	and	t0,~0x20000000
	j	t0
	
	.set noreorder
1:	#cache	Create_Dirty_Exc_D,0(a0)
	sd	zero,0(a0)
	sd	zero,8(a0)
	sd	zero,16(a0)
	addu	a0,32
	bne	a0,a1,1b
	sd	zero,-8(a0)
	.set reorder

.noclear:		
#endif
		
	/* return to generic code, with available memory size */
	move	ra,rasave
	move	v0,msize
	LED(0x2)
	
	j	ra
END(_sbd_reset)


LEAF(_sbd_memfail)
	LED(0xe)
1:	b	1b
	j	ra
END(_sbd_memfail)


	
#include "reginit.s"
	
#include "m1543c.h"
#include "isapnpreg.h"
		
#define N4_INIT(o,v)	WR_INIT(D, VRC5074_BASE+(o), v)
#define LED_INIT(x)	WR_INIT(W, LED_BASE, (x)*0x11111111)
#define PCI_INIT(s,o,v)	WR_INIT(s, VRC5074_BASE+N4_PCICFG+(o), v)
	
#define M1_CFGWR(o,v)	WR_INIT(B, M1543_PCFG_BASE+(o), v)
#define M1_CFGRD(o)	RD_INIT(B, M1543_PCFG_BASE+(o))
#define M1_PNPWR(o,v)	WR_INIT(B, ISAPORT_BASE(ISAPNP_MBADDR+(o)), v)
#define M1_PNPRD(o)	RD_INIT(B, ISAPORT_BASE(ISAPNP_MBADDR+(o)))
#define M1_PNPENTER()	M1_PNPWR(0, 0x51); M1_PNPWR(0, 0x23)
#define M1_PNPEXIT()	M1_PNPWR(0, 0xbb)
#define M1_PNPSET(i,v)	M1_PNPWR(0,i); M1_PNPWR(1,v)
		
	.rdata
	.align	3
reginittab:			
	/* DRAM window */
	N4_INIT(N4_SDRAM0,	DRAM_BASE | N4_PDAR_64BIT | N4_PDAR_64MB \
				| N4_PDAR_MEM | N4_PDAR_VISPCI)
	
	/* XXX The following two would be swapped if we were booting
	   from FLASH.  Presumably we should probe */
	
	/* Boot (eprom) window */
	N4_INIT(N4_BOOTCS,	BOOTPROM_BASE | N4_PDAR_8BIT | N4_PDAR_2MB \
				| N4_PDAR_MEM)
	
	/* Set DCS2 to be (flash rom) chip select */
	N4_INIT(N4_DCSFN,	N4_DCSFN_DCSFN2_BUS)
	
	/* DCS-2 (flash rom) window */
	N4_INIT(N4_DCS2,	FLASH_BASE | N4_PDAR_32BIT | N4_PDAR_4MB \
				| N4_PDAR_MEM)
	
	/* PCI #0 window (PCI i/o space) */
	N4_INIT(N4_PCIW0,	PCI_IO_SPACE | N4_PDAR_64BIT | N4_PDAR_32MB)
	N4_INIT(N4_PCIINIT0,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_IO)
	
	/* PCI #1 window (PCI conf space) */
	N4_INIT(N4_PCIW1,	PCI_CONF_SPACE | N4_PDAR_64BIT | N4_PDAR_64MB)
	N4_INIT(N4_PCIINIT1,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_CONF)
	
	/* Disable CPU interface parity checking and generation */
	N4_INIT(N4_CPUSTAT,	N4_CPUSTAT_TMODE_SGOOD | N4_CPUSTAT_DISCPUPC \
				| N4_CPUSTAT_DISPC)
	
#if defined(NEC5074_ECC)
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKENB | N4_MEMCTRL_CHKMODE_ECC
#elif defined(NEC5074_PARITY)
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKENB | N4_MEMCTRL_CHKMODE_PAR
#else	
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKDIS
#endif
	
	/* Set DRAM configuration */
	N4_INIT(N4_MEMCTRL, 	N4_MEMCTRL_HOLDLD \
				| N4_MEMCTRL_CHK \
				| N4_MEMCTRL_DRAMTYP_64MB_4BANK \
				| N4_MEMCTRL_ENABLE)
	
	/* EPROM/Flash cycle timing (120ns @ 80MHz) */
	N4_INIT(N4_ACSTIME,	N4_ACSTIME_ACCT(10))
	
	/* Give PCI priority on local bus */
	N4_INIT(N4_LCNFG,	N4_LCNFG_PCIHOG(16) | N4_LCNFG_CPUHOG(1))

#if 0	
	/* XXX this seems to cause spontaneous resets! */
	/* Error interrupts to INT[4] */
	N4_INIT(N4_INTCTRL,	N4_INTCTRL_EN(N4_INT_MCE) \
				| N4_INTCTRL_PRI(N4_INT_MCE,4) \
				| N4_INTCTRL_EN(N4_INT_CNTD) \
				| N4_INTCTRL_PRI(N4_INT_CNTD,4)	\
				| N4_INTCTRL_EN(N4_INT_LBRT) \
				| N4_INTCTRL_PRI(N4_INT_LBRT,4)	\
				| N4_INTCTRL_EN(N4_INT_PCIS) \
				| N4_INTCTRL_PRI(N4_INT_PCIS,4)	\
				| N4_INTCTRL_EN(N4_INT_PCIE) \
				| N4_INTCTRL_PRI(N4_INT_PCIE,4))
	
	/* Enable Int4 output driver */
	N4_INIT(N4_INTSTAT1,	N4_INTSTAT1_OE(4))
#endif
		
	/* PCI local config space initialisation */
	PCI_INIT(W, N4P_CMD,	PCI_COMMAND_MASTER_ENABLE \
				| PCI_COMMAND_MEM_ENABLE)
	
	/* 8 x 32-bit cache line size (prefetch size) */
	PCI_INIT(B, N4P_CLSIZ,	8)
	
	/* PCI->local memory address map */
	PCI_INIT(D, N4P_BAR0,	0x0000000008000000 \
				| PCI_MAP_MEMORY \
				| PCI_MAP_MEMORY_TYPE_64BIT)
	
	/* no other spaces visible to PCI */
	
	/* Put all devices in arbitration group 0 */
	N4_INIT(N4_PCIARB,	N4_PCIARB_GROUP0_MASK \
				| N4_PCIARB_CONS0(16) \
				| N4_PCIARB_CONS0N(1) \
				| N4_PCIARB_CONS1(1) \
				| N4_PCIARB_CONS2(1))
	
	/* Set PCLK=SYSCLK/3, no PLL, long timeouts, warm reset */
	N4_INIT(N4_PCICTRL,	N4_PCICTRL_PLL_STBY \
				| N4_PCICTRL_CLKSEL_1_3 \
				| N4_PCICTRL_CPUHOG(4) \
				| N4_PCICTRL_DMAHOG(16) \
				| N4_PCICTRL_RTYLIM(0) \
				| N4_PCICTRL_DISCTIM(0x0100))
	
	/* M1543 pci configuration */
	/* PIC - delayed transactions & passive release */
	M1_CFGWR(0x40, 0x1c)	
	M1_CFGWR(0x5e, 0x01)	
	
	/* PCSAD - LED chip select: IOWQ enable, i/o address 0x80 */
	M1_CFGWR(0x55, 0x80)
	M1_CFGWR(0x56, 0x00)
	M1_CFGWR(0x57, 0x40)
	M1_CFGRD(0x57)		# flush fifo
	
	/* first write to LED */
	LED_INIT(0x0)
	
	/* 
	 * M1543C PnP device configuration 
	 */
	
#if 0	
	/* reset config regs */
	M1_PNPENTER()
	M1_PNPSET(ISAPNP_CONFIG_CONTROL, ISAPNP_CC_RESET)
	M1_PNPEXIT()
#endif	
	
	/* activate UART3 (logical device 5) */
	M1_PNPENTER()
	M1_PNPSET(ISAPNP_LOGICAL_DEV_NUM, 0x05)
	M1_PNPSET(ISAPNP_ACTIVATE, 0x01)
	M1_PNPEXIT()
	
	/* activate UART1 (logical device 4) */
	M1_PNPENTER()
	M1_PNPSET(ISAPNP_LOGICAL_DEV_NUM, 0x04)
	M1_PNPSET(ISAPNP_ACTIVATE, 0x01)
	M1_PNPEXIT()
	
	/* activate LPT (logical device 3) */
	M1_PNPENTER()
	M1_PNPSET(ISAPNP_LOGICAL_DEV_NUM, 0x03)
	M1_PNPSET(ISAPNP_ACTIVATE, 0x01)
	M1_PNPEXIT()
	
	/* point PCI #1 at PCI memory space (for RTC/NVRAM) */
	N4_INIT(N4_PCIW1,	PCI_MEM_SPACE | N4_PDAR_64BIT | N4_PDAR_128MB)
	N4_INIT(N4_PCIINIT1,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_MEM)
	
	LED_INIT(0x1)
	
	EXIT_INIT(0)

	
#ifdef IN_PMON
	
LEAF(sbdberrenb)
	mfc0	v0,C0_SR
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache/parity errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache/parity errors (SR_DE = 0)
	and	t1,v0
2:	mtc0	t1,C0_SR
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
	j	ra
END(sbdberrenb)


LEAF(sbdberrcnt)
	move	v0,zero
	j	ra
END(sbdberrcnt)
	

	.lcomm	wbfltmp,4

LEAF(wbflush)
XLEAF(mips_wbflush)
	la	t0,wbfltmp
	or	t0,K1BASE
	sync
	lw	zero,0(t0)
	j	ra
END(wbflush)

/* N microsecond delay */
LEAF(sbddelay)
	li	t1,CACHEUS(1)
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS(1)
1:	mul	a0,t1
	subu	a0,10		# approx number of loops so far

	.set	noreorder	
	.set	nomacro
	nop
2:	bgtz	a0,2b
	subu	a0,1
	.set	macro
	.set	reorder

	j	ra
END(sbddelay)


LEAF(mips_cycle)
	.set	noreorder	
	.set	nomacro
1:	bgtz	a0,1b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(mips_cycle)
	
#endif /* IN_PMON */
