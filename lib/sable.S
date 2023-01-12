/* $Id: sable.S,v 1.2 1996/01/16 14:18:24 chris Exp $ */
#include "mips.h"
/* polled I/O routines for sable */

#define OP_INIT 1
#define OP_TX 2
#define OP_RX 3
#define OP_RXRDY 4
#define OP_TXRDY 5
#define OP_BAUD 6

	# duart definition
	SR = 0
		RXRDY = 0x01
	RHR = 2
	THR = 3

	# Sioinfo definition
	SIOBASE = 0

#define siobase t2

	.text

	.globl sable
	.ent sable
sable:
	# a0=op a1=siodat a2=chan a3=ch 
	subu	sp,24
	sw	ra,20(sp)

	li	v0,0
	sll	a2,3		# chan *= 8;

	li	t0,OP_INIT
	subu	t0,a0
	bne	t0,zero,9f
	# null function
	b	10f

9:	li	t0,OP_TX
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# siobase[chan+THR] = ch
	addu	t1,a2,THR
	sll	t1,2		# word offset
	addu	t1,siobase
	sw	a3,(t1)
	b	10f

9:	li	t0,OP_RX
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# r = siobase[chan+RHR]
	addu	t1,a2,RHR
	sll	t1,2		# word offset
	addu	t1,siobase
	lw	v0,(t1)
	and	t0,v0,0x7f
	li	t1,0x0C		# ^K?
	bne	t0,t1,1f
	jal	_exit
1:	b	10f

9:	li	t0,OP_TXRDY
	subu	t0,a0
	bne	t0,zero,9f

	li	v0,1
1:	b	10f

9:	li	t0,OP_RXRDY
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# siobase[chan+SR] = ch
	addu	t1,a2,SR
	sll	t1,2		# word offset
	addu	t1,siobase
	lw	t0,(t1)

	/* # if (c&RXRDY) r = 1 */
	and	t0,RXRDY
	beq	t0,zero,1f
	li	v0,1
1:	b	10f
9:
10:
	lw	ra,20(sp)
	addu	sp,24
	j	ra
	.end	sable

