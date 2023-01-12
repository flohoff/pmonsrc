/* $Id: sbdreset.S,v 1.3 1998/01/13 15:44:56 chris Exp $ */
/* cogent/sbdreset.S - low level board dependent startup code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include <cogent/sbd.h>
#include <cogent/cm1629.h>

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

LEAF(sbdreset)
	move	s0,ra			# save return address

#if defined(R4300) && !defined(MIPSEB)
	.set	noreorder
	mfc0	v0,C0_CONFIG
	li	v1,CFG_BE
	not	v1			# just to make sure it takes 2 instructions
	and	v0,v1
	mtc0	v0,C0_CONFIG
	nop				# let it filter through
	nop
	nop
	.set	reorder
	/* now change byte enables */
	li	v0,PHYS_TO_K1(ENDIAN_BASE)
	sw	zero,0(v0)
#endif	
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
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,8f

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
	jal	mips_init_cache

	/* skip memory size/clear if a soft reset */
	and	t0,s1,SR_SR
	bnez	t0,8f

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
END(sbdreset)




#ifdef notdef
LEAF(sbdmsdelay)
        mul	a0,ROMMS
	la	t3,1f
	or	t3,K1BASE
	jr	t3
        .set noreorder
1:	bnez    a0,1b
        subu    a0,1
        .set reorder
	j	ra
END(sbdmsdelay)
#endif
	
LEAF(sbdlcdcmd)
1:	li	t0,PHYS_TO_K1(LCD_BASE)
	
	/* wait for not busy */
1:	lbu	t1,LCD_STAT(t0)
	and	t1,LCD_STAT_BUSY
	bnez	t1,1b
		
	sb	a0,LCD_CMD(t0)
	j	ra
END(sbdlcdcmd)

LEAF(sbdlcddata)
1:	li	t0,PHYS_TO_K1(LCD_BASE)
	
	/* wait for not busy */
1:	lbu	t1,LCD_STAT(t0)
	and	t1,LCD_STAT_BUSY
	bnez	t1,1b
		
	sb	a0,LCD_DATA(t0)
	j	ra
END(sbdlcddata)


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
	
