/* $Id: sbdreset.S,v 1.2 1996/01/16 14:25:06 chris Exp $ */
/* lsipr/sbdreset.s - board-dependent startup code for PMON
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restiction.
 */

#include "mips.h"
#include "pmon.h"
#include "lr33000.h"

CACHE_CFG = (CR_IBLK_4|CR_DBLK_4)
WAIT_CFG = ((6<<CR_IOWAITSHFT)|(6<<CR_PWAITSHFT))
MEM_CFG = (CR_RDYGEN|CR_RFSHEN|CR_DRAMEN)

	.globl	sbdreset
	.ent	sbdreset
sbdreset:
	move	s0,ra

	# turn on the RAM
	li	t0,(CACHE_CFG|WAIT_CFG|MEM_CFG)
	sw	t0,M_CFGREG

	li	t0,375			# 512 rows in 8ms = 15us
	sw	t0,M_RTIC

	# flush the dcache
	li      a0,DCACHE
	jal     flush_cache

	move	ra,s0
	j	ra
	.end	sbdreset


	.comm wbfltmp,4

	.globl wbflush
	.ent wbflush
wbflush:
	la	t0,wbfltmp
	or	t0,K1BASE
	sw	zero,(t0)
	lw	zero,(t0)
	j	ra
	.end wbflush

