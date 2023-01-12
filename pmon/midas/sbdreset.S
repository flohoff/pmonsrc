/* midas/sbdreset.S - low level board dependent startup code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include <midas/arizona.h>
#include <midas/sbd.h>

#include "meminit.s"
		
#define rasave s0
LEAF(sbdreset)
	move	rasave,ra

	SBD_DISPLAY('S','R','S','T', CHKPNT_SBDR)
	
	/* if launched by ITROM, leave Config alone */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
#endif	

	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,.noinit

	/* initialise MC */

	
	la	a0, reginittab
	or	a0, K1BASE
1:
#ifdef MIDASPABUG
	lw	v0,4(a0)
	lw	v1,12(a0)
	addu	a0,16
#else
	lw	v0,0(a0)
	lw	v1,4(a0)
	addu	a0,8
#endif		
	beqz	v0,8f

	not	v0
	beqz	v0, .doread

	not	v0
	sw	v1,0(v0)
	b	1b

.doread:
	lw	zero, 0(v1)
	b	1b	
8:

	/* we can now initialise the caches for a fast clear_mem */
	SBD_DISPLAY ('C','A','C','H',CHKPNT_CACH)
	jal	init_cache

.noinit:

	/* initialise tlb */
	SBD_DISPLAY ('I','T','L','B', CHKPNT_ITLB)
	bal	init_tlb

	move	ra,rasave
	j	ra
END(sbdreset)


LEAF(_sbd_memfail)
	SBD_DISPLAY ('!','M','E','M',CHKPNT_0MEM)
1:	b	1b
	j	ra
END(_sbd_memfail)


#define MCINIT(mcreg, val) INIT(MC_REG(mcreg), val)
#ifdef MIDASPABUG
#define INIT(addr,val) \
	.word	0, addr, 0, val
#else
#define INIT(addr,val) \
	.word	addr, val
#endif	
	.rdata
#ifdef MIDASPABUG
	.align	3
#endif	
reginittab:
	MCINIT(MC_SD_CONF_A, MC_SD_CONF_A_INIT)
	MCINIT(MC_SD_CONF_B, MC_SD_CONF_B_INIT)
	MCINIT(MC_GEN_CTL, MC_GEN_CTL_INIT0)
	MCINIT(MC_SC_IO_MAP, MC_SC_IO_MAP_INIT)
	MCINIT(MC_SC_BURST_MAP, MC_SC_BURST_MAP_INIT)
	MCINIT(MC_SC_IO_B_MAP, MC_SC_IO_B_MAP_INIT)

	INIT(~0, PA_TO_KVA1(0))		# read from DRAM to initialise memory

#ifdef MC_GEN_CTL_INIT1
	MCINIT(MC_GEN_CTL, MC_GEN_CTL_INIT1)
#endif
	
	INIT(0,0)
	.text

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


LEAF(sbddelay)
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far

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


LEAF(sbddisplay)
	li	a0,PA_TO_KVA1(DBGLED_BASE)
	sb	a1,0(a0)
	j	ra
END(sbddisplay)		
