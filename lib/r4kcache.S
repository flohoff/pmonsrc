/*
 * r4kcache.s: R4000 cache support functions for SDE-MIPS
 * Copyright (c) 1998 Algorithmics Ltd
 */

#if #cache(r4k)

#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/r4kc0.h>
#include <mips/prid.h>

#define NO	0
#define YES	1
#define MAYBE	2
	
#ifndef R4KSCACHE
#if #cpu(r4000sc) || #cpu(r4400sc) || #cpu(r4000mc) || #cpu(r4400mc)
#define R4KSCACHE YES
#else
#define R4KSCACHE NO
#endif
#endif
			
#ifndef R4KPC2WAY
#if #cpu(r4600) || #cpu(r4640) || #cpu(r4650) || #cpu(r4700)
#define R4KPC2WAY YES
#elif R4KSCACHE || #cpu(r4100) || #cpu(r4200) || #cpu(r4400) \
	|| #cpu(cw4010) || #cpu(cw4001)
#define R4KPC2WAY NO
#else
#define R4KPC2WAY MAYBE
#endif
#endif
		
/*
 * R4000 cache operations.
 *
 * The _flush and _clean functions are complex composites that do whatever
 * is necessary to flush/clean ALL caches, in the quickest possible way.
 * The other functions are targetted explicitly at a particular cache
 * I, D or SD; it is up to the user to call the correct set of functions
 * for a given system.
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
	


/*
 * static void _size_cache()
 * 
 * Internal routine to determine cache sizes by looking at R4000 config
 * register.  Sizes are returned in registers, as follows:
 */

#define cfg		t0	
#define tmp1		t1	
#define scachesize	t6
#define slinesize	t7
#define cacheflags	t8
#define  CF_SCDISCONTIG	 0x4
#define  CF_SCSPLIT	 0x2
#define  CF_PC2WAY	 0x1

#define SCACHE_MIN_SIZE	0x010000	/* minimum scache:  64Kb */
#define SCACHE_MAX_SIZE	0x400000	/* maximum scache:   4Mb */

SLEAF(_size_cache)
	mfc0	cfg,$config
	move	scachesize,zero
	move	slinesize,zero
	move	cacheflags,zero	
	
#if R4KSCACHE
	
#define tmp2		t2
#define max		t3
#define tagxsave	t4
#define tag0save	t5
#define srsave		t9	
	
	/* no secondary cache if Config.SC = 1 */
	and	tmp1,cfg,CFG_SC
	bnez	tmp1,9f
	
	/* note split cache */
	and	tmp1,cfg,CFG_SS
	beqz	tmp1,1f
	or	cacheflags,CF_SCSPLIT
	
	/* work out secondary cache line size */
1:	and	tmp1,cfg,CFG_SBMASK
	srl	tmp1,CFG_SBSHIFT
	li	slinesize,16
	sll	slinesize,tmp1

	/* disable all i/u and cache exceptions */
	mfc0	srsave,$sr
	li	tmp1,~SR_IE
	and	tmp1,srsave
	or	tmp1,SR_DE
	.set	noreorder
	mtc0	tmp1,$sr
	nop; nop; nop

	/* save and set initial marker */
	cache	Index_Load_Tag_SD,KSEG0_BASE	# get sdcache[0].tag
	nop
	mfc0	tag0save,$taglo
	nop
	mtc0	zero,$taglo			# initial cache tag
	nop; nop
	cache	Index_Store_Tag_SD,KSEG0_BASE	# sdcache[0].tag = 0
	
	/* check all secondary cache boundaries, until we wraparound */
	li	scachesize,SCACHE_MIN_SIZE
	li	max,SCACHE_MAX_SIZE

2:	addu	tmp1,scachesize,KSEG0_BASE	# calc &sdcache[size]
	cache	Index_Load_Tag_SD,0(tmp1)	# save sdcache[size].tag
	nop
	mfc0	tagxsave,$taglo
	nop; nop
	mtc0	scachesize,$taglo		# sdcache[size].tag = size
	nop; nop
	cache	Index_Store_Tag_SD,0(tmp1)
	nop; nop
	cache	Index_Load_Tag_SD,KSEG0_BASE	# get sdcache[0].tag
	nop
	mfc0	tmp2,$taglo			
	nop; nop
	mtc0	tagxsave,$taglo			# restore sdcache[size].tag
	nop; nop
	cache	Index_Store_Tag_SD,0(tmp1)
	.set	reorder
	
	and	tmp2,TAG_STAG_MASK
	bnez	tmp2,7f				# wrap around, got it
	sll	scachesize,1			# try next boundary
	bne	scachesize,max,2b		# up to maximum size
	
7:	/* look to see if we've got a discontiguous unified cache */
	and	tmp1,cacheflags,CF_SCSPLIT
	bnez	tmp1,8f				# it's split, not unified
	li	max,SCACHE_MAX_SIZE/2
	bgeu	scachesize,max,8f		# it's too big
	
	.set	noreorder
	mtc0	zero,$taglo			# initial cache tag
	nop; nop
	cache	Index_Store_Tag_SD,KSEG0_BASE	# sdcache[0].tag = 0
	nop; nop
	addu	tmp1,max,KSEG0_BASE		# calc &sdcache[max/2]
	cache	Index_Load_Tag_SD,0(tmp1)	# save sdcache[max/2].tag
	nop
	mfc0	tagxsave,$taglo
	nop; nop
	mtc0	max,$taglo
	nop; nop
	cache	Index_Store_Tag_SD,0(tmp1)	# sdcache[max/2].tag = max/2
	nop; nop
	cache	Index_Load_Tag_SD,KSEG0_BASE	# check sdcache[0].tag
	nop
	mfc0	tmp2,$taglo
	nop; nop
	mtc0	tagxsave,$taglo
	nop; nop
	cache	Index_Store_Tag_SD,0(tmp1)	# restore sdcache[max/2].tag
	.set	reorder
	
	and	tmp2,TAG_STAG_MASK
	bnez	tmp2,8f				# [0]!=0 => wrap => !discontig
	
	or	cacheflags,CF_SCDISCONTIG	# indicate discontiguous

	.set	noreorder
8:	mtc0	tag0save,$taglo			# restore sdcache[0].tag
	nop; nop
	cache	Index_Store_Tag_SD,KSEG0_BASE
	nop; nop; nop
	mtc0	srsave,$sr			# restore old $sr
	.set	reorder
	
9:		
#undef	tmp2
#undef	max
#undef	tagxsave
#undef	tag0save
#undef	srsave
#endif /* R4KSCACHE */

#define icachesize	t2
#define ilinesize	t3
#define dcachesize	t4
#define dlinesize	t5
	
	/* by default cachesize = 2^(ic+12) */
	li	icachesize,(1<<12)
	li	dcachesize,(1<<12)
	li	ilinesize,16
	li	dlinesize,16
	
	mfc0	tmp1,$prid
	srl	tmp1,8
	andi	tmp1,0xff
	beq	tmp1,PRID_JADE,9f	# JADE: use default?
	bne	tmp1,PRID_R4100,3f	# vr41xx?
	
	/* vr41xx specific config bit selects cache size type */
	and	tmp1,cfg,CFG_CS
	bnez	tmp1,1f
	
	/* original vr41xx had hardwired cache size */
	li	icachesize,2048
	li	dcachesize,1024
	b	4f
	
	/* modern vr41xx cachesize = 2^(ic+10) */
1:	li	icachesize,(1<<10)
	li	dcachesize,(1<<10)
	
	/* compute primary i-cache size */
3:	and	tmp1,cfg,CFG_ICMASK
	srl	tmp1,CFG_ICSHIFT
	sll	icachesize,tmp1

	/* compute primary d-cache size */
	and	tmp1,cfg,CFG_DCMASK
	srl	tmp1,CFG_DCSHIFT
	sll	dcachesize,tmp1
	
	/* compute primary i-cache line size */
4:	and	tmp1,cfg,CFG_IB
	beqz	tmp1,1f
	sll	ilinesize,1
		
	/* compute primary d-cache line size */
1:	and	tmp1,cfg,CFG_DB
	beqz	tmp1,1f
	sll	dlinesize,1
	
1:
9:
#if R4KPC2WAY == MAYBE
	/* have we got 2-way set primary caches */
#undef	cfg	
#define prid	t0	
	mfc0	prid,$prid			# get processor ID
	srl	prid,8				# get implementation
	and     prid,0xff
	beq	prid,PRID_JADE,1f		# Jade
	beq	prid,PRID_R4600,1f		# r4600
	beq	prid,PRID_R4700,1f		# r4700
	beq	prid,PRID_R4650,1f		# r4650/r4640
	beq	prid,PRID_RC6447X,1f		# rc6447x
	beq	prid,PRID_R5000,1f		# r5000
	bne	prid,PRID_RM52XX,4f		# rm52xx
1:	or	cacheflags,CF_PC2WAY
#undef	prid
#elif R4KPC2WAY == YES
	or	cacheflags,CF_PC2WAY
#endif

4:	j	ra
SEND(_size_cache)

#undef	tmp0
	

/*
 * void size_cache()
 * 
 * Work out size of I, D & S caches
 */
LEAF(r4k_size_cache)
	lw	t0,mips_icache_size
	move	v0,ra
	bgtz	t0,8f				# already known?
	bal	_size_cache
	move	ra,v0
	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	sw	scachesize,mips_scache_size
	sw	ilinesize,mips_icache_linesize
	sw	dlinesize,mips_dcache_linesize
	sw	slinesize,mips_scache_linesize
	and	t0,cacheflags,CF_SCSPLIT
	sw	t0,mips_scache_split
	and	t0,cacheflags,CF_SCDISCONTIG
	sw	t0,mips_scache_discontig
	and	t0,cacheflags,CF_PC2WAY
	addu	t0,1
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
	li	t0,1
	sw	t0,mips_scache_ways
8:	j	ra
END(r4k_size_cache)

/*
 * void r4k_init_cache()
 * 
 * Work out size of and initialize I, D & S caches.
 *
 * NOTES
 *  1) assumes enough DRAM has been initialised with correct parity
 */
LEAF(r4k_init_cache)
	/*
 	 * Determine the cache sizes
	 */
	move	v0,ra
	bal	_size_cache
	
	/* now run uncached (PIC) */
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
	nop; nop; nop
	mtc0	zero,$taglo			# initial cache tag 
	nop
	.set reorder

#if R4KSCACHE
	/* 
	 * Initialise secondary cache tags (if present).
	 */
	blez	scachesize,4f			# scache present?
	
	/* first data/unified tags */
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a0,slinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SD,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	and	a0,cacheflags,CF_SCSPLIT	# scache split?	
	beqz	a0,3f		
	
	/* then split icache */
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a0,slinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SI,-4(a0)	# BDSLOT: clear tag
	.set	reorder
	
3:	and	a0,cacheflags,CF_SCDISCONTIG
	beqz	a0,4f				# scache discontig
	
	/* initialise discontiguous cache tags */
	li	a0,KSEG0_BASE|(SCACHE_MAX_SIZE/2)
	addu	a1,a0,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a0,slinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SD,-4(a0)	# BDSLOT: clear tag
	.set	reorder
#endif		
4:		
	/* 
	 * Assume bottom of RAM or scache will generate good parity for the 
	 * primary caches (max 32K)
	 */

	/* 
	 * Initialise primary instruction cache.
	 */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,icachesize		# limit = base + icachesize 
1:	addu	a0,ilinesize
	cache	Index_Store_Tag_I,-4(a0)	# clear tag
	nop
	cache	Fill_I,-4(a0)			# fill line
	nop
	bne	a0,a1,1b
	cache	Index_Store_Tag_I,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* 
	 * Initialise primary data cache.
	 * (for R4600 2-way set caches, we do it in 3 passes).
	 */

	/* 1: initialise dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,dlinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

	/* 2: fill dcache */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize		# limit = base + dcachesize 
1:	addu	a0,dlinesize
	bne	a0,a1,1b
	lw	zero,-4(a0)			# BDSLOT: fill line
	.set	reorder

	/* 3: clear dcache tags */
	.set	noreorder
	li	a0,KSEG0_BASE
	addu	a1,a0,dcachesize        	# limit = base + dcachesize 
1:	addu	a0,dlinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_D,-4(a0)	# BDSLOT: clear tag
	.set	reorder

#if R4KSCACHE
	/*
	 * Initialise the secondary data cache data array
	 */
	blez	scachesize,4f			# scache present?
	
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # a1 = base + scachesize 
	
	.set	noreorder
	/* create matching dirty lines in primary and secondary */
1:	cache	Create_Dirty_Exc_SD,0(a0)
	nop; nop
	cache	Create_Dirty_Exc_D,0(a0)
	nop; nop

	/* set W-bit in primary cache line */
	sw	zero,0(a0)
	nop; nop
	
	/* move primary cache line to secondary */
	cache	Hit_Writeback_Inv_D,0(a0)
	nop; nop
	
	/* reset secondary tag */
	addu	a0,dlinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SD,-4(a0)	# BDSLOT: clear tag
	.set	reorder
	
	/*
	 * Initialise the secondary instruction cache data array
	 */
	and	a0,cacheflags,CF_SCSPLIT	# scache split?	
	beqz	a0,3f		
	
	li	a0,KSEG0_BASE
	addu	a1,a0,scachesize	        # a1 = base + scachesize 
	
	.set	noreorder

	/* fill primary Icache from secondary (ignoring ecc) */
1:	cache	Fill_I,0(a0)
	nop; nop
	
	/* write primary Icache to secondary */
	cache	Hit_Writeback_I,0(a0)
	nop; nop
	
	/* reset secondary tag */
	addu	a0,ilinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SI,-4(a0)	# BDSLOT: clear tag
	.set	reorder

3:	and	a0,cacheflags,CF_SCDISCONTIG
	beqz	a0,4f				# scache discontiguous?
	
	/*
	 * Initialise the discontiguous data cache
	 */
	li	a0,KSEG0_BASE|(SCACHE_MAX_SIZE/2)
	addu	a1,a0,scachesize	        # a1 = base + scachesize 
	
	.set	noreorder
	/* create matching dirty lines in primary and secondary */
1:	cache	Create_Dirty_Exc_SD,0(a0)
	nop; nop
	cache	Create_Dirty_Exc_D,0(a0)
	nop; nop

	/* set W-bit in primary cache line */
	sw	zero,0(a0)
	nop; nop
	
	/* move primary cache line to secondary */
	cache	Hit_Writeback_Inv_D,0(a0)
	nop; nop
	
	/* reset secondary tag */
	addu	a0,dlinesize
	bne	a0,a1,1b
	cache	Index_Store_Tag_SD,-4(a0)	# BDSLOT: clear tag
	.set	reorder
#endif

	/* we store the sizes only after the caches are initialised */
4:	sw	icachesize,mips_icache_size
	sw	dcachesize,mips_dcache_size
	sw	scachesize,mips_scache_size
	sw	ilinesize,mips_icache_linesize
	sw	dlinesize,mips_dcache_linesize
	sw	slinesize,mips_scache_linesize
	and	t0,cacheflags,CF_SCSPLIT
	sw	t0,mips_scache_split
	and	t0,cacheflags,CF_PC2WAY
	addu	t0,1
	sw	t0,mips_icache_ways
	sw	t0,mips_dcache_ways
	li	t0,1
	sw	t0,mips_scache_ways

	mtc0	v0,$sr
	j	ra
END(r4k_init_cache)
	

	
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
	bal	r4k_size_cache;		\
	lw	reg,which;		\
	move	ra,v1;			\
9:	blez	reg,9f
#endif

/*
 * void r4k_flush_cache (void)
 *
 * Writeback and invalidate all caches
 */
LEAF(r4k_flush_cache)
	SIZE_CACHE(a1,mips_dcache_size)

	/* writeback and invalidate primary caches individually */
	lw	a2,mips_dcache_linesize
	li	a0,KSEG0_BASE
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)

	lw	a1,mips_icache_size
	lw	a2,mips_icache_linesize
	cacheop(a0,a1,a2,Index_Invalidate_I)
	
#if R4KSCACHE
	/* index secondary cacheops don't do all the work! */
	lw	a1,mips_scache_size
	blez	a1,9f
	lw	a2,mips_scache_linesize
	lw	v1,mips_scache_split
	lw	a3,mips_scache_discontig
	
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
	beqz	v1,3f			# split scache?
	
	cacheop(a0,a1,a2,Index_Invalidate_SI)
	
3:	beqz	a3,9f			# discontiguous scache?		
	addu	a0,SCACHE_MAX_SIZE/2
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
#endif

9:	j	ra
END(r4k_flush_cache)
	
/*
 * void r4k_flush_dcache (void)
 *
 * Writeback and invalidate data caches only
 */
LEAF(r4k_flush_dcache)
	SIZE_CACHE(a1,mips_dcache_size)

	/* writeback and invalidate primary data cache */
	lw	a2,mips_dcache_linesize
	li	a0,KSEG0_BASE
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)
	
#if R4KSCACHE
	/* use secondary cacheops if present */
	lw	a1,mips_scache_size
	blez	a1,9f
	lw	a2,mips_scache_linesize
	lw	a3,mips_scache_discontig
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
	/* discontiguous? */
	beqz	a3,9f
	addu	a0,SCACHE_MAX_SIZE/2
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
#endif

9:	j	ra
END(r4k_flush_dcache)
	

/*
 * void r4k_flush_icache (void)
 *
 * Writeback and invalidate instruction cache only
 */
LEAF(r4k_flush_icache)
	SIZE_CACHE(a1,mips_icache_size)

	/* writeback and invalidate primary instruction cache */
	lw	a2,mips_icache_linesize
	li	a0,KSEG0_BASE
	cacheop(a0,a1,a2,Index_Invalidate_I)
	
#if R4KSCACHE
	/* use secondary cacheops if present */
	lw	a1,mips_scache_size)
	blez	a1,9f
	lw	t0,mips_scache_split
	lw	a2,mips_scache_linesize
	lw	a3,mips_scache_discontig
	bnez	t0,1f

	/* unified scache - must writeback dirty data */
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
	/* discontiguous? */
	beqz	a3,1f
	addu	a0,SCACHE_MAX_SIZE/2
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	b	9f

	/* split si/sd cache - leave sdcache untouched */
1:	cacheop(a0,a1,a2,Index_Invalidate_SI)
#endif
	
9:	j	ra
END(r4k_flush_icache)
	

	
/*
 * void r4k_clean_cache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(r4k_clean_cache)
#if R4KSCACHE
	/* secondary cacheops do all the work (if fitted) */
	SIZE_CACHE(a2,mips_scache_linesize)
	lw	v1,mips_scache_split
	
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	
	beqz	v1,2f			# split scache?
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
	
	b	2f

9:	lw	a2,mips_dcache_linesize
#else
	SIZE_CACHE(a2,mips_dcache_linesize)
#endif	
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)

	lw	a2,mips_icache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_I)

2:;9:	j	ra
END(r4k_clean_cache)


/*
 * void r4k_clean_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in data caches
 */
LEAF(r4k_clean_dcache)
#if R4KSCACHE
	/* secondary cacheops do all the work (if fitted) */
	SIZE_CACHE(a2,mips_scache_linesize)
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	b	2f

9:	lw	a2,mips_dcache_linesize
#else
	SIZE_CACHE(a2,mips_dcache_linesize)
#endif
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)

2:;9:	j	ra
END(r4k_clean_dcache)

	
/*
 * void r4k_clean_icache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in instruction caches
 */
LEAF(r4k_clean_icache)
#if R4KSCACHE
	/* secondary cacheops do all the work (if fitted) */
	SIZE_CACHE(a2,mips_scache_linesize)
	lw	v1,mips_scache_split
	beqz	v1,1f
	
	/* split Scache - just junk dirty lines */
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
	b	2f

	/* unified Scache - must writeback dirty lines */
1:	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	b	2f

9:	lw	a2,mips_icache_linesize
#else
	SIZE_CACHE(a2,mips_icache_linesize)
#endif
	vcacheop(a0,a1,a2,Hit_Invalidate_I)

2:;9:	j	ra
END(r4k_clean_icache)
	


/*
 * The following functions operate on individual cache levels, or use
 * indexed addressing, so they are probably only useful for cache 
 * diagnostics or possibly virtual memory operating systems.
 */
	
#if defined(_SDE_CACHE_EXTRA) || defined(ITROM)
		
/*
 * void r4k_flush_cache_nowrite (void)
 *
 * Invalidate but don't writeback all caches (probably only
 * sensible for cache diagnostics).
 */
LEAF(r4k_flush_cache_nowrite)
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

#if R4KSCACHE
	SIZE_CACHE(a1,mips_scache_size)
	lw	a2,mips_scache_linesize
	lw	v1,mips_scache_split
	
	li	a0,KSEG0_BASE
	cacheop(a0,a1,a2,Index_Store_Tag_SD)
	
	/* fill SDcache with valid (but clean) data */		
	move	t0,a0
	addu	t1,t0,a1
	subu	t1,a2
	.set	noreorder
10:	lw	zero,0(t0)
	bne	t0,t1,10b
	addu	t0,a2
	.set	reorder
	
	/* invalidate "properly", leaving sensible tags */
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
	beqz	v1,4f			# split scache?
	cacheop(a0,a1,a2,Index_Store_Tag_SI)
	
4:	lw	v1,mips_scache_discontig
	beqz	v1,9f			# discontiguous scache?
	
	/* invalidate SDcache #2 tags manually */		
	addu	a0,SCACHE_MAX_SIZE/2
	cacheop(a0,a1,a2,Index_Store_Tag_SD)
	
	/* fill SDcache #2 with valid (but clean) data */		
	move	t0,a0
	addu	t1,t0,a1
	subu	t1,a2
	.set	noreorder
10:	lw	zero,0(t0)
	bne	t0,t1,10b
	addu	t0,a2
	.set	reorder
	
	/* invalidate "properly", leaving sensible tags */
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
9:	lw	a1,mips_dcache_size
#else
	SIZE_CACHE(a1,mips_dcache_size)
#endif
	/* invalidate primary caches individually */
	lw	a2,mips_dcache_linesize
	li	a0,KSEG0_BASE
	cacheop(a0,a1,a2,Index_Store_Tag_D)

	lw	a1,mips_icache_size
	lw	a2,mips_icache_linesize
	cacheop(a0,a1,a2,Index_Store_Tag_I)

2:;9:		
	.set noreorder
	nop; nop; nop;
	mtc0	a3,$sr
	nop; nop; nop;
	.set reorder
	j	ra
END(r4k_flush_cache_nowrite)
	

/*
 * void r4k_clean_dcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate (but don't writeback) address range in data caches
 */
LEAF(r4k_clean_dcache_nowrite)
#if R4KSCACHE
	/* secondary cacheops do all the work (if fitted) */
	SIZE_CACHE(a2,mips_scache_linesize)
	vcacheop(a0,a1,a2,Hit_Invalidate_SD)
	b	2f

9:	lw	a2,mips_dcache_linesize
#else
	SIZE_CACHE(a2,mips_dcache_linesize)
#endif	
	vcacheop(a0,a1,a2,Hit_Invalidate_D)

2:;9:	j	ra
END(r4k_clean_dcache_nowrite)
	

/*
 * void r4k_clean_dcache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(r4k_clean_dcache_indexed)
	SIZE_CACHE(a2,mips_dcache_linesize)
	lw	a3,mips_dcache_size
	
#if R4KPC2WAY
	/* Handle possible two-way set primaries */
#if R4KPC2WAY == MAYBE
	lw	t8,mips_dcache_ways
	blt	t8,2,2f	
#endif
	srl	a3,1			# do one set (half cache) at a time
	icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)
	addu	a0,a3			# do next set
#endif
		
2:	icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)

9:	j	ra
END(r4k_clean_dcache_indexed)
	
	
/*
 * void r4k_clean_icache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in instruction caches
 */
LEAF(r4k_clean_icache_nowrite)
#if R4KSCACHE
	/* secondary cacheops do all the work (if fitted) */
	SIZE_CACHE(a2,mips_scache_linesize)
	
	/* just junk dirty lines */
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
	b	2f

9:	lw	a2,mips_icache_linesize
#else
	SIZE_CACHE(a2,mips_icache_linesize)
#endif
	vcacheop(a0,a1,a2,Hit_Invalidate_I)

2:;9:	j	ra
END(r4k_clean_icache_nowrite)
	

/*
 * void r4k_clean_icache_indexed (unsigned kva, size_t n)
 *
 * Writeback and invalidate indexed range in primary instruction cache
 */
LEAF(r4k_clean_icache_indexed)
	SIZE_CACHE(a2,mips_icache_linesize)
	lw	a3,mips_icache_size

#if R4KPC2WAY
	/* Handle possible two-way set primaries */
#if R4KPC2WAY == MAYBE
	lw	t8,mips_icache_ways
	blt	t8,2,2f	
#endif
	srl	a3,1			# do one set (half cache) at a time
	icacheop(a0,a1,a2,a3,Index_Invalidate_I)
	addu	a0,a3			# do next set
#endif

2:	icacheop(a0,a1,a2,a3,Index_Invalidate_I)

9:	j	ra
END(r4k_clean_icache_indexed)
	
/*
 * void r4k_clean_scache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(r4k_clean_scache)
#if R4KSCACHE
	SIZE_CACHE(a2,mips_scache_linesize)
	lw	v1,mips_scache_split
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	beqz	v1,9f
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
#endif
9:	j	ra
END(r4k_clean_scache)

	
/*
 * void r4k_clean_scache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate an address range in secondary cache
 */
LEAF(r4k_clean_scache_nowrite)
#if R4KSCACHE
	SIZE_CACHE(a2,mips_scache_linesize)
	lw	v1,mips_scache_split
	vcacheop(a0,a1,a2,Hit_Invalidate_SD)
	beqz	v1,9f
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
#endif	
9:	j	ra
END(r4k_clean_scache_nowrite)

/*
 * void mips_clean_sdcache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate an address range in secondary data cache
 */
LEAF(r4k_clean_sdcache_nowrite)
#if R4KSCACHE
	SIZE_CACHE(a2,mips_scache_linesize)
	vcacheop(a0,a1,a2,Hit_Invalidate_SD)
#endif	
9:	j	ra
END(r4k_clean_sdcache_nowrite)
	
/*
 * void mips_clean_sicache_nowrite (unsigned kva, size_t n)
 *
 * Invalidate an address range in secondary instruction cache
 */
LEAF(r4k_clean_sicache_nowrite)
#if R4KSCACHE
	SIZE_CACHE(a2,mips_scache_linesize)
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
#endif	
9:	j	ra
END(r4k_clean_sicache_nowrite)


/*
 * void r4k_hit_writeback_inv_dcache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(r4k_hit_writeback_inv_dcache)
	SIZE_CACHE(a2,mips_dcache_linesize)
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)
9:	j	ra
END(r4k_hit_writeback_inv_dcache)
	
/*
 * void r4k_hit_writeback_inv_scache (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in secondary cache
 */
LEAF(r4k_hit_writeback_inv_scache)
	SIZE_CACHE(a2,mips_scache_linesize)
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
9:	j	ra
END(r4k_hit_writeback_inv_scache)
	
#endif /* _SDE_CACHE_DIAG */
	
#endif /* #cache(r4k) */
