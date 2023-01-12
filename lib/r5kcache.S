/*
 * r5kcache.s: R5000 cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 */

#if #cache(r5k)
	
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/r5kc0.h>

#define NO	0
#define YES	1
#define MAYBE	2
	
#ifndef R5KSCACHE
#if #cpu(rm5230) || #cpu(rm5260)
#define R5KSCACHE NO
#else
#define R5KSCACHE MAYBE
#endif
#endif

/*
 * R5000 cache operations.
 *
 * The _flush and _clean functions are complex composites that do whatever
 * is necessary to flush/clean ALL caches, in the quickest possible way.
 * The other functions are targetted explicitly at a particular cache
 * I, D or SD; it is up to the user to call the correct set of functions
 * for a given system.
 *
 * NOTE:	
 *
 * The secondary cache code assumes that all addresses presented to these
 * functions are in kseg0, so that the scache index matches the virtual
 * address; if a mapped virtual address is presented then it will not
 * be translated and the wrong part of the scache will be invalidated!
 */

IMPORT(mips_icache_size,4)
IMPORT(mips_icache_linesize,4)
IMPORT(mips_icache_ways,4)
	
IMPORT(mips_dcache_size,4)
IMPORT(mips_dcache_linesize,4)
IMPORT(mips_dcache_ways,4)
	
IMPORT(mips_scache_size,4)
IMPORT(mips_scache_linesize,4)
IMPORT(mips_scache_ways,4)
IMPORT(mips_scache_split,4)
IMPORT(mips_scache_discontig,4)
	
BSS(r5k_scache_tagmask,4)

/* linesize is fixed at 32 bytes for all caches */	
#define LINESIZE	32
	
/*
 * Macros to automate cache operations
 */

#define addr	t0
#define maxaddr	t1
#define mask	t2

#define cacheop(kva, n, op)	\
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

/* virtual cache op: no limit on size of region */
#define vcacheop(kva, n, op)	\
	cacheop(kva, n, op)

/* indexed cache op: region limited to cache size */
#define icacheop(kva, n, size, op) \
	move	t3,n;			\
	bltu	n,size,12f ;		\
	move	t3,size ;		\
12:	cacheop(kva, t3, op)
	


/*
 * static void _size_cache()
 * 
 * Internal routine to determine cache sizes by looking at R5000 config
 * register.  Sizes are returned in registers, as follows:
 */

#define icachesize	t2
#define dcachesize	t3
#define scachesize	t4
#define stagmask	t5

SLEAF(_size_cache)
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

	move	scachesize,zero
	
#if R5KSCACHE
	/* no secondary cache if Config.SC == 1 */
	and	t1,t0,CFG_SC
	bnez	t1,9f
	
	/* also no secondary cache if Config.SS == NONE */
	and	t1,t0,CFG_SSMASK
	beq	t1,CFG_SS_NONE,9f
	
	/* calculate secondary cache size */
	srl	t1,CFG_SSSHIFT
	li	scachesize,512*1024
	sll	scachesize,t1
	
	/* calculate valid secondary cache tag bits */
	li	stagmask,TAG_STAG_MASK
	sll	stagmask,t1
#endif
	
9:	j	ra
SEND(_size_cache)


/*
 * void size_cache()
 * 
 * Work out size of I, D & S caches
 */
LEAF(r5k_size_cache)
	lw	t0,mips_icache_size
	move	v0,ra
	bgtz	t0,8f				# already known?
	bal	_size_cache
	move	ra,v0
	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	sw	scachesize,mips_scache_size
	sw	stagmask,r5k_scache_tagmask
	li	t0,LINESIZE
	sw	t0,mips_icache_linesize
	sw	t0,mips_dcache_linesize
	sw	t0,mips_scache_linesize
	li	t0,2
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
	li	t0,1
	sw	t0,mips_scache_ways
8:	j	ra
END(r5k_size_cache)

/*
 * void r5k_init_cache()
 * 
 * Work out size of and initialize I, D & S caches.
 *
 * NOTES
 *  1) assumes enough DRAM has been initialised with correct parity
 */
LEAF(r5k_init_cache)
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
	addu	a1,a0,icachesize		# limit = base + icachesize 
1:	addu	a0,LINESIZE
	cache	Index_Store_Tag_I,-4(a0)	# clear tag
	nop
	cache	Fill_I,-4(a0)			# fill data line
	nop
	bne	a0,a1,1b
	cache	Index_Store_Tag_I,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* 
	 * Initialise primary data cache.
	 * (for 2-way set caches, we do it in 3 passes).
	 */

	/* 1: initialise dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,LINESIZE
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* 2: fill dcache data */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize		# limit = base + dcachesize 
1:	addu	a0,LINESIZE
	bne	a0,a1,1b
	lw	zero,-4(a0)			# BDSLOT: fill line
	.set	reorder

	/* 3: clear dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,LINESIZE
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* 
	 * Initialise secondary cache tags (if present).
	 */
	blez	scachesize,3f			# scache present?
	
	/* enable secondary cache */
	or	t0,CFG_SE
	.set	noreorder		
	mtc0	t0,$config
	nop; nop; nop; nop
	.set	reorder
	
	li	a0,KSEG0_BASE
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear all secondary cache tags */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* clear one page at a time */
	addu	a1,a0,scachesize		# limit
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE
	bne	a0,a1,1b
#endif		
	
	/* Since the CPU won't look at the secondary cache data
	   unless it gets a tag match, the initial state
	   of the data parity doesn't matter. */

	/* we store the sizes only after the caches are initialised */
3:	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	sw	scachesize,mips_scache_size
	sw	stagmask,r5k_scache_tagmask
	li	t0,LINESIZE
	sw	t0,mips_icache_linesize
	sw	t0,mips_dcache_linesize
	sw	t0,mips_scache_linesize
	li	t0,2
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
	li	t0,1
	sw	t0,mips_scache_ways
	mtc0	v0,$sr
	j	ra
END(r5k_init_cache)
	

	
#if defined(IN_PMON) || defined(ITROM)
/* caches are always sized first */
#define SIZE_CACHE(reg,which)		\
	lw	reg,which;		\
	blez	reg,9f
#else	
/* caches may not have been sized yet */	
#define SIZE_CACHE(reg,which)		\
	lw	reg,which;		\
	move	v1,ra;			\
	bgez	reg,9f;			\
	bal	r5k_size_cache;		\
	lw	reg,which;		\
	move	ra,v1;			\
9:	blez	reg,9f
#endif

/*
 * void r5k_flush_cache (void)
 *
 * Flush and invalidate all caches
 */
LEAF(r5k_flush_cache)
	SIZE_CACHE(a1,mips_dcache_size)
	
	/* flush primary caches */
	li	a0,KSEG0_BASE
	cacheop(a0,a1,Index_Writeback_Inv_D)

	lw	a1,mips_icache_size
	cacheop(a0,a1,Index_Invalidate_I)

#if R5KSCACHE	
	lw	a1,mips_scache_size
	mtc0	zero,$taglo
	blez	a1,9f	
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear all secondary cache tags */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* clear one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE			# += page size
	bne	a0,a1,1b
#endif		
#endif
9:	j	ra
END(r5k_flush_cache)

	
/*
 * void r5k_flush_dcache (void)
 *
 * Flush and invalidate data caches
 */
LEAF(r5k_flush_dcache)
	SIZE_CACHE(a1,mips_dcache_size)
	li	a0,KSEG0_BASE
	cacheop(a0,a1,Index_Writeback_Inv_D)
#if R5KSCACHE	
	lw	a1,mips_scache_size
	mtc0	zero,$taglo
	blez	a1,9f	
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear all secondary cache tags */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* clear one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE			# += page size
	bne	a0,a1,1b
#endif		
#endif
9:	j	ra
END(r5k_flush_dcache)
	

/*
 * void r5k_flush_dcache (void)
 *
 * Flush and invalidate instruction caches
 */
LEAF(r5k_flush_icache)
XLEAF(r5k_flush_icache_nowrite)
	SIZE_CACHE(a1,mips_icache_size)
	li	a0,KSEG0_BASE
	cacheop(a0,a1,Index_Invalidate_I)
#if R5KSCACHE	
	lw	a1,mips_scache_size
	mtc0	zero,$taglo
	blez	a1,9f	
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear all secondary cache tags */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* clear one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE			# += page size
	bne	a0,a1,1b
#endif		
#endif
9:	j	ra
END(r5k_flush_icache)


	
/*
 * void mips_clean_cache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(r5k_clean_cache)
	SIZE_CACHE(a3,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
	vcacheop(a0,a1,Hit_Invalidate_I)
#if R5KSCACHE	
	lw	a3,mips_scache_size
	mtc0	zero,$taglo
	blez	a3,9f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a3,Index_Store_Tag_S)
#endif
9:	j	ra
END(r5k_clean_cache)


/*
 * void mips_clean_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in data caches
 */
LEAF(r5k_clean_dcache)
	SIZE_CACHE(a3,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
#if R5KSCACHE	
	lw	a3,mips_scache_size
	mtc0	zero,$taglo
	blez	a3,9f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a3,Index_Store_Tag_S)
#endif
9:	j	ra
END(r5k_clean_dcache)

	
/*
 * void mips_clean_icache (unsigned kva, size_t n)
 *
 * (Writeback and) invalidate address range in instruction caches
 */
LEAF(r5k_clean_icache)
XLEAF(r5k_clean_icache_nowrite)
	SIZE_CACHE(a3,mips_icache_size)
	vcacheop(a0,a1,Hit_Invalidate_I)
#if R5KSCACHE	
	lw	a3,mips_scache_size
	mtc0	zero,$taglo
	blez	a3,9f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a3,Index_Store_Tag_S)
#endif
9:	j	ra
END(r5k_clean_icache)
	

	
/* 
 * Cache locking
 *
 * Note that while the QED RM52XX family does provide cache
 * locking, it is only on a per-set basis, not line-by-line.  
 *
 * Currently the only R5000 class CPUs with proper line-by-line cache 
 * locking are the NEC Vr5400 & QED RM7000 (see rm7kcache & r54cache
 * modules).
 *
 * WARNING: if you lock any cache lines, then don't call the 
 * mips_flush_xcache routines, because these will flush the 
 * locked data out of the cache too; use only mips_clean_xcache.
 */	
	
#if #cpu(rm52xx)
#define SR_IL		0x00800000	/* icache lock */
#define SR_DL		0x01000000	/* dcache lock */
#endif
	
	
/*
 * void r5k_lock_dcache (void *data, size_t n)
 *
 * Load and lock a block of date into the d-cache
 */
LEAF(r5k_lock_dcache)
#ifdef SR_DL
	SIZE_CACHE(t4,mips_dcache_size)
	
	/* disable interrupts and enable dcache locking */
	mfc0	v0,$sr
	or	v0,SR_DL		# leave enabled afterwards
	and	v1,v0,~SR_IE
	mtc0	v1,$sr
	
	srl	t4,1			# set size = cache size / 2
	
	/* calculate end address */
	addu	a3,a0,a1		# maxaddr = data + n - 1
	subu	a3,1
 	blez	a1,11f			# check for bad size
	
	/* align start and end to line boundaries */
	li	t0,~(LINESIZE-1)	# mask = ~(linesize - 1)
	and	a0,t0			# align start
	and	a3,t0			# align end
	
	/* the main loop (one line at a time) */
	.set	noreorder
	/* clear out both matching sets, to guarantee that the refill
	   will be into set A, which is now "locked" */
10: 	cache	Index_Writeback_Inv_D,0(a0)	# clear set A
	addu	t0,a0,t4			# next set
 	cache	Index_Writeback_Inv_D,0(t0)	# clear set B
	nop
	lw	zero,0(a0)
	bne     a0,a3,10b
	addu   	a0,LINESIZE
	.set	reorder
			
	/* restore the status register */
	.set noreorder
11:	mtc0	v0,$sr
	nop; nop; nop
	.set reorder
#endif /* SR_DL */
	
9:	j	ra
END(r5k_lock_dcache)
	
	
/*
 * void r5k_lock_icache (void *code, size_t n)
 *
 * Load and lock a block of instructions into the i-cache
 */
LEAF(r5k_lock_icache)
#ifdef SR_IL
	SIZE_CACHE(t4,mips_icache_size)
	
	/* run uncached */
	la	t1,1f
	or	t1,KSEG1_BASE
	jr	t1
	
	/* disable interrupts and enable icache locking */
1:	mfc0	v0,$sr
	or	v0,SR_IL		# leave enabled afterwards
	and	v1,v0,~SR_IE
	mtc0	v1,$sr
	
	srl	t4,1			# set size = cache size / 2
	
	/* calculate end address */
	addu	a3,a0,a1		# maxaddr = data + n - 1
	subu	a3,1
 	blez	a1,11f			# check for bad size
	
	/* align start and end to line boundaries */
	li	t0,~(LINESIZE-1)	# mask = ~(linesize - 1)
	and	a0,t0			# align start
	and	a3,t0			# align end
	
	/* the main loop (one line at a time) */
	.set	noreorder
	/* clear out both matching sets, to guarantee that the fill
	   will be into set A, which is now "locked" */
10: 	cache	Index_Invalidate_I,0(a0)	# clear set A
	addu	t0,a0,t4			# next set
 	cache	Index_Invalidate_I,0(t0)	# clear set B
	nop
	cache	Fill_I,0(a0)
	bne     a0,a3,10b
	addu   	a0,LINESIZE
	.set	reorder
			
	/* restore the status register */
	.set noreorder
11:	mtc0	v0,$sr
	nop; nop; nop
	.set reorder
#endif /* SR_IL */
	
9:	j	ra
END(r5k_lock_icache)
	
	
LEAF(r5k_lock_scache)
	j	ra
END(r5k_lock_scache)
	


/*
 * The following functions operate on individual cache levels, or use
 * indexed addressing, so they are probably only useful for cache 
 * diagnostics or possibly virtual memory operating systems.
 */
	
#if defined(_SDE_CACHE_EXTRA) || defined(ITROM)
	
LEAF(r5k_flush_cache_nowrite)
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
		/* else flush primary caches individually */
	li	a0,KSEG0_BASE
	cacheop(a0,a1,Index_Store_Tag_D)

	lw	a1,mips_icache_size
	cacheop(a0,a1,Index_Store_Tag_I)

#ifdef R5KSCACHE
	lw	a1,mips_scache_size
	blez	a1,9f	
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear whole of secondary cache */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* invalidate one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE			# += page size
	bne	a0,a1,1b
#endif		
#endif
		
9:	.set noreorder
	nop; nop; nop;
	mtc0	a3,$sr
	nop; nop; nop;
	.set reorder
	j	ra
END(r5k_flush_cache_nowrite)
	

LEAF(r5k_flush_dcache_nowrite)
	SIZE_CACHE(a1,mips_dcache_size)
	
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

	li	a0,KSEG0_BASE
	cacheop(a0,a1,Index_Store_Tag_D)
	
	.set noreorder
	nop; nop; nop;
	mtc0	a3,$sr
	nop; nop; nop;
	.set reorder
	
9:	j	ra
END(r5k_flush_dcache_nowrite)
	

LEAF(r5k_flush_scache)
#ifdef R5KSCACHE	
	SIZE_CACHE(a1,mips_scache_size)
	mtc0	zero,$taglo
	li	a0,KSEG0_BASE
#ifdef R5KSCACHE_FLASH_CLEAR
	/* flash clear whole of secondary cache */
	cache	Flash_Invalidate_S,0(a0)
#else	
	/* invalidate one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_S,0(a0)
	addu	a0,128*LINESIZE			# += page size
	bne	a0,a1,1b
#endif		
#endif
9:	j	ra
END(r5k_flush_scache)
	

	
/*
 * void mips_clean_dcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in data caches
 */
LEAF(r5k_clean_dcache_nowrite)
	SIZE_CACHE(a3,mips_dcache_size)
	vcacheop(a0,a1,Hit_Invalidate_D)
9:	j	ra
END(r5k_clean_dcache_nowrite)
	
/*
 * void mips_clean_dcache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(r5k_clean_dcache_indexed)
	SIZE_CACHE(a3,mips_dcache_size)
	
	/* Handle two-way set primaries */
	srl	a3,1			# do one set (half cache) at a time
	icacheop(a0,a1,a3,Index_Writeback_Inv_D)
	addu	a0,a3			# do next set
	icacheop(a0,a1,a3,Index_Writeback_Inv_D)
	
9:	j	ra
END(r5k_clean_dcache_indexed)
	
	
/*
 * void r5k_hit_writeback_inv_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(r5k_hit_writeback_inv_dcache)
	SIZE_CACHE(a3,mips_dcache_size)
	vcacheop(a0,a1,Hit_Writeback_Inv_D)
9:	j	ra
END(r5k_hit_writeback_inv_dcache)
	
	
/*
 * void mips_clean_icache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary instruction cache
 */
LEAF(r5k_clean_icache_indexed)
	SIZE_CACHE(a3,mips_icache_size)

	/* Handle two-way set primaries */
	srl	a3,1			# do one set (half cache) at a time
	icacheop(a0,a1,a2,Index_Invalidate_I)
	addu	a0,a3			# do next set
	icacheop(a0,a1,a2,Index_Invalidate_I)

9:	j	ra
END(r5k_clean_icache_indexed)
	
	
/*
 * void mips_clean_scache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(r5k_clean_scache)
XLEAF(r5k_clean_scache_nowrite)
#ifdef R5KSCACHE	
	SIZE_CACHE(a3,mips_scache_size)
	mtc0	zero,$taglo
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a3,Index_Store_Tag_S)
#endif	
9:	j	ra
END(r5k_clean_scache)
	
#endif /* _SDE_CACHE_DIAGS */
	
#endif /* #cache(r5k) */
