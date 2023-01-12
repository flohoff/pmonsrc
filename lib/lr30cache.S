/*
 * lr30cach.S: LR330x0 cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 */

#if #cache(lr30)

#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/r3kc0.h>
#include <lr33000.h>

IMPORT(mips_icache_size,4)
IMPORT(mips_icache_linesize,4)
IMPORT(mips_icache_ways,4)
	
IMPORT(mips_dcache_size,4)
IMPORT(mips_dcache_linesize,4)
IMPORT(mips_dcache_ways,4)
	
/* linesize is fixed at 16 bytes for all caches */	
#define LINESIZE	16

/*
 * void lr30_size_cache()
 * Initialise I and D caches and flush them, but only if size is unknown.
 */
LEAF(lr30_size_cache)
	lw	t0,mips_icache_size
	blez	t0,lr30_init_cache
	j	ra
END(lr30_size_cache)


/* register usage for _init_cache() routine */	
#define initaddr	t4
#define initsize	t6
		
/*
 * void lr30_init_cache()
 * Work out size of I and D caches and flush them.
 */
LEAF(lr30_init_cache)
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

	/* LSI LR330x0 caches: hard-wired sizes */

	or	t3,SR_CU1|SR_CU2	# enable coprocs 1 & 2
	mtc0	t3,$sr

	li	v0,1*1024		# lr33000: 1k d-cache
	li	v1,4*1024		# lr330x0: 4k i-cache
	li	t1,0xaaaa5555		# pattern

#if !#fpu(no)
	/* look for CP1 (lr33050) */
	mtc1	t1,$f0
	mtc1	zero,$f1
	mfc1	t2,$f0
	mfc1	t3,$f1
	bne	t1,t2,1f
	beq	zero,t3,2f
#endif

	/* look for CP2 (lr33020) */
1:	mtc2	t1,$28
	mtc2	zero,$29
	mfc2	t2,$28
	mfc2	t3,$29
	bne	t1,t2,1f
	beq	zero,t3,2f

	/* neither CP is present, it must be an lr33000 */
1:	li	v1,8*1024		# lr33000: 8k i-cache

	/*
 	 * Initialise d-cache
	 */
2:	mfc0	t0,$dcic
	li	t1,DCIC_D
	mtc0	t1,$dcic
	move	initsize,v0
	bal	_init_cache

	/*
 	 * Initialise i-cache
	 */
	li	t1,DCIC_I
	mtc0	t1,$dcic
	move	initsize,v1
	bal	_init_cache

	.set	noreorder
	mtc0	t0,$dcic		# restore $dcic
	mtc0	t7,$sr			# restore IE 
	nop; nop
	.set	reorder

	/* save results */
	sw	v0,mips_dcache_size
	sw	v1,mips_icache_size
	li	t0,LINESIZE
	sw	t0,mips_dcache_linesize
	sw	t0,mips_icache_linesize
	li	t0,1
	sw	t0,mips_dcache_ways
	sw	t0,mips_icache_ways

	move	ra,t8
	j	ra
END(lr30_init_cache)


/*
 * Internal func to initialise LR330x0 cache.  
 */
SLEAF(_init_cache)
	.set	noreorder
	li	initaddr,KSEG0_BASE
	addu	initsize,initaddr
1:	sb	zero,0(initaddr)
	addu	initaddr,32
	bne	initaddr,initsize,1b
	sb	zero,-16(initaddr)		# BDSLOT
	nop
	.set	reorder
	j	ra
SEND(_init_cache)


	
#if defined(IN_PMON) || defined(ITROM)
/* caches are always sized first */
#define SIZE_CACHE(which)		\
	lw	v0,which;		\
	blez	v0,9f
#else	
#define SIZE_CACHE(which)		\
	lw	v0,which;		\
	move	t5,ra;			\
	bgez	v0,9f;			\
	bal	lr30_init_cache;	\
	lw	v0,which;		\
	move	ra,t5;			\
9:	blez	v0,9f
#endif

/*
 * void lr30_clean_dcache (unsigned kva, size_t n)
 * 
 * Invalidate address range in data cache
 */
LEAF(lr30_clean_dcache)
XLEAF(lr30_clean_dcache_nowrite)
XLEAF(lr30_clean_dcache_indexed)
	SIZE_CACHE(mips_dcache_size)

	/* n = MIN(dcachesize,n) */
	bltu	a1,v0,1f
	move	a1,v0
1:
	/* ensure address inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	addu	a1,a0		/* length -> ending address */
	
	.set	noreorder
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,SR_BEV
	mtc0	v1,$sr

	/* lr330x0: dcache invalidate enable */
	mfc0	t0,$dcic
	li	t1,DCIC_D
	mtc0	t1,$dcic
	nop; nop
	
	/* unrolled loop: flush 32-bytes at a time */
	/* lr330x0: 16-byte cache lines */
1:	sb	zero,0(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sb	zero,-16(a0)
	
	/* isolated stores out of pipe */
	nop; nop; nop
	
	/* restore status reg (pipe clean) */
	mtc0	v0,$sr
	mtc0	t0,$dcic
	nop; nop
	.set reorder

9:	j	ra
END(lr30_clean_dcache)

	
/*
 * void lr30_clean_icache (unsigned kva, size_t n)
 * 
 * Invalidate address range in instruction cache
 */
LEAF(lr30_clean_icache)
XLEAF(lr30_clean_icache_indexed)
XLEAF(lr30_clean_icache_nowrite)
	SIZE_CACHE(mips_icache_size)

	/* n = MIN(icachesize, n) */
	bltu	a1,v0,1f
	move	a1,v0
1:
	/* ensure address inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	addu	a1,a0		/* length -> ending address */
	
	.set	noreorder
	/* run uncached */
	la	v1,1f
	or	v1,0xa0000000
	jr	v1
	nop
1:
	/* disable intrs */
	mfc0	v0,$sr
	and	v1,v0,SR_BEV
	mtc0	v1,$sr

	/* lr330x0: icache invalidate enable */
	mfc0	t0,$dcic
	li	t1,DCIC_I
	mtc0	t1,$dcic
	nop; nop

	/* unrolled loop: flush 32-bytes at a time */
	/* lr330x0: 16-byte cache lines */
1:	sb	zero,0(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sb	zero,-16(a0)

	nop; nop; nop			# pipe clean of stores 
	
	/* restore status reg (pipe clean) */
	mtc0	v0,$sr
	mtc0	t0,$dcic
	nop; nop
	.set reorder
	
	/* restore cached mode */
9:	j	ra
END(lr30_clean_icache)

/* 
 * void lr30_clean_cache (unsigned kva, size_t n)
 * 
 * Invalidate address range in all caches
 */
LEAF(lr30_clean_cache)
	move	t9,ra
	move	a2,a0
	move	a3,a1

	bal	lr30_clean_dcache

	move	a0,a2
	move	a1,a3
	bal	lr30_clean_icache
	
	move	ra,t9
	j	ra
END(lr30_clean_cache)

/* 
 * void lr30_flush_cache ()
 * 
 * Invalidate all caches
 */
LEAF(lr30_flush_cache)
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	lr30_clean_dcache

9:	lw	a1,mips_icache_size
	li	a0,KSEG0_BASE
	blez	a1,9f
	bal	lr30_clean_icache
	
9:	move	ra,t9
	j	ra
END(lr30_flush_cache)

/* 
 * void lr30_flush_dcache ()
 * 
 * Invalidate data cache
 */
LEAF(lr30_flush_dcache)
XLEAF(lr30_flush_dcache_nowrite)
XLEAF(lr30_flush_dcache_indexed)
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	lr30_clean_dcache

9:	move	ra,t9
	j	ra
END(lr30_flush_dcache)
	
/* 
 * void lr30_flush_icache ()
 * 
 * Invalidate instruction cache
 */
LEAF(lr30_flush_icache)
XLEAF(lr30_flush_icache_nowrite)
XLEAF(lr30_flush_icache_indexed)
	move	t9,ra

	SIZE_CACHE(mips_icache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	lr30_clean_icache

9:	move	ra,t9
	j	ra
END(lr30_flush_icache)

#endif #cache(lr30)
