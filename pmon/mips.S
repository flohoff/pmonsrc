/* $Id: mips.S,v 1.33 2001/11/07 12:16:32 chris Exp $ */
/* mips.s - startup code for PMON
 *	The contents of this file are not copyrighted in any
 *	way, and may therefore be used without restiction.
 */

#include "mips/cpu.h"
#include "mips/prid.h"
	
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
#define REGSZ			8
#define sr			sd
#define lr 			ld	
#define MTC0			dmtc0
#define MFC0			dmfc0
#else
#define REGSZ			4
#define sr 			sw
#define lr 			lw	
#define MTC0			mtc0
#define MFC0			mfc0
#endif
	
#define SREG(reg,rno,base)	sr	reg,+((rno)*REGSZ)(base)
#define LREG(reg,rno,base)	lr	reg,+((rno)*REGSZ)(base)

#ifndef LOCAL_MEM
#define LOCAL_MEM	0
#endif
			
	.text

#ifdef BOOTPKG

#include "bt.h"	
	
/* jump table offsets in ITROM */
#define IT_BOOTPKG	(0*8)
#define IT_BEVTLB	(1*8)
#define IT_BEVGEN	(2*8)
#define IT_BEVXTLB	(3*8)
#define IT_BEVCACHE	(4*8)
#define IT_BEVDBG	(5*8)
#define IT_BEVBAD	(6*8)
#define IT_MAIN		(7*8)

#define itbev(itdest,localdest) \
	li	k1,PACKAGEINFO+(7<<(PKGSHIFT+2)) ; \
	lw	k1,oSTART(k1) ; \
	addu	k1,1 ; \
	beqz	k1,localdest ; \
	addu	k1,itdest-1 ; \
	j	k1
	
#ifdef R4000
#define r3k_utlb_bev	itbev(IT_BEVBAD,__bev)
#define lr33k_dbg_bev	itbev(IT_BEVBAD,__bev)
#define r3k_gen_bev	itbev(IT_BEVBAD,__bev)
#define r4k_tlb_bev	itbev(IT_BEVTLB,__bev)
#define r4k_xtlb_bev	itbev(IT_BEVXTLB,__bev)
#define r4k_cache_bev	itbev(IT_BEVCACHE,cerrhandler)
#define r4k_gen_bev	itbev(IT_BEVGEN,__bev)
#else /* !R4000 */
#define r3k_utlb_bev	itbev(IT_BEVUTLB,__bev)
#define lr33k_dbg_bev	itbev(IT_BEVDBG,__bev)
#define r3k_gen_bev	itbev(IT_BEVGEN,__bev)
#define r4k_tlb_bev	itbev(IT_BEVBAD,__bev)
#define r4k_xtlb_bev	itbev(IT_BEVBAD,__bev)
#define r4k_cache_bev	itbev(IT_BEVBAD,__bev)
#define r4k_gen_bev	itbev(IT_BEVBAD,__bev)
#endif /* R4000 */
#else /* !BOOTPKG */
#define r3k_utlb_bev	b __bev
#define lr33k_dbg_bev	b __bev
#define r3k_gen_bev	b __bev
#define r4k_tlb_bev	b __bev
#define r4k_xtlb_bev	b __bev
#ifdef R4000
#define r4k_cache_bev	b cerrhandler
#else
#define r4k_cache_bev	b __bev
#endif
#define r4k_gen_bev	b __bev
#endif

	.ent	reset_exception
reset_exception:
	.set noreorder
	.set noat

	b	prestart		# Reset entrypoint
	move	k0,zero
	b	prestart		# VxWorks Reboot entrypoint
	move	k0,zero
	b	__start			# ITROM secondary entrypoint
	move	k0,zero
	.set reorder


/*
 * Boot Exception Vectors
 */
	.origin 0x100	/* bfc00100: r3000 utlb miss */
	r3k_utlb_bev

	.origin	0x140	/* bfc00140: lr33000 debug  */
	lr33k_dbg_bev

	.origin 0x180	/* bfc00180: r3000 general */
	r3k_gen_bev

	.origin 0x200	/* bfc00200: r4000 tlbmiss */
	r4k_tlb_bev

	.origin 0x280	/* bfc00280: r4000 xtlbmiss */
	r4k_xtlb_bev

	.origin 0x300	/* bfc00300: r4000 cache error */
	r4k_cache_bev

	.origin 0x380	/* bfc00380: r4000 general */
	r4k_gen_bev

	.end	reset_exception
	
	.set	at

#ifdef BOOTPKG
	.origin 0x400		/* bfc00400: ITPROM boot package table */
	
boot_table:
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 0
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 1
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 2
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 3
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 4
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 5

	/* PMON package */
	.word	BTMAGIC
	.word	start		# XXX won't work if PMON is linked for RAM 
	.word	etext		# XXX won't work if PMON is linked for RAM 
	.word	-1		# checksum
	.word	0xbfc00010	# see reset_exception, above
	.word	-1,-1,-1		

	/* ITROM package */
	/* gets filled in by s3merge */
	.word	-1,-1,-1,-1,-1,-1,-1,-1		# pkg 7
#endif /* BOOTPKG */

	.origin 0x500		/* bfc00500: entrypoint table */
	
	.globl util_routines
util_routines:
	.word	read		# 0
	.word	write		# 1
	.word	open		# 2
	.word	close		# 3
	.word	ioctl		# 4
	.word	printf		# 5
	.word	vsprintf	# 6
	.word	ttctl		# 7

	.word	cliexit		# 8
	.word	getenv		# 9
	.word	onintr		# 10
	.word	flush_cache	# 11
	.word	_exception	# 12
#if defined(FLOATINGPT)
	.word	_fpstatesz	# 13
#ifdef NEWFP
	.word	0		# 14
#else
	.word	_fpinit		# 14
#endif
	.word	_fpstate	# 15
	.word	cop1		# 16
#else
	.word	0
	.word	0
	.word	0
	.word	0
#endif
	.word	adr2symoff	# 17
	.word	sym2adr		# 18
	.word	sbdcpufreq	# 19
#ifdef INET
	.word	_getperrno	# 20
	.word	strerror	# 21
	.word	soc_syscall	# 22
	.word	soc_exit	# 23
#else
	.word	0		# 20
	.word	0		# 21
	.word	0		# 22
	.word	0		# 23
#endif

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
	.ent	prestart
prestart:
#ifdef BOOTPKG
	/* jump at itrom (but only if it is there...) */
	li	k1,PACKAGEINFO+(7<<(PKGSHIFT+2))+oENTRY	# package entrypoint
	lw	k1,0(k1)
	addu	k1,1
	beqz	k1,1f			# is this a blank ROM location?
	subu	k1,1
	li	a0,7			# start with package 7
	j	k1
1:	
#endif
	.end	prestart
	
	.globl start
	.ent start
start:
	.globl __start
	.aent __start
__start:
	move	k0,zero
	
	# force kSeg1 in case control is passed here from Kseg0
	
	.set	noreorder
	.set	nomacro
	bal	1f
	lui	t0,K1BASE>>16
1:	or	ra,t0
	addiu	ra,16
	j	ra
	nop
	.set	macro
	.set	reorder
	
	# set SR and CAUSE to something sensible
#ifdef R4000
	.set noreorder
	mfc0	t0,C0_PRID
	mfc0	v0,C0_SR
	MTC0	zero,C0_WATCHLO
	MTC0	zero,C0_WATCHHI
	.set reorder
	and	v0,SR_SR		# preserve Soft Reset
	or	v0,SR_BEV		# set Boot Exceptions
	beq	t0,0x0a11,1f		# R4200 rev 1.1: disable cache errors
	beq	t0,0x5413,1f		# R5432 rev 1.3: disable cache errors
	beq	t0,0x5510,1f		# R5500 rev 1.0: disable cache errors
	b	2f
1:	or	v0,SR_DE
2:	
#elif defined (LR33000)
	li	v0,SR_BEV
#else
	li	v0,SR_BEV|SR_PE		# Set Boot Exceptions; clear Parity Error
#endif /* R4000 */
	.set noreorder
	mtc0	v0,C0_SR
	mtc0	zero,C0_CAUSE
	.set reorder

        # set up a K1seg stack
1:	la      sp,stack+STKSIZE-24
	or      sp,K1BASE

	# s8 == reloc for SDE-MIPS startup code
	bal	1f
1:	la	t1,1b
	subu	s8,ra,t1

	# board-dependent low-level initialisation
	bal	sbdreset

#if !#cpu(l64360)
	/* see if relocation involves more than kseg1 -> kseg0 change */
	and	t0,s8,~0x20000000
	beqz	t0,2f
#else	
	beqz	s8,2f
#endif
	
	# copy .text & .rdata to RAM
	# src=RELOC(_ftext) dst=_ftext stop=_etext
	SBD_DISPLAY('C','O','D','E', CHKPNT_CODE)
	la	t0,_ftext
	addu	t0,s8		# RELOC
	or	t0,K1BASE
 	la	t1,_ftext
	or	t1,K1BASE
	la	t2,_etext
	or	t2,K1BASE
	
	addu	t2,REGSZ-1
	and	t2,-REGSZ
	.set	noreorder
1:	lr	t3,0(t0)
	addu	t0,REGSZ
	addu	t1,REGSZ
	bne	t1,t2,1b
	sr	t3,-REGSZ(t1)
	.set	reorder
	
	# start running from RAM copy
	la	t0,2f
	or	t0,K1BASE
	j	t0
	
2:	# copy .data to RAM
	# src=etext dst=_fdata stop=edata
	SBD_DISPLAY('D','A','T','A', CHKPNT_DATA)
	la	t0,etext
	addu	t0,s8		# RELOC
	or	t0,K1BASE
	/* sde-conv puts data on next 16-byte boundary */
	addu	t0,15
	and	t0,~15
 	la	t1,_fdata
	or	t1,K1BASE
	la	t2,edata
	or	t2,K1BASE

	addu	t2,REGSZ-1
	and	t2,-REGSZ
	beq	t1,t2,zbss
	.set	noreorder
1:	lr	t3,0(t0)
	addu	t0,REGSZ
	addu	t1,REGSZ
	bne	t1,t2,1b
	sr	t3,-REGSZ(t1)
	.set	reorder
		
zbss:	SBD_DISPLAY('Z','B','S','S', CHKPNT_ZBSS)
	# clear bss
	la	v0,_fbss
	la	v1,end
	or	v0,K1BASE
	or	v1,K1BASE
1:	.set	noreorder
	sw	$0,0(v0)
	sw	$0,4(v0)
	sw	$0,8(v0)
	addu	v0,16
	bltu	v0,v1,1b
	sw	$0,-4(v0)
	.set	reorder
	
	# boot exceptions can now vector to normal (uncached) handler 
	la	t0,_exception
	or	k0,t0,K1BASE
	
#if LOCAL_MEM == 0
	SBD_DISPLAY('H','A','N','D', CHKPNT_HAND)
	# copy handler
	la      a0,handler
	la      a1,ehandler
	li      a2,0x0000000           # utlb miss
	bal     copyHandler
#ifdef LR33000
	li      a2,0x0000040           # debug 
	bal     copyHandler
#endif
	li      a2,0x0000080           # general/r4000 xtlb
	bal     copyHandler
#ifdef R4000
	li	a2,0x0000180		# r4000 general
	bal     copyHandler

	la      a0,cerrhandler
	la      a1,ecerrhandler
	li	a2,0x0000100		# r4000 cache error
	bal     copyHandler
#endif
#endif	

	SBD_DISPLAY('C','A','C','H', CHKPNT_CACH)
#ifdef R4000
#if LOCAL_MEM != 0
#error Need to revive mips_init_cache_from!
#else		
	jal	mips_init_cache
#endif	
#else
	# flush the caches
	li	a0,DCACHE
	jal	flush_cache
	li	a0,ICACHE
	jal	flush_cache
#endif
	
	# boot exceptions can now vector to normal (cached) handler
	la	k0,_exception

	# ok to use k0seg stack and pc now
	la	sp,stack+STKSIZE-24
	la	t0,1f
	j	t0
1:

	# initialise C level board-dependent devices
	SBD_DISPLAY('S','B','D','M', CHKPNT_SBDM)
	jal	sbdmachinit


	SBD_DISPLAY('C','R','E','G', CHKPNT_CREG)

	#########################################################
	#	Set initial client conditions			#
	#
	la	t0,DBGREG

	.set noreorder
 	mfc0	t3,C0_SR
	mfc0	t1,C0_CAUSE
	mfc0	t2,C0_PRID
#if defined(R4000)
	mfc0	t4,C0_CONFIG
	and	t5,t3,SR_SR
	beqz	t5,1f
	nop
	MFC0	t5,C0_ERREPC		# errepc contains address after soft reset
	nop
	nop
1:		
#endif
#if LOCAL_MEM == 0
	and	t3,~SR_BEV		# switch off Boot Exceptions
 	mtc0	t3,C0_SR
#endif	
  	SREG(t3,R_STATUS,t0)
	SREG(t1,R_CAUSE,t0)
	SREG(t2,R_PRID,t0)
#if defined(R4000)
	SREG(t4,R_CONFIG,t0)
	SREG(t5,R_ERREPC,t0)
#endif

#if defined(R4000)
	mfc0	t1,C0_LLADDR
	mfc0	t4,C0_WIRED
	mfc0	t5,C0_PGMASK
	SREG(t1,R_LLADDR,t0)
	SREG(t4,R_WIRED,t0)
	SREG(t5,R_PGMASK,t0)
	mfc0	t1,C0_INX
	mfc0	t2,C0_RAND
 	mfc0	t3,C0_TLBLO0
	mfc0	t4,C0_TLBLO1
	mfc0	t5,C0_TLBHI
#ifdef C0_INFO	
	mfc0	t6,C0_INFO		# RM7000 specific
#else
	move	t6,zero	
#endif
	SREG(t1,R_INDEX,t0)
	SREG(t2,R_RANDOM,t0)
	SREG(t3,R_ENTRYLO,t0)
	SREG(t4,R_ENTRYLO1,t0)
	SREG(t5,R_ENTRYHI,t0)
	SREG(t6,R_INFO,t0)
#endif
	.set reorder

	SBD_DISPLAY('D','B','G','I', CHKPNT_DBGI)
#ifdef ENB_SHRC
	# provide address for use by shrc function. This feature 
	# is normally disabled because it can cause 
	# problems if the PROM has junk in the end.
#ifdef SABLE
	li	a0,0xa0070000
#else
	la	a0,edata
	la	t1,_fdata
	subu	a0,t1
	la	t1,etext
	addu	a0,t1
#endif
#endif
	jal	dbginit		# initialize PMON
	# ints might be enabled from here on

	#							#
	#########################################################

	SBD_DISPLAY('M','A','I','N', CHKPNT_MAIN)
#ifndef MAIN
#define MAIN main
#endif	
	jal	MAIN		# transfer to main part of PMON

	j	start
	.end start

#if LOCAL_MEM == 0
	.globl handler
	.ent handler
/*
 * This is the handler that gets copied to the exception vector addresses.
 */
#if __mips >= 3
	.align	3
#endif	
handler:
	.set noat
	la	k0,_exception
	j	k0
ehandler:
	.set at
	.end handler
#endif


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
	/* reset status register (minus interrupts) */
	mfc0	t2,C0_PRID
	lw	t1,initial_sr
	and	t1,~SR_IMASK
	mtc0	t1,C0_SR
	
#ifdef R4000
	/* reset RM7000 interrupt extensions */
	srl	t2,8
	bne	t2,PRID_RM7000,1f
	ctc0	zero,C0C_ICR
	ctc0	zero,C0C_IPLLO
	ctc0	zero,C0C_IPLHI
1:		
#endif	
	
	SBD_DISPLAY('E','X','I','T', CHKPNT_EXIT)
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
	mfc0	t2,C0_PRID
	mtc0	t0,C0_SR

#ifdef R4000	
	li	AT,PRID_RM7000
	srl	t2,8
	bne	t2,AT,1f
	nop
		
	/* RM7000 specific */
	LREG(t1,R_ICR,gp)		/* ICR */
	LREG(AT,R_WATCHMASK,gp)		/* WatchLo */
	ctc0	t1,C0C_ICR
	mtc0	AT,C0_WATCHMASK
	LREG(t1,R_IPLLO,gp)		/* IPLLo */
	LREG(AT,R_IPLHI,gp)		/* IPLHi */
	ctc0	t1,C0C_IPLLO
	ctc0	AT,C0C_IPLHI
1:		
#endif
		
#ifdef FLOATINGPT
#ifdef R4000
	and	t1,t0,SR_CU1
	beqz	t1,2f
	li	AT,PRID_R4650		# BDSLOT
	beq	t2,AT,.rest4650
	LREG(t2,R_FCR,gp)		# BDSLOT
#else
	and	t1,t0,SR_CU1
	beqz	t1,2f
	LREG(t2,R_FCR,gp)		# BDSLOT
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
#endif

#ifndef R3041
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
#endif /* !R3041 */
	
#ifdef R4000
	LREG(t0,R_WATCHLO,gp)		/* WatchpointLo */
	LREG(t1,R_WATCHHI,gp)		/* WatchpointHi */
	MTC0	t0,C0_WATCHLO
	MTC0	t1,C0_WATCHHI
	
	LREG(t0,R_ENTRYLO1,gp)		/* TLB EntryLo1 */
	LREG(t1,R_PGMASK,gp)		/* TLB PageMask */
	LREG(AT,R_WIRED,gp)		/* TLB Wired */
	mtc0	t0,C0_TLBLO1
	mtc0	t1,C0_PGMASK
	mtc0	AT,C0_WIRED
	LREG(t0,R_ECC,gp)		/* ECC */
	LREG(t1,R_TAGLO,gp)		/* TagLo */
	LREG(AT,R_TAGHI,gp)		/* TagHi */
	mtc0	t0,C0_ECC
	mtc0	t1,C0_TAGLO
	mtc0	AT,C0_TAGHI

	LREG(t2,R_EPC,gp)		/* EPC */
	LREG(AT,R_CAUSE,gp)		/* Cause - For Sw field only */
	MTC0	t2,C0_EPC
	mtc0	AT,C0_CAUSE
	LREG(AT,R_AT,gp)		/* $1  (AT) */
	LREG(t0,R_T0,gp)		/* $8  (t0) */
	LREG(t1,R_T1,gp)		/* $9  (t1) */
	LREG(t2,R_T2,gp)		/* $10 (t2) */
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


	.set noat
	.set noreorder
	.ent __bev
__bev:
	# k0 can be used to point to an exception handler
	beqz	k0,1f
	nop
	j	k0
	move	k0,zero
	.set reorder
	.set at

1:	MFC0	s0,C0_EPC
	MFC0	s1,C0_CAUSE
	MFC0	s3,C0_BADVADDR
	move	s2,ra
	
	SBD_DISPLAY('B','E','V',' ', CHKPNT_BEV)
	
	move	a0,s0
	move	a1,s1
	move	a2,s2
	move	a3,s3
	
__bev_fail:
	/* are we executing at our "link address" */
	bal	1f
1:	la	t1,1b
	and	t1,K1SIZE-1
	and	ra,K1SIZE-1
	bne	t1,ra,1f		# sorry, can't execute C code
	
	la      sp,stack+STKSIZE-24
	or      sp,K1BASE
	jal	pmexception
	/* shouldn't return */
1:	b	1b
	.end __bev


	.globl	_exception
	.ent _exception
/*
 * This routine is used to save the state of a client program after
 * an exception is encountered. But it first checks to see if there
 * is a user defined exception handler (via onintr), and then checks
 * to see if it's a floating-point instruction (if PMON has fp emulation
 * enabled.)
 */
_exception:
	.set noat
	
	la	k0,DBGREG
	SREG(k1,R_K1TMP,k0)

	/* if (curlst == &pmlst) branch to exc2 */
	la	k0,curlst
	lw	k0,(k0)
	la	k1,pmlst
	beq	k0,k1,exc2	# skip if in PMON

	# see if we have a user defined handler
	.set noreorder
	mfc0	k0,C0_CAUSE
	.set reorder
	la	k1,hndlrtbl
	and	k0,CAUSE_EXCMASK
	addu	k1,k0		# calc table entry addr
	move	k0,zero
	lw	k1,(k1)		# get contents of table entry
	beq	k1,zero,exc2
	lw	k0,4(k1)	# user routine addr
	la	k1,DBGREG
	LREG(k1,R_K1TMP,k1)	# restore k1
	.set noreorder
	j	k0		# jump to user handler
	move	k0,zero
	.set reorder

exc2:
#if defined(FLOATINGPT) && !defined(R4000)
	/* see if it's a cp1 unusable */
	.set noreorder
	mfc0	k0,C0_CAUSE

	.set reorder
	li	k1,(CAUSE_CEMASK|CAUSE_EXCMASK)
	and	k0,k1
	li	k1,(1<<CAUSE_CESHIFT)|CEXC_CPU
	bne	k0,k1,1f

	la	k0,cop1		# jump to emulation package
	la	k1,DBGREG
	LREG(k1,R_K1TMP,k1)	# restore k1
	.set noreorder
	j	k0
	move	k0,zero
	.set reorder
#endif

1:	# if (curlst == &clilst) branch to 1f
	la	k0,curlst
	lw	k0,(k0)
	la	k1,clilst
	.set noreorder
	beq	k0,k1,1f
	move	k0,zero			# BDSLOT

	MFC0	a0,C0_EPC
	MFC0	a1,C0_CAUSE
#ifndef LR33000
	MFC0	a3,C0_BADVADDR
#endif

	lw	t1,initial_sr
	li	t2,~SR_IMASK
	and	t1,t2
	mtc0	t1,C0_SR
	la	sp,stack+STKSIZE-24
	j	pmexception
	move	a2,ra
	.set reorder

1:	la	k1,DBGREG
	SREG(t0,R_T0,k1)		/* $8  (t0) */
	move	t0,k1			# now use t0 as pointer
	LREG(k1,R_K1TMP,t0)
	SREG(AT,R_AT,t0)		/* $1  (AT) */
	.set at
	SREG(v0,R_V0,t0)		/* $2  (v0) */
	SREG(v1,R_V1,t0)		/* $3  (v1) */
	SREG(a0,R_A0,t0)		/* $4  (a0) */
	SREG(a1,R_A1,t0)		/* $5  (a1) */
	SREG(a2,R_A2,t0)		/* $6  (a2) */
	SREG(a3,R_A3,t0)		/* $7  (a3) */
	SREG(t1,R_T1,t0)		/* $9  (t1) */
	SREG(t2,R_T2,t0)		/* $10 (t2) */
	SREG(t3,R_T3,t0)		/* $11 (t3) */
	SREG(t4,R_T4,t0)		/* $12 (t4) */
	SREG(t5,R_T5,t0)		/* $13 (t5) */
	SREG(t6,R_T6,t0)		/* $14 (t6) */
	SREG(t7,R_T7,t0)		/* $15 (t7) */
	SREG(s0,R_S0,t0)		/* $16 (s0) */
	SREG(s1,R_S1,t0)		/* $17 (s1) */
	SREG(s2,R_S2,t0)		/* $18 (s2) */
	SREG(s3,R_S3,t0)		/* $19 (s3) */
	SREG(s4,R_S4,t0)		/* $20 (s4) */
	SREG(s5,R_S5,t0)		/* $21 (s5) */
	SREG(s6,R_S6,t0)		/* $22 (s6) */
	SREG(s7,R_S7,t0)		/* $23 (s7) */
	SREG(t8,R_T8,t0)		/* $24 (t8) */
	SREG(t9,R_T9,t0)		/* $25 (t9) */
					/* $26 (k0) */
	SREG(k1,R_K1,t0)		/* $27 (k1) */
	SREG(gp,R_GP,t0)		/* $28 (gp) */
	SREG(sp,R_SP,t0)		/* $29 (sp) */
	SREG(s8,R_FP,t0)		/* $30 (s8) */
	SREG(ra,R_RA,t0)		/* $31 (ra) */

	mfhi	t1
	mflo	t2
	SREG(t1,R_HI,t0)		/* Hi */
	SREG(t2,R_LO,t0)		/* Lo */

	.set noreorder
#ifdef R4000	
	/* store and disable watchpoints */
	MFC0	t1,C0_WATCHLO
	MFC0	t2,C0_WATCHHI
	mfc0	t3,C0_PRID
	MTC0	zero,C0_WATCHLO
	MTC0	zero,C0_WATCHHI
	SREG(t1,R_WATCHLO,t0)		/* WatchLo */
	SREG(t2,R_WATCHHI,t0)		/* WatchHi */
	
	
	/* RM7000 only */
	srl	t3,8
	bne	t3,PRID_RM7000,1f
	nop
	mfc0	t1,C0_WATCHMASK
	cfc0	t2,C0C_ICR
	cfc0	t3,C0C_IPLLO
	SREG(t1,R_WATCHMASK,t0)		/* WatchLo */
	SREG(t2,R_ICR,t0)		/* ICR */
	SREG(t3,R_IPLLO,t0)		/* IPLLo */
	cfc0	t1,C0C_IPLHI
	ctc0	zero,C0C_ICR
	ctc0	zero,C0C_IPLLO
	ctc0	zero,C0C_IPLHI
	SREG(t1,R_IPLHI,t0)		/* IPLHi */
1:		
#endif
	
#ifdef LR33000
	mfc0	t1,C0_DCIC
	mfc0	t2,C0_BPC
	mfc0	t3,C0_BDA
	SREG(t1,R_DCIC,t0)		/* DCIC */
	SREG(t2,R_BPC,t0)		/* BPC */
	SREG(t3,R_BDA,t0)		/* BDA */
	and	t1,~DCIC_DE		# make sure the DE bit is cleared
	mtc0	t1,C0_DCIC
#else
#ifndef R3041
	mfc0	t1,C0_TLBHI
	MFC0	t2,C0_CTEXT
	mfc0	t3,C0_INX
	SREG(t1,R_ENTRYHI,t0)		/* EntryHi */
	SREG(t2,R_CONTEXT,t0)		/* Context */
	SREG(t3,R_INDEX,t0)		/* Index */
	mfc0	t1,C0_RAND
	mfc0	t2,C0_TLBLO
	MFC0	t3,C0_BADVADDR
	SREG(t1,R_RANDOM,t0)		/* Random */
	SREG(t2,R_ENTRYLO,t0)		/* EntryLo */
	SREG(t3,R_BADVA,t0)		/* BadVA */
#endif	/* !R3041 */
#ifdef R4000
	mfc0	t1,C0_TLBLO1
	mfc0	t2,C0_PGMASK
	mfc0	t3,C0_WIRED
	SREG(t1,R_ENTRYLO1,t0)		/* TLB EntryLo1 */
	SREG(t2,R_PGMASK,t0)		/* TLB PageMask */
	SREG(t3,R_WIRED,t0)		/* TLB Wired */
	mfc0	t1,C0_LLADDR
	mfc0	t2,C0_ECC
	mfc0	t3,C0_TAGLO
	SREG(t1,R_LLADDR,t0)		/* LLAddr */
	SREG(t2,R_ECC,t0)		/* ECC */
	SREG(t3,R_TAGLO,t0)		/* Taglo */
	mfc0	t1,C0_TAGHI
	mfc0	t2,C0_CACHEERR
	mfc0	t3,C0_ERREPC
#if __mips >= 3
	MFC0	t4,C0_XCTEXT
#endif
	SREG(t1,R_TAGHI,t0)		/* TagHi */
	SREG(t2,R_CACHERR,t0)		/* CacheErr */
	SREG(t3,R_ERREPC,t0)		/* ErrorEPC */
#if __mips >= 3
	SREG(t4,R_XCONTEXT,t0)		/* XContext */
#endif
#endif /* R4000 */
#endif /* !LR33000 */

	mfc0	t1,C0_CAUSE
	MFC0	t2,C0_EPC
	mfc0	t3,C0_SR
	SREG(t1,R_CAUSE,t0)		/* Cause */
	SREG(t2,R_EPC,t0)		/* EPC */
	mtc0	zero,C0_CAUSE		/* Clear pending watchpoint */
	
#ifdef R4000
	# save the SR value but clear EXL bit to
	# reflect state before the exception
	and	t3,~SR_EXL		# switch off EXL bit
#else
	# save the SR value but fixup the KU & IE bits to 
	# reflect their state before the exception
	li	t1,0x3f			# KU&IE mask
	and	t2,t3,t1		# extract KU&IE bits
	not	t1			# invert the mask
	and	t3,t1			# clear KU&IE field
	srl	t2,2			# fixup KU&IE field
	or	t3,t2			# insert the KU&IE field
#endif
	SREG(t3,R_STATUS,t0)		# save Status

#ifdef FLOATINGPT
#ifdef R4000
	mfc0	t1,C0_PRID
	and	t2,t3,SR_CU1
	beqz	t2,1f
	srl	t1,8			# BDSLOT
	cfc1	t2,C1_CSR		# flush fpu pipeline
	beq	t1,PRID_R4650,.save4650
	nop		
#else
	and	t2,t3,SR_CU1
	beqz	t2,1f
	nop		
	cfc1	t2,C1_CSR		# flush fpu pipeline
#endif
#if __mips >= 3
	sdc1	$f0,R_F0*8(t0)
	sdc1	$f2,R_F2*8(t0)
	sdc1	$f4,R_F4*8(t0)
	sdc1	$f6,R_F6*8(t0)
	sdc1	$f8,R_F8*8(t0)
	sdc1	$f10,R_F10*8(t0)
	sdc1	$f12,R_F12*8(t0)
	sdc1	$f14,R_F14*8(t0)
	sdc1	$f16,R_F16*8(t0)
	sdc1	$f18,R_F18*8(t0)
	sdc1	$f20,R_F20*8(t0)
	sdc1	$f22,R_F22*8(t0)
	sdc1	$f24,R_F24*8(t0)
	sdc1	$f26,R_F26*8(t0)
	sdc1	$f28,R_F28*8(t0)

	and	t1,t3,SR_FR
	beqz	t1,2f
	sdc1	$f30,R_F30*8(t0)

	/* SR.FR is set: save the other 16 registers too */
	sdc1	$f1,R_F1*8(t0)
	sdc1	$f3,R_F3*8(t0)
	sdc1	$f5,R_F5*8(t0)
	sdc1	$f7,R_F7*8(t0)
	sdc1	$f9,R_F9*8(t0)
	sdc1	$f11,R_F11*8(t0)
	sdc1	$f13,R_F13*8(t0)
	sdc1	$f15,R_F15*8(t0)
	sdc1	$f17,R_F17*8(t0)
	sdc1	$f19,R_F19*8(t0)
	sdc1	$f21,R_F21*8(t0)
	sdc1	$f23,R_F23*8(t0)
	sdc1	$f25,R_F25*8(t0)
	sdc1	$f27,R_F27*8(t0)
	sdc1	$f29,R_F29*8(t0)
	b	2f
	sdc1	$f31,R_F31*8(t0)
	
.save4650:	
	swc1	$f0,R_F0*8+LSW(t0)
	swc1	$f2,R_F2*8+LSW(t0)
	swc1	$f4,R_F4*8+LSW(t0)
	swc1	$f6,R_F6*8+LSW(t0)
	swc1	$f8,R_F8*8+LSW(t0)
	swc1	$f10,R_F10*8+LSW(t0)
	swc1	$f12,R_F12*8+LSW(t0)
	swc1	$f14,R_F14*8+LSW(t0)
	swc1	$f16,R_F16*8+LSW(t0)
	swc1	$f18,R_F18*8+LSW(t0)
	swc1	$f20,R_F20*8+LSW(t0)
	swc1	$f22,R_F22*8+LSW(t0)
	swc1	$f24,R_F24*8+LSW(t0)
	swc1	$f26,R_F26*8+LSW(t0)
	swc1	$f28,R_F28*8+LSW(t0)

	and	t1,t3,SR_FR
	beqz	t1,2f
	swc1	$f30,R_F30*8+LSW(t0)

	/* SR.FR is set: save the other 16 registers too */
	swc1	$f1,R_F1*8+LSW(t0)
	swc1	$f3,R_F3*8+LSW(t0)
	swc1	$f5,R_F5*8+LSW(t0)
	swc1	$f7,R_F7*8+LSW(t0)
	swc1	$f9,R_F9*8+LSW(t0)
	swc1	$f11,R_F11*8+LSW(t0)
	swc1	$f13,R_F13*8+LSW(t0)
	swc1	$f15,R_F15*8+LSW(t0)
	swc1	$f17,R_F17*8+LSW(t0)
	swc1	$f19,R_F19*8+LSW(t0)
	swc1	$f21,R_F21*8+LSW(t0)
	swc1	$f23,R_F23*8+LSW(t0)
	swc1	$f25,R_F25*8+LSW(t0)
	swc1	$f27,R_F27*8+LSW(t0)
	swc1	$f29,R_F29*8+LSW(t0)
	swc1	$f31,R_F31*8+LSW(t0)
#else
	swc1	$f0,R_F0*4(t0)
	swc1	$f1,R_F1*4(t0)
	swc1	$f2,R_F2*4(t0)
	swc1	$f3,R_F3*4(t0)
	swc1	$f4,R_F4*4(t0)
	swc1	$f5,R_F5*4(t0)
	swc1	$f6,R_F6*4(t0)
	swc1	$f7,R_F7*4(t0)
	swc1	$f8,R_F8*4(t0)
	swc1	$f9,R_F9*4(t0)
	swc1	$f10,R_F10*4(t0)
	swc1	$f11,R_F11*4(t0)
	swc1	$f12,R_F12*4(t0)
	swc1	$f13,R_F13*4(t0)
	swc1	$f14,R_F14*4(t0)
	swc1	$f15,R_F15*4(t0)
	swc1	$f16,R_F16*4(t0)
	swc1	$f17,R_F17*4(t0)
	swc1	$f18,R_F18*4(t0)
	swc1	$f19,R_F19*4(t0)
	swc1	$f20,R_F20*4(t0)
	swc1	$f21,R_F21*4(t0)
	swc1	$f22,R_F22*4(t0)
	swc1	$f23,R_F23*4(t0)
	swc1	$f24,R_F24*4(t0)
	swc1	$f25,R_F25*4(t0)
	swc1	$f26,R_F26*4(t0)
	swc1	$f27,R_F27*4(t0)
	swc1	$f28,R_F28*4(t0)
	swc1	$f29,R_F29*4(t0)
	swc1	$f30,R_F30*4(t0)
	b	2f
	swc1	$f31,R_F31*4(t0)
	
#ifdef R4000
.save4650:	
	swc1	$f0,R_F0*4(t0)
	swc1	$f2,R_F2*4(t0)
	swc1	$f4,R_F4*4(t0)
	swc1	$f6,R_F6*4(t0)
	swc1	$f8,R_F8*4(t0)
	swc1	$f10,R_F10*4(t0)
	swc1	$f12,R_F12*4(t0)
	swc1	$f14,R_F14*4(t0)
	swc1	$f16,R_F16*4(t0)
	swc1	$f18,R_F18*4(t0)
	swc1	$f20,R_F20*4(t0)
	swc1	$f22,R_F22*4(t0)
	swc1	$f24,R_F24*4(t0)
	swc1	$f26,R_F26*4(t0)
	swc1	$f28,R_F28*4(t0)

	and	t1,t3,SR_FR
	beqz	t1,2f
	swc1	$f30,R_F30*4(t0)

	/* SR.FR is set: save the other 16 registers too */
	swc1	$f1,R_F1*4(t0)
	swc1	$f3,R_F3*4(t0)
	swc1	$f5,R_F5*4(t0)
	swc1	$f7,R_F7*4(t0)
	swc1	$f9,R_F9*4(t0)
	swc1	$f11,R_F11*4(t0)
	swc1	$f13,R_F13*4(t0)
	swc1	$f15,R_F15*4(t0)
	swc1	$f17,R_F17*4(t0)
	swc1	$f19,R_F19*4(t0)
	swc1	$f21,R_F21*4(t0)
	swc1	$f23,R_F23*4(t0)
	swc1	$f25,R_F25*4(t0)
	swc1	$f27,R_F27*4(t0)
	swc1	$f29,R_F29*4(t0)
	swc1	$f31,R_F31*4(t0)
#endif
#endif
2:	SREG(t2,R_FCR,t0)
	ctc1	zero,C1_CSR
	nop; nop
1:
#endif /* FLOATINGPT */

	# switch us back to the correct kernel/interrupt mode, but with
	# all external interrupts masked out.
	la	k0,_exception		# reset boot exception vector
	lw	t3,initial_sr
	and	t3,~SR_IMASK
	mtc0	t3,C0_SR
	.set reorder

	la	t0,exception
	la	sp,stack+STKSIZE-24
	j	t0
	.end _exception

	.globl onintr
	.ent onintr
/*
 * Used to install user defined exception handlers.
 */
onintr: # a0=exec a1=struct
	sll	a0,2
	la	t0,idummy
	sw	t0,(a1)
	la	t1,hndlrtbl
	addu	t1,a0
	lw	t0,(t1)
	beq	t0,zero,1f
	sw	t0,(a1)
1:	sw	a1,(t1)
	j	ra
	.end onintr

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

#if LOCAL_MEM == 0
	# recopy the handler
	la      a0,handler
	la      a1,ehandler
	li      a2,0x0000000           # utlb miss
	jal     copyHandler
#ifdef LR33000
	li      a2,0x0000040           # debug 
	jal     copyHandler
#endif
	li      a2,0x0000080           # general/r4000 xtlb vector
	jal     copyHandler
#ifdef R4000
	li	a2,0x0000180		# r4000 general
	jal     copyHandler

	la      a0,cerrhandler
	la      a1,ecerrhandler
	li	a2,0x0000100		# r4000 cache error
	jal     copyHandler
#endif
#endif
	
	li	a0,ICACHE
	jal	flush_cache

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

#if 0	
	.globl clr_iec
	.ent clr_iec
/*
 * This is used make sure that interrupts are disabled.
 */
clr_iec:
	.set noreorder
	mfc0	t0,C0_SR
	nop
#ifdef R4000
	and	t0,~(SR_IE | SR_EXL | SR_ERL)
#else
	and	t0,~SR_IEC
#endif
	mtc0	t0,C0_SR
	.set reorder
	j	ra
	.end clr_iec
#endif

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
	or	a0,v0
	mtc0	a0,C0_SR
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
	not	a0
	and	a0,v0
	mtc0	a0,C0_SR
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
	mtc0	a0,C0_SR
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
#if __mipsfp64
	or	v0,t0,SR_FR|SR_CU1
#else
	or	v0,t0,SR_CU1
#endif
	mtc0	v0,C0_SR
	nop
#ifdef R4000
	nop
#endif		
	mfc0	v0,C0_SR	# make sure CU1 got written...
	nop
#ifdef R4000	
	nop
#endif	
	and	t1,v0,SR_CU1
	beqz	t1,1f
	nop			# DELAY SLOT

	# stuff pattern into f0 and f1
	li	t1,0xaaaa5555
#ifdef R4000
	nop
#endif

	mtc1	t1,$f0
	mtc1	zero,$f2

	# retrieve pattern
	mfc1	t2,$f0
	mfc1	t3,$f2
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
#else
	mfc0	t0,C0_SR
	and	t0,~SR_CU1
	mtc0	t0,C0_SR	
#endif /*FLOATINGPT*/
	move	v0,t0
	j	ra
	.end enableCU1

        .globl copyHandler
	.ent copyHandler
/*
 * Used to copy the exception handler to the various
 * vector addresses.
 */
copyHandler:
	# a0=src a1=end a2=dst
	# must not change a0 or a1
	# must force a0 & a1 to kseg1
	or      t0,a0,K1BASE
	or      t1,a1,K1BASE
	or      a2,K1BASE
1:      lr      v0,(t0)
	sr      v0,(a2)
	addu    t0,REGSZ
	addu    a2,REGSZ
	bltu    t0,t1,1b
	j       ra
	.end copyHandler


#ifdef R4000
/* save registers just below general exception handler */
#define AT_SAVE	+(0x180-0x08)
#define V0_SAVE	+(0x180-0x10)
#define V1_SAVE	+(0x180-0x18)
#define A0_SAVE	+(0x180-0x20)
#define A1_SAVE	+(0x180-0x28)
#define A2_SAVE	+(0x180-0x30)
#define A3_SAVE	+(0x180-0x38)
#define SR_SAVE	+(0x180-0x40)

#if __mips >= 3
#define sreg	sd
#define lreg	ld
#else
#define sreg	sw
#define lreg	lw
#endif
	

	.globl	cerrhandler
	.ent	cerrhandler
#if __mips >= 3
	.align	3
#endif		
cerrhandler:
	.set	noat
	sreg	AT,AT_SAVE(zero)
	sreg	v0,V0_SAVE(zero)
	.set	at
	la	v0,cache_err
	or	v0,K1BASE
	j	v0
	# end of low-memory vector
ecerrhandler:

cache_err:
	.set	noreorder
	sreg	v1,V1_SAVE(zero)
	mfc0	v0,C0_CACHEERR	
	mfc0	v1,C0_TAGLO		# save taglo
	sreg	a0,A0_SAVE(zero)
	mfc0	a0,C0_SR
	sreg	a1,A1_SAVE(zero)
	sreg	a2,A2_SAVE(zero)
	sreg	a3,A3_SAVE(zero)
	sreg	a0,SR_SAVE(zero)
	.set	reorder

	or	a0,SR_DE
	mtc0	a0,C0_SR
	
	and	a1,v0,CACHEERR_BUS	# sysAD bus error?
	bnez	a1,mem_err
	
	and	a1,v0,CACHEERR_TAG	# cache tag error?
	bnez	a1,tag_err

	and	a1,v0,CACHEERR_DATA	# cache data error?
	beqz	a1,fixed		# no error!

	/* generate primary cache address to use for indexed cacheops */
	/* low order bits of address (virtual == physical) */
	and	a0,v0,CACHEERR_SIDX_MASK & ((1<<CACHEERR_PIDX_SHIFT)-1)
	/* high order bits of virtual address */
	and	a1,v0,CACHEERR_PIDX_MASK
	sll	a1,CACHEERR_PIDX_SHIFT
	or	a0,a1			# a0 = primary cache index
	addu	a0,K0BASE
	
	and	a1,v0,CACHEERR_LEVEL	# secondary cache error?
	bnez	a1,scache_err

	# cache error is in one or both primary caches
	
pcache_err:
	/* determine which cache */
	and	a1,v0,CACHEERR_TYPE
	bnez	a1,pdcache

picache:
	/* zap primary instruction cache line */
	.set	noreorder
	mtc0	zero,C0_TAGLO
	nop
        cache   Index_Store_Tag_I,0(a0)	# clear tag
	nop; nop
        cache   Fill_I,0(a0)		# overwrite data
	nop; nop
        cache   Index_Store_Tag_I,0(a0)	# clear tag
	.set	reorder
	
	and	a1,v0,CACHEERR_BOTH	# data error too?
	beqz	a1,fixed		# no - all done
#if 0
	/* XXX flush data caches */
#else	
	b	giveup
#endif
	
pdcache:
	/* give up on a dirty line (no ecc) */
	
	/* Note that this tests the tag's state field for Shared or
	   Dirty-Exclusive.  On the R4x00PC this is not very useful as
	   all valid lines are Dirty-Exclusive and we cannot get at the
	   W-bit which indicates that the line is modified.  So any
	   D-cache error on an R4000PC will be fatal. */
	.set	noreorder
        cache   Index_Load_Tag_D,0(a0)
	nop
	mfc0	a1,C0_TAGLO
	nop; nop
	and	a1,1<<TAG_PSTATE_SHIFT	# Shared/Dirty-Exclusive?
	bnez	a1,giveup
	nop

	/* Clean/Invalid line: we can safely zap it */
	mtc0	zero,C0_TAGLO
	nop
        cache   Index_Store_Tag_D,0(a0)	# clear tag
        lw	zero,0(a0)		# write data from secondary/memory
        cache   Index_Store_Tag_D,0(a0)	# clear tag
	.set	reorder
	
	b	fixed
	
scache_err:
	/* cache error is in one or both secondary caches */
	/* generate cache address to use for cacheops */
	and	a2,v0,CACHEERR_SIDX_MASK
	addu	a2,K0BASE

	/* determine which cache */
	and	a1,v0,CACHEERR_TYPE
	bnez	a1,sdcache

sicache:
	/* just zap the secondary instruction cache line */
	.set	noreorder
	mtc0	zero,C0_TAGLO
	nop
        cache   Index_Store_Tag_SI,0(a2)	# clear SI tag
	nop; nop	
        cache   Index_Store_Tag_I,0(a2)		# clear I tag
	nop; nop	
	cache	Fill_I,0(a2)			# fill I from SI (ignore ecc)
	nop; nop	
	cache	Hit_Writeback_I,0(a2)		# write I to SI (generate ecc)
	nop; nop	
        cache   Index_Store_Tag_I,0(a2)		# clear I tag
	nop; nop	
        cache   Index_Store_Tag_SI,0(a2)	# clear SI tag
	
	/* need to reinitialise data field too */
	.set	reorder
	
	and	a1,v0,CACHEERR_BOTH	# data error too?
	beqz	a1,fixed		# no - all done
#if 0	
	/* flush data caches */
#else	
	b	giveup
#endif
	
sdcache:
	/* give up on a dirty line (XXX could try ECC correction) */
	.set	noreorder
        cache   Index_Load_Tag_SD,0(a2)
	nop
#ifndef FIXSCACHE
	mtc0	zero,C0_TAGLO
	nop; nop
        cache   Index_Store_Tag_SD,0(a2)	/* clear error? */
	nop; nop
	sw	zero,0(a2)
	nop; nop
	cache	Hit_Writeback_Inv_D,0(a2)
	.set	reorder
	b	giveup	
#else
	mfc0	a1,C0_TAGLO
	nop; nop
	and	a1,1<<(TAG_SSTATE_SHIFT+1)	# Shared/Dirty-Exclusive?
	bnez	a1,giveup			# ECC?
	nop

	mtc0	zero,C0_TAGLO
	and	a1,v0,CACHEERR_REFILL
	beqz	a1,1f
	nop
	
	/* Zap invalid line in dcache */
	cache	Index_Store_Tag_D,0(a0)
	
	/* Clean/Invalid line: we can safely zap it */
1:	cache	Create_Dirty_Exc_SD,0(a2)
	cache	Create_Dirty_Exc_D,0(a2)
        sw	zero,0(a2)		# set W-bit
	nop; nop
	cache	Hit_Writeback_Inv_D,0(a2)
	nop; nop
        cache   Index_Store_Tag_SD,0(a2) # clear tag
        cache   Index_Store_Tag_D,0(a2)	# clear tag
	.set	reorder
	
	b	fixed
#endif
		
fixed:	
	/* record CacheErr register in fifo for later output */
	la	a0,cerrfifo
	or	a0,K1BASE
	lw	a1,ce_in(a0)
	addu	a2,a1,1
	and	a2,CERR_FIFO_MASK
	sw	a2,ce_in(a0)
	sll	a1,2
	addu	a0,a1
	sw	v0,ce_buf(a0)
	
	.set	noreorder
	lreg	a0,SR_SAVE(zero)
	mtc0	v1,C0_TAGLO	 	# restore taglo
	lreg	a3,A3_SAVE(zero)
	lreg	a2,A2_SAVE(zero)
	mtc0	a0,C0_SR
	lreg	a1,A1_SAVE(zero)
	lreg	a0,A0_SAVE(zero)
	lreg	v1,V1_SAVE(zero)
	lreg	v0,V0_SAVE(zero)
	.set	noat
	lreg	AT,AT_SAVE(zero)
	eret
	nop
	.set	at
	.set	reorder

	
tag_err:	
mem_err:
giveup:	
	/* we're not going to return, so we don't have to save registers */
	move	s2,ra
	move	s3,v0

	# make sure we stay uncached, even if linked in kseg0
	.set	noreorder
	mfc0	t1,C0_CONFIG
	nop
	and	t1,~CFG_K0MASK
	or	t1,CFG_C_UNCACHED
	mtc0	t1,C0_CONFIG
	nop; nop; nop
	.set	reorder
	
	/* get relocation offset */
	bal	1f
1:	la	t1,1b
	subu	s8,ra,t1

	/* flush all caches */
	la	t0,mips_flush_cache
	addu	t0,s8
	jal	t0
	
	.set	noreorder
	mfc0	a0,C0_ERREPC
	.set	reorder
	li	a1,CEXC_CACHE
	move	a2,s2
	move	a3,s3
	
	b	__bev_fail
	.end	cerrhandler


	.globl	set_count
	.ent	set_count
set_count:
	.set	noreorder
	mtc0	a0,C0_COUNT
	.set	reorder
	j	ra
	.end	set_count

	.globl	get_count
	.ent	get_count
get_count:
	.set	noreorder
	mfc0	v0,C0_COUNT
	.set	reorder
	j	ra
	.end	get_count

	.globl	set_compare
	.ent	set_compare
set_compare:
	.set	noreorder
	mtc0	a0,C0_COMPARE
	.set	reorder
	j	ra
	.end	set_compare

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
	
	.globl	__assfail
	.ent	__assfail
__assfail:	
	jal	printf
1:	b	1b
	.end	__assfail
	
/*
 * This is used by onintr to terminate the list of user defined handlers
 * for a given EXC value.
 */
	.data
idummy:	.word	0
	.word	exc2


