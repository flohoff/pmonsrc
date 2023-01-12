/* $Id: sbdreset.S,v 1.2 1996/01/16 14:25:24 chris Exp $ */
/* xds/sbdreset.S - low level board dependent startup code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include <xds/sbd.h>

#ifndef M_BUSWIDTH
# define M_BUSWIDTH	16
#endif

#define MEMINCR		0x100000

#define DELAY(n) \
        .set    noreorder; .set    noat; 		\
        li      AT,(n); 	\
9:	bnez    AT,9b;	 	\
        subu    AT,1; 		\
        .set    at; .set    reorder

#define MSDELAY(x)	DELAY((x) * ROMMS)

	.globl	sbdreset
	.ent	sbdreset
sbdreset:
	move	s0,ra			# save return address

	/* 
	 * Disable "cache errors" caused by memory parity errors
	 * and ensure kseg 0 is set to "cached, non coherent" 
	 * and 32 bytes/cache line
	 */
	.set	noreorder
	mfc0	s1,C0_SR
	mfc0	t1,C0_CONFIG
	nop
	or	t0,s1,SR_DE
	mtc0	t0,C0_SR
#ifndef ITBASE	
	/* if launched by ITROM, leave Config alone */
	and	t1,~0x3f		# bits 5..0
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	mtc0	t1,C0_CONFIG
#endif	
	.set	reorder 

	/* initialise tlb */
	.set noreorder
	mtc0	zero,C0_TLBLO0		/* tlblo0 = invalid */
	mtc0	zero,C0_TLBLO1		/* tlblo1 = invalid */
	mtc0	zero,C0_PGMASK
	li	a0,K1BASE		/* tlbhi  = impossible vpn */
	li	a1,(NTLBENTRIES-1)	/* index */
	nop

1:	mtc0	a0,C0_TLBHI
	mtc0	a1,C0_INX
	addu	a0,0x2000		/* inc vpn */
	tlbwi
	bnez	a1,1b
	subu	a1,1			# BDSLOT
	.set reorder

	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
/*	bnez	s7,8f */

	/* dram system may need ~128us to start up */
	li	v0,128
1:	subu	v0,1
	bnez	v0,1b

	/*
	 * explicitly clear parity in bottom 64K for primary cache
	 * initialisation (XXX what about secondary cache)
	 */
	li	t0,PHYS_TO_K1(0)
	li	t5,PHYS_TO_K1(0x10000)

	.set	noreorder
1:	sw	zero,0(t0)
	sw	zero,4(t0)
	sw	zero,8(t0)
	addu	t0,16
	bne	t0,t5,1b
	sw	zero,-4(t0)		# BDSLOT
	.set	reorder

	/*
	 * now we can safely initialises the R4000 caches
	 */
	jal	init_cache

#ifdef notdef
	/* skip memory size/clear if a soft reset */
	and	t0,s1,SR_SR
	bnez	t0,8f
#endif
	/*
	 * we *must* now size and clear all of memory, so that it
	 * has good parity
	 */
	li	t0, PHYS_TO_K1(0)	# start of test
	move	t1, t0			# bottom of memory
	li	t2, 0xaa55aa55		# test pattern
	not	t3, t2			# ~(test pattern)

memprobe:
        addu    t0, MEMINCR
	move	t4, t0

	/* store pattern in bank 0, line 0 */
	sw	t2, 0(t4)
	addu	t4, 4

#if M_BUSWIDTH > 4
	/* fill remainder of line with zeros */
	li	t5, M_BUSWIDTH-4
1:	sw	zero, 0(t4)
	subu	t5,4
	addu	t4,4
	bnez	t5,1b
#endif

	/* store inverse pattern in bank 0, line 1 */
	sw	t3, 0(t4)
	addu	t4, 4

#if M_BUSWIDTH > 4
	/* fill remainder of line with zeros */
	li	t5, M_BUSWIDTH-4
1:	sw	zero, 0(t4)
	subu	t5,4
	addu	t4,4
	bnez	t5,1b
#endif

	/* defeat write buffering */
	sync

	lw	t4,0(t0)		# first word of line
	lw	t5,0(t1)		# start of memory
	bne	t4,t2,memfail		# first word wrong?
        beq     t5,zero,memprobe        # wrap-around clobbered start?

memfail:
	/* check for no (< mimimum) memory */
	bgtu	t0,PHYS_TO_K1(MEMINCR),1f
9:	b	9b

1:	li	t1,PHYS_TO_K1(0)
	
	/* allow memory clear loop to run cached if .text linked to kseg0 */
	la	t4,1f
	jr	t4

	.set	noreorder
#if __mips >= 3	
1:	sd	zero,0(t1)
	sd	zero,8(t1)
	sd	zero,16(t1)
	sd	zero,24(t1)
	sd	zero,32(t1)
	sd	zero,40(t1)
	sd	zero,48(t1)
	addu	t1,64
	bne	t1,t0,1b
	sd	zero,-8(t1)		# BDSLOT
#else
1:	sw	zero,0(t1)
	sw	zero,4(t1)
	sw	zero,8(t1)
	sw	zero,12(t1)
	sw	zero,16(t1)
	sw	zero,20(t1)
	sw	zero,24(t1)
	addu	t1,32
	bne	t1,t0,1b
	sw	zero,-4(t1)		# BDSLOT
#endif	
	.set	reorder

8:
	mtc0	s1,C0_SR
	move	ra,s0
	j	ra
	.end	sbdreset




	.globl	sbdberrenb
	.ent	sbdberrenb
sbdberrenb:
	.set noreorder	
	mfc0	v0,C0_SR
	.set reorder	
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache/parity errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache/parity errors (SR_DE = 0)
	and	t1,v0
	.set noreorder
2:	mtc0	t1,C0_SR
	.set reorder
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
	j	ra
	.end	sbdberrenb


	.globl	sbdberrcnt
	.ent	sbdberrcnt
sbdberrcnt:
	move	v0,zero
	j	ra
	.end	sbdberrcnt


	.globl	wbflush
	.ent	wbflush
wbflush:
	sync
	j	ra
	.end	wbflush

	.globl sbddelay
	.ent sbddelay
sbddelay:
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far
	.set	noreorder	
1:	bgtz	a0,1b
	subu	a0,1
	.set	reorder
	j	ra
	.end sbddelay
	
