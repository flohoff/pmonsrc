/* $Id: cflu.S,v 1.2 1998/10/06 14:16:58 nigel Exp $ */
#ifdef R4000

#include "mips.h"
#include "mips/prid.h"

/*
 * R4000/R5000 cache operations.
 */

#define DECL(x, val)	\
EXPORTS(x, 4)		\
	.word	val

	.sdata
DECL(mips_icache_size,-1)
DECL(mips_icache_linesize,-1)
DECL(mips_icache_ways,1)
	
DECL(mips_dcache_size,-1)
DECL(mips_dcache_linesize,-1)
DECL(mips_dcache_ways,1)
	
DECL(mips_scache_size,-1)
DECL(mips_scache_linesize,-1)
DECL(mips_scache_ways,1)
DECL(mips_scache_split,0)
DECL(mips_scache_discontig,0)
	
DECL(mips_tcache_size,-1)
DECL(mips_tcache_linesize,-1)
DECL(mips_tcache_ways,1)
	
#define SWITCH(name) \
LEAF(mips_/**/name); \
	la	$t1, R4K_/**/name; \
	la	$t2, r5k_/**/name; \
	la	$t3, rm7k_/**/name; \
	b	cache_switch; \
END(mips_/**/name)
	
SWITCH(size_cache)
SWITCH(init_cache)
SWITCH(flush_cache)
SWITCH(flush_dcache)
SWITCH(clean_cache)
SWITCH(clean_dcache)
#if 0
SWITCH(iaflush)
#endif
		

SLEAF(cache_switch)
	mfc0	$t0,C0_PRID
	srl	$t0,8
	and	$t0,0xff
	
	/* R5000 */
	.set	noreorder
	beq	$t0,PRID_R5000,1f
	move	$t8,$t2
	.set	reorder
	
	/* RM52xx */
	.set	noreorder
	beq	$t0,PRID_RM52XX,1f
	move	$t8,$t2
	.set	reorder
	
	/* RM7000 */
	.set	noreorder
	beq	$t0,PRID_RM7000,1f
	move	$t8,$t3
	.set	reorder
	
	/* R4000 (default) */
	move	$t8,$t1

1:	and	$t0,$ra,0xa0000000
	or	$t8,$t0
	j	$t8
SEND(cache_switch)
			
#endif /* R4000 */
