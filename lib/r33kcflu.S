/* $Id: r33kcflu.S,v 1.2 1996/01/16 14:18:20 chris Exp $ */
#ifdef LR33000
#include "mips.h"

	.globl r33k_iflush
	.ent r33k_iflush
r33k_iflush:
	# disable ints
	.set noreorder
	mfc0	t7,C0_SR
	nop
	and	t0,t7,~SR_IEC
	mtc0	t0,C0_SR
	.set reorder

	# switch to Kseg1
	la	t0,1f
	li	t1,K1BASE
	or	t0,t1
	j	t0

1:	li	t0,K0BASE
	addu	t4,t0,511*16
	li	t2,DCIC_I

	.set noreorder
	mfc0	t8,C0_DCIC
	nop
	mtc0	t2,C0_DCIC
	nop
	nop
	.set reorder

1:	sw	zero,(t0)
	addu	t0,16
	bne	t4,t0,1b

	.set noreorder
	nop
	nop
	nop
	mtc0	t8,C0_DCIC	# restore DCIC
	mtc0	t7,C0_SR	# restore SR
	.set reorder
	j	ra
	.end r33k_iflush

	.globl r33k_dflush
	.ent r33k_dflush
r33k_dflush:
	# disable ints
	.set noreorder
	mfc0	t7,C0_SR
	nop
	and	t0,t7,~SR_IEC
	mtc0	t0,C0_SR
	.set reorder

	# switch to Kseg1
	la	t0,1f
	li	t1,K1BASE
	or	t0,t1
	j	t0

1:	li	t0,K0BASE
	addu	t4,t0,63*16
	li	t2,DCIC_D

	.set noreorder
	mfc0	t8,C0_DCIC
	nop
	mtc0	t2,C0_DCIC
	nop
	nop
	.set reorder

1:	sw	zero,(t0)
	addu	t0,16
	bne	t4,t0,1b

	.set noreorder
	nop
	nop
	nop
	mtc0	t8,C0_DCIC
	mtc0	t7,C0_SR	# restore SR
	.set reorder
	j	ra
	.end r33k_dflush

	.globl r33k_iaflush
	.ent r33k_iaflush
r33k_iaflush:
	# a0=addr
	# word align the address
	li	t0,~3
	and	a0,t0

	# disable ints
	.set noreorder
	mfc0	t7,C0_SR
	nop
	and	t0,t7,~SR_IEC
	mtc0	t0,C0_SR
	.set reorder

	# switch to Kseg1
	la	t0,1f
	li	t1,K1BASE
	or	t0,t1
	j	t0

1:	li	t2,DCIC_I

	.set noreorder
	mfc0	t8,C0_DCIC
	nop
	mtc0	t2,C0_DCIC
	nop
	nop
	.set reorder

	sw	zero,(a0)	# THE FLUSH

	.set noreorder
	nop
	nop
	nop
	mtc0	t8,C0_DCIC	# restore DCIC
	mtc0	t7,C0_SR	# restore SR
	.set reorder
	j	ra
	.end r33k_iaflush

#endif /* LR33000 */
