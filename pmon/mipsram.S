/* $Id: mipsram.S,v 1.12 1999/12/08 12:08:52 nigel Exp $ */
/* mipsram.s - startup code for PMON in RAM
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restiction.
 */

#include "mips.h"
#include "pmon.h"
#ifdef R3081
#include "r3081.h"
#endif
#ifdef R3041
#include "r3041.h"
#endif

#ifdef R4000
#define MFC_NOP	nop; nop
#else
#define MFC_NOP	nop
#endif

#if __mips >= 3
#define SREG(reg,rno,base)	sd	reg,+((rno)*8)(base)
#define LREG(reg,rno,base)	ld	reg,+((rno)*8)(base)
#define sr sd
#define lr ld	
#define MTC0			dmtc0
#define MFC0			dmfc0
#else
#define SREG(reg,rno,base)	sw	reg,+((rno)*4)(base)
#define LREG(reg,rno,base)	lw	reg,+((rno)*4)(base)
#define sr sw
#define lr lw	
#define MTC0			mtc0
#define MFC0			mfc0
#endif
			
#ifndef LOCAL_MEM
#define LOCAL_MEM	0
#endif
			
	.data
	.globl	_pmon_in_ram
_pmon_in_ram:
	.word	1
	
	
	.text

#ifdef INET
#define STKSIZE 0x3000
#else
#define STKSIZE 8192
#endif

	.comm	stack,STKSIZE
	.comm	hndlrtbl,32*4
	.comm	_filebase,4

/*
 * This is the entry point of the entire PROM Monitor
 */
	.globl start
	.ent start
start:
	.globl __start
	.aent __start
__start:
	move	k0,zero
	
	# clear bss
	la	v0,_fbss
	la	v1,end
1:
	sw	$0,0x0(v0)
	sw	$0,0x4(v0)
	sw	$0,0x8(v0)
	sw	$0,0xc(v0)
	addu	v0,16
	bltu	v0,v1,1b

	and	a0,sp,0x1fffffff
	sw	a0,memorysize
	la      sp,stack+STKSIZE-24
	
	la	a0,end
	addu	a0,0x20000
	sw	a0,heaptop
	
#ifdef R4000	
	# caches already initialsed, just size and flush them
	jal	mips_size_cache
	jal	mips_flush_cache
#endif
	
	# initialise C level board-dependent devices
	jal	sbdmachinit


	#########################################################
	#	Set initial client conditions			#
	#
	la	t0,DBGREG

	.set noreorder
	mfc0	t1,C0_CAUSE
	mfc0	t2,C0_PRID
 	mfc0	t3,C0_SR
#if defined(R4000)
	mfc0	t4,C0_CONFIG
#endif
	SREG(t1,R_CAUSE,t0)
	SREG(t2,R_PRID,t0)
  	SREG(t3,R_STATUS,t0)
#if defined(R4000)
	SREG(t4,R_CONFIG,t0)
#endif

	.set reorder

	jal	dbginit		# initialize PMON
	
	jal	main		# transfer to main part of PMON

	j	start
	.end start


	.globl _exit
	.ent _exit
/*
 * This is an exit routine, it should never be called except when PMON
 * is aborted while running under SABLE.
 */
_exit:	
#ifndef SABLE 
	break 0
#endif
	j	ra
	.end _exit


	.globl cliexit
	.ent cliexit
/*
 * This is called when a client terminates normally.
 */
cliexit:
	/* reset stack and status register (minus interrupts) */
	lw	t1,initial_sr
	and	t1,~SR_IMASK
	mtc0	t1,C0_SR
	jal	clrhndlrs
	lw	a0,gobufvalid
	beqz	a0,1f
	
	/* restart via longjmp to gobuf */
	la	a0,gobuf
	li	a1,1
	jal	longjmp

1:	la	sp,stack+STKSIZE-24
	j	main
	.end cliexit

	.globl	_go
	.ent _go
/*
 * This routine is used to transfer control to a client program.
 */
_go:
	li	a0,2
	jal	swlst

	la	gp,DBGREG

	# at restored later		/* $1  (at) */
	LREG(v0,R_V0,gp)		/* $2  (v0) */
	LREG(v1,R_V1,gp)		/* $3  (v1) */
	LREG(a0,R_A0,gp)		/* $4  (a0) */
	LREG(a1,R_A1,gp)		/* $5  (a1) */
	LREG(a2,R_A2,gp)		/* $6  (a2) */
	LREG(a3,R_A3,gp)		/* $7  (a3) */
	# t0 restored later		/* $8  (t0) */
	# t1 restored later		/* $9  (t1) */
	# t2 restored later		/* $10 (t2) */
	LREG(t3,R_T3,gp)		/* $11 (t3) */
	LREG(t4,R_T4,gp)		/* $12 (t4) */
	LREG(t5,R_T5,gp)		/* $13 (t5) */
	LREG(t6,R_T6,gp)		/* $14 (t6) */
	LREG(t7,R_T7,gp)		/* $15 (t7) */
	LREG(s0,R_S0,gp)		/* $16 (s0) */
	LREG(s1,R_S1,gp)		/* $17 (s1) */
	LREG(s2,R_S2,gp)		/* $18 (s2) */
	LREG(s3,R_S3,gp)		/* $19 (s3) */
	LREG(s4,R_S4,gp)		/* $20 (s4) */
	LREG(s5,R_S5,gp)		/* $21 (s5) */
	LREG(s6,R_S6,gp)		/* $22 (s6) */
	LREG(s7,R_S7,gp)		/* $23 (s7) */
	LREG(t8,R_T8,gp)		/* $24 (t8) */
	LREG(t9,R_T9,gp)		/* $25 (t9) */
	# k0 not restored		/* $26 (k0) */
	# k1 restored later		/* $27 (k1) */
	# gp restored later		/* $28 (gp) */
	LREG(sp,R_SP,gp)		/* $29 (sp) */
	LREG(s8,R_FP,gp)		/* $30 (s8) */
	LREG(ra,R_RA,gp)		/* $31 (ra) */

	LREG(t0,R_HI,gp)		/* Hi */
	LREG(t1,R_LO,gp)		/* Lo */
	mthi	t0
	mtlo	t1

	.set	noat
	.set noreorder

	# restore SR from Status but fixup the KU&IE field
	# to make sure that correct value is restored after the rfe
#ifdef R4000
	mtc0	zero,C0_SR		# set IMASK & IE = 0 before setting EXL
	LREG(t0,R_STATUS,gp)		# get Status
	or	t0,SR_EXL		# set EXL
	nop
#else
	mtc0	zero,C0_SR		# clear IEC (and everything else)
	# interrupts now off (k0 & k1 safe)
	LREG(t1,R_STATUS,gp)		# get Status
	li	AT,0x3f			# KU&IE mask
	and	t0,t1,AT		# get KU&IE field
	not	AT			# invert mask
	and	t1,AT			# clear KU&IE field
	sll	t0,2			# fixup KU&IE field
	or	t0,t1			# insert KU&IE field
#endif
	mtc0	t0,C0_SR

#ifdef FLOATINGPT
#ifdef R4000
	nop
	mfc0	t2,C0_PRID
	and	t1,t0,SR_CU1
	beqz	t1,2f
	srl	t2,8			# BDSLOT
	li	AT,0x22		
	beq	t2,AT,.rest4650
	LREG(t2,R_FCR,gp)		# BDSLOT
#else
	LREG(t2,R_FCR,gp)
	and	t1,t0,SR_CU1
	beqz	t1,2f
	nop
#endif
	
#if __mips >= 3
	ldc1	$f0,R_F0*8(gp)
	ldc1	$f2,R_F2*8(gp)
	ldc1	$f4,R_F4*8(gp)
	ldc1	$f6,R_F6*8(gp)
	ldc1	$f8,R_F8*8(gp)
	ldc1	$f10,R_F10*8(gp)
	ldc1	$f12,R_F12*8(gp)
	ldc1	$f14,R_F14*8(gp)
	ldc1	$f16,R_F16*8(gp)
	ldc1	$f18,R_F18*8(gp)
	ldc1	$f20,R_F20*8(gp)
	ldc1	$f22,R_F22*8(gp)
	ldc1	$f24,R_F24*8(gp)
	ldc1	$f26,R_F26*8(gp)
	ldc1	$f28,R_F28*8(gp)

	and	t1,t0,SR_FR
	beqz	t1,1f
	ldc1	$f30,R_F30*8(gp)		# BDSLOT
	
	
	/* SR.FR is set: restore the other 16 registers too */
	ldc1	$f1,R_F1*8(gp)
	ldc1	$f3,R_F3*8(gp)
	ldc1	$f5,R_F5*8(gp)
	ldc1	$f7,R_F7*8(gp)
	ldc1	$f9,R_F9*8(gp)
	ldc1	$f11,R_F11*8(gp)
	ldc1	$f13,R_F13*8(gp)
	ldc1	$f15,R_F15*8(gp)
	ldc1	$f17,R_F17*8(gp)
	ldc1	$f19,R_F19*8(gp)
	ldc1	$f21,R_F21*8(gp)
	ldc1	$f23,R_F23*8(gp)
	ldc1	$f25,R_F25*8(gp)
	ldc1	$f27,R_F27*8(gp)
	ldc1	$f29,R_F29*8(gp)
#ifdef R4000
	b	1f
	ldc1	$f31,R_F31*8(gp)
#else
	ldc1	$f31,R_F31*8(gp)
	b	1f
	nop
#endif		
#ifdef R4000
.rest4650:	
#ifdef MIPSEB
# define LSW	4
#else 
# define LSW	0
#endif
	lwc1	$f0,R_F0*8+LSW(gp)
	lwc1	$f2,R_F2*8+LSW(gp)
	lwc1	$f4,R_F4*8+LSW(gp)
	lwc1	$f6,R_F6*8+LSW(gp)
	lwc1	$f8,R_F8*8+LSW(gp)
	lwc1	$f10,R_F10*8+LSW(gp)
	lwc1	$f12,R_F12*8+LSW(gp)
	lwc1	$f14,R_F14*8+LSW(gp)
	lwc1	$f16,R_F16*8+LSW(gp)
	lwc1	$f18,R_F18*8+LSW(gp)
	lwc1	$f20,R_F20*8+LSW(gp)
	lwc1	$f22,R_F22*8+LSW(gp)
	lwc1	$f24,R_F24*8+LSW(gp)
	lwc1	$f26,R_F26*8+LSW(gp)
	lwc1	$f28,R_F28*8+LSW(gp)

	and	t1,t0,SR_FR
	beqz	t1,1f
	lwc1	$f30,R_F30*8+LSW(gp)

	/* SR.FR is set: restore the other 16 registers too */
	lwc1	$f1,R_F1*8+LSW(gp)
	lwc1	$f3,R_F3*8+LSW(gp)
	lwc1	$f5,R_F5*8+LSW(gp)
	lwc1	$f7,R_F7*8+LSW(gp)
	lwc1	$f9,R_F9*8+LSW(gp)
	lwc1	$f11,R_F11*8+LSW(gp)
	lwc1	$f13,R_F13*8+LSW(gp)
	lwc1	$f15,R_F15*8+LSW(gp)
	lwc1	$f17,R_F17*8+LSW(gp)
	lwc1	$f19,R_F19*8+LSW(gp)
	lwc1	$f21,R_F21*8+LSW(gp)
	lwc1	$f23,R_F23*8+LSW(gp)
	lwc1	$f25,R_F25*8+LSW(gp)
	lwc1	$f27,R_F27*8+LSW(gp)
	lwc1	$f29,R_F29*8+LSW(gp)
	lwc1	$f31,R_F31*8+LSW(gp)
#endif	
#else
	lwc1	$f0,R_F0*4(gp)
	lwc1	$f1,R_F1*4(gp)
	lwc1	$f2,R_F2*4(gp)
	lwc1	$f3,R_F3*4(gp)
	lwc1	$f4,R_F4*4(gp)
	lwc1	$f5,R_F5*4(gp)
	lwc1	$f6,R_F6*4(gp)
	lwc1	$f7,R_F7*4(gp)
	lwc1	$f8,R_F8*4(gp)
	lwc1	$f9,R_F9*4(gp)
	lwc1	$f10,R_F10*4(gp)
	lwc1	$f11,R_F11*4(gp)
	lwc1	$f12,R_F12*4(gp)
	lwc1	$f13,R_F13*4(gp)
	lwc1	$f14,R_F14*4(gp)
	lwc1	$f15,R_F15*4(gp)
	lwc1	$f16,R_F16*4(gp)
	lwc1	$f17,R_F17*4(gp)
	lwc1	$f18,R_F18*4(gp)
	lwc1	$f19,R_F19*4(gp)
	lwc1	$f20,R_F20*4(gp)
	lwc1	$f21,R_F21*4(gp)
	lwc1	$f22,R_F22*4(gp)
	lwc1	$f23,R_F23*4(gp)
	lwc1	$f24,R_F24*4(gp)
	lwc1	$f25,R_F25*4(gp)
	lwc1	$f26,R_F26*4(gp)
	lwc1	$f27,R_F27*4(gp)
	lwc1	$f28,R_F28*4(gp)
	lwc1	$f29,R_F29*4(gp)
	lwc1	$f30,R_F30*4(gp)
#ifdef R4000
	b	1f
	lwc1	$f31,R_F31*4(gp)
#else
	lwc1	$f31,R_F31*4(gp)
	b	1f
	nop
#endif	
#ifdef R4000
.rest4650:	
	lwc1	$f0,R_F0*4(gp)
	lwc1	$f2,R_F2*4(gp)
	lwc1	$f4,R_F4*4(gp)
	lwc1	$f6,R_F6*4(gp)
	lwc1	$f8,R_F8*4(gp)
	lwc1	$f10,R_F10*4(gp)
	lwc1	$f12,R_F12*4(gp)
	lwc1	$f14,R_F14*4(gp)
	lwc1	$f16,R_F16*4(gp)
	lwc1	$f18,R_F18*4(gp)
	lwc1	$f20,R_F20*4(gp)
	lwc1	$f22,R_F22*4(gp)
	lwc1	$f24,R_F24*4(gp)
	lwc1	$f26,R_F26*4(gp)
	lwc1	$f28,R_F28*4(gp)

	and	t1,t0,SR_FR
	beqz	t1,1f
	lwc1	$f30,R_F30*4(gp)

	/* SR.FR is set: restore the other 16 registers too */
	lwc1	$f1,R_F1*4(gp)
	lwc1	$f3,R_F3*4(gp)
	lwc1	$f5,R_F5*4(gp)
	lwc1	$f7,R_F7*4(gp)
	lwc1	$f9,R_F9*4(gp)
	lwc1	$f11,R_F11*4(gp)
	lwc1	$f13,R_F13*4(gp)
	lwc1	$f15,R_F15*4(gp)
	lwc1	$f17,R_F17*4(gp)
	lwc1	$f19,R_F19*4(gp)
	lwc1	$f21,R_F21*4(gp)
	lwc1	$f23,R_F23*4(gp)
	lwc1	$f25,R_F25*4(gp)
	lwc1	$f27,R_F27*4(gp)
	lwc1	$f29,R_F29*4(gp)
	lwc1	$f31,R_F31*4(gp)
#endif	
#endif
1:	ctc1	t2,C1_CSR
	nop
2:	
#endif /* FLOATINGPT */


#ifdef LR33000
	LREG(t0,R_DCIC,gp)		/* DCIC */
	LREG(t1,R_BPC,gp)		/* BPC */
	LREG(AT,R_BDA,gp)		/* BDA */
	mtc0	t0,C0_DCIC
	mtc0	t1,C0_BPC
	mtc0	AT,C0_BDA
#else
	LREG(t0,R_ENTRYHI,gp)		/* EntryHi */
	LREG(t1,R_CONTEXT,gp)		/* Context */
	LREG(AT,R_INDEX,gp)		/* Index */
	mtc0	t0,C0_TLBHI
	MTC0	t1,C0_CTEXT
	mtc0	AT,C0_INX
	LREG(t0,R_ENTRYLO,gp)		/* EntryLo */
#if __mips >= 3
	LREG(t1,R_XCONTEXT,gp)		/* XContext */
#else
	nop
#endif
	mtc0	t0,C0_TLBLO
#if __mips >= 3
	MTC0	t1,C0_XCTEXT
#endif

#ifdef R4000
	LREG(t0,R_ENTRYLO1,gp)		/* TLB EntryLo1 */
	LREG(t1,R_PGMASK,gp)		/* TLB PageMask */
	LREG(AT,R_WIRED,gp)		/* TLB Wired */
	mtc0	t0,C0_TLBLO1
	mtc0	t1,C0_PGMASK
	mtc0	AT,C0_WIRED
	LREG(t0,R_WATCHLO,gp)		/* WatchpointLo */
	LREG(t1,R_WATCHHI,gp)		/* WatchpointHi */
	/*LREG(AT,R_LLADDR,gp)		/* LLAddr */
	mtc0	t0,C0_WATCHLO
	mtc0	t1,C0_WATCHHI
	/*mtc0	t1,C0_LLADDR*/
	LREG(t0,R_ECC,gp)		/* ECC */
	LREG(t1,R_TAGLO,gp)		/* TagLo */
	LREG(AT,R_TAGHI,gp)		/* TagHi */
	mtc0	t0,C0_ECC
	mtc0	t1,C0_TAGLO
	mtc0	t1,C0_TAGHI
#endif
#endif

#ifdef R4000
	LREG(AT,R_EPC,gp)		/* EPC */
	LREG(t1,R_CAUSE,gp)		/* Cause - For Sw field only */
	LREG(t0,R_T0,gp)		/* $8  (t0) */
	LREG(t2,R_T2,gp)		/* $10 (t2) */
	MTC0	AT,C0_EPC
	mtc0	t1,C0_CAUSE
	LREG(AT,R_AT,gp)		/* $1  (AT) */
	LREG(t1,R_T1,gp)		/* $8  (t0) */
	LREG(k1,R_K1,gp)		/* $27 (k1) */
	LREG(gp,R_GP,gp)		/* $28 (gp) */
#if LOCAL_MEM != 0
	la	k0,_exception
#endif	
	eret
	nop
#else
	LREG(t1,R_CAUSE,gp)		/* Cause - For Sw field only */
	LREG(t0,R_T0,gp)		/* $8  (t0) */
	LREG(t2,R_T2,gp)		/* $10 (t2) */
	mtc0	t1,C0_CAUSE
	LREG(AT,R_AT,gp)		/* $1  (AT) */
	LREG(t1,R_T1,gp)		/* $8  (t0) */
	LREG(k0,R_EPC,gp)		/* EPC */
	LREG(k1,R_K1,gp)		/* $27 (k1) */
	LREG(gp,R_GP,gp)		/* $28 (gp) */
#if LOCAL_MEM != 0
	/* unfortunately, we cannot preserve k1 */
	move	k1,k0
	la	k0,_exception
	j	k1			# transfer control to client
	rfe
#else
	j	k0			# transfer control to client
	rfe
#endif	
#endif
	.set at
	.set reorder
	.end _go



	.globl clrhndlrs
	.ent clrhndlrs
/*
 * Used to remove user defined exception handlers.
 * also recopies exception vector handler.
 */
clrhndlrs:
	subu	sp,24
	sw	ra,20(sp)

	# remove user defined handlers
	la	t1,hndlrtbl
	li	t2,0
	li	t3,32
1:	sll	t0,t2,2
	addu	t0,t1
	sw	zero,(t0)
	addu	t2,1
	bne	t2,t3,1b

	lw	ra,20(sp)
	addu	sp,24
	j	ra
	.end clrhndlrs

	.globl clienttos
	.ent clienttos
/*
 * This routine returns the correct top-of-stack value.
 * Used by C routines, when they need to set the client stack pointer.
 */
clienttos:
	lw	v0,memorysize
	addu	v0,K0BASE
	and	v0,~7 			# double word align
	subu	v0,24			# make room for a0-a3..
	j	ra
	.end clienttos


	.globl bis_sr
	.ent bis_sr
/*
 * Set bits in status register, returning old value
 */
bis_sr:
	.set noreorder
	mfc0	v0,C0_SR
	nop
#ifdef R4000
	nop
#endif
#if 0	
	or	a0,v0
	mtc0	a0,C0_SR
#endif	
	.set reorder
	j	ra
	.end bis_sr


	.globl bic_sr
	.ent bic_sr
/*
 * Clear bits in status register, returning old value
 */
bic_sr:
	.set noreorder
	mfc0	v0,C0_SR
#ifdef R4000
	nop
#endif
#if 0	
	not	a0
	and	a0,v0
	mtc0	a0,C0_SR
#endif	
	.set reorder
	j	ra
	.end bic_sr

	.globl set_sr
	.ent set_sr
/*
 * Set status register, returning old value
 */
set_sr:
	.set noreorder
	mfc0	v0,C0_SR
#if 0	
	mtc0	a0,C0_SR
#endif
	.set reorder
	j	ra
	.end set_sr

/*
 * Used to enable coprocessor 1 (the FPU)
 */
	.globl enableCU1
	.ent enableCU1
enableCU1:
#ifdef FLOATINGPT	
	.set noreorder
	mfc0	t0,C0_SR
	nop
	or	v0,t0,SR_CU1
	mtc0	v0,C0_SR
	nop; nop
	
	mfc0	v0,C0_SR	# make sure CU1 got written...
	nop; nop
	and	t1,v0,SR_CU1
	beqz	t1,1f
	nop
		
	# stuff pattern into f0 and f1
	li	t1,0xaaaa5555
#ifdef R4000
	nop
#endif

	mtc1	t1,$f0
	mtc1	zero,$f1

	# retrieve pattern
	mfc1	t2,$f0
	mfc1	t3,$f1
	.set reorder

	# check pattern
	bne	t1,t2,1f
	bne	zero,t3,1f

	# OK we have got an fpu, initialise CSR and save ID
 	cfc1	t1,C1_FRID
	ctc1	zero,C1_CSR
	la	t2,DBGREG
 	SREG(t1,R_FID,t2)
	j	ra

1:	mtc0	t0,C0_SR
	move	v0,t0
#else	
	.set	noreorder
	mfc0	v0,C0_SR
	nop
	.set	reorder
#endif	/* FLOATINGPT */
	j	ra
	.end enableCU1


#ifdef R4000	
	.globl	get_count
	.ent	get_count
get_count:
	.set	noreorder
	mfc0	v0,C0_COUNT
	.set	reorder
	j	ra
	.end	get_count


	.globl	get_compare
	.ent	get_compare
get_compare:
	.set	noreorder
	mfc0	v0,C0_COMPARE
	.set	reorder
	j	ra
	.end	get_compare
	

	.globl	get_ecc
	.ent	get_ecc
get_ecc:
	.set	noreorder
	mfc0	v0,C0_ECC
	.set	reorder
	j	ra
	.end	get_ecc

	.globl	get_errepc
	.ent	get_errepc
get_errepc:
	.set	noreorder
	mfc0	v0,C0_ERREPC
	.set	reorder
	j	ra
	.end	get_errepc

#endif /* R4000 */

/* Algor PMON entrypoint */
#define TBLBASE 0xbfc00500
#define TBLENTRY(n,m) 			\
	.globl m; 			\
	.ent m;				\
m:	li	t0,TBLBASE+(4*n);	\
	lw	t0,(t0);		\
	j	t0;			\
	.end m
	
TBLENTRY(5,_pmon_printf)

	.globl	__assfail
	.ent	__assfail
__assfail:	
	jal	_pmon_printf
	break	0
	.end	__assfail
	
	.globl	get_cause
	.ent	get_cause
get_cause:
	.set	noreorder
	mfc0	v0,C0_CAUSE
	.set	reorder
	j	ra
	.end	get_cause


#ifndef LR33000
	.globl	tlbread
	.ent	tlbread
tlbread:
	.set	noreorder
	mfc0	v1, C0_SR	# Save the status register.
	mtc0	zero, C0_SR	# Disable interrupts
	nop
	nop
	nop
	MFC0	t0, C0_TLBHI	# Get current PID

	mtc0	a0, C0_INX	# Set the index register
	nop
	tlbr			# Read from the TLB
	nop
	nop
	nop
#ifdef R4000
	MFC0	t2, C0_PGMASK	# fetch the hi entry
	MFC0	t3, C0_TLBHI	# fetch the hi entry
	MFC0	t4, C0_TLBLO0	# See what we got
	MFC0	t5, C0_TLBLO1	# See what we got
	MTC0	t0, C0_TLBHI	# restore PID
	nop
	nop
	nop			# wait for PID active
	mtc0	v1, C0_SR	# Restore the status register
	.set	reorder
	sr	t2, TLB_MASK(a1)
	sr	t3, TLB_HI(a1)
	sr	t4, TLB_LO0(a1)
	sr	t5, TLB_LO1(a1)
#else
       	MFC0	t2, C0_TLBHI	# fetch the hi entry
	MFC0	t3, C0_TLBLO	# See what we got
	MTC0	t0, C0_TLBHI	# restore PID
	nop
	nop
	nop			# wait for PID active
	mtc0	v1, C0_SR	# Restore the status register
	.set	reorder
	sr	t2, TLB_HI(a1)
	sr	t3, TLB_LO(a1)
#endif
	j	ra
	.end	tlbread
#endif
