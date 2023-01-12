/* $Id: clock.S,v 1.2 1996/01/16 14:17:43 chris Exp $ */
#ifdef LR33000
#include "mips.h"

#define MSECS(x)	1000*(x)

	.extern _time,4
	.extern	_tenths,4
	.extern	_acc,4
	.extern	_tstart,4
	.extern clkfreq,4

	.sdata
	.globl clkdat
clkdat:	.word	0
	.word 	clkisr

/*************************************************************
*  clkisr
*	The interrupt handler used to support the functions in
*	time.c
*/
	.text
	.globl clkisr
	.ent clkisr
	.set noat
clkisr: /* first check to see if it's an int0 */
	.set noreorder
	mfc0	k0,C0_CAUSE
	nop
	.set reorder
	/* if it's not for me, return */
	sll	k0,(31-11)
	bltz	k0,1f	# branch if bit 11 is set (int1)

2:	la	k0,clkdat
	lw	k0,(k0)
	lw	k0,4(k0)
	j	k0	# return to exception handler

   /* It is an int0 so first acknowledge the timer's int request */
1: # do this by clearing the INT bit (bit 0).	

	li	k0,M_TC2	/* check if it's a clock interrupt */
	lw	k0,(k0)
	sll	k0,31
	beq	k0,zero,2b	/* no it's not so go to next handler */

	subu	sp,4
	sw	k1,(sp)		# save k1

	li	k0,M_TC2
	lw	k1,(k0)
	srl	k1,1
	sll	k1,1
	sw	k1,(k0)

	# inc tenths
	la	k0,_tenths
	lw	k1,(k0)
	addu	k1,1
	sw	k1,(k0)

	# overflow?
	subu	k1,10
	bne	k1,zero,2f

	# yes, clr tenths
	la	k0,_tenths
	sw	zero,(k0)

	# update time
	la	k0,_time
	lw	k1,(k0)
	addu	k1,1
	sw	k1,(k0)

2:	# inc acc
	la	k0,_acc
	lw	k1,(k0)
	addu	k1,1
	sw	k1,(k0)

 # return to the interrupted program
	lw	k1,(sp)
	addu	sp,4		# restore k1

	.set noreorder
	mfc0	k0,C0_EPC
	nop
	j	k0
	rfe
	.set reorder
	.set at
	.end clkisr

/*************************************************************
*  startclk
*	Normally this is handled when you make the first call to
*	one of the functions in time.c, but it's here if you need
*	it.
*/
	.globl startclk
	.ent startclk
startclk: 
	# enable int0 and IEC
	.set noreorder
	mfc0	t0,C0_SR
	li	t1,(SR_IEC|SR_IBIT4)
	or	t0,t1
	mtc0	t0,C0_SR
	.set reorder

	# set the rate
	li	t1,M_TIC2
	lw	t0,clkfreq
	mul	t0,MSECS(100)
	sw	t0,(t1)

	# enable counting and ints
	li	t0,(TC_CE|TC_IE)
	li	t1,M_TC2
	sw	t0,(t1)
	j	ra
	.end startclk
#endif
