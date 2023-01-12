/* $Id: longjmp.S,v 1.2 1996/01/16 14:18:10 chris Exp $ */
#include <mips.h>

#if __mips >= 3
#define SREG(reg,rno,base)	sd	reg,+((rno)*8)(base)
#define LREG(reg,rno,base)	ld	reg,+((rno)*8)(base)
#else
#define SREG(reg,rno,base)	sw	reg,+((rno)*4)(base)
#define LREG(reg,rno,base)	lw	reg,+((rno)*4)(base)
#endif

/** setjmp(buffer) save context */
	.globl	setjmp
	.ent setjmp
setjmp:
	SREG(s0,JB_S0,a0)
	SREG(s1,JB_S1,a0)
	SREG(s2,JB_S2,a0)
	SREG(s3,JB_S3,a0)
	SREG(s4,JB_S4,a0)
	SREG(s5,JB_S5,a0)
	SREG(s6,JB_S6,a0)
	SREG(s7,JB_S7,a0)
	SREG(fp,JB_FP,a0)
	SREG(sp,JB_SP,a0)
	SREG(ra,JB_RA,a0)
	move	v0,zero
	j	ra
	.end setjmp

/** longjmp(buffer,value) restore context */
	.globl	longjmp
	.ent longjmp
longjmp:
	LREG(s0,JB_S0,a0)
	LREG(s1,JB_S1,a0)
	LREG(s2,JB_S2,a0)
	LREG(s3,JB_S3,a0)
	LREG(s4,JB_S4,a0)
	LREG(s5,JB_S5,a0)
	LREG(s6,JB_S6,a0)
	LREG(s7,JB_S7,a0)
	LREG(fp,JB_FP,a0)
	LREG(sp,JB_SP,a0)
	LREG(ra,JB_RA,a0)
	move	v0,a1
	jr	ra
	.end longjmp

