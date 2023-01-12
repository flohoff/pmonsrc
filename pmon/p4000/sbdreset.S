/* $Id: sbdreset.S,v 1.3 1997/10/09 23:01:35 chris Exp $ */
/* p4000/p4000.S - low level board dependent startup code for PMON
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restiction.
 */

#include <mips.h>
#include <pmon.h>
#include <p4000/sbd.h>

#ifndef M_BUSWIDTH
# define M_BUSWIDTH	16
#endif

#define MEMINCR		0x100000

#if #cpu(r4000)
#define ROMMS   1000000/(3*1080)        /* ROM - 1.08us instruction XXX */
#else
#define ROMMS   1000000/(2*1080)        /* ROM - 1.08us instruction XXX */
#endif

#define DELAY(n) \
        .set    noreorder; .set    noat; 		\
        li      AT,(n); 	\
9:	bnez    AT,9b;	 	\
        subu    AT,1; 		\
        .set    at; .set    reorder

#define MSDELAY(x)	DELAY((x) * ROMMS)

#define DISPLAY(d0, d1, d2, d3)			\
	li	t6,PHYS_TO_K1(ALPHA_BASE);	\
	li	t7,(d0)<<24; 			\
	sw	t7,ALPHN_ENTRY(0)(t6); 		\
	li	t7,(d1)<<24; 			\
	sw	t7,ALPHN_ENTRY(1)(t6); 		\
	li	t7,(d2)<<24; 			\
	sw	t7,ALPHN_ENTRY(2)(t6); 		\
	li	t7,(d3)<<24; 			\
	sw	t7,ALPHN_ENTRY(3)(t6)


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

	/* table driven hardware register initialisation */
	DISPLAY ('P','4','0','0')
	la	a0, sbditab		/* in p4000/sbd.c */
	or	a0, K1BASE		/* force to kseg1 */

1:	lw	v0,0(a0)
	lw	v1,4(a0)
	addu	a0,8
	beqz	v0,8f
	sw	v1,0(v0)
	b	1b
8:


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
	DISPLAY ('Z','6','4','K')
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
	DISPLAY ('C','A','C','H')
	jal	mips_init_cache

	/* skip memory size/clear if a soft reset */
	and	t0,s1,SR_SR
	bnez	t0,8f

	/*
	 * we *must* now size and clear all of memory, so that it
	 * has good parity
	 */
	DISPLAY ('S','M','E','M')
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
	DISPLAY ('0','M','E','M')
	b	sbdflash

1:	DISPLAY ('Z','M','E','M')
	li	t1,PHYS_TO_K1(0)
	
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
	DISPLAY ('D','O','N','E')
	mtc0	s1,C0_SR
	move	ra,s0
	j	ra
	.end	sbdreset




	.globl	sbddisplay
	.ent	sbddisplay
sbddisplay:
	/* write 4 characters to display */
	la	t0,PHYS_TO_K1(ALPHN_CHAR(0))
1:	sw	a0,0(t0)
	subu	t0,4		# display addressing goes backwards!
	sll	a0,8		# get next character into top byte
	bnez	a0,1b		# continue til all done
	/* unblank the display */
	li	t0,RESET_ONE
	sw	t0,PHYS_TO_K1(ALPHN_BLNK_)
	j	ra
	.end	sbddisplay


	.globl	sbdblank
	.ent	sbdblank
sbdblank:
	li	t0,RESET_ZERO	# blank enable
	bnez	a0,1f		# yes!
	li	t0,RESET_ONE	# no! blank disable.
1:	sw	t0,PHYS_TO_K1(ALPHN_BLNK_)
	j	ra
	.end	sbdblank



	.globl	sbdflash
	.ent	sbdflash
sbdflash:
	la	t0,PHYS_TO_K1(ALPHN_BLNK_)
	li	t1,RESET_ONE
	li	t2,RESET_ZERO
	la	t3,1f
	or	t3,K1BASE
	jr	t3

1:	sw	t1,(t0)
	MSDELAY(250)
	sw	t2,(t0)
	MSDELAY(250)
	b	1b
	.end	sbdflash



	.globl	sbdmsdelay
	.ent	sbdmsdelay
sbdmsdelay:
        mul	a0,ROMMS
	la	t3,1f
	or	t3,K1BASE
	jr	t3
        .set noreorder
1:	bnez    a0,1b
        subu    a0,1
        .set reorder
	j	ra
	.end	sbdmsdelay



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


#define ADDR(x) PHYS_TO_K1(x)

	.rdata
sbditab:
  /* clear and unblank display */
	.word	ADDR(ALPHN_CLR_),	RESET_ZERO
	.word	ADDR(ALPHN_CSR),	RESET_ZERO
	.word	ADDR(ALPHN_BLNK_),	RESET_ONE

  /* reset devices */
	.word	ADDR(SIO_RESET_),	RESET_ZERO
	.word	ADDR(NET_RESET_),	RESET_ZERO

  /* disable daughterboard interrupts */
	.word	ADDR(INT_IMR0),		IMR_DISABLE
	.word	ADDR(INT_IMR1),		IMR_DISABLE
	.word	ADDR(INT_IMR2),		IMR_DISABLE
	.word	ADDR(INT_IMR3),		IMR_DISABLE

  /* configure ethernet interrupt on Intr0 */
	.word	ADDR(INT_ICG0),		ICG_CLR
	.word	ADDR(INT_ICG1),		ICG_CLR

  /* configure serial interrupt on Intr1 */
	.word	ADDR(INT_ICG2),		ICG_SET
	.word	ADDR(INT_ICG3),		ICG_CLR

  /* enable alphanumeric display */
	.word	ADDR(ALPHN_CLR_),	RESET_ONE

  /* enable devices */
	.word	ADDR(SIO_RESET_),	RESET_ONE
	.word	ADDR(NET_RESET_),	RESET_ONE

	.word	0, 0
