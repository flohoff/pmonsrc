/* $Id: haswhat.S,v 1.2 1996/01/16 14:18:03 chris Exp $ */
#if !defined(LR33020) && !defined(R4000)
#define LR33020
#endif

#include <regdef.h>

/*
** Test for presence of various bits of hardware.
*/

	.globl hasFPU
	.ent hasFPU
hasFPU:
#ifdef R4000
	li	v0,1
#else 
	.set noreorder
	mfc0	t8,C0_SR
	nop
	or	t0,t8,SR_CU1
	mtc0	t0,C0_SR
	nop
	nop
	.set reorder

	li	v0,0		# default return = not present
	ctc1	zero,$31
	li	t0,0xffffffff
	ctc1	t0,$31
	cfc1	t1,$31

	li	t0,0x0083ffff
	and	t1,t0
	bne	t1,t0,3f

	# cp1 is present
	# check to see if CpCond1 pin is connected
	bc1f	3f		# should be high

	ctc1	zero,$31
	bc1t	3f		# should be low

	# all present and correct
	li	v0,1

3:	# restore SR
	.set noreorder
	mtc0	t8,C0_SR
	.set reorder
#endif
	j	ra
	.end hasFPU

	.globl hasCP2
	.ent hasCP2
hasCP2: 
#ifdef R4000
	move	v0,zero
#else
	.set noreorder
	mfc0	t8,C0_SR
	nop
	li	t1,SR_CU2
	or	t0,t8,t1
	mtc0	t0,C0_SR
	nop

	nop
	mtc2	zero,C2_BLANKE
	nop
	li	t1,0x5555aaaa
	mtc2	t1,C2_BLANKE
	nop
	mfc2	t2,C2_BLANKE
	nop
	mtc0	t8,C0_SR		# restore SR
	.set reorder

	bne	t2,t1,1f
	li	v0,1
	b	2f

1:	li	v0,0
#endif
2:	j	ra
	.end hasCP2

	.globl hasTLB
	.ent hasTLB
hasTLB: /* don't know how to do this yet, just assume no */
#ifdef R4000
	li	v0,1
#else
	li	v0,0
#endif
	j	ra
	.end hasTLB

