/* $Id: e4isr.s,v 1.2 1996/01/16 14:16:53 chris Exp $ */
#include <mips.h>

/*************************************************************
* ex4isr.c
*	Part of the program onintr.c. For more information
*	about writing exception handlers, see LSI Logic's
*	application note "Interrupt Handlers and Interrupt Latency
*	Calculations for the LR33000 Self-Embedding Processor".
*	This example increments the variable 'ticks' each time
*	a timer2 interrupt occurs.
*	
*/

	.extern ticks,4		# permit GP relative addressing

/*************************************************************
*  exisr()
*	The exception handler for onintr.c
*/
	.globl e4isr
	.ent e4isr
        .set noat
e4isr:
        .set noreorder
        mfc0    k0,C0_CAUSE
        nop
        .set reorder
        /* check if it's an int1 (timer2) */
        sll     k0,(31-11)	# test bit 11 (IP4,int1)
        bgez    k0,1f   	# branch if bit not set

	/* check if it's a timer2 interrupt */
        li      k0,M_TC2       
        lw      k0,(k0)
        sll     k0,(31-0)	# test bit 0 (TC_INT)
        bltz    k0,2f      	# branch if bit set

	# call the next handler in the chain
1:	la      k0,e4dat
        lw      k0,(k0)
        lw      k0,4(k0)
        j       k0

2:	#########################################################
	#							#
	#	Body of users ISR				#

	#   k0 is the only register you can use without
	#   saving it first. So save t0
	subu	sp,4
	sw	t0,(sp)

	# acknowledge the interrupt
        li      k0,M_TC2
        lw      t0,(k0)
        srl     t0,1
        sll     t0,1
        sw      t0,(k0)

	# increase the tick count
#ifdef ALGSDE
	la	k0,ticks
	lw	t0,(k0)
	addu	t0,1
	sw	t0,(k0)
#else
	/* this code relies on gp being set correctly! */
	lw	k0,ticks
	addu	k0,1
	sw	k0,ticks
#endif
	# restore t0
	lw	t0,(sp)
	addu	sp,4

	#							#
	#########################################################

	# return to the interrupted program
        .set noreorder
        mfc0    k0,C0_EPC
        nop
        j       k0
        rfe
        .set reorder
        .set at
	.end e4isr

/*************************************************************
*  enabint(ints)
*	Enable the specified interrupts
*/
	.globl enabint
	.ent enabint
enabint:
	.set noreorder
	mfc0	t0,C0_SR
	nop
	or	t0,a0
	or	t0,SR_IEC
	mtc0	t0,C0_SR
	.set reorder
	j	ra
	.end enabint

