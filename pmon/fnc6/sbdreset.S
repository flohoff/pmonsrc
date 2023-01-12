/* ctech/sbdreset.S - low level board dependent startup code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include "sbd.h"

#define DISPLAY(d0,d1,d2,d3)			\
	li	t8,PA_TO_KVA1(LED_BASE);	\
	li	t9,d0; 				\
	sw	t9,LED(0)(t8); 			\
	li	t9,d1; 				\
	sw	t9,LED(1)(t8);	 		\
	li	t9,d2;	 			\
	sw	t9,LED(2)(t8);	 		\
	li	t9,d3;	 			\
	sw	t9,LED(3)(t8)

#define MEG	0x100000		

/* 
 * Include standard memory test/clear code.
 */
#define BUSWIDTH	4		/* 32-bit, non-interleaved */
#define MEMSTART	(32*MEG)	/* simm1 start */
#define MEMINCR		(8*MEG)		/* simm1 quanta */
#include "meminit.s"

#define	softptr	s2
#define	crptr	s3
#define	crval	s4
#define msize	s5
#define	rasave	s6
#define	RELOC	s8

#define SOFTC_STORE(offs,reg) \
	sb	reg,offs(softptr)

#define SOFTC_LOAD(offs,reg) \
	lbu	reg,offs(softptr)

LEAF(sbdreset)
	move	rasave,ra

	/* if launched by ITROM, leave Config alone */
#ifndef ITBASE	
	/* set config register for 32b/32b cachelines, kseg0 cacheable */
	mfc0	t1,C0_CONFIG
	and	t1,~0x3f		# set bits 5..0 only
	or	t1,CFG_IB | CFG_DB | CFG_C_NONCOHERENT
	/* On the R4300 we also have control over the endianness */
#ifdef MIPSEB
	or	t1,CFG_BE	
#else
	and	t1,~CFG_BE	
#endif
	mtc0	t1,C0_CONFIG
#endif	

	/*
	 * slight amount of kludgery here to stop RAM resident
	 * program from overwriting itself...
	 */
	li	v1,0x1fc00000		/* check return address is in ROM */
	and	v0,ra,v1
	bne	v0,v1,.noinit

	/* initialise board control register */
	li	crptr,PA_TO_KVA1(BCR_BASE)
	
	/* switch on LED, and set up i/o endianness */
#ifdef MIPSEB
	li	crval,BCR_LED_ON
#else
	li	crval,BCR_LED_ON|BCR_IO_LE
#endif
	bal	crwrite

	li	softptr,SBD_SOFTC
	SOFTC_STORE(S_BCR, crval)

	/* initialise I/O registers */
1:	la	a0,reginittab
	addu	a0,RELOC
1:	lw	v0,0(a0)		# reg addr
	lw	v1,4(a0)		# reg data
	beqz	v0,2f
	sw	v1,0(v0)
	addu	a0,8
	b	1b
2:

	/* skip memory size/clear if a soft reset */
	mfc0	t1,C0_STATUS
	and	t1,SR_SR
	bnez	t1,.noinit

	/*
	 * Determine DRAM bank arrangement and size
	 */
#define aces	t0
#define fives	t1
#define base	t2
#define tval	t3
#define simmmap	t4
#define   SIMM0_0	0x01	
#define   SIMM0_1	0x02
#define   SIMM1_0	0x04	
#define   SIMM1_1	0x08	
#define msize0	v0
#define msize1	v1
	
	/* initialise DCR memory speed and type from option links */
	lw	crval,PA_TO_KVA1(OPTION_BASE)
	and	crval,DCR_SIMM1_DRAM|DCR_TYPE|DCR_DRAMFAST

	/* initialise registers */
	move	msize0,zero
	move	msize1,zero
	li	aces,0xaaaaaaaa
	not	fives,aces
	li	base,PA_TO_KVA1(0)
	
	/* make sure memory has started up properly */
	/* configure for 4Mb double sided and read at 4Mb intervals */
	or	crval,DCR_4MB
	DISPLAY ('D','C','R','I')
	li	crptr,PA_TO_KVA1(DCR_BASE)
	bal	crwrite
	
	DISPLAY ('M','E','M','I')
	li	tval,8
.initmem:
	sw	zero,+0*MEG(base)
	sw	zero,+4*MEG(base)
	sw	zero,+8*MEG(base)
	sw	zero,12*MEG(base)
	subu	tval,1
	bnez	tval,.initmem

	/* now detect the presense of SIMMS & their sides */
	DISPLAY ('S','L','O','T')

	sw	aces,+0*MEG(base)
	sw	aces,+4*MEG(base)
	sw	aces,+8*MEG(base)
	sw	aces,12*MEG(base)

	sw	fives,+0*MEG+4(base)
	sw	fives,+4*MEG+4(base)
	sw	fives,+8*MEG+4(base)
	sw	fives,12*MEG+4(base)

	sw	zero,+0*MEG+8(base)
	sw	zero,+4*MEG+8(base)
	sw	zero,+8*MEG+8(base)
	sw	zero,12*MEG+8(base)


	/* generate map of SIMM slots/sides */		
	move	simmmap,zero
	
	lw	tval,+0*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM0_0
1:
	lw	tval,+4*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM0_1
1:
	lw	tval,+8*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM1_0
1:
	lw	tval,+12*MEG(base)
	bne	tval,aces,1f
	or	simmmap,SIMM1_1
1:

	/* SIMM1 cannot be double sided if SIMM0 is single sided */
	and	tval,simmmap,SIMM0_1|SIMM1_1
	bne	tval,SIMM1_1,1f
	and	simmmap,~SIMM1_1
1:	
	
	/* any memory in SIMM0 ? */
	and	tval,simmmap,SIMM0_0
	beqz	tval,_sbd_memfail

	/* SIMM0 single sided ? */
	and	tval,simmmap,SIMM0_1
	bnez	tval,1f
	or	crval,DCR_SIMM0_SGL
1:		

	/* any memory in SIMM1 ? */
	and	tval,simmmap,SIMM1_0
	beqz	tval,.confsimms

	/* do we have DRAM configured? */
	and	tval,crval,DCR_SIMM1_DRAM
	bnez	tval,1f
	DISPLAY('F','L','S','H')
2:	b	2b	
1:	

	/* SIMM1 single sided ? */
	and	tval,simmmap,SIMM1_1
	bnez	tval,1f
	or	crval,DCR_SIMM1_SGL
1:		
		
	/* now determine SIMM sizes */
.confsimms:	
	/* start with max SIMM0: 16Mb, ds = 32Mb, ss=16Mb */
	and	crval,~DCR_SIMM0_SIZE
	or	crval,DCR_16MB
	li	crptr,PA_TO_KVA1(DCR_BASE)
	DISPLAY ('D','C', 'R','0')
	bal	crwrite

	DISPLAY ('S','I','M','0')

	and	tval,simmmap,SIMM0_1
	beqz	tval,.chkss
	
.chkds:
	/* write data to wraparound points (mod 32Mb) */
	li	tval,DCR_16MB
	sw	tval,0(base)
	li	tval,DCR_8MB
	sw	tval,+2*8*MEG(base)
	li	tval,DCR_4MB
	sw	tval,+2*4*MEG(base)
	b	1f

.chkss:	
	/* write data to wraparound points (mod 32Mb) */
	li	tval,DCR_16MB
	sw	tval,0(base)
	li	tval,DCR_8MB
	sw	tval,+8*MEG(base)
	li	tval,DCR_4MB
	sw	tval,+4*MEG(base)
	/* b	1f */
	
1:				
	sw	aces,4(base)		# kill bus capacitance
	
	/* read SIMM0 size */
	lw	tval,0(base)
	
	/* see which it was */
	bne	tval,DCR_16MB,1f
	li	msize0,2*16*MEG
	b	.gotsimm0

1:	bne	tval,DCR_8MB,1f
	li	msize0,2*8*MEG
	b	.gotsimm0
	
1:	bne	tval,DCR_4MB,_sbd_memfail	# error if not one of above
	li	msize0,2*4*MEG
	#b	.gotsimm0

	/* store the new SIMM0 size */
.gotsimm0:
	and	crval,~DCR_SIMM0_SIZE
	or	crval,tval

	/* if SIMM0 is single sided then halve the detected size */
	and	tval,simmmap,SIMM0_1
	bnez	tval,1f
	srl	msize0,1
1:		
	/* configure size field */
	la	crptr,PA_TO_KVA1(DCR_BASE)
	DISPLAY ('D','C', 'R','1')
	bal	crwrite
	
	/* if SIMM1 is FLASH prom, then skip the SIMM1 tests */
	and	tval,crval,DCR_SIMM1_DRAM
	beqz	tval,.alldone

	/* if we did not detect SIMM1 then skip remainder */
	and	tval,simmmap,SIMM1_0
	beqz	tval,.alldone
	
	DISPLAY ('S','I','M','1')

	/*
	 * starting at the base of SIMM1, start writing locations until
	 * it wraps or falls of the end of memory
	 */
	addu	base,msize0
	sw	aces,0(base)
	li	msize1,1*MEG

	addu	tval,base,msize1
1:	sw	msize0,0(tval)
	lw	tval,0(base)
	bne	tval,aces,.alldone

	sll	msize1,1
	addu	tval,base,msize1
	bltu	tval,PA_TO_KVA1(64*MEG),1b

	/* overrun - shift back down */
	srl	msize1,1
		
.alldone:	
	/* store SIMM0 size for later */
	srl	tval,msize0,20
	SOFTC_STORE(S_SIMM0SZ, tval)		
	/* store SIMM1 size for later */
	srl	tval,msize1,20
	SOFTC_STORE(S_SIMM1SZ, tval)		

	/* we must have got *some* memory by now */
	addu	msize, msize0, msize1
	beqz	msize,_sbd_memfail

	/* write the new DCR value */
	bal	crwrite
	SOFTC_STORE(S_DCR, crval)

	/* store memory size in rtc sram */
	srl	msize,20
	SOFTC_STORE(S_MEMSZ, msize)
	
	/* we can now initialise the caches for a fast clear_mem */
	DISPLAY ('C','A','C','H')
	jal	init_cache

#if 0
		/* clear all of memory (to set correct parity) */
	DISPLAY ('Z','M','E','M')
	la	t0,clear_mem_range
	addu	t0,RELOC
	move	a0,msize
	la	a1,PA_TO_KVA1(0)
	jal	t0
#endif

		
.noinit:

	/* initialise tlb */
	DISPLAY ('I','T','L','B')
	bal	init_tlb

	/* initialise display */
	jal	sbd_dispinit

	move	ra,rasave
	j	ra
END(sbdreset)


LEAF(_sbd_memfail)
	DISPLAY ('!','M','E','M')
1:	b	1b
	j	ra
END(_sbd_memfail)


/* crwrite (base, val) 
 *	Write 8-bit <val> to 8 consecutive 1-bit registers, 
 *	starting at <base>.
 * Uses:	t8, t9
 */
SLEAF(crwrite)
	move	t9,crval
	li	t8,8
1:	.set	noat
	and	$at,t9,1
	sw	$at,0(crptr)
	.set	at
	srl	t9,1
	subu	t8,1
	addu	crptr,4
	bnez	t8,1b

	subu	crptr,8*4	# reset pointer
	j	ra
SEND(crwrite)
		
#define INIT(addr,val) \
	.word	PA_TO_KVA1(addr), val

	.rdata
reginittab:
	/* led message */
	INIT(LED_BASE+LED(0), 'C')
	INIT(LED_BASE+LED(1), 'T')
	INIT(LED_BASE+LED(2), 'E')
	INIT(LED_BASE+LED(3), 'K')
	/* XXX anything else? */	
	.word	0,0


LEAF(sbdberrenb)
	mfc0	v0,C0_SR
	li	t0,SR_DE	
	bnez	a0,1f
	or	t1,v0,t0	# disable cache/parity errors (SR_DE = 1)
	b	2f
1:	not	t1,t0		# enable cache/parity errors (SR_DE = 0)
	and	t1,v0
2:	mtc0	t1,C0_SR
	and	v0,t0		# get old SR_DE bit
	xor	v0,t0		# and invert to make it an enable bit
	j	ra
END(sbdberrenb)


LEAF(sbdberrcnt)
	move	v0,zero
	j	ra
END(sbdberrcnt)

	.lcomm	wbfltmp,4

LEAF(wbflush)
XLEAF(mips_wbflush)
	sync
	la	t0,wbfltmp
	or	t0,K1BASE
	lw	zero,0(t0)
	j	ra
END(wbflush)


LEAF(sbddelay)
	li	t1,CACHEUS
	and	t0,ra,0x20000000
	beqz	t0,1f
	li	t1,ROMUS
1:	mul	a0,t1
	subu	a0,15		# approx number of loops so far

	.set	noreorder	
	.set	nomacro
	nop
2:	bgtz	a0,2b
	subu	a0,1
	.set	macro
	.set	reorder

	j	ra
END(sbddelay)


LEAF(mips_cycle)
	.set	noreorder	
	.set	nomacro
1:	bgtz	a0,1b
	subu	a0,1
	.set	macro
	.set	reorder
	j	ra
END(mips_cycle)
