/* $Id: r4kcflu.S,v 1.6 1997/09/01 14:46:03 nigel Exp $ */
#ifdef R4000

#include "mips.h"

/*
 * R4000 cache operations.
 *
 * The _flush and _clean functions are complex composites that do whatever
 * is necessary to flush/clean ALL caches, in the quickest possible way.
 * The other functions are targetted explicitly at a particular cache
 * I, D or SD; it is up to the user to call the correct set of functions
 * for a given system.
 */

#define SCACHE_MIN_SIZE	0x010000	/* minimum scache: 64Kb */
#define SCACHE_MAX_SIZE	0x400000	/* maximum scache:  4Mb */
	
	.comm	icache_size,4
	.comm	icache_linesize,4
	.comm	dcache_size,4
	.comm	dcache_linesize,4
	.comm	scache_size,4
	.comm	scache_linesize,4
	.comm	scache_split,4
	.comm	pcache_2way,4

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
	/* check for no cache */	\
	blez	linesize,11f ;		\
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

#define icachesize	t2
#define dcachesize	t3
#define scachesize	t4
#define ilinesize	t5
#define dlinesize	t6
#define slinesize	t7
#define pcache2way	t8
#define scachesplit	v1

	.ent	_size_cache
_size_cache:
	.set noreorder
	mfc0	t0,C0_CONFIG
	nop; nop
	.set reorder

	and	t1,t0,CFG_K0MASK
	bne	t1,CFG_C_UNCACHED,1f
	
	move	icachesize,zero
	move	dcachesize,zero
	move	scachesize,zero
	move	ilinesize,zero
	move	dlinesize,zero
	move	slinesize,zero
	move	pcache2way,zero
	move	scachesplit,zero
	j	ra
	
1:	mfc0	t1,C0_PRID
	srl	t1,8
	bne	t1,0x0c,1f
	li	icachesize,2048
	li	ilinesize,16
	li	dcachesize,1024
	li	dlinesize,16
	b	.checkscache
	
	/* work out primary i-cache size */
1:	and	t1,t0,CFG_ICMASK
	srl	t1,CFG_ICSHIFT
	li	icachesize,0x1000
	sll	icachesize,t1

	/* work out primary i-cache line size */
	li	ilinesize,32
	and	t1,t0,CFG_IB
	bnez	t1,1f
	li	ilinesize,16
1:	

	/* work out primary d-cache size */
	and	t1,t0,CFG_DCMASK
	srl	t1,CFG_DCSHIFT
	li	dcachesize,0x1000
	sll	dcachesize,t1

	/* work out primary d-cache line size */
	li	dlinesize,32
	and	t1,t0,CFG_DB
	bnez	t1,1f
	li	dlinesize,16
1:	
.checkscache:
	move	scachesize,zero
	move	slinesize,zero	
	move	scachesplit,zero	
	
	/* no secondary cache if Config.SC = 1 */
	and	t1,t0,CFG_SC
	bnez	t1,9f
	
	/* note split cache */
	and	scachesplit,t0,CFG_SS
	
	/* work out secondary cache line size */
	and	t1,t0,CFG_SBMASK
	srl	t1,CFG_SBSHIFT
	li	slinesize,16
	sll	slinesize,t1

	/* disable all i/u and cache exceptions */
	.set	noreorder
	mfc0	t9,C0_SR
	nop 
	li	t8,SR_BEV
	and	t8,t9
	or	t8,SR_DE
	mtc0	t8,C0_SR
	nop; nop; nop
	
	/* set initial zero marker */
	mtc0	zero,C0_TAGLO			# initial cache tag
	li	t8,K0BASE
	cache	Index_Store_Tag_SD,0(t8)	# sdcache[0].tag = 0
	
	/* check all secondary cache boundaries, until we wraparound */
	li	scachesize,SCACHE_MIN_SIZE
	li	t0,SCACHE_MAX_SIZE

2:	mtc0	scachesize,C0_TAGLO
	addu	t1,t8,scachesize		# calc &sdcache[size]
	cache	Index_Store_Tag_SD,0(t1)	# sdcache[size].tag = size
	nop; nop
	cache	Index_Load_Tag_SD,0(t8)		# check sdcache[0].tag == 0
	nop
	
	mfc0	t1,C0_TAGLO
	nop; nop
	and	t1,TAG_STAG_MASK
	bnez	t1,8f				# wrap around, got it
	nop
	sll	scachesize,1			# try next boundary
	bne	scachesize,t0,2b		# up to maximum size
	nop
	
8:	mtc0	t9,C0_SR
	nop; nop; nop
	
9:	mfc0	t0,C0_PRID			# get processor ID
	nop; nop
	and	t0,0xff00			# get implementation
	.set	reorder
	seq	pcache2way,t0,0x2000		# is it an r4600
	seq	t1,t0,0x2200			# is it an r4650
	or	pcache2way,t1		
	seq	t1,t0,0x2800			# is it an rm52xx
	or	pcache2way,t1		
	j	ra
	.end	_size_cache


/*
 * void size_cache()
 * 
 * Work out size of I, D & S caches
 */
	.globl	size_cache
	.ent	size_cache
size_cache:
	lw	t0,icache_size
	move	v0,ra
	bgtz	t0,8f				# already known?
	bal	_size_cache
	move	ra,v0
	sw	icachesize,icache_size
	sw	dcachesize,dcache_size
	sw	scachesize,scache_size
	sw	ilinesize,icache_linesize
	sw	dlinesize,dcache_linesize
	sw	slinesize,scache_linesize
	sw	pcache2way,pcache_2way
	sw	scachesplit,scache_split
8:	j	ra
	.end	size_cache

/*
 * void init_cache()
 */
	.globl	init_cache
init_cache:
	li	a0,K0BASE
	/* drop through */
	

/*
 * void init_cache_from()
 * 
 * Work out size of and initialize I, D & S caches.
 *
 * NOTES
 *  1) assumes enough DRAM has been initialised with correct parity.
 */
	.globl	init_cache_from
	.ent	init_cache_from
init_cache_from:
	/*
 	 * First work out the sizes
	 */
	move	v0,ra
	bal	_size_cache
	move	ra,v0
	
	/*
	 * The caches may be in an indeterminate state,
	 * so we force good parity into them by doing an
	 * invalidate, load/fill, invalidate for each line.
	 */

	/* disable all i/u and cache exceptions */
	.set noreorder
	mfc0	v0,C0_SR
	nop
	li	a1,SR_BEV
	and	a1,v0
	or	a1,SR_DE
	mtc0	a1,C0_SR
	nop; nop; nop
	
	mtc0	zero,C0_TAGLO			# initial cache tag 
	nop
	.set reorder

	/*
	 * Is there any cache present?
	 */
	blez	dcachesize,3f

	/* 
	 * Initialise secondary cache tags (if present).
	 */
	blez	scachesize,4f			# scache present?
	
	/* first data/unified tags */
	li	a2,K0BASE
	addu	a1,a2,scachesize	        # limit = base + scachesize 
	.set	noreorder
1:	addu	a2,slinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_SD,-4(a2)	# BDSLOT: clear tag
	.set	reorder
	
	beqz	scachesplit,4f			# scache split?
	
	/* then split icache */
	li	a2,K0BASE
	.set	noreorder
1:	addu	a2,slinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_SI,-4(a2)	# BDSLOT: clear tag
	.set	reorder
4:		
	/* 
	 * Initialise primary instruction cache from memory/secondary
	 */

	.set	noreorder
	move	a2,a0
	addu	a1,a2,icachesize		# limit = base + icachesize 
1:	addu	a2,ilinesize
	cache	Index_Store_Tag_I,-4(a2)	# clear tag
	nop
	cache	Fill_I,-4(a2)			# fill line
	nop
	bne	a2,a1,1b
	cache	Index_Store_Tag_I,-4(a2)	# BDSLOT: clear tag
	.set	reorder

	/* 
	 * Initialise primary data cache.
	 * (for R4600 2-way set caches, we do it in 3 passes).
	 */

	/* 1: initialise dcache tags */
	.set	noreorder
	move	a2,a0
	addu	a1,a2,dcachesize        	# limit = base + dcachesize 
1:	addu	a2,dlinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_D,-4(a2)	# BDSLOT: clear tag
	.set	reorder

	/* 2: fill dcache */
	.set	noreorder
	move	a2,a0
1:	addu	a2,dlinesize
	bne	a2,a1,1b
	lw	zero,-4(a2)			# BDSLOT: fill line
	.set	reorder

	/* 3: clear dcache tags */
	.set	noreorder
	move	a2,a0
1:	addu	a2,dlinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_D,-4(a2)	# BDSLOT: clear tag

	/*
	 * Initialise the secondary data cache data array
	 */
	blez	scachesize,3f			# scache present?
	li	a2,K0BASE			# BDSLOT
	
	addu	a1,a2,scachesize	        # a1 = base + scachesize 
	
	/* create dirty lines in primary and secondary */
1:	cache	Create_Dirty_Exc_SD,0(a2)
	nop; nop
	
	cache	Create_Dirty_Exc_D,0(a2)
	nop; nop

	/* set W-bit in primary cache line */
	sw	zero,0(a2)
	nop; nop
	
	/* move primary cache line to secondary */
	cache	Hit_Writeback_Inv_D,0(a2)
	nop; nop
	
	/* reset secondary tag */
	addu	a2,dlinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_SD,-4(a2)	# BDSLOT: clear tag
	
	/*
	 * Initialise the secondary instruction cache data array
	 */
	beqz	scachesplit,3f			# scache split?
	li	a2,K0BASE			# BDSLOT

	/* fill primary Icache from secondary (ignoring ECC) */
1:	cache	Fill_I,0(a2)
	nop; nop
	
	/* write primary Icache to secondary (setting ECC) */
	cache	Hit_Writeback_I,0(a2)
	nop; nop
	
	/* reset secondary tag */
	addu	a2,ilinesize
	bne	a2,a1,1b
	cache	Index_Store_Tag_SI,-4(a2)	# BDSLOT: clear tag
	.set	reorder

	/* we store the sizes only after the caches are initialised */
3:	sw	icachesize,icache_size
	sw	dcachesize,dcache_size
	sw	scachesize,scache_size
	sw	ilinesize,icache_linesize
	sw	dlinesize,dcache_linesize
	sw	slinesize,scache_linesize
	sw	pcache2way,pcache_2way
	sw	scachesplit,scache_split
	.set noreorder
	mtc0	v0,C0_SR
	nop
	.set reorder
	j	ra
	.end	init_cache_from

/*
 * void r4k_flush (void)
 *
 * Flush and invalidate all caches
 */
	.globl	r4k_flush
	.ent	r4k_flush
r4k_flush:
	/* secondary cacheops do all the work if present */
	lw	a1,scache_size
	lw	a2,scache_linesize
	li	a0,K0BASE
	blez	a1,9f
	lw	v1,scache_split
	
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	
	beqz	v1,2f			# split scache?
	cacheop(a0,a1,a2,Index_Invalidate_SI)
	
	b	2f

	/* else flush primary caches individually */
9:	lw	a1,dcache_size
	lw	a2,dcache_linesize
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)

	lw	a1,icache_size
	lw	a2,icache_linesize
	cacheop(a0,a1,a2,Index_Invalidate_I)

2:	j	ra
	.end	r4k_flush


/*
 * void r4k_dflush (void)
 *
 * Flush and invalidate data cache only
 */
	.globl	r4k_dflush
	.ent	r4k_dflush
r4k_dflush:
	/* use secondary cacheops if present */
	lw	a1,scache_size
	lw	a2,scache_linesize
	li	a0,K0BASE
	blez	a1,1f
	cacheop(a0,a1,a2,Index_Writeback_Inv_SD)
	b	2f

	/* else flush primary data cache */
1:	lw	a1,dcache_size
	lw	a2,dcache_linesize
	cacheop(a0,a1,a2,Index_Writeback_Inv_D)

2:	j	ra
	.end	r4k_dflush

	
/*
 * void r4k_clean (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in all caches
 */
	.globl	r4k_clean
	.ent	r4k_clean
r4k_clean:
	/* secondary cacheops do all the work (if fitted) */
	lw	a2,scache_linesize
	lw	v1,scache_split
	blez	a2,9f	
	
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	
	beqz	v1,2f			# split scache?
	vcacheop(a0,a1,a2,Hit_Invalidate_SI)
	
	b	2f

9:	lw	a2,dcache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)

	lw	a2,icache_linesize
	vcacheop(a0,a1,a2,Hit_Invalidate_I)

2:	j	ra
	.end	r4k_clean


/*
 * void r4k_dclean (unsigned kva, size_t n)
 *
 * Writeback and invalidate address range in data caches
 */
	.globl	r4k_dclean
	.ent	r4k_dclean
r4k_dclean:
	/* secondary cacheops do all the work (if fitted) */
	lw	a2,scache_linesize
	blez	a2,1f
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_SD)
	b	2f

1:	lw	a2,dcache_linesize
	vcacheop(a0,a1,a2,Hit_Writeback_Inv_D)

2:	j	ra
	.end	r4k_dclean

		

/*
 * void r4k_iaflush (unsigned kva)
 *
 * Writeback and invalidate line containing address in all caches
 */
	.globl	r4k_iaflush
	.ent	r4k_iaflush
r4k_iaflush:
	/* secondary cacheops do all the work (if fitted) */
	lw	a2,scache_linesize
	and	a0,~7			# dword align
	blez	a2,1f
	cache	Hit_Writeback_Inv_SD,0(a0)
	cache	Hit_Invalidate_SI,0(a0)
	b	2f

1:	lw	a1,icache_size
	blez	a1,2f
	cache	Hit_Writeback_Inv_D,0(a0)
	cache	Hit_Invalidate_I,0(a0)

2:	j	ra
	.end	r4k_iaflush
	
#endif /* R4000 */
