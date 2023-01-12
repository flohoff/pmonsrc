/*
 * TEMIC/sbdreset.S: low level reset code for Algor/Temic module
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#if !__mips64
# error Must be compiled with 64-bit support
#endif
	
/* Enable Parity or ECC (or neither) for memory */
/*#define TEMIC_PARITY*/
/*#define TEMIC_ECC*/
		
#ifdef IN_PMON	
#include <mips.h>
#include <pmon.h>
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
	
#define LED(x)
	
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
	
#ifdef IN_PMON	
	/* trigger analyser */
	lbu	zero,PA_TO_KVA1(0x1fc7ff00)+CHKPNT_SBDR
#endif
	
	/* NILE-4 base address at reset */
	li	a1,PA_TO_KVA1(0x1fa00000)
	
#if VRC5074_BASE != 0x1fa00000	
	/* Set Vrc5074's own base address */
	dli	t0,VRC5074_BASE | N4_PDAR_64BIT | N4_PDAR_2MB
	sd	t0,N4_INTCS(a1)
	li	a1,PA_TO_KVA1(VRC5074_BASE)
#endif
	
	/* XXX pick correct entrypoint based on initial BOOTCS PDAR size */
		
	la	a0,reginiteprom
	addu	a0,RELOC
	bal	reginit

	/* initialise the DPRAM semaphores */
	li	a0,PA_TO_KVA1(SEM_BASE)
	li	a1,8
	li	v0,-1
1:	addu	a1,-1
	addu	a0,4
	sw	v0,-4(a0)
	bnez	a1,1b
	
	/* 
	 * Now determine DRAM configuration
	 * XXX Fixed 32MB on this board.
	 */
	li	msize,32*MEG
	
#if defined(TEMIC_ECC) || defined(TEMIC_PARITY)
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
	la	t0,mips_init_cache
	addu	t0,RELOC
	jal	t0
#else	
	bal	mips_init_cache	
#endif
		
	/* Finally clear rest of memory running cached */
	li	a0,PA_TO_KVA1(65536)
	subu	a1,msize,65536
	blez	a1,.noclear
	addu	a1,a0
	
	/* clear all of memory (to set correct parity bits) */
	
#if 0	
	/* run memory clear loop cached */
	/* XXX sorry, can't run cached from ROM */
	la	t0,1f
	addu	t0,RELOC
	and	t0,~0x20000000
	j	t0
#endif
		
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
	
#define N4_INIT(o,v)	WR_INIT(D, VRC5074_BASE+(o), v)
#define PCI_INIT(s,o,v)	WR_INIT(s, VRC5074_BASE+N4_PCICFG+(o), v)
	
	.rdata
	.align	3
	
reginitflash:			
	/* Boot window is flash */
	N4_INIT(N4_BOOTCS,	BOOTPROM_BASE | N4_PDAR_32BIT | N4_PDAR_16MB \
				| N4_PDAR_LOC)
		
	/* DCS-8 window is eprom */
	N4_INIT(N4_DCS8,	FLASH_BASE | N4_PDAR_8BIT | N4_PDAR_16MB \
				| N4_PDAR_LOC)
	
	JUMP_INIT(reginitmain)
	
	
reginiteprom:			
	/* Boot window is eprom */
	N4_INIT(N4_BOOTCS,	BOOTPROM_BASE | N4_PDAR_8BIT | N4_PDAR_32MB \
				| N4_PDAR_LOC)
		
	/* DCS-8 window is flash */
	N4_INIT(N4_DCS8,	FLASH_BASE | N4_PDAR_32BIT | N4_PDAR_16MB \
				| N4_PDAR_LOC)
	
reginitmain:			
	/* Set all gp outputs to zero */
	N4_INIT(N4_DCSIO,	0)
	
	/* DCS2-5=EEROM i/f; DCS6-7=DPRAM CS; DCS8=Flash CS */
	N4_INIT(N4_DCSFN,	N4_DCSFN_DCSFN2_GPO | \
				N4_DCSFN_DCSFN3_GPI | \
				N4_DCSFN_DCSFN4_GPI | \
				N4_DCSFN_DCSFN5_GPO | \
				N4_DCSFN_DCSFN6_BUS | \
				N4_DCSFN_DCSFN7_BUS | \
				N4_DCSFN_DCSFN8_BUS)
	
	/* DRAM window */
	N4_INIT(N4_SDRAM0,	DRAM_BASE | N4_PDAR_64BIT | N4_PDAR_32MB \
				| N4_PDAR_MEM | N4_PDAR_VISPCI)
	
	/* DCS-7 window is DPRAM memory */
	N4_INIT(N4_DCS7,	DPRAM_BASE | N4_PDAR_32BIT | N4_PDAR_2MB \
				| N4_PDAR_LOC)
	
	/* DCS-6 window is DPRAM semaphores */
	N4_INIT(N4_DCS6,	SEM_BASE | N4_PDAR_32BIT | N4_PDAR_2MB \
				| N4_PDAR_LOC)
	
	
	/* PCI #0 window (PCI i/o space) */
	N4_INIT(N4_PCIW0,	PCI_IO_SPACE | N4_PDAR_64BIT | N4_PDAR_32MB)
	N4_INIT(N4_PCIINIT0,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_IO)
	
	/* PCI #1 window (PCI conf space) */
	N4_INIT(N4_PCIW1,	PCI_CONF_SPACE | N4_PDAR_64BIT | N4_PDAR_64MB)
	N4_INIT(N4_PCIINIT1,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_CONF)
	
	/* Disable CPU interface parity checking and generation */
	N4_INIT(N4_CPUSTAT,	N4_CPUSTAT_TMODE_SGOOD | N4_CPUSTAT_DISCPUPC \
				| N4_CPUSTAT_DISPC)
	
#if defined(TEMIC_ECC)
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKENB | N4_MEMCTRL_CHKMODE_ECC
#elif defined(TEMIC_PARITY)
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKENB | N4_MEMCTRL_CHKMODE_PAR
#else	
#define N4_MEMCTRL_CHK		N4_MEMCTRL_CHKDIS
#endif
	
	/* Set DRAM configuration */
	N4_INIT(N4_MEMCTRL, 	N4_MEMCTRL_HOLDLD \
				| N4_MEMCTRL_CHK \
				| N4_MEMCTRL_DRAMTYP_64MB_4BANK \
				| N4_MEMCTRL_ENABLE)
	
	/* Local bus device timing */
#if 1
/* tuned fast timings */	
/* EPROM/Flash:	 150ns @ 25MHz local bus */
#define LCST_ROM	(N4_LCST_CSON(1) \
			| N4_LCST_CONSET(1) \
			| N4_LCST_CONWID(4) \
			| N4_LCST_SUBSCWID(4) \
			| N4_LCST_CSOFF(0) \
			| N4_LCST_COFHOLD(0) \
			| N4_LCST_BUSIDLE(1))
/* DPRAM:	 35ns @ 25MHz local bus */
#define LCST_DPRAM	(N4_LCST_CSON(0) \
			| N4_LCST_CONSET(0) \
			| N4_LCST_CONWID(1) \
			| N4_LCST_SUBSCWID(1) \
			| N4_LCST_CSOFF(0) \
			| N4_LCST_COFHOLD(0) \
			| N4_LCST_BUSIDLE(1))
#else
/* default slow timings */	
#define LCST_ROM	(N4_LCST_CSON(1) \
			| N4_LCST_CONSET(3) \
			| N4_LCST_CONWID(8) \
			| N4_LCST_SUBSCWID(8) \
			| N4_LCST_CSOFF(3) \
			| N4_LCST_COFHOLD(3) \
			| N4_LCST_BUSIDLE(7))
#define LCST_DPRAM	LCST_ROM
#endif		
	
	/* BOOTCS=EPROM/Flash cycle timing */
	/* DCS8=Flash/EPROM cycle timing */
	N4_INIT(N4_BCST,	LCST_ROM)
	N4_INIT(N4_LCST8,	LCST_ROM)
	
	/* DCS6-7=DPRAM timing */
	N4_INIT(N4_LCST7,	LCST_DPRAM)
	N4_INIT(N4_LCST6,	LCST_DPRAM)
			
	/* PCI priority on local bus & run lock bus at SYSCLK/4 */
	N4_INIT(N4_LCNFG,	N4_LCNFG_PCIHOG(16) | N4_LCNFG_CPUHOG(1) \
				| N4_LCNFG_FLCLCLK_DIV4)
	
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
#elif defined(TEMIC_ECC) || defined(TEMIC_PARITY)
	/* Memory error interrupt to INT[4] */
	N4_INIT(N4_INTCTRL,	N4_INTCTRL_EN(N4_INT_MCE) \
				| N4_INTCTRL_PRI(N4_INT_MCE,4) \
				| N4_INTCTRL_EN(N4_INT_CNTD) \
				| N4_INTCTRL_PRI(N4_INT_CNTD,4)	\
				| N4_INTCTRL_EN(N4_INT_LBRT) \
				| N4_INTCTRL_PRI(N4_INT_LBRT,4))
	
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
				| PCI_MAP_MEMORY_TYPE_32BIT)
	
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
	
	/* point PCI #1 at PCI memory space (for RTC/NVRAM) */
	N4_INIT(N4_PCIW1,	PCI_MEM_SPACE | N4_PDAR_64BIT | N4_PDAR_128MB)
	N4_INIT(N4_PCIINIT1,	N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_MEM)
	
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

	
/* void sbddisplay (unsigned long msg, unsigned char code) */
LEAF(sbddisplay)
	/* something that the analyser can trigger on */
	and	a1,0xff
	addu	a1,PA_TO_KVA1(0x1fc7ff00)
	lbu	zero,0(a1)
	j	ra
END(sbddisplay)

	
/* control the l.e.d. blank enable */
/* void sbdblank (int blankon) */
LEAF(sbdblank)
	j	ra
END(sbdblank)


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
