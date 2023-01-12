/*
 * r3ktlb.h : SDE-MIPS R3000 MMU/TLB definitions
 *
 * Copyright (c) 1999-1999, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

#ifndef _R3KTLB_H_
#define _R3KTLB_H_

#ifdef __cplusplus
extern "C" {
#endif

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

/* macros to constuct tlbhi and tlblo */
#define mktlbhi(vpn,id)   	(((unsigned)(vpn) << TLBHI_VPNSHIFT) | \
				 ((id) << TLBHI_PIDSHIFT))
#define mktlblo(pn,flags) 	(((unsigned)(pn) << TLB_PFNSHIFT) | (flags))

/* and destruct them */
#define tlbhiVpn(hi) 	((hi) >> TLBHI_VPNSHIFT)
#define tlbhiId(hi) 	(((hi) & TLBHI_PIDMASK) >> TLBHI_PIDSHIFT)
#define tlbloPn(lo)	((lo) >> TLB_PFNSHIFT)
#define tlbloFlags(lo)	((lo) & TLB_FLAGS)

#ifndef ROM_BASE
#define ROM_BASE	0xbfc00000	/* standard ROM base address */
#endif

#ifdef __ASSEMBLER__

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

#else /* __ASSEMBLER__ */

/*
 * Standard address types
 */
typedef unsigned long	paddr_t;	/* a physical address */
typedef unsigned long	vaddr_t;	/* a virtual address */
typedef unsigned long	tlblo_t;	/* the tlblo field */
typedef unsigned long	tlbhi_t;	/* the tlbhi field */

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

/* virtual address to virtual page number and back */
#define vaToVpn(va)	((unsigned)(va) >> VMPGSHIFT)
#define vpnToVa(vpn)	(void *)((vpn) << VMPGSHIFT)

/* physical address to phyiscal page number and back */
#define paToPn(pa)	((pa) >> VMPGSHIFT)
#define pnToPa(pn)	((paddr_t)((pn) << VMPGSHIFT))

/* 
 * R3000 TLB acccess functions
 */
void	r3k_tlbri (tlbhi_t *, tlblo_t *, unsigned);
void	r3k_tlbwi (tlbhi_t, tlblo_t, unsigned);
void	r3k_tlbwr (tlbhi_t, tlblo_t);
int	r3k_tlbrwr (tlbhi_t, tlblo_t);
int	r3k_tlbprobe (tlbhi_t, tlblo_t *);
void	r3k_tlbinval (tlbhi_t);
void	r3k_tlbinvalall (void);

/* R3000 CP0 Context register */
#define r3k_getcontext()	_mips_mfc0(C0_CONTEXT)
#define r3k_setcontext(v)	_mips_mtc0(C0_CONTEXT,v)
#define r3k_xchcontext(v)	_mips_mxc0(C0_CONTEXT,v)

/* R3000 CP0 EntryHi register */
#define r3k_getentryhi()	_mips_mfc0(C0_ENTRYHI)
#define r3k_setentryhi(v)	_mips_mtc0(C0_ENTRYHI,v)
#define r3k_xchentryhi(v)	_mips_mxc0(C0_ENTRYHI,v)

/* R3000 CP0 EntryLo register */
#define r3k_getentrylo()	_mips_mfc0(C0_ENTRYLO)
#define r3k_setentrylo(v)	_mips_mtc0(C0_ENTRYLO,v)
#define r3k_xchentrylo(v)	_mips_mxc0(C0_ENTRYLO,v)

#endif /* __ASSEMBLER__ */

#ifdef __cplusplus
}
#endif
#endif /* _R3KTLB_H_*/
