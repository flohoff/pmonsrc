/*
 * r54cache.s: Vr5400 cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 *	
 * XXX Completely untested and therefore probably wrong!!
 */

#if #cache(r5400)
		
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/r5kc0.h>

/*
 * R5400 cache operations.
 *
 * The _flush and _clean functions are complex composites that do whatever
 * is necessary to flush/clean ALL caches, in the quickest possible way.
 * The other functions are targetted explicitly at a particular cache
 * I or D; it is up to the user to call the correct set of functions
 * for a given system.
 *
 * NOTE:	
 * This code is untested - we haven't seen real silicon yet.
 */

IMPORT(mips_icache_size,4)
IMPORT(mips_icache_linesize,4)
IMPORT(mips_icache_ways,4)
	
IMPORT(mips_dcache_size,4)
IMPORT(mips_dcache_linesize,4)
IMPORT(mips_dcache_ways,4)
	
/* linesize is fixed at 32 bytes for all caches */	
#define LINESIZE	32
	
/*
 * Macros to automate cache operations
 */

#define addr	t0
#define maxaddr	t1
#define mask	t2

/* virtual cache op: no limit on size of region */
#define vcacheop(kva, n, op)	\
	.set	noreorder ;		\
	/* check for bad size */	\
 	blez	n,11f ;			\
	addu	maxaddr,kva,n ;		\
	/* align to line boundaries */	\
	li	mask,~(LINESIZE-1) ;	\
	and	addr,kva,mask ;		\
	addu	maxaddr,-1 ;		\
	and	maxaddr,mask ;		\
	/* the cacheop loop */		\
10: 	cache	op,0(addr) ;	 	\
	bne     addr,maxaddr,10b ;	\
	addu   	addr,LINESIZE ;		\
11:	.set	reorder

/* indexed cache op: no limit on size of region */
/* NOTE both sets are operated on */
#define icacheop(kva, n, op)	\
	.set	noreorder ;		\
	/* check for bad size */	\
 	blez	n,11f ;			\
	addu	maxaddr,kva,n ;		\
	/* align to line boundaries */	\
	li	mask,~(LINESIZE-1) ;	\
	and	addr,kva,mask ;		\
	addu	maxaddr,-1 ;		\
	and	maxaddr,mask ;		\
	/* the cacheop loop */		\
10: 	cache	op,0(addr) ;	 	\
	nop;				\
	cache	op,1(addr);		\
	bne     addr,maxaddr,10b ;	\
	addu   	addr,LINESIZE ;		\
11:	.set	reorder

/* indexed cache op: region limited to cache size */
/* NOTE both sets are operated on;
	real cache size should be divided by 2 */
#define icacheop_lim(kva, n, cachesize, op) \
	move	t3,n;			\
	bltu	n,cachesize,12f ;	\
	move	t3,cachesize ;		\
12:	icacheop(kva,t3,op)
	


/*
 * static void _size_cache()
 * 
 * Internal routine to determine cache sizes by looking at R5400 config
 * register.  Sizes are returned in registers, as follows:
 */

#define icachesize	t2
#define dcachesize	t3

SLEAF(_size_cache)
	sync
	mfc0	t0,$config

	/* work out primary i-cache size */
	and	t1,t0,CFG_ICMASK
	srl	t1,CFG_ICSHIFT
	li	icachesize,0x1000
	sll	icachesize,t1

	/* work out primary d-cache size */
	and	t1,t0,CFG_DCMASK
	srl	t1,CFG_DCSHIFT
	li	dcachesize,0x1000
	sll	dcachesize,t1

	j	ra
SEND(_size_cache)


/*
 * void size_cache()
 * 
 * Work out size of I, D & S caches
 */
LEAF(r54_size_cache)
	lw	t0,mips_icache_size
	move	v0,ra
	bgtz	t0,8f				# already known?
	bal	_size_cache
	move	ra,v0
	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	li	t0,LINESIZE
	sw	t0,mips_icache_linesize
	sw	t0,mips_dcache_linesize
	li	t0,2
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
8:	j	ra
END(r54_size_cache)

/*
 * void r54_init_cache()
 * 
 * Work out size of and initialise I & D caches.
 *
 * NOTES
 *  1) assumes enough DRAM has been initialised with correct parity
 */
LEAF(r54_init_cache)
	/*
 	 * Determine the cache sizes
	 */
	move	v0,ra
	bal	_size_cache
	
	/* Run uncached (PIC) */
	.set	noreorder
	.set	nomacro
	bal	1f
	li	t1,KSEG1_BASE
1:	or	t1,ra
	addu	t1,16
	jr	t1
	move	ra,v0
	.set	macro
	.set	reorder

	/*
	 * The caches may be in an indeterminate state,
	 * so we force good parity into them by doing an
	 * invalidate, load/fill, invalidate for each line.
	 */

	/* disable all i/u and cache exceptions */
	mfc0	v0,$sr
	li	a0,~SR_IE
	and	a0,v0
	or	a0,SR_DE
	
	.set noreorder
	mtc0	a0,$sr
	nop
	
	/* disable secondary cache and set zero tag */
	mfc0	t0,$config
	nop
	mtc0	zero,$taglo
	and	t0,~CFG_SE
	mtc0	t0,$config
	nop; nop; nop; nop
	.set	reorder
	
	/* 
	 * Assume bottom of RAM will generate good parity for the 
	 * primary caches (max 32K)
	 */

	/* 
	 * Initialise primary instruction cache.
	 */
	.set	noreorder
	li	a0,KSEG0_BASE
	srl	t1,icachesize,1
	addu	a1,a0,t1			# limit = base + icachesize/2 
1:	addu	a0,LINESIZE
	cache	Index_Store_Tag_I,-4(a0)	# clear tag (set #0)
	nop
	cache	Index_Store_Tag_I,-3(a0)	# clear tag (set #1)
	nop
	cache	Fill_I,-4(a0)			# fill data line (set #0)
	nop
	cache	Fill_I,-3(a0)			# fill data line (set #1)
	nop
	cache	Index_Store_Tag_I,-4(a0)	# clear tag (set #0)
	bne	a0,a1,1b
	cache	Index_Store_Tag_I,-3(a0)	# BDSLOT: clear tag (set #1)
	.set	reorder
	
	/* 
	 * Initialise primary data cache.
	 * (for 2-way set caches, we do it in 3 passes).
	 */

	/* 1: initialise dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	srl	t1,dcachesize,1
	addu	a1,a0,t1	        	# limit = base + dcachesize/2
1:	addu	a0,LINESIZE
	cache	Index_Store_Tag_D,-4(a0)	# clear tag (set #0)
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-3(a0)	# BDSLOT: clear tag (set #1)
	.set	reorder
	
	sync

	/* 2: fill dcache data */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize		# limit = base + dcachesize 
1:	addu	a0,LINESIZE
	bne	a0,a1,1b
	lw	zero,-4(a0)			# BDSLOT: fill line
	.set	reorder
	
	sync

	/* 3: clear dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	srl	t1,dcachesize,1
	addu	a1,a0,t1	        	# limit = base + dcachesize/2
1:	addu	a0,LINESIZE
	cache	Index_Store_Tag_D,-4(a0)	# clear tag (set #0)
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-3(a0)	# BDSLOT: clear tag (set #1)
	.set	reorder

	/* we store the sizes only after the caches are initialised */
3:	sync
	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	li	t0,LINESIZE
	sw	t0,mips_icache_linesize
	sw	t0,mips_dcache_linesize
	li	t0,2
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways

	mtc0	v0,$sr
	j	ra
END(r54_init_cache)

#if defined(IN_PMON) || defined(ITROM)
/* caches are always sized first */
#define SIZE_CACHE(reg,which)		\
	lw	reg,which;		\
	blez	reg,9f
#else	
#define SIZE_CACHE(reg,which)		\
	sync;				\
	li	a2,LINESIZE;		\
	lw	reg,which;		\
	move	v1,ra;			\
	bgez	reg,9f;			\
	bal	r54_size_cache;		\
	lw	reg,which;		\
	move	ra,v1;			\
9:	blez	reg,9f
#endif
	

/*
 * void r54_flush_cache (void)
 *
 * Flush and invalidate all caches
 */
LEAF(r54_flush_cache)
	SIZE_CACHE(a1,mips_dcache_size)

	li	a0,KSEG0_BASE
	srl	a1,1
	icacheop(a0,a1,Index_Writeback_Inv_D)

	lw	a1,mips_icache_size
	srl	a1,1
	icacheop(a0,a1,Index_Invalidate_I)

9:	j	ra
END(r54_flush_cache)

	
/*
 * void r54_flush_dcache (void)
 *
 * Flush and invalidate data cache only
 */
LEAF(r54_flush_dcache)
	SIZE_CACHE(a1,mips_dcache_size)
	li	a0,KSEG0_BASE
	srl	a1,1
	icacheop(a0,a1,Index_Writeback_Inv_D)
9:	j	ra
END(r54_flush_dcache)

	
/*
 * void r54_flush_icache (void)
 *
 * Flush and invalidate instruction cache only
 */
LEAF(r54_flush_icache)
XLEAF(r54_flush_icache_nowrite)
	SIZE_CACHE(a1,mips_icache_size)
	li	a0,KSEG0_BASE
	srl	a1,1
	icacheop(a0,a1,Index_Invalidate_I)
9:	j	ra
END(r54_flush_icache)


	
/*
 * void r54_clean_cache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(r54_clean_cache)
	SIZE_CACHE(a2,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
	vcacheop(a0,a1,Hit_Invalidate_I)
9:	j	ra
END(r54_clean_cache)


/*
 * void r54_clean_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in data caches
 */
LEAF(r54_clean_dcache)
	SIZE_CACHE(a2,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
9:	j	ra
END(r54_clean_dcache)

	
/*
 * void r54_clean_icache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in instruction caches
 */
LEAF(r54_clean_icache)
XLEAF(r54_clean_icache_nowrite)
	SIZE_CACHE(a2,mips_icache_size)
	vcacheop(a0,a1,Hit_Invalidate_I)
9:	j	ra
END(r54_clean_icache)
	


/*
 * The following functions operate on individual cache levels, or use
 * indexed addressing, so they are probably only useful for cache 
 * diagnostics or possibly virtual memory operating systems.
 */
	
#if defined(_SDE_CACHE_EXTRA) || defined(ITROM)
	
LEAF(r54_flush_cache_nowrite)
	/* disable all i/u and cache exceptions */
	mfc0	a3,$sr
	li	a0,~SR_IE
	and	a0,a3
	or	a0,SR_DE
	.set noreorder
	mtc0	a0,$sr
	nop; nop; nop
	mtc0	zero,$taglo			# initial cache tag 
	nop
	.set reorder

	SIZE_CACHE(a1,mips_dcache_size)
	li	a0,KSEG0_BASE
	srl	a1,1
	icacheop(a0,a1,Index_Store_Tag_D)

	lw	a1,mips_icache_size
	srl	a1,1
	icacheop(a0,a1,Index_Invalidate_I)

9:	.set noreorder
	nop; nop; nop;
	mtc0	a3,$sr
	nop; nop; nop;
	.set reorder
	j	ra
END(r54_flush_cache_nowrite)


/*
 * void r54_clean_dcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in data caches
 */
LEAF(r54_clean_dcache_nowrite)
	SIZE_CACHE(a2,mips_dcache_size)
	vcacheop(a0,a1,Hit_Invalidate_D)
9:	j	ra
END(r54_clean_dcache_nowrite)
	

/*
 * void r5k_hit_writeback_inv_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(r54_hit_writeback_inv_dcache)
	SIZE_CACHE(a3,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
9:	j	ra
END(r54_hit_writeback_inv_dcache)
	

/*
 * void r54_clean_dcache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(r54_clean_dcache_indexed)
	SIZE_CACHE(a3,mips_dcache_size)
	srl	a3,1
	icacheop_lim(a0,a1,a3,Index_Writeback_Inv_D)
9:	j	ra
END(r54_clean_dcache_indexed)
	
	
/*
 * void r54_clean_icache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary instruction cache
 */
LEAF(r54_clean_icache_indexed)
	SIZE_CACHE(a3,mips_icache_size)
	srl	a3,1
	icacheop_lim(a0,a1,a3,Index_Invalidate_I)
9:	j	ra
END(r54_clean_icache_indexed)
	
#endif /* _SDE_CACHE_DIAG */
		
#endif /* #cache(r5400) */
