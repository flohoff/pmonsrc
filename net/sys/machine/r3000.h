/*
 * r3000.h : SDE R3000 coprocessor 0 definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */

#ifndef _R3000_H_
#define _R3000_H_

/*
 * R3000 Exception Codes
 */
#define EXC_INTR	0x0       /* interrupt */                        
#define EXC_MOD		0x1       /* tlb modification */                 
#define EXC_TLBL	0x2       /* tlb miss (load/i-fetch) */          
#define EXC_TLBS	0x3       /* tlb miss (store) */                 
#define EXC_ADEL	0x4       /* address error (load/i-fetch) */     
#define EXC_ADES	0x5       /* address error (store) */            
#define EXC_IBE		0x6       /* bus error (i-fetch) */              
#define EXC_DBE		0x7       /* data bus error (load/store) */      
#define EXC_SYS		0x8       /* system call */                      
#define EXC_BP		0x9       /* breakpoint */                       
#define EXC_RI		0xa       /* reserved instruction */             
#define EXC_CPU		0xb       /* coprocessor unusable */             
#define EXC_OVF		0xc       /* integer overflow */                 
#define EXC_RESD	0xd
#define EXC_RESE	0xe
#define EXC_RESF	0xf


/*
 * R3000 Cause Register 
 */
#define CR_BD		0x80000000      /* branch delay */   
#define CR_CEMASK	0x30000000      /* coprocessor used */
#define CR_CESHIFT	28

/* interrupt pending bits */	                             
#define CR_SINT0	0x00000100      /* s/w interrupt 0 */
#define CR_SINT1	0x00000200      /* s/w interrupt 1 */
#define CR_HINT0	0x00000400      /* h/w interrupt 0 */
#define CR_HINT1	0x00000800      /* h/w interrupt 1 */
#define CR_HINT2	0x00001000      /* h/w interrupt 2 */
#define CR_HINT3	0x00002000      /* h/w interrupt 3 */
#define CR_HINT4	0x00004000      /* h/w interrupt 4 */
#define CR_HINT5	0x00008000      /* h/w interrupt 5 */

/* alternative interrupt pending bit naming */
#define CR_IP0		0x00000100
#define CR_IP1		0x00000200
#define CR_IP2		0x00000400
#define CR_IP3		0x00000800
#define CR_IP4		0x00001000
#define CR_IP5		0x00002000
#define CR_IP6		0x00004000
#define CR_IP7		0x00008000

#define CR_IMASK	0x0000ff00 	/* interrupt pending mask */
#define CR_XMASK	0x0000007c 	/* exception code mask */
#define CR_XCPT(x)	((x)<<2)


/*
 * R3000 Status Register 
 */
#define SR_IEC		0x00000001	/* interrupt enable current */
#define SR_KUC		0x00000002	/* kernel mode current */

#define SR_IEP		0x00000004	/* interrupt enable prev */
#define SR_KUP		0x00000008	/* kernel mode prev */

#define SR_IEO		0x00000010	/* interrupt enable old */
#define SR_KUO		0x00000020	/* kernel mode old */

#define SR_IE		SR_IEC		/* generic alias for int enable */

#define SR_SINT0	0x00000100      /* enable s/w interrupt 0 */
#define SR_SINT1	0x00000200      /* enable s/w interrupt 0 */
#define SR_HINT0	0x00000400      /* enable h/w interrupt 1 */
#define SR_HINT1	0x00000800      /* enable h/w interrupt 2 */
#define SR_HINT2	0x00001000      /* enable h/w interrupt 3 */
#define SR_HINT3	0x00002000      /* enable h/w interrupt 4 */
#define SR_HINT4	0x00004000      /* enable h/w interrupt 5 */
#define SR_HINT5	0x00008000      /* enable h/w interrupt 6 */

/* alternative interrupt mask naming */
#define SR_IM0		0x00000100
#define SR_IM1		0x00000200
#define SR_IM2		0x00000400
#define SR_IM3		0x00000800
#define SR_IM4		0x00001000
#define SR_IM5		0x00002000
#define SR_IM6		0x00004000
#define SR_IM7		0x00008000

#define SR_IMASK	0x0000ff00

/* diagnostic field */
#define SR_ISC		0x00010000	/* isolate caches */
#define SR_SWC		0x00020000	/* swap caches */
#define SR_PZ		0x00040000	/* cache parity zero */
#define SR_CM		0x00080000	/* cache miss */
#define SR_PE		0x00100000	/* cache parity error */
#define SR_TS		0x00200000	/* tlb shutdown */
#define SR_BEV		0x00400000	/* boot exception vectors */

#define SR_RE		0x02000000	/* reverse endian (user mode) */

#define SR_CU0		0x10000000      /* coprocessor 0 enable */
#define SR_CU1		0x20000000      /* coprocessor 1 enable */
#define SR_CU2		0x40000000      /* coprocessor 2 enable */
#define SR_CU3		0x80000000      /* coprocessor 3 enable */


/* R3000 EntryHi bits */
#define TLBHI_VPNMASK	0xfffff000
#define TLBHI_VPNSHIFT	12
#define TLBHI_PIDMASK	0x00000fc0
#define TLBHI_PIDSHIFT	6


/* R3000 EntryLo bits */
#define TLB_PFNMASK	0xfffff000
#define TLB_PFNSHIFT	12
#define TLB_FLAGS	0x00000f00
#define TLB_N		0x00000800
#define TLB_D		0x00000400
#define TLB_V		0x00000200
#define TLB_G		0x00000100

/* R3000 Index bits */
#define TLBIDX_MASK	0x3f00
#define TLBIDX_SHIFT	8

/* R3000 Random bits */
#define TLBRAND_MASK	0x3f00
#define TLBRAND_SHIFT	8

#define NTLBID		64	/* total number of tlb entries */
#define NTLBWIRED	8	/* number of wired tlb entries */

#ifndef ROM_BASE
#define ROM_BASE	0xbfc00000	/* standard ROM base address */
#endif


#ifdef __ASSEMBLER__
#define C0_INDEX	$0
#define C0_INX		$0
#define C0_RANDOM	$1
#define C0_RAND		$1
#define C0_ENTRYLO	$2
#define C0_TLBLO	$2
#define C0_CONTEXT	$4
#define C0_CTXT		$4
#define C0_BADVADDR 	$8
#define C0_VADDR 	$8
#define C0_ENTRYHI	$10
#define C0_TLBHI	$10
#define C0_STATUS	$12
#define C0_SR		$12
#define C0_CAUSE	$13
#define C0_CR		$13
#define C0_EPC 		$14
#define C0_PRID		$15

$index		=	$0
$random		=	$1
$entrylo	=	$2
$context	=	$4
$vaddr 		=	$8
$entryhi	=	$10
$sr		=	$12
$cr		=	$13
$epc 		=	$14
$prid		=	$15

/* 
 * R3000 virtual memory regions 
 */
#define	KSEG0_BASE	0x80000000
#define	KSEG1_BASE	0xa0000000
#define	KSEG2_BASE	0xc0000000

#define KUSEG_SIZE	0x80000000
#define KSEG0_SIZE	0x20000000
#define KSEG1_SIZE	0x20000000
#define KSEG2_SIZE	0x40000000

#define RVEC_BASE	ROM_BASE

/* 
 * Translate a kernel virtual address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((pa) | 0x80000000)
#define PA_TO_KVA1(pa)	((pa) | 0xa0000000)

/* translate between KSEG0 and KSEG1 virtual addresses */
#define KVA0_TO_KVA1(v)	((v) | 0x20000000)
#define KVA1_TO_KVA0(v)	((v) & ~0x20000000)

#else

#define C0_INDEX	0
#define C0_INX		0
#define C0_RANDOM	1
#define C0_RAND		1
#define C0_ENTRYLO	2
#define C0_TLBLO	2
#define C0_CONTEXT	4
#define C0_CTXT		4
#define C0_BADVADDR 	8
#define C0_VADDR 	8
#define C0_ENTRYHI	10
#define C0_TLBHI	10
#define C0_STATUS	12
#define C0_SR		12
#define C0_CAUSE	13
#define C0_CR		13
#define C0_EPC 		14
#define C0_PRID		15

/* 
 * R3000 virtual memory regions 
 */
#define KUSEG_BASE 	((void  *)0x00000000)
#define KSEG0_BASE	((void  *)0x80000000)
#define KSEG1_BASE	((void  *)0xa0000000)
#define KSEG2_BASE	((void  *)0xc0000000)

#define KUSEG_SIZE	0x80000000u
#define KSEG0_SIZE	0x20000000u
#define KSEG1_SIZE	0x20000000u
#define KSEG2_SIZE	0x40000000u

#define RVEC_BASE	((void *)ROM_BASE)	/* reset vector base */

/* 
 * Translate a kernel virtual address in KSEG0 or KSEG1 to a real
 * physical address and back.
 */
#define KVA_TO_PA(v) 	((paddr_t)(v) & 0x1fffffff)
#define PA_TO_KVA0(pa)	((void *) ((pa) | 0x80000000))
#define PA_TO_KVA1(pa)	((void *) ((pa) | 0xa0000000))

/* translate between KSEG0 and KSEG1 virtual addresses */
#define KVA0_TO_KVA1(v)	((void *) ((unsigned)(v) | 0x20000000))
#define KVA1_TO_KVA0(v)	((void *) ((unsigned)(v) & ~0x20000000))

/* Test for KSEGS */
#define IS_KVA(v)	((int)(v) < 0)
#define IS_KVA0(v)	(((unsigned)(v) >> 29) == 0x4)
#define IS_KVA1(v)	(((unsigned)(v) >> 29) == 0x5)
#define IS_KVA01(v)	(((unsigned)(v) >> 30) == 0x2)
#define IS_KVA2(v)	(((unsigned)(v) >> 30) == 0x3)
#define IS_UVA(v)	((int)(v) >= 0)

/* convert register type to address and back */
#define VA_TO_REG(v)	((long)(v))
#define REG_TO_VA(v)	((void *)(v))

/*
 * R3000 fixed virtual memory page size.
 */
#define VMPGSIZE 	4096
#define VMPGMASK 	(VMPGSIZE-1)
#define VMPGSHIFT 	12

#endif /* !ASSEMBLER */

#endif /* _R3000_H_ */
