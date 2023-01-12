/* $Id: sbdreset.S,v 1.1 1996/06/28 12:29:54 nigel Exp $ */
/*
** Copyright 1994 Algorithmics Ltd
**	All Rights Reserved
**
** libraio/sbdreset.S -- low level board dependent routines
*/

#include "mips.h"
#include "pmon.h"
#ifdef R3081	
#include "r3081.h"
#endif	
#ifdef R3041	
#include "r3041.h"
#endif	
#include "sbd.h"

/*
 * lowlevel initialisation called at ROM startup and reinitialisation
 * All registers can be used but no memory!
 */

	.globl	sbdreset
	.ent	sbdreset
sbdreset:
	subu	sp,24
	sw	ra,16(sp)
	
#ifdef R3081	
	/* r3081: set config register to 8k+8k cache */
	mfc0	a0,C0_CONFIG
	or	a0,CFG_AC
	mtc0	a0,C0_CONFIG
#endif
		
#ifdef R3041
	/* r3041: initialise bus control register */
	li	a0,BUSC_LOCK | BUSC_SET |			\
			(BUSC_CTL_WR   << BUSC_MEM_SH) |	\
			(BUSC_CTL_RDWR << BUSC_ED_SH) |		\
			(BUSC_CTL_RDWR << BUSC_IO_SH) |		\
			BUSC_BE16 | BUSC_BE | BUSC_BTA_0;
	mtc0	a0,C0_BUSCTRL

	/* r3041: set portsize register (all 32-bit) */
	li	a0,PRTSZ_LOCK
	mtc0	a0,C0_PORTSIZE

	/* r3041: set cacheconfig register (data block refill on) */
	mfc0	a0,C0_CACHECFG	# dont upset r3081 config register
	or	a0,CCFG_DBR
	mtc0	a0,C0_CACHECFG
#endif
		
	/* table driven hardware register initialisation */
	la	a0, sbditab
	or	a0, K1BASE		/* force to kseg1 */

1:	lw	v0,0(a0)
	lw	v1,4(a0)
	addu	a0,8
	beqz	v0,9f
	sw	v1,0(v0)
	b	1b
	
9:	
	lw	ra,16(sp)
	addu	sp,24
	j	ra
	.end	sbdreset

	
	.globl sbddelay
	.ent sbddelay
sbddelay:
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far
	.set	noreorder	
1:	bgtz	a0,1b
	subu	a0,1
	.set	reorder
	j	ra
	.end sbddelay
	
	.lcomm wbfltmp,4
	.globl wbflush
	.ent wbflush
wbflush:
	/* On an R30x1 any uncached read will flush the write buffer */
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,(t0)
	j	ra
	.end wbflush

	
	
	.globl	sbd_getfpaintr	
	.ent	sbd_getfpaintr	
sbd_getfpaintr:	
#ifdef R3081
	mfc0	v0,C0_CONFIG
	and	v0,CFG_FPINT_MASK
	srl	v0,CFG_FPINT_SHIFT
	addu	v0,2		# convert to interrupt number
#else	
	li	v0,-1		# No FPU on this chip
#endif
	j	ra
	.end	sbd_getfpaintr	
