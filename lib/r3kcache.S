/*
 * r3kcache.S: generic R3000 cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 */

#if #cache(r3k)

#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/r3kc0.h>
#include <mips/prid.h>
	
IMPORT(mips_icache_size,4)
IMPORT(mips_icache_linesize,4)
IMPORT(mips_icache_ways,4)
	
IMPORT(mips_dcache_size,4)
IMPORT(mips_dcache_linesize,4)
IMPORT(mips_dcache_ways,4)


/*
 * void mips_size_cache()
 * Initialise I and D caches and flush them, but only if size is unknown.
 */
LEAF(r3k_size_cache)
	lw	t0,mips_icache_size
	blez	t0,r3k_init_cache
	j	ra
END(r3k_size_cache)


/*
 * void mips_init_cache()
 *
 * Work out size of I and D caches and flush them.
 *	
 * Uses:	v0,v1,t0,t1,t2,t3,t4,t7,t8,ra
 * Save regs:	t3,t7,t8,v1
 * Calls:	_init_cache
 */
LEAF(r3k_init_cache)
	move	t8,ra

	mfc0	t7,$sr
	and	t3,t7,SR_BEV		# disable interrupts
	mtc0	t3,$sr

	/* run uncached (PIC) */
	.set	noreorder
	.set	nomacro
	bal	1f	
	li	t1,KSEG1_BASE
1:	or	t1,ra
	addu	t1,16
	jr	t1
	nop
	.set	macro
	.set	reorder
	
	/* Generic R3000 caches */

	/* make sure write buffer is empty */
	lw	zero, 0(t1)
	
	/*
	 * Size and initialise instruction cache
	 */
	or	t4,t3,SR_ISC|SR_SWC	# isolate and swap cache
	mtc0	t4,$sr
	bal	_init_cache
	move	v1,v0

	/*
	 * Size and initialise data cache
	 */
	or	t4,t3,SR_ISC		# isolate cache
	mtc0	t4,$sr
	bal	_init_cache

	.set	noreorder
	mtc0	t7,$sr			# restore IE and ISC to normal
	nop; nop; nop; nop; nop
	.set	reorder

	/* save results */
	sw	v0,mips_dcache_size
	sw	v1,mips_icache_size

	/* most r3000 family cpus have 1 word (4 bytes) per cache line */
	mfc0	t2,$prid
	li	t0,4
	li	t1,4
	and	t2,0xff00
	srl	t2,8
	bne	t2,PRID_R3IDT,1f	# IDT core has 4 word/line icache
	li	t0,16
	/* can't identify other r30x1 variants, since PrID == R3000A */
1:	sw	t0,mips_icache_linesize
	sw	t1,mips_dcache_linesize
	
	li	t0,1
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
	
	move	ra,t8
	j	ra
END(r3k_init_cache)


/*
 * Internal func to size+initialise R3000 cache; size returned in v0.
 * It works by writing address to address at all possible cache
 * boundaries (256K down to 512).  Due to address wraparound,
 * whatever is found at cache location zero is the cache size.
 */

#define	MIN_CACHE_SIZE		(512)
#define	MAX_CACHE_SIZE		(256*1024)

/*
 * int _init_cache()
 *
 * Returns size of one cache and flushes it.
 *	
 * Uses:	v0,t1,t2,a0,a1,ra
 * Save regs:	none
 * Calls:	nothing
 */
SLEAF(_init_cache)
#ifdef R3KEMU
	li	v0,0
#else
	.set	noreorder
	li	t1,MIN_CACHE_SIZE
	li	t2,MAX_CACHE_SIZE
	nop
	
1:	sw	t2,KSEG0_BASE(t2)
	bne	t2,t1,1b
	srl	t2,1			# BDSLOT

	lw	v0,KSEG0_BASE(zero)	# get cache size

	/*
	 * Now flush the cache
 	 */
	li	t1,KSEG0_BASE
	addu	t2,t1,v0
1:	sw	zero,0(t1)		# clear parity
	sw	zero,4(t1)
	sw	zero,8(t1)
	sw	zero,12(t1)
	sb	zero,0(t1)		# invalidate
	sb	zero,4(t1)
	sb	zero,8(t1)
	addu	t1,16
	bne	t1,t2,1b
	sb	zero,-4(t1)		# BDSLOT
	
	nop; nop; nop			# pipe clear of stores
	.set	reorder
#endif
	j	ra
SEND(_init_cache)


#define SIZE_CACHE(which)		\
	lw	v0,which;		\
	move	t5,ra;			\
	bgez	v0,9f;			\
	bal	r3k_init_cache;		\
	lw	v0,which;		\
	move	ra,t5;			\
9:	blez	v0,9f


/*
 * void mips_clean_dcache (unsigned kva, size_t n)
 * 
 * Invalidate address range in data cache
 */
LEAF(r3k_clean_dcache)
XLEAF(r3k_clean_dcache_nowrite)
XLEAF(r3k_clean_dcache_indexed)
	SIZE_CACHE(mips_dcache_size)

	/* n = MIN(dcachesize,n) */
	bltu	a1,v0,1f
	move	a1,v0
1:
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,SR_BEV
	mtc0	v1,$sr
	
	/* make sure write buffer is empty */
	la	v1,1b
	or	v1,KSEG1_BASE
	lw	zero, 0(v1)
	
	.set	noreorder

	/* r3000: isolate caches, no swap */
	or	v1,SR_ISC
	mtc0	v1,$sr
	
	/* ensure address is inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	addu	a1,a0		/* length -> ending address */
	
	/* unrolled loop: flush 32-bytes at a time */
	/* r3000: 4-byte cache lines */
1:	sb	zero,0(a0)
	sb	zero,4(a0)
	sb	zero,8(a0)
	sb	zero,12(a0)
	sb	zero,16(a0)
	sb	zero,20(a0)
	sb	zero,24(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sb	zero,-4(a0)
	
	/* isolated stores out of pipe */
	nop; nop; nop
	
	/* restore status reg (pipe clean) */
	mtc0	v0,$sr
	nop
	.set reorder
	
9:	j	ra
END(r3k_clean_dcache)

	
/*
 * void mips_clean_icache (unsigned kva, size_t n)
 * 
 * Invalidate address range in instruction cache
 */
LEAF(r3k_clean_icache)
XLEAF(r3k_clean_icache_indexed)
	SIZE_CACHE(mips_icache_size)

	/* n = MIN(icachesize, n) */
	bltu	a1,v0,1f
	move	a1,v0
1:	
	/* disable intrs */
	mfc0	v0,$sr
	and	v1,v0,SR_BEV
	mtc0	v1,$sr
	
	/* run uncached */
	la	v1,1f
	or	v1,KSEG1_BASE
	jr	v1
1:
	/* make sure write buffer is empty */
	lw	zero, 0(v1)

	.set	noreorder
	/* r3000: isolate caches and swap */
	or	v1,SR_ISC|SR_SWC
	mtc0	v1,$sr

	/* ensure address is inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	addu	a1,a0		/* length -> ending address */
	
	/* unrolled loop: flush 32-bytes at a time */
	/* r3000: 4-byte cache lines */
1:	sb	zero,0(a0)
	sb	zero,4(a0)
	sb	zero,8(a0)
	sb	zero,12(a0)
	sb	zero,16(a0)
	sb	zero,20(a0)
	sb	zero,24(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sb	zero,-4(a0)

	nop; nop; nop 			# pipe clean of stores 
	
	/* restore status reg (pipe clean) */
	mtc0	v0,$sr
	nop
	.set reorder
	
	/* restore cached mode */
9:	j	ra
END(r3k_clean_icache)

/* 
 * void mips_clean_cache (unsigned kva, size_t n)
 * 
 * Invalidate address range in all caches
 */
LEAF(r3k_clean_cache)
	move	t9,ra
	move	a2,a0
	move	a3,a1

	bal	r3k_clean_dcache

	move	a0,a2
	move	a1,a3
	bal	r3k_clean_icache
	
	move	ra,t9
	j	ra
END(r3k_clean_cache)


	
/* 
 * void mips_flush_cache ()
 * 
 * Invalidate all caches
 */
LEAF(r3k_flush_cache)
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	r3k_clean_dcache

9:	lw	a1,mips_icache_size
	li	a0,KSEG0_BASE
	blez	a1,9f
	bal	r3k_clean_icache
	
9:	move	ra,t9
	j	ra
END(r3k_flush_cache)
	
/* 
 * void mips_flush_dcache ()
 * 
 * Invalidate instruction caches
 */
LEAF(r3k_flush_dcache)
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	r3k_clean_dcache

9:	move	ra,t9
	j	ra
END(r3k_flush_dcache)
	
/* 
 * void mips_flush_icache ()
 * 
 * Invalidate instruction caches
 */
LEAF(r3k_flush_icache)
	move	t9,ra

	SIZE_CACHE(mips_icache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	r3k_clean_icache
	
9:	move	ra,t9
	j	ra
END(r3k_flush_icache)



#if #cpu(r3081)

#include <idt/r3081.h>
	
LEAF(r3081_cacheconfig)
	subu	sp,24
	sw	ra,16(sp)
	
	mfc0	t1,$config
	xor	t2,t1,a0
	and	t2,CFG_AC
	beqz	t2,9f		# return if config unchanged
	xor	t1,t2

	/* run uncached */
	la	v0,1f
	or	v0,KSEG1_BASE
	jr	v0

1:	/* set new configuration and reinitialise caches */
	mtc0	t1,$config
	bal	r3k_init_cache

9:	lw	ra,16(sp)
	addu	sp,24
	j	ra
END(r3081_cacheconfig)

#endif /* r3081 */

#endif /* #cache(r3k) */
