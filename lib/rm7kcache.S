/*
 * rm7kcache.sx: QED RM7000 cache support functions for SDE-MIPS
 *
 * Copyright (c) 1998-1999 Algorithmics Ltd - all rights reserved.
 * 
 * This program is NOT free software, it is supplied under the terms
 * of the SDE-MIPS License Agreement, a copy of which is available at:
 *
 *  http://www.algor.co.uk/algor/info/sde-license.pdf
 *
 * Any company which has obtained and signed a valid SDE-MIPS license
 * may use and modify this software internally and use (without
 * restrictions) any derived binary.  You may not, however,
 * redistribute this in whole or in part as source code, nor may you
 * modify or remove any part of this copyright message.
 */

#if #cache(rm7k)
	
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/rm7kc0.h>

#define NO	0
#define YES	1
#define MAYBE	2
	
#ifndef RM7KTCACHE
#define RM7KTCACHE MAYBE
#endif

/*
 * QED RM7000 cache operations.
 *
 * The _flush and _clean functions are complex composites that do whatever
 * is necessary to flush/clean ALL caches, in the quickest possible way.
 * The other functions are targetted explicitly at a particular cache
 * I, D, S or T; it is up to the user to call the correct set of functions
 * for a given system.
 *
 * NOTE:	
 *
 * The tertiary cache code assumes that all addresses presented to these
 * functions are in kseg0, so that the tcache index matches the virtual
 * address; if a mapped virtual address is presented then it will not
 * be translated and the wrong part of the tcache will be invalidated!
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
	
IMPORT(mips_tcache_size,4)
IMPORT(mips_tcache_linesize,4)
IMPORT(mips_tcache_ways,4)
	
BSS(rm7k_tcache_tagmask,4)

/*
 * Macros to automate cache operations
 */

#define addr	t0
#define maxaddr	t1
#define mask	t2

#define cacheop(kva, n, linesize, op)	\
	.set	noreorder ;		\
	/* check for bad size */	\
 	blez	n,11f ;			\
	addu	maxaddr,kva,n ;		\
	/* align to line boundaries */	\
	subu	mask,linesize,1 ;	\
	not	mask ;			\
	and	addr,kva,mask ;		\
	addu	maxaddr,-1 ;		\
	and	maxaddr,mask ;		\
	/* the cacheop loop */		\
10: 	cache	op,0(addr) ;	 	\
	bne     addr,maxaddr,10b ;	\
	addu   	addr,linesize ;		\
11:	.set	reorder

/* virtual cache op: no limit on size of region */
#define vcacheop(kva, n, linesize, op)	\
	cacheop(kva, n, linesize, op)

/* indexed cache op: region limited to cache size */
#define icacheop(kva, n, linesize, size, op) \
	move	t3,n;			\
	bltu	n,size,12f ;		\
	move	t3,size ;		\
12:	cacheop(kva, t3, linesize, op)
	
/* ultra-paranoid pipeline cleaner */	
#define CLEAN_PIPE \
	.set noat; mflo $1; mflo $1; mflo $1; mflo $1; .set at
		
#if defined(IN_PMON) || defined(ITROM) || defined(_SDE_CACHE_EXTRA)
/* caches are always sized first */
#define SIZE_CACHE(reg,which)		\
	sync;				\
	lw	reg,which;		\
	blez	reg,9f
#else	
#define SIZE_CACHE(reg,which)		\
	sync;				\
	lw	reg,which;		\
	move	v1,ra;			\
	bgez	reg,9f;			\
	bal	rm7k_size_cache;	\
	lw	reg,which;		\
	move	ra,v1;			\
9:	blez	reg,9f
#endif


#ifndef _SDE_CACHE_EXTRA
	
/*
 * static void _size_cache()
 * 
 * Internal routine to determine cache sizes by looking at RM7000 config
 * register.  Sizes are returned in registers, as follows:
 */

#define icachesize	t2
#define dcachesize	t3
#define scachesize	t4
#define tcachesize	t5
#define sizebase	t6
#define sizeptr		t7
#define sizesr		t8
#define ttagmask	t9

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

	/* work out secondary cache size */
	/* no secondary cache if Config.SC != 0 */
	move	scachesize,zero
	and	t1,t0,CFG_SC
	bnez	t1,3f
	
	mfc0	t1,C0_PRID
	and	t1,0xff
	bgeu	t1,0x20,1f
	
	/* Rev 1.0: fixed 256KB scache size */
	li	scachesize,256*1024
	b	3f
	
1:	/* Rev 2.0+: get scache size from info register */
	mfc0	scachesize,C0_INFO
	and	scachesize,INFO_SSMASK
	sll	scachesize,17 - INFO_SSSHIFT
	
3:	move	tcachesize,zero
	
#if RM7KTCACHE	
	/* no tertiary cache if Config.TC != 0 */
	and	t1,t0,CFG_TC
	bnez	t1,9f
	
	/* Run uncached (PIC) */
	move	sizesr,ra
	.set	noreorder
	.set	nomacro
	bal	1f
	li	t1,KSEG1_BASE
1:	or	t1,ra
	addu	t1,16
	jr	t1
	move	ra,sizesr
	.set	macro
	.set	reorder
	
	/* disable i/us and cache exceptions */
	mfc0	sizesr,$sr
	and	t1,sizesr,~SR_IE
	or	t1,SR_DE
	.set noreorder
	mtc0	t1,$sr
	CLEAN_PIPE
	.set	reorder
	
	/* enable tertiary cache */
	or	t1,t0,CFG_TE
	mtc0	t1,$config
	
	/* store zero at Tcache[0].taghi */
	mtc0	zero,$taghi
	mtc0	zero,$taglo
	la	sizebase,KSEG0_BASE
	cache	Index_Store_Tag_T,0(sizebase)
	
	/* calculate tertiary cache size */
	li	tcachesize,512*1024
	li	ttagmask,TAGHI_TTAG_MASK
	
1:	/* Tcache[size].taghi = -1 */
	li	t1,-1
	mtc0	t1,$taghi
	addu	sizeptr,sizebase,tcachesize
	
	.set noreorder
	cache	Index_Store_Tag_T,0(sizeptr)
	nop; nop
	cache	Index_Load_Tag_T,0(sizebase)
	.set reorder
	
	/* if (Tcache[0].taghi != 0) break; */
	mfc0	t1,$taghi
	bnez	t1,7f

	/* tcachesize *= 2 */
	sll	tcachesize,1
	sll	ttagmask,1

	/* while (tcachesize < 8MB) */
	bltu	tcachesize,8*1024*1024,1b

7:	mtc0	t0,$config
	mtc0	sizesr,$sr
	CLEAN_PIPE
#endif /* RM7KTCACHE */

9:	j	ra
SEND(_size_cache)


/*
 * void size_cache()
 * 
 * Work out size of I, D & S caches
 */
LEAF(rm7k_size_cache)
	lw	t0,mips_icache_size
	move	v0,ra
	bgtz	t0,8f				# already known?
	bal	_size_cache
	move	ra,v0
	
.savesize:	
	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	sw	scachesize,mips_scache_size
	sw	tcachesize,mips_tcache_size
	sw	ttagmask,rm7k_tcache_tagmask
	mfc0	t0,C0_CONFIG
	
	/* compute icache line size */
	and	v1,t0,CFG_IB
	li	t1,16
	beqz	v1,1f
	li	t1,32
	sw	t1,mips_icache_linesize
	
	/* compute dcache line size */
	and	v1,t0,CFG_DB
	li	t1,16
	beqz	v1,1f
	li	t1,32
	sw	t1,mips_dcache_linesize
	
	/* compute scache and tcache line size (2^(4+sb)) */
	and	v1,t0,CFG_SBMASK
	srl	v1,CFG_SBSHIFT
	li	t1,16
	sll	t1,v1
	sw	t1,mips_scache_linesize
	sw	t1,mips_tcache_linesize
	
	mfc0	t0,C0_PRID
	and	t0,0xff
	bgeu	t0,0x20,1f
	
	/* Rev 1.0: fake info register */
	li	t0,(4<<INFO_IWSHIFT)|(4<<INFO_DWSHIFT)|(4<<INFO_SWSHIFT)
	b	2f
	
1:	/* Rev 2.0+: read info register */
	mfc0	t0,C0_INFO
	
	/* compute icache associativity */
2:	and	t1,t0,INFO_IWMASK
	srl	t1,INFO_IWSHIFT
	sw	t1,mips_icache_ways
	
	/* compute dcache associativity */
	and	t1,t0,INFO_DWMASK
	srl	t1,INFO_DWSHIFT
	sw	t1,mips_dcache_ways
	
	/* compute scache associativity */
	and	t1,t0,INFO_SWMASK
	srl	t1,INFO_SWSHIFT
	sw	t1,mips_scache_ways
	
	li	t1,1
	sw	t1,mips_tcache_ways
	
8:	j	ra
END(rm7k_size_cache)

/*
 * void rm7k_init_cache()
 * 
 * Work out size of and initialize I, D & S caches.
 *
 * NOTES
 *  1) assumes enough DRAM has been initialised with correct parity
 */
LEAF(rm7k_init_cache)
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
	CLEAN_PIPE
	
	/* disable secondary and tertiary caches and set tag & ecc to 0 */
	mfc0	t0,$config
	mtc0	zero,$taglo
	mtc0	zero,$taghi
	mtc0	zero,$ecc
	and	t0,~(CFG_TE|CFG_SE)
	mtc0	t0,$config
	CLEAN_PIPE
	.set	reorder
	
	/* 
	 * Assumes bottom of RAM will generate good parity 
	 * for primary and secondary caches (max 256K)
	 */

	/* compute icache linesize */
	and	v1,t0,CFG_IB
	li	t1,16
	beqz	v1,1f
	li	t1,32
1:		
	
	/* 
	 * Initialise primary instruction cache.
	 */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,icachesize		# limit = base + icachesize 
1:	addu	a0,t1				# base += linesize
	cache	Index_Store_Tag_I,-4(a0)	# clear tag
	nop; nop
	cache	Fill_I,-4(a0)			# fill data line
	nop
	bne	a0,a1,1b
	cache	Index_Store_Tag_I,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* compute dcache linesize */
	and	v1,t0,CFG_DB
	li	t1,16
	beqz	v1,1f
	li	t1,32
1:		
	/* 
	 * Initialise primary data cache.
	 * (for n-way set caches, we do it in 3 passes).
	 */

	/* 1: initialise dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	sync
	
	/* 2: fill dcache data */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize		# limit = base + dcachesize 
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	lw	zero,-4(a0)			# BDSLOT: fill line
	.set	reorder
	
	sync

	/* 3: clear dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/*
	 * Initialise the secondary cache
	 * (for n-way set caches, we do it in 3 passes).
         * XXX Requires 256KB memory to provide good parity
	 */
	/* compute scache line size (2^(4+sb)) */
	and	v1,t0,CFG_SBMASK
	srl	v1,CFG_SBSHIFT
	li	t1,16
	sll	t1,v1
	
	blez	scachesize,3f			# scache present?
	
	/* enable secondary cache */
	or	t0,CFG_SE
	.set	noreorder		
	mtc0	t0,$config
	CLEAN_PIPE
	.set	reorder
	
	/* 1: initialise scache tags */
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_S,-4(a0)	# BDSLOT: clear tag
	.set	reorder
	
	sync
	
	/* 1: fill scache data */
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # a1 = base + scachesize 
	.set	noreorder
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	lw	zero,-4(a0)			# BDSLOT: load line
	.set	reorder
	
	sync
	
	/* 3: clear scache tags */
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a0,t1				# base += linesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_S,-4(a0)	# BDSLOT: clear tag
	.set	reorder
	
	sync
	
#if RM7KTCACHE
	/* 
	 * Initialise tertiary cache tags (if present).
	 */
3:	blez	tcachesize,3f			# tcache present?
	
	/* enable tertiary cache */
	or	t0,CFG_TE
	.set	noreorder		
	mtc0	t0,$config
	CLEAN_PIPE
	.set	reorder
	
	li	a0,KSEG0_BASE
#ifdef RM7KTCACHE_FLASH_CLEAR
	/* flash clear all secondary cache tags */
	.set	noreorder
	cache	Flash_Invalidate_T,0(a0)
	nop; nop
	.set	reorder
#else	
	/* clear one page at a time */
	sll	t1,7				# linesize *= 128
	addu	a1,a0,tcachesize		# limit
1:	cache	Page_Invalidate_T,0(a0)
	addu	a0,t1				# base += pagesize
	bne	a0,a1,1b
#endif		
	sync
#endif
	
	/* Since the CPU won't look at the tertiary cache data
	   unless it gets a tag match, the initial state
	   of the data parity doesn't matter. */

	/* we store the sizes only after the caches are initialised */
3:	mtc0	v0,$sr
	CLEAN_PIPE
	b	.savesize
	
	j	ra
END(rm7k_init_cache)
	

	
/*
 * void rm7k_flush_cache (void)
 *
 * Flush and invalidate all caches
 */
LEAF(rm7k_flush_cache)
	li	a0,KSEG0_BASE
	SIZE_CACHE(a1,mips_dcache_size)
	
	/* always flush primaries (in case of orphans) XXX */
	lw	a2,mips_dcache_linesize
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)
	
	lw	a1,mips_icache_size
	lw	a2,mips_icache_linesize
	cacheop(a0,a1,a2,Index_Invalidate_I)
	
	sync

	/* flush secondary */
	lw	a1,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a1,8f
	cacheop(a0,a1,a2,Index_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE	
	/* flush tertiary */
8:	lw	a1,mips_tcache_size
	mtc0	zero,$taglo
	blez	a1,8f	
#ifdef RM7KTCACHE_FLASH_CLEAR
	/* flash clear all tertiary cache tags */
	.set	noreorder
	cache	Flash_Invalidate_T,0(a0)
	nop; nop
	.set	reorder
#else	
	/* clear one page at a time */
	sll	a2,7				# linesize * 128
	addu	a1,a0				# limit = base + tcachesize
1:	cache	Page_Invalidate_T,0(a0)
	addu	a0,a2				# += page size
	bne	a0,a1,1b
#endif		
	sync
#endif
			
8:;9:	j	ra
END(rm7k_flush_cache)
	
	
/*
 * void rm7k_flush_dcache (void)
 *
 * Flush and invalidate data caches only
 */
LEAF(rm7k_flush_dcache)
	/* flush primary data cache */
	SIZE_CACHE(a1,mips_dcache_size)
	li	a0,KSEG0_BASE
	lw	a2,mips_dcache_linesize
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)
	
	sync
	
	/* flush secondary cache */
	lw	a1,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a1,8f
	cacheop(a0,a1,a2,Index_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE
	/* clear tertiary cache */
8:	lw	a1,mips_tcache_size
	mtc0	zero,$taglo
	blez	a1,8f	
	sll	a2,7				# linesize * 128
#ifdef RM7KTCACHE_FLASH_CLEAR
	/* flash clear all tertiary cache tags */
	.set	noreorder
	cache	Flash_Invalidate_T,0(a0)
	nop; nop
	.set	reorder
#else	
	/* clear one page at a time */
	addu	a1,a0				# limit = base + scachesize
1:	cache	Page_Invalidate_T,0(a0)
	addu	a0,a2			# += page size
	bne	a0,a1,1b
#endif		
	sync
#endif
8:;9:
	j	ra
END(rm7k_flush_dcache)
	
	
/*
 * void rm7k_flush_icache (void)
 *
 * Flush and invalidate instruction caches only
 */
LEAF(rm7k_flush_icache)
	/* flush primary instruction cache */
	SIZE_CACHE(a1,mips_icache_size)
	li	a0,KSEG0_BASE
	lw	a2,mips_icache_linesize
	cacheop(a0,a1,a2,Index_Invalidate_I)
	
	/* flush secondary cache */
	lw	a1,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a1,8f
	cacheop(a0,a1,a2,Index_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE
	/* clear tertiary cache */
8:	lw	a1,mips_tcache_size
	mtc0	zero,$taglo
	blez	a1,8f	
#ifdef RM7KTCACHE_FLASH_CLEAR
	/* flash clear all tertiary cache tags */
	.set	noreorder
	cache	Flash_Invalidate_T,0(a0)
	nop; nop
	.set	reorder
#else	
	/* clear one page at a time */
	addu	a1,a0				# limit = base + scachesize
	sll	a2,7				# linesize * 128
1:	cache	Page_Invalidate_T,0(a0)
	addu	a0,a2				# += page size
	bne	a0,a1,1b
#endif		
	sync
#endif
8:;9:
	j	ra
END(rm7k_flush_icache)
	
	
	
/*
 * void rm7k_clean_cache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(rm7k_clean_cache)
	SIZE_CACHE(a3,mips_dcache_size)
	
	/* clean primary caches */
	lw	a2,mips_dcache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)
	
	lw	a2,mips_icache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_I)
	
	sync
	
	/* clean secondary cache */
	lw	a3,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a3,8f
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE
	/* clean tertiary cache */
8:	
	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
	sync
#endif		
8:;9:	j	ra
END(rm7k_clean_cache)


/*
 * void rm7k_clean_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all data caches
 */
LEAF(rm7k_clean_dcache)
	SIZE_CACHE(a3,mips_dcache_size)
	
	/* clean primary dcache */
	lw	a2,mips_dcache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)
	
	sync
	
	/* clean secondary cache */
	lw	a3,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a3,8f
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE
	/* clean tertiary cache */
8:	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
	sync
#endif
8:;9:	
	j	ra
END(rm7k_clean_dcache)
	
	
/*
 * void rm7k_clean_dcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in data caches
 * XXX Only safe if region is totally cache-line aligned.
 */
LEAF(rm7k_clean_dcache_nowrite)
	SIZE_CACHE(a3,mips_dcache_size)
	
	/* invalidate primary dcache */
	lw	a2,mips_dcache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_D)
	
	/* invalidate secondary cache */
	lw	a3,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a3,8f
	vcacheop(a0,a1,a2,Hit_Invalidate_S)
	
#if RM7KTCACHE
	/* invalidate tertiary cache */
8:	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
#endif
8:;9:	j	ra
END(rm7k_clean_dcache_nowrite)
	

/*
 * void rm7k_clean_icache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in instruction caches
 */
LEAF(rm7k_clean_icache)
	SIZE_CACHE(a3,mips_icache_size)
	
	/* clean primary icache */
	lw	a2,mips_icache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_I)
	
	/* clean secondary cache */
	lw	a3,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a3,8f
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_S)
	
	sync	
	
#if RM7KTCACHE
	/* clear tertiary cache */
8:	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
	sync
#endif
8:;9:
	j	ra
END(rm7k_clean_icache)
	

		
/* 
 * Cache locking
 *
 * WARNING: if you lock any cache lines, then don't call the 
 * mips_flush_xcache routines, because these will flush the 
 * locked data out of the cache too; use only mips_clean_xcache.
 */	
	
/*
 * void rm7k_lock_dcache (void *data, size_t n)
 *
 * Load and lock a block of date into the d-cache
 */
LEAF(rm7k_lock_dcache)
	SIZE_CACHE(t4,mips_dcache_size)
	
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,~SR_IE
	mtc0	v1,$sr
	
	/* limit to half cache size (one set) */
	srl	t4,1
	bltu	a1,t4,1f	
	move	a1,t4
1:	/* calculate end address */
	addu	a3,a0,a1		# maxaddr = data + n - 1
	subu	a3,1
	
	/* enable dcache locking (set 0 only, sorry) */
	li	t8,ECC_D
	mtc0	t8,$ecc

	/* align start and end to line boundaries */
	lw	t5,mips_dcache_linesize
	subu	t0,t5,1
	not	t0
	and	a0,t0			# align start
	and	a3,t0			# align end
	
	/* the main loop (one cache line per loop) */
	.set	noreorder
10:	/* flush any exiting copy of the line and load into set 0 */
 	cache	Hit_Writeback_Inv_D,0(a0)
	nop; nop
	sync
	lw	zero,0(a0)
	bne     a0,a3,10b
	addu   	a0,t5
	.set	reorder
	
	/* restore the ecc & status registers */
	.set noreorder
	.set nowarn
	mtc0	zero,$ecc
11:	mtc0	v0,$sr
	CLEAN_PIPE
	.set warn
	.set reorder
	
9:	j	ra
END(rm7k_lock_dcache)
	
	
/*
 * void rm7k_lock_icache (void *code, size_t n)
 *
 * Load and lock a block of instructions into the i-cache
 */
LEAF(rm7k_lock_icache)
	SIZE_CACHE(t4,mips_icache_size)
	
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,~SR_IE
	mtc0	v1,$sr
	
	/* limit to half cache size (one set) */
	srl	t4,1
	bltu	a1,t4,1f	
	move	a1,t4
1:	/* calculate end address */
	addu	a3,a0,a1		# maxaddr = data + n - 1
	subu	a3,1
	
	/* run uncached while lock bit set */
	la	t0,1f
	or	t0,KSEG1_BASE		
	j	t0
1:	
		
	/* enable icache locking (set 0 only, sorry) */
	li	t8,ECC_I
	mtc0	t8,$ecc

	/* align start and end to line boundaries */
	lw	t5,mips_icache_linesize
	subu	t0,t5,1			# mask = ~(linesize - 1)
	not	t0			# mask = ~(linesize - 1)
	and	a0,t0			# align start
	and	a3,t0			# align end
	
	/* the main loop (one cache line per loop) */
	.set	noreorder
10:	/* flush any existing copy of line and load into set 0 */
 	cache	Hit_Invalidate_I,0(a0)
	nop; nop
	cache	Fill_I,0(a0)
	bne     a0,a3,10b
	addu   	a0,t5
	.set	reorder
	
	/* restore the ecc & status registers */
	.set noreorder
	.set nowarn
	mtc0	zero,$ecc
11:	mtc0	v0,$sr
	CLEAN_PIPE
	.set warn
	.set reorder
	
9:	j	ra
END(rm7k_lock_icache)
	
	
LEAF(rm7k_lock_scache)
	SIZE_CACHE(t4,mips_scache_size)
	
	/* disable interrupts */
	mfc0	v0,$sr
	and	v1,v0,~SR_IE
	mtc0	v1,$sr
	
	/* limit to half cache size (one set) */
	srl	t4,1
	bltu	a1,t4,1f	
	move	a1,t4
1:	/* calculate end address */
	addu	a3,a0,a1		# maxaddr = data + n - 1
	subu	a3,1
	
	/* run uncached while lock bit set */
	la	t0,1f
	or	t0,KSEG1_BASE		
	j	t0
1:	
		
	/* enable scache locking (set 0 only, sorry) */
	li	t8,ECC_S
	mtc0	t8,$ecc

	/* align start and end to line boundaries */
	lw	t5,mips_scache_size
	subu	t0,t5,1			# mask = ~(linesize - 1)
	not	t0			
	and	a0,t0			# align start
	and	a3,t0			# align end
	
	/* the main loop (one cache line per loop) */
	.set	noreorder
10:	/* flush line from caches and load into set 0 */
 	cache	Hit_Writeback_Inv_D,0(a0)
	nop; nop
	sync
	cache	Hit_Writeback_Inv_S,0(a0)
	nop; nop
	sync
	lw	zero,0(a0)
	bne     a0,a3,10b
	addu   	a0,t5
	.set	reorder
	
	/* restore the ecc & status registers */
	.set noreorder
	.set nowarn
	mtc0	zero,$ecc
11:	mtc0	v0,$sr
	CLEAN_PIPE
	.set warn
	.set reorder
	
9:	j	ra
END(rm7k_lock_scache)
	
#endif /* ! _SDE_CACHE_EXTRA */
	

		
/*
 * The following functions operate on individual cache levels, or use
 * indexed addressing, so they are probably only useful for cache 
 * diagnostics or possibly virtual memory operating systems.
 */
	
#if defined(_SDE_CACHE_EXTRA) || defined(ITROM)
		
/*
 * void rm7k_flush_cache_nowrite (void *base)
 *
 * Invalidate but don't writeback all caches (probably only
 * sensible for cache diagnostics). Use "base" as the
 * cacheable base address, in case kseg0 is uncacheable.
 */
LEAF(rm7k_flush_cache_nowrite)
	/* disable all i/u and cache exceptions */
	mfc0	a3,$sr
	li	a1,~SR_IE
	and	a1,a3
	or	a1,SR_DE
	.set noreorder
	mtc0	a1,$sr
	CLEAN_PIPE
	mtc0	zero,$taglo			# initial cache tag 
	nop
	.set reorder

	/* li	a0,KSEG0_BASE */
	SIZE_CACHE(a1,mips_scache_size)
	lw	a2,mips_scache_linesize
9:	lw	t3,mips_dcache_size
	lw	t4,mips_icache_size
	lw	t5,mips_tcache_size
	lw	t6,mips_dcache_linesize
	lw	t7,mips_icache_linesize
		
	/* flush secondary cache if present */
	blez	a1,9f
	cacheop(a0,a1,a2,Index_Store_Tag_S)

	/* flush primary caches */
9:	cacheop(a0,t3,t6,Index_Store_Tag_D)
	cacheop(a0,t4,t7,Index_Store_Tag_I)

#if RM7KTCACHE
	/* flush tertiary caches */
	blez	t5,9f	
#ifdef RM7KTCACHE_FLASH_CLEAR
	/* flash clear whole of tertiary cache */
	.set	noreorder
	cache	Flash_Invalidate_T,0(a0)
	nop; nop
	.set	reorder
#else	
	/* invalidate one page at a time */
	addu	a1,t5,a0			# limit = base + scachesize
	sll	a2,7				# linesize * 128
1:	cache	Page_Invalidate_T,0(a0)
	addu	a0,a2				# += page size
	bne	a0,a1,1b
#endif		
#endif
	
9:	sync
	.set noreorder
	CLEAN_PIPE
	mtc0	a3,$sr
	CLEAN_PIPE
	.set reorder
	j	ra
END(rm7k_flush_cache_nowrite)


/*
 * void rm7k_clean_dcache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(rm7k_clean_dcache_indexed)
	SIZE_CACHE(a3,mips_dcache_size)
	
	lw	a2,mips_dcache_linesize	
	
	/* Handle multi-way set primaries */
	lw	t4,mips_dcache_ways
	divu	a3,t4			# do one set at a time

1:	icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)
	addu	a0,a3			# next set
	subu	t4,1
	bnez	t4,1b
	
	sync

9:	j	ra
END(rm7k_clean_dcache_indexed)

	
/*
 * void rm7k_clean_icache (unsigned kva, size_t n)
 *
 * Invalidate address range in instruction caches
 */
LEAF(rm7k_clean_icache_nowrite)
	/* flush primary icache */
	SIZE_CACHE(a3,mips_icache_size)
	lw	a2,mips_icache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_I)
	
	/* flush secondary cache */
	lw	a3,mips_scache_size
	lw	a2,mips_scache_linesize
	blez	a3,8f
	vcacheop(a0,a1,a2,Hit_Invalidate_S)
	
#if RM7KTCACHE
	/* clear tertiary cache */
8:	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
#endif
8:;9:	sync
	j	ra
END(rm7k_clean_icache_nowrite)

	
/*
 * void rm7k_clean_icache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary instruction cache
 */
LEAF(rm7k_clean_icache_indexed)
	SIZE_CACHE(a3,mips_icache_size)

	lw	a2,mips_icache_linesize
	lw	t4,mips_icache_ways
	
	/* Handle multi-way set primaries */
	divu	a3,t4
1:	icacheop(a0,a1,a2,a3,Index_Invalidate_I)
	addu	a0,a3			# do next set
	subu	t4,1
	bnez	t4,1b

9:	sync
	j	ra
END(rm7k_clean_icache_indexed)
	
	
/*
 * void rm7k_clean_scache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(rm7k_clean_scache)
	/* flush secondary cache */
	SIZE_CACHE(a3,mips_scache_size)
	lw	a2,mips_scache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_S)
	
	sync
	
#if RM7KTCACHE
	/* clear tertiary cache */
	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
	sync
#endif
8:;9:
	j	ra
END(rm7k_clean_scache)
	
	
/*
 * void rm7k_clean_scache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in secondary cache
 */
LEAF(rm7k_clean_scache_nowrite)
	/* flush secondary cache */
	SIZE_CACHE(a3,mips_scache_size)
	lw	a2,mips_scache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_S)
	
#if RM7KTCACHE
	/* clear tertiary cache */
	lw	a3,mips_tcache_size
	mtc0	zero,$taglo
	blez	a3,8f
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
#endif
8:;9:	sync
	j	ra	
END(rm7k_clean_scache_nowrite)
	

/*
 * void rm7k_clean_tcache (unsigned kva, size_t n)
 *
 * (Writeback and) invalidate address range in tertiary cache
 */
LEAF(rm7k_clean_tcache)
XLEAF(rm7k_clean_tcache_nowrite)
#if RM7KTCACHE
	SIZE_CACHE(a3,mips_tcache_size)
	lw	a2,mips_scache_linesize
	mtc0	zero,$taglo
	/* XXX assumes kseg0 so that physaddr = virtaddr & 0x1fffffff */
	icacheop(a0,a1,a2,a3,Index_Store_Tag_T)
	sync
#endif	
9:	j	ra
END(rm7k_clean_tcache)

	
/*
 * void rm7k_hit_writeback_inv_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(rm7k_hit_writeback_inv_dcache)
	SIZE_CACHE(a3,mips_dcache_size)
	lw	a2,mips_dcache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)
	sync
9:	j	ra
END(rm7k_hit_writeback_inv_dcache)
	
	
/*
 * void rm7k_hit_writeback_inv_scache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(rm7k_hit_writeback_inv_scache)
	SIZE_CACHE(a3,mips_scache_size)
	lw	a2,mips_scache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_S)
	sync
9:	j	ra
END(rm7k_hit_writeback_inv_scache)
	
#endif /* _SDE_CACHE_EXTRA */
	
#endif /* #cache(rm7k)*/
