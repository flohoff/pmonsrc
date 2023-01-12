/* $Id: p2681.S,v 1.2 1996/01/16 14:24:42 chris Exp $ */
#include "mips.h"
#include "termio.h"
#include "p2681.h"

/* polled I/O routines for the 2681 */

	DELAY = 30	# delay used between writes
	IACR  = 0xf0	# initial value for ACR acr[7] timer=7

/* offsets and bit field definitions for the 2681 duart */
	SR = 1
		RXRDY = 0x01
		TXRDY = 0x04
	CSR = 1
	CMD = 2
	RHR = 3
	THR = 3
	ACR = 4
	IMR = 5
		TXINT = 0x01
		RXINT = 0x02
		CNTINT = 0x08
	ISR = 5
		ITXRDY = 0x01
		IRXRDY = 0x02
		CNTRDY = 0x08
	SETOCR = 14
	STOPCNT = 15
	CLROCR = 15

	.data
	.globl inittab
inittab:
	.byte	5,0x00		# mask off all ints

	# resets
	.byte	2,0x0a		# disable tx & rx
	.byte	2,0x10		# reset MR ptr
	.byte	2,0x20		# reset rx
	.byte	2,0x30		# reset tx
	.byte	10,0x0a		# disable tx & rx
	.byte	10,0x10		# reset MR ptr
	.byte	10,0x20		# reset rx
	.byte	10,0x30		# reset tx

	# MRs
	.byte 	0,0x13		# no parity, 8 bits data
	.byte	0,0x0f		# 2 stop bits
	.byte 	8,0x13		# no parity, 8 bits data
	.byte	8,0x0f		# 2 stop bits

	# timer
	.byte	4,IACR		# 
	.byte	6,0x2d		# 11,520 (0.1 secs)
	.byte	7,0x00

	# enable status outputs
	.byte	13,0x00
	.byte	15,0xff		# reset all output bits
	.byte	14,0x00		/* don't set any output bits */

	# enable rxs& txs
	.byte	2,0x05
	.byte	10,0x05

	# enable ints, rx A&B + timer
 	# .byte	5,0x2a		

	.byte	255

	.globl btab
btab:
 	#  CSR,ACR7	
	ACR7 = 1
	/* if CSR == 255 baud rate not supported */
	/* if ACR7 == 2 then that bit is don't care */
	.byte 	255,2	# B0
	.byte 	0,0	# B50
	.byte 	0,1	# B75
	.byte 	1,2	# B110
	.byte 	2,2	# B134
	.byte 	3,1	# B150
	.byte 	3,0	# B200
	.byte 	4,2	# B300
	.byte 	5,2	# B600
	.byte 	6,2	# B1200
	.byte 	10,1	# B1800
	.byte 	8,2	# B2400
	.byte 	9,2	# B4800
	.byte 	11,2	# B9600
	.byte 	12,1	# B19200
	.byte 	12,0	# B38400

	.text

#define siobase s0

	.globl	p2681
	.ent p2681
p2681:
	# a0=op a1=siodat a2=chan a3=ch 
	subu	sp,32
	sw	ra,28(sp)
	sw	s1,24(sp)
	sw	s0,20(sp)

	li	v0,0

9:	li	t0,OP_RXRDY
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# c = siobase[chan+SR]
	sll	a2,3		# chan *= 8;
	addu	t1,a2,SR
	sll	t1,2		# word offset
	addu	t1,siobase
	lbu	t0,(t1)

	/* if (c&RXRDY) r = 1 */
	and	t0,RXRDY
	beq	t0,zero,1f
	li	v0,1
1:	b	10f

9:	li	t0,OP_RX
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# r = siobase[chan+RHR]
	sll	a2,3		# chan *= 8;
	addu	t1,a2,RHR
	sll	t1,2		# word offset
	addu	t1,siobase
	lbu	v0,(t1)
1:	b	10f

9:	li	t0,OP_TXRDY
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# siobase[chan+SR] = ch
	sll	a2,3		# chan *= 8;
	addu	t1,a2,SR
	sll	t1,2		# word offset
	addu	t1,siobase
	lbu	t0,(t1)

	/* if (c&TXRDY) r = 1 */
	and	t0,TXRDY
	beq	t0,zero,1f
	li	v0,1
1:	b	10f

9:	li	t0,OP_TX
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# siobase[chan+THR] = ch
	sll	a2,3		# chan *= 8;
	addu	t1,a2,THR
	sll	t1,2		# word offset
	addu	t1,siobase
	sb	a3,(t1)
1:	b	10f

9:	li	t0,OP_INIT
	subu	t0,a0
	bne	t0,zero,9f

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	la	s1,inittab

2:	lbu	t0,(s1)
	li	t2,255
	beq	t0,t2,2f	# table done
	sll	t0,2
	addu	t0,siobase
	lbu	t1,1(s1)
	sb	t1,(t0)

	jal	wbflush

	addu	s1,2		# next table entry
	li	t0,DELAY
1:	subu	t0,1
	bne	t0,zero,1b

	b	2b

2:	# set the initial value of CURACR
	li	t0,IACR
	sb	t0,CURACR(a1)

 	b	10f

9:	li	t0,OP_BAUD
	subu	t0,a0
	bne	t0,zero,9f

	jal	setbr

1:	b	10f
9:
10:
	lw	ra,28(sp)
	lw	s1,24(sp)
	lw	s0,20(sp)
	addu	sp,32
	j	ra
	.end	p2681

	.globl setbr
	.ent setbr
setbr:	# a0=op a1=siodat a2=chan a3=brate
	subu	sp,32
	sw	ra,28(sp)
	sw	s1,24(sp)
	sw	s0,20(sp)

	li	v0,1		# default is return with error

	# siobase = siodat->siobase
	lw	siobase,SIOBASE(a1)

	# check brate <= 15
	li	t0,15
	bgt	a3,t0,10f	# baud rate too large

	# t4 = &btab[a3]
	sll	t4,a3,1	# hword aligned table entries
	la	t0,btab
	addu	t4,t0

	/* see if it's a supported value */
	lbu	t1,(t4)
	li	t0,255
	beq	t1,t0,10f	# unsupported baud rate

	/* see if it's an allowed combination */

	# find existing brate for other channel
	# t5 = siodat->brate[otherchan]
	addu	t0,a1,BRATE
	xor	t1,a2,1		# other chan
	addu	t0,t1
	lbu	t5,(t0)

	# t6 = &btab[t5]
	sll	t6,t5,1	# hword aligned table entries
	la	t0,btab
	addu	t6,t0

	/* if ACRs add up to 1 it's not allowed */
	lbu	t1,ACR7(t4)
	lbu	t0,ACR7(t6)
	addu	t1,t0
	li	t0,1
	beq	t0,t1,10f	# disallowed combination

	/* we're going to change something, so disable rx and tx */
	sll	t2,a2,3		# chan *= 8
	addu	t1,t2,CMD
	sll	t1,2		# word offset
	addu	t1,siobase
	li	t0,0x0a		# disable rx & tx
	sb	t0,(t1)

	# do we need to change ACR?
	lbu	t1,ACR7(t4)
	li	t0,2		/* is ours a don't care? */
	beq	t1,t0,2f

	# is our.ACR7 == his.ACR7
	lbu	t1,ACR7(t4)	# ours
	lbu	t0,ACR7(t6)	# his
	beq	t1,t0,2f	# no change necessary

	# need to change ACR7

	# read existing ACR value from memory copy
	lbu	t7,CURACR(a1)

	# or in our bit ACR7
	and	t7,0x7f		# clear it first
	lbu	t1,ACR7(t4)	# get required value
	sll	t1,7		# shift it into position
	or	t7,t1		# set it

	# wait a while
	li	t0,DELAY
1:	subu	t0,1
	bne	t0,zero,1b

	# write new ACR
	li	t1,ACR
	sll	t1,2		# word offset
	addu	t1,siobase
	sb	t7,(t1)

	# update memory copy
	sb	t7,CURACR(a1)

2:	# need duplicate values for rx and tx
	lbu	t1,(t4)
	sll	t7,t1,4
	or	t7,t1

	# wait a while
	li	t0,DELAY
1:	subu	t0,1
	bne	t0,zero,1b

	# siobase[chan+CSR] = rate
	sll	t2,a2,3		# chan *= 8;
	addu	t1,t2,CSR
	sll	t1,2		# word offset
	addu	t1,siobase
	sb	t7,(t1)

	# wait a while
	li	t0,DELAY
1:	subu	t0,1
	bne	t0,zero,1b

	# reenable rx and tx
	sll	t2,a2,3		# chan *= 8
	addu	t1,t2,CMD
	sll	t1,2		# word offset
	addu	t1,siobase
	li	t0,0x05		# enable rx & tx
	sb	t0,(t1)

	# update memory copy
	addu	t0,a1,BRATE
	addu	t0,a2		# chan
	sb	a3,(t0)
	li	v0,0

10:	lw	ra,28(sp)
	lw	s1,24(sp)
	lw	s0,20(sp)
	addu	sp,32
	j	ra
	.end setbr

