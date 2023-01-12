/*
 * cwcache.sx: LSI CoreWare cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 */

#if #cache(cw)
	
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>
#include <mips/prid.h>
#include <sbd.h>	/* for _SBD_[ID]CACHE_WAYS options */
		
#if !#cpu(cw401x)
#define WBFLUSH(r) \
	lw	zero,(r) ; \
	addu	zero,1
#endif	

IMPORT(mips_icache_size,4)
IMPORT(mips_icache_linesize,4)
IMPORT(mips_icache_ways,4)
	
IMPORT(mips_dcache_size,4)
IMPORT(mips_dcache_linesize,4)
IMPORT(mips_dcache_ways,4)
	
	
/*
 * void cw_size_cache()
 * Determine sizes of I and D caches, without invalidating
 * dirty lines in cw401x write-back cache.
 */
LEAF(cw_size_cache)
	lw	t0,mips_icache_size
	blez	t0,_size_cache
	j	ra
END(cw_size_cache)


/*
 * void cw_init_cache()
 * Initialise I and D caches and flush them
 */
LEAF(cw_init_cache)
#if #cpu(cw401x)	
	move	t8,ra
	
	/* The _size_cache routine implicitly flushes the cw400x
	   write-through caches, but it doesn't touch the cw401x
	   write-back caches, so they require an explicit flush. */
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
	
	.set	noreorder
	flushid				# flush both caches
	nop; nop; nop
	.set	reorder
	
	mtc0	t7,$sr
	b	_size_cache_2
#else	
	b	_size_cache
#endif	
END(cw_init_cache)


/*
 * static void _size_cache()
 *
 * Work out size of I and D caches, but for the cw401x
 * write-back dcache don't modify it, it may contain
 * valid dirty lines.
 *	
 * Uses:	v0,v1,t0,t1,t2,t3,t4,t7,t8,ra
 * Save regs:	t3,t7,t8,v1
 * Calls:	_init_cache
 */
SLEAF(_size_cache)
	move	t8,ra

#if #cpu(cw401x)
_size_cache_2:		
	mfc0	t4,$ccc
	
	# instruction cache size
	and	t3,t4,CCC_IS
	srl	t3,14
	li	v1,1024
	sll	v1,t3
	sw	v1,mips_icache_size
	
	# data cache size
	and	t3,t4,CCC_DS
	srl	t3,10
	li	v0,1024
	sll	v0,t3
	sw	v0,mips_dcache_size
	
	/* get the cache line sizes & ways */
	li	v0,32
	sw	v0,mips_dcache_linesize	
	sw	v0,mips_icache_linesize
	
	/*
	 * the icache & dcache configuration is a core option
	 * so rely on the SBD code to tell us about it
	 */
#ifdef _SBD_ICACHE_WAYS
	li	v0,_SBD_ICACHE_WAYS
#else
	li	v0,2
#endif		
	sw	v0,mips_icache_ways
	
#ifdef _SBD_DCACHE_WAYS
	li	v0,_SBD_DCACHE_WAYS
#else
	li	v0,2
#endif		
	sw	v0,mips_dcache_ways
	
#else /* !#cpu(cw401x) */
	
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
	
	li	t0,BSYS_SCR
	lw	t4,(t0)
	
	/*
	 * Size and initialise instruction cache set 0
	 */
	li	t3,SCR_CM_ITAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_IE
	sw	t3,(t0)
	WBFLUSH(t0)
	
	bal	_init_cache
	move	v1,v0

	/*
	 * Size and initialise instruction cache set 1
	 */
	li	t3,SCR_CM_ITAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_1E|SCR_IE
	sw	t3,(t0)
	WBFLUSH(t0)
	
	bal	_init_cache
	addu	v1,v0
	
	/*
	 * Size and initialise data cache
	 */
	li	t3,SCR_CM_DTAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_IE
	sw	t3,(t0)
	WBFLUSH(t0)
	
	bal	_init_cache

	sw	t4,(t0)
	WBFLUSH(t0)
	
	.set	noreorder
	mtc0	t7,$sr			# restore IE and ISC to normal
	nop; nop; nop; nop; nop
	.set	reorder
	
	/* save results */
	sw	v0,mips_dcache_size
	sw	v1,mips_icache_size

	/* get the cache line sizes & ways */
	and	v1,t4,SCR_DRS_MASK
	srl	v1,SCR_DRS_SHIFT
	li	v0,1
	sll	v0,v1
	sll	v0,2
	sw	v0,mips_dcache_linesize

	and	v1,t4,SCR_IRS_MASK
	srl	v1,SCR_IRS_SHIFT
	li	v0,1
	sll	v0,v1
	sll	v0,2
	sw	v0,mips_icache_linesize

	li	v0,1
	sw	v0,mips_dcache_ways
	
	and	v1,t4,SCR_1E
	beqz	v1,1f
	li	v0,2
1:	sw	v0,mips_icache_ways
#endif	
	
	move	ra,t8
	j	ra
SEND(_size_cache)


#if !#cpu(cw401x)	
/*
 * Internal func to size+initialise cw400x cache; size returned in v0.
 * It works by writing address to address at all possible cache
 * boundaries (32k down to 1k).  Due to address wraparound,
 * whatever is found at cache location zero is the cache size.
 */

#define	MIN_CACHE_SIZE		(1024)
#define	MAX_CACHE_SIZE		(32*1024)

/*
 * int _init_cache()
 *
 * Returns size of one cache and flushes it.
 *	
 * Uses:	v0,t1,t2,ra
 * Save regs:	none
 * Calls:	nothing
 */
SLEAF(_init_cache)
	.set	noreorder
	li	t1,MIN_CACHE_SIZE
	li	t2,MAX_CACHE_SIZE
	
1:	sw	t2,KSEG0_BASE(t2)
	bne	t2,t1,1b
	srl	t2,1			# BDSLOT

	lw	v0,KSEG0_BASE(zero)	# get cache size

	/*
	 * Now flush the cache
 	 */
	li	t1,KSEG0_BASE
	addu	t2,t1,v0
1:	sw	zero,0(t1)		# invalidate
	sw	zero,16(t1)
	sw	zero,32(t1)
	addu	t1,64
	bne	t1,t2,1b
	sw	zero,-16(t1)		# BDSLOT
	
	nop; nop; nop			# pipe clear of stores
	.set	reorder
	j	ra
SEND(_init_cache)
#endif
	

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
	bal	_size_cache;		\
	lw	v0,which;		\
	move	ra,t5;			\
9:	blez	v0,9f
#endif

/*
 * void cw_clean_dcache (unsigned kva, size_t n)
 * 
 * Invalidate address range in data cache
 */
LEAF(cw_clean_dcache)
XLEAF(cw_clean_dcache_indexed)
#if !#cpu(cw401x)
XLEAF(cw_clean_dcache_nowrite)
#endif	
	SIZE_CACHE(mips_dcache_size)

	/* n = MIN(dcachesize,n) */
	bltu	a1,v0,1f
	move	a1,v0
1:
	/* ensure address is inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	
	addu	t3,a1,a0	/* length -> ending address */
	
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,SR_BEV
	mtc0	v1,$sr
	
	/* run uncached */
	la	v1,1f
	or	v1,KSEG1_BASE
	jr	v1
1:
#if #cpu(cw401x)
	/* note wb writes back both sets, i.e. 32 bytes at a time */
	move	t2,a0
	.set	noreorder
1:	wb	0(t2)
	addu	t2,64	
	bltu	t2,t3,1b
	wb	-32(t2)
	.set	reorder
#else
	li	v1,BSYS_SCR
	lw	t0,(v1)
	
	li	t1,SCR_CM_DTAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_IE
	sw	t1,(v1)
	WBFLUSH(v1)
	
	/* unrolled loop: flush 32-bytes at a time */
	/* cw400x: 16-byte cache lines */
	.set noreorder
1:	sw	zero,(a0)
	addu	a0,32
	bltu	a0,t3,1b
	sw	zero,-16(a0)	

	/* isolated stores out of pipe */
	nop; nop; nop
	.set reorder
	
	sw	t0,(v1)		# restore SCR
	WBFLUSH(v1)
#endif	
	
	/* restore status reg (pipe clean) */
	.set noreorder
	mtc0	v0,$sr
	nop
	.set reorder
	
#if #cpu(cw401x)
	/* now clear the cache lines */
	b	cw_clean_dcache_nowrite
#endif	
	
	/* restore cached mode */
9:	j	ra
END(cw_clean_dcache)
	
	
#if #cpu(cw401x)
/*
 * void cw_clean_dcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in data caches
 */
LEAF(cw_clean_dcache_nowrite)
	SIZE_CACHE(mips_dcache_size)

	/* disable interrupts */
	mfc0	t5,$sr
	and	v1,t5,SR_BEV
	mtc0	v1,$sr
	
	/* run uncached */
	la	v1,1f
	or	v1,KSEG1_BASE
	jr	v1
	
1:	/* n = MIN(dcachesize,n) */
	bltu	a1,v0,1f
	
	/* flush whole of dcache quickly */
	.set	noreorder
	flushd
	nop; nop; nop
	.set	reorder
	b	8f
	
1:	/* ensure address is inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
	addu	a1,a0		/* length -> ending address */
	
	mfc0	v1,$ccc
	and	t1,v1,~(CCC_IIE)
	/*
	 * what about CCC_DE1 & CCC_DE0 ?
	 * for now we assume they are set correctly
	 */
	or	t1,v1,CCC_DIE|CCC_INV
	.set	noreorder	
	mtc0	t1,$ccc
	nop; nop; nop
	
1:	sw	zero,0(a0)
	addu	a0,64	
	bltu	a0,a1,1b
	sw	zero,-32(a0)

	mtc0	v1,$ccc
	nop; nop; nop
	.set	reorder
	
	/* restore status reg (pipe clean) */
	.set noreorder
8:	mtc0	t5,$sr
	nop
	.set reorder
	
	/* restore cached mode */
9:	j	ra
END(cw_clean_dcache_nowrite)
#endif
		
/*
 * void cw_clean_icache (unsigned kva, size_t n)
 * 
 * Invalidate address range in instruction cache
 */
LEAF(cw_clean_icache)
XLEAF(cw_clean_icache_indexed)
XLEAF(cw_clean_icache_nowrite)
	SIZE_CACHE(mips_icache_size)

	/* disable intrs */
	mfc0	t5,$sr
	and	v1,t5,SR_BEV
	mtc0	v1,$sr
	
	/* run uncached */
	la	v1,1f
	or	v1,KSEG1_BASE
	jr	v1
1:
	
	/* n = MIN(icachesize, n) */
	bltu	a1,v0,1f
	
#if #cpu(cw401x)	
	/* flush whole icache quickly */
	.set	noreorder
	flushi
	nop; nop; nop
	.set	reorder
	b	8f
#else	
	move	a1,v0		# limit to cache size
#endif	
	
1:	/* ensure address is inside kseg0 */
	and	a0,~KSEG1_BASE
	or	a0,KSEG0_BASE
		
	addu	a1,a0		/* length -> ending address */
	move	t3,a0		/* save start address */
	
#if #cpu(cw401x)
	mfc0	v1,$ccc
	and	t1,v1,~(CCC_DIE)
	/*
	 * what about CCC_IE1 & CCC_IE0 ?
	 * for now we assume they are set correctly
	 */
	or	t1,v1,CCC_IIE|CCC_INV
	.set	noreorder	
	mtc0	t1,$ccc
	nop; nop; nop
	
1:	sw	zero,0(a0)
	addu	a0,64	
	bltu	a0,a1,1b
	sw	zero,-32(a0)

	mtc0	v1,$ccc
	nop; nop; nop
	.set	reorder
#else
	li	v1,BSYS_SCR
	lw	t0,(v1)
	
	/* select set 0 */	
	li	t1,SCR_CM_DTAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_IE
	sw	t1,(v1)
	WBFLUSH(v1)
		
	/* unrolled loop: flush 32-bytes at a time */
	/* cw400x: 16-byte cache lines */
	.set	noreorder
#ifdef _SBD_FLUSHNONLOCKED
1:	lw	t2,(a0)
	sll	t2,27
	bgezl	t2,2f
	sw	zero,(a0)
	
2:	lw	t2,16(a0)
	sll	t2,27
	bgezl	t2,2f
	sw	zero,16(a0)
	
2:	addu	a0,32
	bltu	a0,a1,1b
	nop
#else
1:	sw	zero,(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sw	zero,-16(a0)	
#endif

	/* select set 1 */
	li	t1,SCR_CM_DTAG|SCR_R|SCR_DRS_8|SCR_D|SCR_IRS_8|SCR_1E|SCR_IE
	sw	t1,(v1)
	WBFLUSH(v1)
		
	move	a0,t3
	
	/* unrolled loop: flush 32-bytes at a time */
	/* cw400x: 16-byte cache lines */
#ifdef _SBD_FLUSHNONLOCKED
1:	lw	t2,(a0)
	sll	t2,27
	bgezl	t2,2f
	sw	zero,(a0)
	
2:	lw	t2,16(a0)
	sll	t2,27
	bgezl	t2,2f
	sw	zero,16(a0)
	
	addu	a0,32
2:	bltu	a0,a1,1b
	nop
#else
1:	sw	zero,(a0)
	addu	a0,32
	bltu	a0,a1,1b
	sw	zero,-16(a0)	
#endif

	/* isolated stores out of pipe */
	nop; nop; nop
	.set reorder
		
	sw	t0,(v1)		# restore SCR
	WBFLUSH(v1)
#endif
		
	/* restore status reg (pipe clean) */
	.set noreorder
8:	mtc0	t5,$sr
	nop
	.set reorder
	
	/* restore cached mode */
9:	j	ra
END(cw_clean_icache)

/* 
 * void cw_clean_cache (unsigned kva, size_t n)
 * 
 * Writeback and invalidate address range in all caches
 */
LEAF(cw_clean_cache)
	move	t9,ra
	move	a2,a0
	move	a3,a1

	bal	cw_clean_dcache

	move	a0,a2
	move	a1,a3
	bal	cw_clean_icache
	
	move	ra,t9
	j	ra
END(cw_clean_cache)


/* 
 * void cw_flush_cache ()
 * 
 * Writeback and invalidate all caches
 */
LEAF(cw_flush_cache)
#if !#cpu(cw401x)
XLEAF(cw_flush_cache_nowrite)
#endif	
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	cw_clean_dcache

9:	lw	a1,mips_icache_size
	li	a0,KSEG0_BASE
	blez	a1,9f
	bal	cw_clean_icache
	
9:	move	ra,t9
	j	ra
END(cw_flush_cache)
	

#if #cpu(cw401x) && defined(_SDE_CACHE_EXTRAS)
/* 
 * void cw_flush_cache_nowrite ()
 * 
 * Invalidate but don't writeback all caches (probably only
 * sensible for cache diagnostics).
 */
LEAF(cw_flush_cache_nowrite)
	/* disable intrs */
	mfc0	t5,$sr
	and	v1,t5,SR_BEV
	mtc0	v1,$sr
	
	/* run uncached */
	la	v1,1f
	or	v1,KSEG1_BASE
	jr	v1
1:
	.set	noreorder
	flushid
	nop; nop; nop
	.set	reorder
	
	mtc0	t5,$sr
	j	ra
END(cw_flush_cache_nowrite)
#endif
	
	
/*
 * void cw_flush_dcache (void)
 *
 * Writeback and invalidate data cache only
 */
LEAF(cw_flush_dcache)
	move	t9,ra

	SIZE_CACHE(mips_dcache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	cw_clean_dcache

9:	move	ra,t9
	j	ra
END(cw_flush_dcache)
	

/*
 * void cw_flush_icache (void)
 *
 * Invalidate instruction cache only
 */
LEAF(cw_flush_icache)
	move	t9,ra

	SIZE_CACHE(mips_icache_size)
	li	a0,KSEG0_BASE
	move	a1,v0
	bal	cw_clean_icache

9:	move	ra,t9
	j	ra
END(cw_flush_icache)

#endif /* #cache(cw) */
