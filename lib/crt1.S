/* $Id: crt1.S,v 1.3 1999/06/23 13:13:45 nigel Exp $ */
#include "mips.h"

/*
 * Start-up code for executing a C program under LSI Logic's Prom Monitor 
 */


#define TIMER_ENABLED /* enable the timer to support the 'g -t' command */
#define TIMEREQ 0x74

#if 0	
#ifdef R4000
#define TBLBASE 0xbfc00400
#else
#define TBLBASE 0xbfc00200
#endif
#else
/* Algor PMON */
#define TBLBASE 0xbfc00500
#endif
	
#define TBLENTRY(n,m) 			\
	.globl m; 			\
	.ent m;				\
m:	li	t0,TBLBASE+(4*n);	\
	lw	t0,(t0);		\
	j	t0;			\
	.end m

#ifndef BSOTSUN
	.comm _timing,8
	.comm _filebase,4
#endif

	.globl _exit
	.globl _start
	.ent _start
_start:
	# clear bss
#ifdef BSOTSUN
	la	t0,__lc_cp
#else
	la	v0,_fbss
	la	v1,end
#endif
1:
	sw	zero,0(v0)
	sw	zero,4(v0)
	sw	zero,8(v0)
	sw	zero,12(v0)
	addiu	v0,v0,16
	sltu	t0,v0,v1
	bne	t0,zero,1b

	# set the global data pointer
#ifdef BSOTSUN
	la	gp,__lc_zp
#else
	la	gp,_gp

	# save file base address
	sw	a3,_filebase
#endif

#ifdef TIMER_ENABLED
	# is this a timing request?
	lw	t0,(a1)
	lbu	t0,(t0)
	bne	t0,TIMEREQ,1f

	move	s0,a0
	move	s1,a1
	li	a0,0
	jal	time
	la	t4,_timing
	li	t0,1
	sw	t0,(t4)
	sw	v0,4(t4)
	move	a0,s0
	move	a1,s1
#endif

1: 	# call the main C routine
	jal	main
_exit:

#ifdef TIMER_ENABLED
	# was this a timing request?
	la	s4,_timing
	lw	t0,(s4)
	beq	t0,zero,1f

	li	a0,0
	jal	time
	lw	t0,4(s4)
	subu	a1,v0,t0
	la	a0,timefmt
	jal	printf
#endif

1:
	li	t0,TBLBASE+(4*8)
	lw	t0,(t0)
	j	t0
	.end _start


/*************************************************************
*  sbrk(size)
*	returns a pointer to a block of memory of the requested size.
*	Returns zero if heap overflow is detected. Heap overflow occurs
*	when the upper limit of the requested size, overlaps the stack
*	pointer.
*/
	.globl sbrk
	.ent sbrk
sbrk:
	li	v0,0
	la	t0,allocp1
	lw	t6,(t0)
	and	t1,t6,0xe0000000	# get seg
	and	t2,sp,~0xe0000000
	or	t1,t2			# heap limit
	addu	t7,t6,a0
	blt	t7,t1,1f
	j	ra
1:	sw	t7,(t0)
	subu	v0,t7,a0
	j	ra
	.end sbrk

/*************************************************************
*  PMON entry table
*/
TBLENTRY(0,_read)
TBLENTRY(1,_write)
TBLENTRY(2,_open)
TBLENTRY(3,_close)
TBLENTRY(4,ioctl)
TBLENTRY(5,printf)
TBLENTRY(6,vsprintf)
TBLENTRY(7,ttctl)
TBLENTRY(9,getenv)
TBLENTRY(10,onintr)
TBLENTRY(11,flush_cache)

	.data
	.globl allocp1
#ifdef BSOTSUN
allocp1: .word  0x80030000
#else
	.word _ftext
	.word etext
allocp1: .word  end
#endif
timefmt: .asciiz "time %d secs\n"

