/*
 * r4000.h : SDE R4000 coprocessor 0 definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#ifndef _R4000_H_
#define _R4000_H_

/*
 * Cause Register
 */

/* exception codes */
#define EXC_INTR	0
#define EXC_MOD		1
#define EXC_TLBL	2
#define EXC_TLBS	3
#define EXC_ADEL	4
#define EXC_ADES	5
#define EXC_IBE		6
#define EXC_DBE		7
#define EXC_SYS		8
#define EXC_BP		9
#define EXC_RI		10
#define EXC_CPU		11
#define EXC_OVF		12
#define EXC_TRAP	13
#define EXC_VCEI	14
#define EXC_FPE		15
#define EXC_RES16	16
#define EXC_RES17	17
#define EXC_RES18	18
#define EXC_RES19	19
#define EXC_RES20	20
#define EXC_RES21	21
#define EXC_RES22	22
#define EXC_WATCH	23
#define EXC_RES24	24
#define EXC_RES25	25
#define EXC_RES26	26
#define EXC_RES27	27
#define EXC_RES28	28
#define EXC_RES29	29
#define EXC_RES30	30
#define EXC_VCED	31

/* interrupts and others */
#define CR_BD		0x80000000
#define CR_CE1		0x20000000
#define CR_CE0		0x10000000
#define CR_SINT0	0x00000100
#define CR_SINT1	0x00000200
#define CR_HINT0	0x00000400
#define CR_HINT1	0x00000800
#define CR_HINT2	0x00001000
#define CR_HINT3	0x00002000
#define CR_HINT4	0x00004000
#define CR_HINT5	0x00008000
#define CR_IMASK	0x0000ff00
#define CR_XMASK	0x0000007c
#define CR_XCPT(x)	(x<<2)

/*
 * Status Register 
 */

#define SR_IE		0x00000001
#define SR_EXL		0x00000002
#define SR_ERL		0x00000004
#define SR_KSMASK	0x00000018
#define SR_KSUSER	0x00000010
#define SR_KSSUPER	0x00000008
#define SR_KSKERNEL	0x00000000

#define SR_SINT0	0x00000100
#define SR_SINT1	0x00000200
#define SR_HINT0	0x00000400
#define SR_HINT1	0x00000800
#define SR_HINT2	0x00001000
#define SR_HINT3	0x00002000
#define SR_HINT4	0x00004000
#define SR_HINT5	0x00008000

#define SR_IMASK	0x0000ff00

#define SR_DE		0x00010000
#define SR_CE		0x00020000
#define SR_CH		0x00040000
#define SR_SR		0x00100000
#define SR_TS		0x00200000
#define SR_BEV		0x00400000

#define SR_RE		0x02000000
#define SR_FR		0x04000000
#define SR_RP		0x08000000

#define SR_CU0		0x10000000
#define SR_CU1		0x20000000
#define SR_CU2		0x40000000
#define SR_CU3		0x80000000


/*
 * Config Register
 */

#define CFG_CM		0x80000000	/* Master-Checker mode */
#define CFG_ECMASK	0x70000000	/* System Clock Ratio */
#define CFG_ECBY2	0x00000000 	/* divide by 2 */
#define CFG_ECBY3	0x00000000 	/* divide by 3 */
#define CFG_ECBY4	0x00000000 	/* divide by 4 */
#define CFG_EPMASK	0x0f000000	/* Transmit data pattern */
#define CFG_EPD		0x00000000	/* D */
#define CFG_EPDDX	0x01000000	/* DDX */
#define CFG_EPDDXX	0x02000000	/* DDXX */
#define CFG_EPDXDX	0x03000000	/* DXDX */
#define CFG_EPDDXXX	0x04000000	/* DDXXX */
#define CFG_EPDDXXXX	0x05000000	/* DDXXXX */
#define CFG_EPDXXDXX	0x06000000	/* DXXDXX */
#define CFG_EPDDXXXXX	0x07000000	/* DDXXXXX */
#define CFG_EPDXXXDXXX	0x08000000	/* DXXXDXXX */
#define CFG_SBMASK	0x00c00000	/* Secondary cache block size */
#define CFG_SB4		0x00000000	/* 4 words */
#define CFG_SB8		0x00400000	/* 8 words */
#define CFG_SB16	0x00800000	/* 16 words */
#define CFG_SB32	0x00c00000	/* 32 words */
#define CFG_SS		0x00200000	/* Split secondary cache */
#define CFG_SW		0x00100000	/* Secondary cache port width */
#define CFG_EWMASK	0x000c0000	/* System port width */
#define CFG_EW64	0x00000000	/* 64 bit */
#define CFG_EW32	0x00010000	/* 32 bit */
#define CFG_SC		0x00020000	/* Secondary cache absent */
#define CFG_SM		0x00010000	/* Dirty Shared mode disabled */
#define CFG_BE		0x00008000	/* Big Endian */
#define CFG_EM		0x00004000	/* ECC mode enable */
#define CFG_EB		0x00002000	/* Block ordering */
#define CFG_ICMASK	0x00000e00	/* Instruction cache size */
#define CFG_ICSHIFT	9
#define CFG_DCMASK	0x000001c0	/* Data cache size */
#define CFG_DCSHIFT	6
#define CFG_IB		0x00000020	/* Instruction cache block size */
#define CFG_DB		0x00000010	/* Data cache block size */
#define CFG_CU		0x00000008	/* Update on Store Conditional */
#define CFG_K0MASK	0x00000007	/* KSEG0 coherency algorithm */

/*
 * primary cache mode
 */
#define CFG_C_UNCACHED		2
#define CFG_C_NONCOHERENT	3
#define CFG_C_COHERENTXCL	4
#define CFG_C_COHERENTXCLW	5
#define CFG_C_COHERENTUPD	6

/*
 * cache operations (should be in assembler...?)
 */
#define Index_Invalidate_I               0x0         /* 0       0 */
#define Index_Writeback_Inv_D            0x1         /* 0       1 */
#define Index_Invalidate_SI              0x2         /* 0       2 */
#define Index_Writeback_Inv_SD           0x3         /* 0       3 */
#define Index_Load_Tag_I                 0x4         /* 1       0 */
#define Index_Load_Tag_D                 0x5         /* 1       1 */
#define Index_Load_Tag_SI                0x6         /* 1       2 */
#define Index_Load_Tag_SD                0x7         /* 1       3 */
#define Index_Store_Tag_I                0x8         /* 2       0 */
#define Index_Store_Tag_D                0x9         /* 2       1 */
#define Index_Store_Tag_SI               0xA         /* 2       2 */
#define Index_Store_Tag_SD               0xB         /* 2       3 */
#define Create_Dirty_Exc_D               0xD         /* 3       1 */
#define Create_Dirty_Exc_SD              0xF         /* 3       3 */
#define Hit_Invalidate_I                 0x10        /* 4       0 */
#define Hit_Invalidate_D                 0x11        /* 4       1 */
#define Hit_Invalidate_SI                0x12        /* 4       2 */
#define Hit_Invalidate_SD                0x13        /* 4       3 */
#define Hit_Writeback_Inv_D              0x15        /* 5       1 */
#define Hit_Writeback_Inv_SD             0x17        /* 5       3 */
#define Fill_I                           0x14        /* 5       0 */
#define Hit_Writeback_D                  0x19        /* 6       1 */
#define Hit_Writeback_SD                 0x1B        /* 6       3 */
#define Hit_Writeback_I                  0x18        /* 6       0 */
#define Hit_Set_Virtual_SI               0x1E        /* 7       2 */
#define Hit_Set_Virtual_SD               0x1F        /* 7       3 */

/* coprocessor 0 register numbers */
#define COP0R_INDEX	0
#define COP0R_RANDOM	1
#define COP0R_ENTRYLO0	2
#define COP0R_ENTRYLO1	3
#define COP0R_CONTEXT	4
#define COP0R_PAGEMASK	5
#define COP0R_WIRED	6

#define COP0R_VADDR 	8
#define COP0R_COUNT 	9
#define COP0R_ENTRYHI	10
#define COP0R_COMPARE	11
#define COP0R_SR	12
#define COP0R_CR	13
#define COP0R_EPC 	14
#define COP0R_PRID	15
#define COP0R_CONFIG	16
#define COP0R_LLADDR	17
#define COP0R_WATCHLO	18
#define COP0R_WATCHHI	19

#define COP0R_ECC	26
#define COP0R_CACHEERR	27
#define COP0R_TAGLO	28
#define COP0R_TAGHI	29
#define COP0R_ERRPC	30

/* entry hi bits */
#define TLBHI_VPN2MSK	0xffffe000
#define TLBHI_PIDMSK	0x000000ff
#define TLBHI_G		0x00001000

/* entry lo bits */

#define TLB_PFNMSK	0x3ffffc00
#define TLB_V		0x00000002
#define TLB_D		0x00000004
#define TLB_CMASK	0x00000038

#define TLB_UNCACHED		(CFG_C_UNCACHED<<3)
#define TLB_NONCOHERENT		(CFG_C_NONCOHERENT<<3)
#define TLB_COHERENTXCL		(CFG_C_COHERENTXCL<<3)
#define TLB_COHERENTXCLW	(CFG_C_COHERENTXCLW<<3)
#define TLB_COHERENTUPD		(CFG_C_COHERENTUPD<<3)

#define NTLBID	48

#ifdef __ASSEMBLER__
	$index		= $0
	$random		= $1
	$entrylo0	= $2
	$entrylo1	= $3
	$context	= $4
	$pagemask	= $5
	$wired		= $6
	$vaddr 		= $8
	$count 		= $9
	$entryhi	= $10
	$compare	= $11
	$sr		= $12
	$cr		= $13
	$epc 		= $14
	$prid		= $15
	$config		= $16
	$lladdr		= $17
	$watchlo	= $18
	$watchhi	= $19
	$ecc		= $26
	$cacheerr	= $27
	$taglo		= $28
	$taghi		= $29
	$errpc		= $30


#define	KSEG0_BASE	0x80000000
#define	KSEG1_BASE	0xa0000000
#define	KSEG2_BASE	0xc0000000
#define	KSEGS_BASE	0xc0000000
#define	KSEG3_BASE	0xe0000000
#define RVEC_BASE	0xbfc00000	/* reset vector base */

#define KUSEG_SIZE	0x80000000
#define KSEG0_SIZE	0x20000000
#define KSEG1_SIZE	0x20000000
#define KSEG2_SIZE	0x40000000
#define KSEGS_SIZE	0x20000000
#define KSEG3_SIZE	0x20000000

/* Translate a kernel physical address K0,K1 to a real
 * physical address and back
 */
#define KVA_TO_PA(v) 	((v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((pa) | 0x80000000)
#define PA_TO_KVA1(pa)	((pa) | 0xa0000000)

/* Translate KSEG0 KSEG1 addresses */
#define KVA0_TO_KVA1(v)	((v) | 0x20000000)
#define KVA1_TO_KVA0(v)	((v) & ~0x20000000)

#else

/* definitions for R4000 virtual memory */
#define KUSEG_BASE 	((void  *)0x00000000)
#define KSEG0_BASE	((void  *)0x80000000)
#define KSEG1_BASE	((void  *)0xa0000000)
#define KSEG2_BASE	((void  *)0xc0000000)
#define KSEGS_BASE	((void  *)0xc0000000)
#define KSEG3_BASE	((void  *)0xe0000000)
#define RVEC_BASE	((void	*)0xbfc00000)	/* reset vector base */

#define KUSEG_SIZE	0x80000000u
#define KSEG0_SIZE	0x20000000u
#define KSEG1_SIZE	0x20000000u
#define KSEG2_SIZE	0x40000000u
#define KSEGS_SIZE	0x20000000u
#define KSEG3_SIZE	0x20000000u

/* translate a kernel physical address K0,K1 to a real
 * physical address and back
*/
#define KVA_TO_PA(v) 	((unsigned)(v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((void *) ((pa) | 0x80000000))
#define PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))

/* translate KSEG0 KSEG1 addresses
*/
#define KVA0_TO_KVA1(v)	((void *) ((unsigned)(v) | 0x20000000))
#define KVA1_TO_KVA0(v)	((void *) ((unsigned)(v) & ~0x20000000))

/*
 * test for ksegs
*/
#define IS_KVA(v)	( (int)(v) < 0)
#define IS_KVA0(v)	( ((unsigned)(v) >> 29) == 0x4)
#define IS_KVA1(v)	( ((unsigned)(v) >> 29) == 0x5)
#define IS_KVA01(v)	( ((unsigned)(v) >> 30) == 0x2)
#define IS_KVAS(v)	( ((unsigned)(v) >> 29) == 0x6)
#define IS_KVA2(v)	( ((unsigned)(v) >> 29) == 0x7)
#define IS_UVA(v)	( (int)(v) >= 0)
#endif /* !__ASSEMBLER__ */

#endif /* _R4000_H_ */
