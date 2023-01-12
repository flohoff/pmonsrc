/*
 * NEC41XX/sbdreset.S: low level board initialisation for NEC Vr41xx board
 * Copyright (c) 1998 Algorithmics Ltd.
 */

#include <mips.h>
#include <pmon.h>
#include "sbd.h"
	
#if 0	
/* generic memory sizing/clearing code */	
#define BUSWIDTH	4		/* 32-bit, non-interleaved */ 
#define MEMLIMIT	(64*1024*1024)	/* max 64MB */
#include "meminit.s"
#endif
	
/*
 * Basic board initialisation, called called straight from RESET.
 */
	
#define RELOC	zero
			
LEAF(sbdreset)
	move	s7,ra			# save return address
	
	mfc0	t1,C0_CONFIG
	mfc0	t2,C0_STATUS
	
	/* disable cache parity check (WinCE init) */
	li	t0,0x00410000		# magic!!
	mtc0	t0,C0_ECC
	
	/* set kseg0 = cacheable write-back */
	/* set s/w controllable bits */
	and	t1,~(CFG_AD|CFG_EPMASK|CFG_K0MASK)
	or	t1,CFG_EPD|CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
	
	/* clear watchpoint */
	mtc0	zero,C0_WATCHLO
	mtc0	zero,C0_WATCHHI
	
	/* initialise status register */
	and	t2,SR_SR		# leave the SoftReset bit
	or	t2,SR_BEV		# force boot exceptions
	mtc0	t2,C0_STATUS
	
	/* initialise bus controller */
	li	t0,PA_TO_KVA1(BCU_BASE)
	
#ifdef NEC41XX_DRAM64
	li	t1,BCUCNT1_DRAM64	# special 64MB DRAM mode
#else	
	li	t1,0			# default 16MB DRAM mode
#endif	
	sh	t1,BCU_CNT1(t0)
	
	li	t1,BCUCNT2_GMODE	# non-inverted LCD writes
	sh	t1,BCU_CNT2(t0)
	
	#li	t1,4901			# ISA bus speed
	li	t1,789			# ISA bus speed
	sh	t1,BCU_SPEED(t0)
	
	lhu	t2,BCU_CLKSPEED(t0)	# get CPU clock
	and	t2,BCUCLKSPEED_CLKSP_MASK
	srl	t2,BCUCLKSPEED_CLKSP_SHIFT
	li	t1,8800
	divu	t1,t2			# refresh rate = ~8800/clksp
	sh	t1,BCU_RFCNT(t0)	# set refresh rate
		
	/* get CPU/chip type */
	lh	t1,BCU_REVID(t0)
	and	t1,BCUREVID_RID_MASK
	srl	t1,BCUREVID_RID_SHIFT
	bltu	t1,RID_VR4121,1f
	
	/* Vr4121/22 only from here (code from NECMON mon.S file) */
	li	t1,BCUCNT3_EXTDRAM64|BCUCNT3_EXTMEM
	sh	t1,BCU_CNT3(t0)
	li	t1,0			# ISA bus speed!!!
	sh	t1,BCU_SPEED(t0)
	li	t1,0x600		# fixed refresh????
	sh	t1,BCU_RFCNT(t0)
	li	t1,0x2222		# 32MB
	sh	t1,BCU_ROMSIZE(t0)
	sh	t1,BCU_RAMSIZE(t0)
	li	t1,0x8039		# default 
	sh	t1,BCU_SDRAMMODE(t0)
	li	t1,0x006D		# default 6 RAS latency
	sh	t1,BCU_SROMMODE(t0)
	li	t1,0x0944		# default 9 Tclk, 4 Tclk, 4Tclk
	sh	t1,BCU_SDRAMCNT(t0)
	
	/* write something interesting to the 2 digit LED */
1:	li	t0,PA_TO_KVA1(LEDWR_BASE)
	li	t1,CHKPNT_SBDR
	sb	t1,0(t0)

	/* reset HAL timer so PMU does not close us down again */
	li	t0,PA_TO_KVA1(PMU1_BASE)
	lhu	t1,PMU1_CNT(t0)
	and	t1,0xff80
	or	t1,PMUCNT_HALTIMERRST|PMUCNT_SET
	sh	t1,PMU1_CNT(t0)
	
	/* supply power and clocks to serial, parallel and keybd port */
	li	t0,PA_TO_KVA1(CMU_BASE)
	li	t1,CLKMSK_PIU|CLKMSK_SIU|CLKMSK_KIU|CLKMSK_DSIU|CLKMSK_SSIU
	sh	t1,CMU_CLKMSK(t0)
	
#ifdef IN_PMON	
	bal	init_tlb
#endif
	
	/* return to generic reset, with memory size */
	move	ra,s7
	j	ra
END(sbdreset)


SLEAF(init_tlb)
	li	t0,PA_TO_KVA1(LEDWR_BASE)
	li	t1,CHKPNT_ITLB
	sb	t1,0(t0)
	
	/* initialise tlb */
	mtc0	zero,C0_TLBLO0		/* tlblo0 = invalid */
	mtc0	zero,C0_TLBLO1		/* tlblo1 = invalid */
	mtc0	zero,C0_PGMASK
	li	t8,K1BASE		/* tlbhi  = impossible vpn */
	li	t9,(NTLBENTRIES-1)	/* index */
	
	.set noreorder
	nop
1:	mtc0	t8,C0_TLBHI
	mtc0	t9,C0_INX
	addu	t8,0x2000		/* inc vpn */
	tlbwi
	bnez	t9,1b
	subu	t9,1			# BDSLOT
	.set reorder

	j	ra
END(init_tlb)


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
	sync
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,0(t0)
	j	ra
END(wbflush)

	
LEAF(sbddisplay)
	li	t1,PA_TO_KVA1(LEDWR_BASE)
	sb	a1,0(t1)
#ifdef SBDDEBUG
	/* drop through .5 sec delay */
	li	a0,500000
#else
	j	ra
#endif
END(sbddisplay)
	
	
LEAF(sbddelay)
	li	t1,CACHEUS(1)
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS(1)
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far
XLEAF(mips_cycle)
	.set	noreorder	
	.set	nomacro
1:	bgtz	a0,1b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(sbddelay)
	
