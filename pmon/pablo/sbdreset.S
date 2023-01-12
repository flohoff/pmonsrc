/*
** Copyright 1996 Algorithmics Ltd
**	All Rights Reserved
**
** pablo/sbdreset.S -- low level board dependent routines for UBI Pablo
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
	
#if defined(R3081) && defined(ALTCACHE)
	/* 8k I + 8k D caches */
	mfc0	a0,C0_CONFIG
	or	a0,CFG_AC
	mtc0	a0,C0_CONFIG
#endif /* R3081 */
		
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
	
	bal	fpgaload
					
	lw	ra,16(sp)
	addu	sp,24
	j	ra
	.end	sbdreset

	
/* Load FPGA firmware via JTAG interface */
	.globl	fpgaload
	.ent	fpgaload
fpgaload:	
	/* get address of jtag data table */
	la	a0,PA_TO_KVA1(FPGAROM_BASE)

	/* get jtag data size and point to end of table */
	li	a1,FPGAROM_SIZE*4
	addu	a1,a0

	/* get address of fpga jtag data port */
	la	a2,PA_TO_KVA1(FPGA_BASE)
	move	v1,zero		
	
#define	fpga_bis(x) \
	or	v1,x; sh	v1,0(a2)
#define	fpga_bic(x) \
	and	v1,~(x); sh	v1,0(a2)
	
	fpga_bis(FPGA_TCK)	# active low reg, set clockline low

	/* hold FPGA in reset */
	fpga_bis(FPGA_RST)

.byteloop:	
	lhu	v0,0(a0)
	/* register is active low, invert the bits now */
	not	v0
	li	t0,4
.bitloop:	
	/* stuff two data bits into pio */
	fpga_bic(FPGA_DMASK)
	and	t1,v0,0x3
	sll	t1,FPGA_DSHFT
	fpga_bis(t1)
		
	/* toggle clock line */
	fpga_bic(FPGA_TCK)	# active low (going up)
	fpga_bis(FPGA_TCK)
	
	/* get next bits */
	subu	t0,1
	srl	v0,2
	bnez	t0,.bitloop

	/* get next byte */
	addu	a0,4
	bltu	a0,a1,.byteloop	
	
	/* clock 8 lots of zero through to be safe */
	fpga_bic(FPGA_DMASK);
	li	t0,8
1:	fpga_bis(FPGA_TCK);
	fpga_bic(FPGA_TCK);
	subu	t0,1
	bnez	t0,1b

        /* 5ms delay before removing RESET */			
	li	t0,ROMMS*5
	.set	noreorder	
1:	bgtz	t0,1b
	subu	t0,1
	.set	reorder	
	
	/* clear the FPGA reset */
	fpga_bic(FPGA_RST);
	
	j	ra
	.end
	
	
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
	nop
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
