/* p4032/sbdreset.S - low level board dependent startup code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include <glacier/sbd.h>


#define MEG	0x100000		

# define DISPLAY(a,b,c,d)
	
/* 
 * Include standard memory test/clear code.
 */
#define BUSWIDTH	4		/* 32-bit, non-interleaved */
	
#include "meminit.s"

#define	rasave	s6
#define	RELOC	s8


LEAF(sbdreset)
	move	rasave,ra

	/* if launched by ITROM, leave Config alone */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	/* On the R4300 we also have control over the endianness */
#ifdef MIPSEB
	or	t1,CFG_BE	
#else
	and	t1,~CFG_BE	
#endif
	mtc0	t1,C0_CONFIG
#endif	


	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,.noinit

	/* initialise I/O registers */
1:	la	a0,reginittab
	addu	a0,RELOC
1:	lw	v0,0(a0)		# reg addr
	lw	v1,4(a0)		# reg data
	beqz	v0,2f
	sw	v1,0(v0)
	addu	a0,8
	b	1b
2:

	/* skip memory size/clear if a soft reset */
	mfc0	t1,C0_STATUS
	and	t1,SR_SR
	bnez	t1,.noinit


	/* we can now initialise the caches for a fast clear_mem */
	DISPLAY ('C','A','C','H')
	jal	init_cache

#if 0
	/* clear all of memory (to set correct parity) */
	DISPLAY ('Z','M','E','M')
	la	t0,clear_mem_range
	addu	t0,RELOC
	move	a0,msize
	la	a1,PA_TO_KVA1(0)
	jal	t0
#endif
	
.noinit:

	/* initialise tlb */
	DISPLAY ('I','T','L','B')
	bal	init_tlb

	move	ra,rasave
	j	ra
END(sbdreset)


LEAF(_sbd_memfail)
	DISPLAY ('!','M','E','M')
1:	b	1b
	j	ra
END(_sbd_memfail)


		
#define INIT(addr,val) \
	.word	PA_TO_KVA1(addr), val

	.rdata
reginittab:
	.word	0,0


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
