/* $Id: r3kcflu.S,v 1.2 1996/01/16 14:18:20 chris Exp $ */
#ifdef R3000

#include "mips.h"

.comm	icache_size,4
.comm	dcache_size,4

	.globl	r3k_iflush
	.ent	r3k_iflush
r3k_iflush:
	subu	sp,24
	sw	ra,20(sp)
	
 	# get size
	la	t0,icache_size
	or	t0,K1BASE
	lw	v0,(t0)

	# set SR bits
	li	a0,(SR_ISC|SR_SWC)

	# make me uncacheable
	la	t0,1f
	or	t0,K1BASE
	j	t0

	/* if size == 0, call size_cache */
1:	bne	v0,zero,1f

	jal	size_cache

	/* # if size == 0, return */
	beq	v0,zero,10f
	# update size
	la	t0,icache_size
	or	t0,K1BASE
	sw	v0,(t0)

1:	move	a1,v0			# size
	li	a2,K0BASE
	jal	flush_common

10:	lw	ra,20(sp)
	addu	sp,24
	j	ra
	.end	r3k_iflush

	.globl	r3k_dflush
	.ent	r3k_dflush
r3k_dflush:
	subu	sp,24
	sw	ra,20(sp)

	# get size
	la	t0,dcache_size
	or	t0,K1BASE
	lw	v0,(t0)

	li	a0,SR_ISC	# sr bits

	/* # if size == 0, call size_cache */
	bne	v0,zero,1f

	jal	size_cache

	/* # if size == 0, return */
	beq	v0,zero,10f

	# update size
	la	t0,dcache_size
	or	t0,K1BASE
	sw	v0,(t0)

1:	move	a1,v0		# size
	li	a2,K0BASE
	jal	flush_common

10:	lw	ra,20(sp)
	addu	sp,24
	j	ra
	.end	r3k_dflush

	
	.globl	dcache_clean
	.ent	dcache_clean
dcache_clean:
	subu	sp,24
	sw	ra,20(sp)

	move	t6,a0
	move	t7,a1
	
	# get size
	la	t0,dcache_size
	or	t0,K1BASE
	lw	a1,(t0)

	li	a0,SR_ISC	# sr bits

	/* # if size == 0, call size_cache */
	bne	a1,zero,1f

	jal	size_cache

	/* # if size == 0, return */
	beq	v0,zero,10f

	# update size
	la	t0,dcache_size
	or	t0,K1BASE
	sw	v0,(t0)
	move	a1,v0
	
1:	bgtu	t7,a1,2f
	move	a1,t7		# a1 = min(size, dcache_size)
2:	move	a2,t6		# a2 = start
	jal	flush_common

10:	lw	ra,20(sp)
	addu	sp,24
	j	ra
	.end	dcache_clean
	

	.globl flush_common
	.ent flush_common
flush_common:
	# a0=sr bits a1=size a2=start

	.set noreorder
	mfc0	t8,C0_SR
	nop
	or	t0,a0,t8
	and	t0,~SR_IEC	# disable ints
	mtc0	t0,C0_SR

	addu	v1,a2,a1
1:	sb	zero,0(a2)	/* See - "LR3000 and LR3000A MIPS RISC	*/
	sb	zero,4(a2)	/*        Microprocessor User's Manual"	*/
	sb	zero,8(a2)	/*	  P8-9 'Cache Isolation'	*/
	addiu	a2,16
	bltu	a2,v1,1b
	sb	zero,-4(a2)

	mtc0	t8,C0_SR	# restore sr
	.set reorder
	j	ra
	.end flush_common

	.globl size_cache
	.ent size_cache
size_cache:
	# a0=sr bits rtn=size
	.set noreorder
	mfc0	t8,C0_SR
	nop
	or	t0,a0,t8
	and	t0,~SR_IEC	# disable ints
	mtc0	t0,C0_SR

	li	v0,512		# min cache size = 512b 
	li	v1,(256*1024)	# max cache size = 256Kb
	
	# store cache size at cache size offsets, working down.
1:	sw	v1,K0BASE(v1)
	bne	v1,v0,1b
	srl	v1,1		# BDSLOT

	# now we can just read the cache size from the cache[0]
	lw	v0,K0BASE

 	mtc0	t8,C0_SR
	.set	reorder
	j	ra
	.end size_cache

	.globl r3k_iaflush
	.ent r3k_iaflush
r3k_iaflush:
	# a0=addr
	# run this uncaches
	la	t0,1f
	or	t0,K1BASE
	j	t0
1:

	.set noreorder
	mfc0	t8,C0_SR
	or	t0,t8,(SR_ISC|SR_SWC)
	and	t0,~SR_IEC	# disable ints
	mtc0	t0,C0_SR
	nop
	nop
	.set reorder

	sb	zero,(a0)

	.set noreorder
	mtc0	t8,C0_SR
	.set reorder
	j	ra
	.end r3k_iaflush
#endif /* R3000 */
