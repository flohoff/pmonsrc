/* $Id: setjmp.h,v 1.2 1996/01/16 14:17:25 chris Exp $ */

#ifndef _SETJMP_
#define _SETJMP_

/* defines for longjmp buffer */
#define JB_S0		0
#define JB_S1		1
#define JB_S2		2
#define JB_S3		3
#define JB_S4		4
#define JB_S5		5
#define JB_S6		6
#define JB_S7		7
#define JB_FP		8
#define JB_SP		9
#define JB_RA		10
#define JB_SIZ		11

#ifdef LANGUAGE_C
#if __mips >= 3
typedef long long jmp_buf[JB_SIZ];
#else
typedef int jmp_buf[JB_SIZ];
#endif
#endif

#endif /* _SETJMP_ */
