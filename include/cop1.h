/* $Id: cop1.h,v 1.2 1996/01/16 14:17:15 chris Exp $ */
#ifndef _COP1_
#define _COP1_

#define MULTITASK

#ifdef NEWFP
/* rename global variables to minimize possiblity of collisions */
#define GPR	_LSI_GPR
#define	FPR	_LSI_FPR
#define	FCR	_LSI_FCR
#define	ibuf	_LSI_ibuf
#define	brkadr	_LSI_brkadr
#endif

/* Target registers index */
#define	S_ZERO		0
#define	S_AT		1
#define	S_V0		2
#define	S_V1		3
#define	S_A0		4
#define	S_A1		5
#define	S_A2		6
#define	S_A3		7
#define	S_T0		8
#define	S_T1		9
#define	S_T2		10
#define	S_T3		11
#define	S_T4		12
#define	S_T5		13
#define	S_T6		14
#define	S_T7		15
#define	S_S0		16
#define	S_S1		17
#define	S_S2		18
#define	S_S3		19
#define	S_S4		20
#define	S_S5		21
#define	S_S6		22
#define	S_S7		23
#define	S_T8		24
#define	S_T9		25
#define	S_K0		26
#define	S_K1		27
#define	S_GP		28
#define	S_SP		29
#define	S_FP		30
#define	S_RA		31
#define S_HI		32
#define S_LO		33
#define S_STATUS	34
#define S_CAUSE		35
#define S_EPC		36
#define NGREGS		37

#ifdef LANGUAGE_C
typedef unsigned long Word; 

#ifndef NEWFP
#ifdef MULTITASK

struct c1state {
	Word fpr[32];
#define Fpr 	(c1dat->fpr)
	Word fcr;
#define Fcr	(c1dat->fcr)
	Word buf[2];
#define ibuf	(c1dat->buf)
	Word *adr;
#define brkadr	(c1dat->adr)
	INTERNAL_DECIMAL_FORMAT dec_val;
#define decimal_value 	(c1dat->dec_val)
	INTERNAL_BINARY_FORMAT  bin_val;
#define binary_value 	(c1dat->bin_val)
	char out_buf[1024];
#define output_buffer 	(c1dat->out_buf)
	unsigned int hex_buf[NUMBER_OF_HEX_DIGITS];
#define hex_buffer 	(c1dat->hex_buf)
	DECIMAL_BUFFER dec_buf;
#define dec_buffer 	(c1dat->dec_buf)
	BINARY_BUFFER  bin_buf;
#define bin_buffer 	(c1dat->bin_buf)
	int round_mode;
#define Rounding_mode 	(c1dat->round_mode)
	int round_prec;
#define Rounding_precision 	(c1dat->round_prec)
	int targ_upper;
#define Target_exp_upper_limit 	(c1dat->targ_upper)
	int targ_lower;
#define Target_exp_lower_limit 	(c1dat->targ_lower)
	int conufl_flag;
#define Conversion_underflow_flag 	(c1dat->conufl_flag)
	};

struct c1state *mkc1dat();
extern struct c1state *c1dat;
#else
extern Word Fcr;
#endif /* MULTITASK */

#endif NEWFP
#endif /* LANGUAGE_C */

#endif /* _COP1_ */
