/*
 * cache.s: switchable cache support for SDE-MIPS kit
 * Copyright (c) 1998 Algorithmics Ltd
 *
 * The function of this module is to support boards which can handle
 * multiple MIPS cpus with different cache architectures.
 */	
	
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/prid.h>

#if defined(IN_PMON) || defined(ITROM)
#define DECL(x, val)		\
	BSS(x,4)
#else	
#define DECL(x, val)		\
	.sdata;			\
	EXPORTS(x, 4);		\
	.word	val
#endif	

DECL(mips_icache_size,-1)
DECL(mips_icache_linesize,-1)
DECL(mips_icache_ways,1)
	
DECL(mips_dcache_size,-1)
DECL(mips_dcache_linesize,-1)
DECL(mips_dcache_ways,1)
	
DECL(mips_scache_size,-1)
DECL(mips_scache_linesize,-1)
DECL(mips_scache_ways,1)
	
/* R4000 specials */	
DECL(mips_scache_split,0)
DECL(mips_scache_discontig,0)
	
DECL(mips_tcache_size,-1)
DECL(mips_tcache_linesize,-1)
DECL(mips_tcache_ways,1)

#if #cache(r5k)+#cache(rm7k)+#cache(r5400)+#cache(lr30)+#cache(cw)+#cache(r4k)+#cache(r3k) > 1
# define GETPRID 		\
	mfc0	t0,$15;		\
	srl	t0,8;		\
	and	t0,0xff;
# define SWTEST(prid,dest) 	\
	beq	t0,prid,dest;
#else	
# define GETPRID
# define SWTEST(prid,dest) 	\
	b	dest;
#endif	
	
#if #cache(r5k)
#define R5KSWITCH(x)	SWTEST(PRID_R5000,r5k_/**/x) \
			SWTEST(PRID_RM52XX,r5k_/**/x)
#else
#define R5KSWITCH(x)
#endif
	
#if #cache(rm7k)
#define RM7KSWITCH(x)	SWTEST(PRID_RM7000,rm7k_/**/x)
#else
#define RM7KSWITCH(x)
#endif
	
#if #cache(r5400)
#define R54SWITCH(x)	SWTEST(PRID_R5400,r54_/**/x) \
			SWTEST(PRID_R5500,r54_/**/x)
#else
#define R54SWITCH(x)
#endif
	
#if #cache(lr30) + #cache(cw) + #cache(r4k) + #cache(r3k) > 1
#error Only one of lr30, cw, r4k or r3k should be asserted for -Acache
#endif	
	
/* LSI LR33K is a default fall-through if enabled */
#if #cache(lr30)
#define LR30SWITCH(x)	b	lr30_/**/x;
#else
#define LR30SWITCH(x)
#endif
	
/* LSI CW is a default fall-through if enabled */
#if #cache(cw)
#define CWSWITCH(x)	b	cw_/**/x;
#else
#define CWSWITCH(x)
#endif
			
/* Vanilla R4000 is a default fall-through if enabled */
#if #cache(r4k)
#define R4KSWITCH(x)	b	r4k_/**/x;
#else
#define R4KSWITCH(x)
#endif
	
/* Vanilla R3000 is a default fall-through if enabled */
#if #cache(r3k)
#define R3KSWITCH(x)	b	r3k_/**/x;
#else
#define R3KSWITCH(x)
#endif
	
#define SWCACHE(name) 		\
LEAF(mips_/**/name); 		\
	GETPRID			\
	R5KSWITCH(name)		\
	RM7KSWITCH(name)	\
	R54SWITCH(name)		\
	CWSWITCH(name)		\
	LR30SWITCH(name)		\
	R4KSWITCH(name)		\
	R3KSWITCH(name)		\
	j	$ra;		\
END(mips_/**/name)
	
	
	
/* 
 * void mips_init_cache (void)
 *
 * Size and (re)initialise caches from scratch; usually only called
 * once during power on reset.
 *
 * NOTE: loses any dirty lines in writeback caches
 */
SWCACHE(init_cache)
	
/* 
 * void mips_size_cache (void)
 *
 * Size caches without reinitialising and losing dirty cache lines.
 */
SWCACHE(size_cache)
	
/* 
 * void mips_flush_cache (void)
 *
 * Writeback and invalidate all caches in the quickest way possible.
 */
SWCACHE(flush_cache)
	
/* 
 * void mips_flush_dcache (void)
 *
 * Writeback and invalidate only the data caches.  
 * Joint caches (i.e. combined I & D) will be flushed too. 
 */
SWCACHE(flush_dcache)
	
/* 
 * void mips_flush_icache (void)
 *
 * Writeback and invalidate only the instruction caches.  
 * Joint caches (i.e. combined I & D) will be flushed too. 
 */
SWCACHE(flush_icache)
	
/* 
 * void mips_clean_cache (vaddr_t va, unsigned int size)
 *
 * Writeback and invalidate a virtual address range in all caches.
 */
SWCACHE(clean_cache)
	
/* 
 * void mips_clean_dcache (vaddr_t va, unsigned int size)
 *
 * Writeback and invalidate a virtual address range in data caches.
 * Joint caches (i.e. combined I & D) will be cleaned too. 
 */
SWCACHE(clean_dcache)
	
/* 
 * void mips_clean_icache (vaddr_t va, unsigned int size)
 *
 * Writeback and invalidate a virtual address range in instruction caches.
 * Joint caches (i.e. combined I & D) will be cleaned too. 
 */
SWCACHE(clean_icache)
