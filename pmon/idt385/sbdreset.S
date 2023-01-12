/* $Id: sbdreset.S,v 1.2 1996/01/16 14:24:46 chris Exp $ */
/* idt385/sbdreset.s - board-dependent startup code for PMON
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restiction.
 */

#include "mips.h"
#include "pmon.h"
#include "r3041.h"
#include "r3081.h"

	.globl	sbdreset
	.ent	sbdreset
sbdreset:
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

	j	ra
	.end	sbdreset


	.comm wbfltmp,4

	.globl wbflush
	.ent wbflush
wbflush:
	/* On an R30x1 any uncached read will flush the write buffer */
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,(t0)
	j	ra
	.end wbflush



	.lcomm	buserr_cnt,4

	.ent buserr
	.globl buserr
buserr:
	.set noat
	la	k1,buserr_cnt
	lw	k0,0(k1)	# increment buserr_cnt
	addu	k0,1
	sw	k0,0(k1)
	.set noreorder
	mfc0	k1,C0_EPC
	la	k0,buserr
	addu	k1,4		# skip offending instruction
	j	k1
	rfe
	.set reorder
	.set at
	.end buserr


	/* Return number of bus error faults. */
	.globl	sbdberrcnt
	.ent	sbdberrcnt
sbdberrcnt:
	lw	v0,buserr_cnt
	j	ra
	.end	sbdberrcnt

	
	/* Enable/disable bus error faults. */
	.globl	sbdberrenb
	.ent	sbdberrenb
sbdberrenb:
	seq	v0,k0,zero		# currently "enabled" if k0==0
	bnez	a0,1f
	la	k0,buserr		# !enb: ignore exceptions
	b	2f
1:	move	k0,zero			# enb: let exceptions fault
2:	sw	zero,buserr_cnt
	j	ra
	.end	sbdberrenb
