/*
 * CCUBE/sbdreset.sx: low level code for Algorithmics CCUBE module
 * Copyright (c) 1998 Algorithmics Ltd.
 */

#ifdef IN_PMON
#include <mips.h>
#include <pmon.h>
#include <r3041.h>
#include <ccube/sbd.h>
#else		
#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/cpu.h>

#include <idt/r3041.h>
#include "sbd.h"
#endif	

#ifndef IN_PMON
/* 
 * Include standard memory test/clear code.
 */
#define BUSWIDTH	4		/* 32-bit, non-interleaved */
#include "../share/memprobe.s"
#endif

/*
 * Basic board initialisation, called straight from RESET
 * It is only called if this program is built for ROM.  
 * It can use any registers except s8, k0 and k1.
 *
 * Note that s8 holds a "relocation factor" (see ../share/romlow.sx)
 * which must be added to any address before it is used.  This
 * is to support relocatable roms.
 */
	
#define	RELOC		s8
	
#ifdef IN_PMON
#define SBDRESET	sbdreset
#else
#define SBDRESET	_sbd_reset
#endif			
LEAF(SBDRESET)
	move	s7,ra

	/* initialise status register to SR_BEV (setting SR_PE clears it) */
	li	a0,SR_BEV|SR_PE
	mtc0	a0,C0_SR
	mtc0	zero,C0_CAUSE

	/* setting these r3041 registers won't hurt an r3081, except $cacheconfig */

	/* r3041: initialise bus control register */
	li	a0,BUSC_LOCK | BUSC_SET |			\
			(BUSC_CTL_HIGH << BUSC_MEM_SH) |	\
			(BUSC_CTL_HIGH << BUSC_ED_SH) |		\
			(BUSC_CTL_HIGH << BUSC_IO_SH) |		\
			BUSC_BE16 | BUSC_BTA_0
	mtc0	a0,C0_BUSCTRL

	/* r3041: set portsize register (all 16-bit) */
	li	a0,PRTSZ_LOCK | \
		(PRTSZ_16 << PRTSZ_KSEG2B_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG2A_SH) | \
		(PRTSZ_16 << PRTSZ_KUSEGD_SH) | \
		(PRTSZ_16 << PRTSZ_KUSEGC_SH) | \
		(PRTSZ_16 << PRTSZ_KUSEGB_SH) | \
		(PRTSZ_16 << PRTSZ_KUSEGA_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1H_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1G_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1F_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1E_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1D_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1C_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1B_SH) | \
		(PRTSZ_16 << PRTSZ_KSEG1A_SH)

	mtc0	a0,C0_PORTSIZE

	/* r3041: set cacheconfig register (data block refill on) */
	mfc0	a0,C0_CACHECFG		# preserve r3081 initial state
	or	a0,CCFG_DBR		
	mtc0	a0,C0_CACHECFG

	/* Program resets */
	li	a0,PA_TO_KVA1(MBREG_BASE)
	sh	zero,0(a0)
	sh	zero,2(a0)
	.set	noreorder
	li	t0,ROMMS(1)
1:	bnez	t0,1b
	subu	t0,1
	.set	reorder

	li	a1,0x300
	sh	a1,2(a0)
	.set	noreorder
	li	t0,ROMMS(1)
1:	bnez	t0,1b
	subu	t0,1
	.set	reorder
	
	
	/* Register initialisation */
1:	la	a0,reginittab
	addu	a0,RELOC
1:	lw	v0,0(a0)	# reg addr
	lw	v1,4(a0)	# reg data
	beqz	v0,2f
	sh	v1,0(v0)
	addu	a0,8
	b	1b
2:
	
#ifndef IN_PMON
	/* determine memory size */
	bal	size_mem
#endif

	/* no need to clear mem, it has no parity/ecc */

	/* return to generic reset, with memory size */
	move	ra,s7
	j	ra
END(SBDRESET)


#ifdef IN_PMON
LEAF(sbddelay)
#ifdef R3000
	li	t1,CACHECYCLE
#else
	mfc0	t0,C0_CONFIG
	li	t1,CACHEMISS+ROMCYCLE
	and	t0,CFG_K0MASK
	beq	t0,CFG_C_UNCACHED,1f
	and	t0,ra,0x20000000
	bnez	t0,1f
	li	t1,CACHECYCLE
#endif
1:	mul	a0,1000
	addu	a0,t1
	sll	t1,1
	divu	a0,t1
	subu	a0,48		# approx number of loops so far
	.set	noreorder	
	.set	nomacro
	nop
2:	bgtz	a0,2b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(sbddelay)

	.lcomm wbfltmp,4
LEAF(wbflush)
	/* On an R30x1 any uncached read will flush the write buffer */
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,(t0)
	j	ra
END(wbflush)
		
#endif
	

	
#define INIT(addr,val) \
	.word	PA_TO_KVA1(addr), val

#define INITGTX(addr,val) \
	INIT(AVIAGTX_BASE+0x400000+(addr),val) \
	
	.rdata
reginittab:
	/* This is proprietary, so it doesn't get documented... */
	INITGTX(0x100,0xfbff)
	INITGTX(0x104,0x0050)
		
	.word	0,0

		
