/* $Id: kern_malloc.c,v 1.3 1997/09/01 14:46:08 nigel Exp $ */
 
/*
 * Copyright (c) 1987, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)kern_malloc.c	7.25 (Berkeley) 5/8/91
 */

#include "param.h"
#include "proc.h"
#include "map.h"
#include "kernel.h"
#include "malloc.h"
#include "vm/vm.h"
#include "vm/vm_kern.h"

#ifdef DEBUG
static void check_addr();
#endif

struct kmembuckets bucket[MINBUCKET + 16];
struct kmemstats kmemstats[M_LAST];
struct kmemusage *kmemusage;
char *kmembase, *kmemlimit;
const char * const memname[] = INITKMEMNAMES;

/*
 * Allocate a block of memory
 */
void *
malloc(size, type, waitflag)
	unsigned long size;
	int type, waitflag;
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	unsigned long allocsize;
	long indx, npg, alloc;
	int s;
	caddr_t va, cp, savedlist;
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];

	if (((unsigned long)type) > M_LAST)
		panic("malloc - bogus type");
#endif

	indx = BUCKETINDX(size);
	kbp = &bucket[indx];
	s = splimp();
#ifdef KMEMSTATS
	while (ksp->ks_memuse >= ksp->ks_limit) {
		if (waitflag == M_NOWAIT) {
			splx(s);
			return ((void *) NULL);
		}
		if (ksp->ks_limblocks < 65535)
			ksp->ks_limblocks++;
		tsleep((caddr_t)ksp, PSWP+2, memname[type], 0);
	}
#endif
	if (kbp->kb_next == NULL) {
		if (size > MAXALLOCSAVE)
			allocsize = roundup(size, CLBYTES);
		else
			allocsize = 1 << indx;
		npg = clrnd(btoc(allocsize));
		va = (caddr_t) kmem_malloc(kmem_map, (vm_size_t)ctob(npg),
		    waitflag);
		if (va == NULL) {
			splx(s);
			return ((void *) NULL);
		}
#ifdef KMEMSTATS
		kbp->kb_total += kbp->kb_elmpercl;
#endif
		kup = btokup(va);
		kup->ku_indx = indx;
		if (allocsize > MAXALLOCSAVE) {
			if (npg > 65535)
				panic("malloc: allocation too large");
			kup->ku_pagecnt = npg;
#ifdef KMEMSTATS
			ksp->ks_memuse += allocsize;
#endif
			goto out;
		}
#ifdef KMEMSTATS
		kup->ku_freecnt = kbp->kb_elmpercl;
		kbp->kb_totalfree += kbp->kb_elmpercl;
#endif
		/*
		 * Just in case we blocked while allocating memory,
		 * and someone else also allocated memory for this
		 * bucket, don't assume the list is still empty.
		 */
		savedlist = kbp->kb_next;
		kbp->kb_next = va + (npg * NBPG) - allocsize;
		for (cp = kbp->kb_next; cp > va; cp -= allocsize)
			*(caddr_t *)cp = cp - allocsize;
		*(caddr_t *)cp = savedlist;
	}
	va = kbp->kb_next;
#ifdef DEBUG
	check_addr(va, btokup(va)->ku_indx, 0);
#endif
	kbp->kb_next = *(caddr_t *)va;
#ifdef DEBUG
	if (kbp->kb_next) {
		struct kmemusage *nkup = btokup(kbp->kb_next);
		if (nkup->ku_indx != indx)
			panic("malloc: next wrong bucket");
		check_addr(kbp->kb_next, nkup->ku_indx, 0);
	}
#endif
#ifdef KMEMSTATS
	kup = btokup(va);
#ifdef DIAGNOSTIC
	if (kup->ku_indx != indx)
		panic("malloc: wrong bucket");
	if (kup->ku_freecnt == 0)
		panic("malloc: lost data");
#endif
	kup->ku_freecnt--;
	kbp->kb_totalfree--;
	ksp->ks_memuse += 1 << indx;
out:
	kbp->kb_calls++;
	ksp->ks_inuse++;
	ksp->ks_calls++;
	if (ksp->ks_memuse > ksp->ks_maxused)
		ksp->ks_maxused = ksp->ks_memuse;
#else
out:
#endif
	splx(s);
	return ((void *) va);
}

#if defined(DEBUG) && defined(i386)
caddr_t	lastfreepc;
#endif

/*
 * Free a block of memory allocated by malloc.
 */
void
free(addr, type)
	void *addr;
	int type;
{
	register struct kmembuckets *kbp;
	register struct kmemusage *kup;
	unsigned long size;
	int s;
#ifdef KMEMSTATS
	register struct kmemstats *ksp = &kmemstats[type];
#endif

	kup = btokup(addr);
	size = 1 << kup->ku_indx;
#ifdef DEBUG
	check_addr(addr, kup->ku_indx, type);
#ifdef i386
	lastfreepc = *(caddr_t *)((caddr_t) &addr - sizeof (caddr_t));
#endif
#endif
	kbp = &bucket[kup->ku_indx];
	s = splimp();

	/*
	 * If freeing a size that is at least pagesize,
	 * we can free the actual memory without coalescing
	 * free sections; no other parts of the pages are on
	 * the free list.  If we're over the highwater mark,
	 * free the actual memory rather than putting it
	 * on the free list.  Note that buckets over MAXALLOCSAVE
	 * have highwater marks set to zero.
	 */
#ifndef KMEMSTATS
	if (size > MAXALLOCSAVE)
#else
	if (size >= CLBYTES && kbp->kb_totalfree >= kbp->kb_highwat)
#endif
	{
		if (size > MAXALLOCSAVE)
			size = ctob(kup->ku_pagecnt);
		kmem_free(kmem_map, (vm_offset_t)addr, size);
#ifdef KMEMSTATS
		ksp->ks_memuse -= size;
		kup->ku_indx = 0;
		kup->ku_pagecnt = 0;
		if (ksp->ks_memuse + size >= ksp->ks_limit &&
		    ksp->ks_memuse < ksp->ks_limit)
			wakeup((caddr_t)ksp);
		ksp->ks_inuse--;
		kbp->kb_total--;
		kbp->kb_couldfree++;
#endif
		splx(s);
		return;
	}

#ifdef KMEMSTATS
	kup->ku_freecnt++;
	if (kup->ku_freecnt >= kbp->kb_elmpercl)
		if (kup->ku_freecnt > kbp->kb_elmpercl) {
		        printf("free: multiple frees, type=%s\n", memname[type]);
			panic("free: multiple frees");
		}
		else if (kbp->kb_totalfree > kbp->kb_highwat)
			kbp->kb_couldfree++;
	kbp->kb_totalfree++;
	ksp->ks_memuse -= size;
	if (ksp->ks_memuse + size >= ksp->ks_limit &&
	    ksp->ks_memuse < ksp->ks_limit)
		wakeup((caddr_t)ksp);
	ksp->ks_inuse--;
#endif
#ifdef DEBUG
	if (size < NBPG) {
		/*
		 * Clean out the freed memory (foil readers) and
		 * record the pc of our caller (trace writers).
		 * Assumes that MINALLOCSIZE >= 8.
		 */
		bzero(addr, size);
	}
#ifdef i386
	*((caddr_t *)((caddr_t) addr + size) - 1) = lastfreepc;
#endif
#endif
	*(caddr_t *)addr = kbp->kb_next;
	kbp->kb_next = addr;
	splx(s);
}

#ifdef DEBUG

static long addrmask[] = { 0x00000000,
	0x00000001, 0x00000003, 0x00000007, 0x0000000f,
	0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
	0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
	0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
};

static void
check_addr(va, index, type)
	caddr_t va;
	int index;
	int type;
{
	long size;
	u_long alloc;

	if ((unsigned) index < MINBUCKET || (unsigned) index > MINBUCKET + 15) {
		printf("check_addr: out of range index 0x%x\n", index);
		panic("check_addr: index");
	}

	if ((char *) va < kmembase || (char *) va >= kmemlimit) {
		printf("check_addr: out of range addr 0x%x\n", va);
		panic("check_addr: bounds");
	}

	size = 1 << index;
	if (size > NBPG * CLSIZE)
		alloc = addrmask[BUCKETINDX(NBPG * CLSIZE)];
	else
		alloc = addrmask[index];
	if (((u_long)va & alloc) != 0) {
		printf("check_addr: unaligned addr 0x%x, size %d, type %s\n",
			va, size, memname[type]);
		panic("check_addr: unaligned addr");
	}
}
#endif /* DEBUG */

/*
 * Declare this as initialized data so we can patch it.
 */
int	maxbufmem;

/*
 * Initialize the kernel memory allocator
 */
kmeminit()
{
	register struct kmembuckets *kbp;
	register long indx;
	extern int physmem;
	int npg, size;

#if	((MAXALLOCSAVE & (MAXALLOCSAVE - 1)) != 0)
		ERROR!_kmeminit:_MAXALLOCSAVE_not_power_of_2
#endif
#if	(MAXALLOCSAVE > MINALLOCSIZE * 32768)
		ERROR!_kmeminit:_MAXALLOCSAVE_too_big
#endif
#if	(MAXALLOCSAVE < CLBYTES)
		ERROR!_kmeminit:_MAXALLOCSAVE_too_small
#endif

#ifndef PROM
	/*
	 * Compute the amount of memory for buffer pool.
	 * This doesn't belong here, but has a major impact
	 * on the malloc arena size that's required.
	 * We choose to allow a maximum of 10% of memory to
	 * be allocated to the buffer pool.
	 */
#ifdef	BUFMEM
	maxbufmem = BUFMEM * 1024;	/* BUFMEM in KB, maxbufmem in bytes */
#else
	if (maxbufmem == 0) {
		maxbufmem = (physmem / 10) * NBPG;
		if (maxbufmem < 16*MAXBSIZE)
			maxbufmem = 16*MAXBSIZE;
	}
#endif
	npg = (physmem / 5) + (VM_KMEM_SIZE / NBPG);
#else /* PROM */
	npg = VM_KMEM_SIZE / NBPG;
#endif /* PROM */

	kmemusage = (struct kmemusage *) kmem_alloc(kernel_map,
		(vm_size_t)(npg * sizeof(struct kmemusage)));
	kmem_map = kmem_suballoc(kernel_map, (vm_offset_t)&kmembase,
		(vm_offset_t)&kmemlimit, (vm_size_t)(npg * NBPG), FALSE);

#ifdef KMEMSTATS
	kbp = bucket;
	for (indx = 0; indx < MINBUCKET + 16; indx++, kbp++) {
		size = 1 << indx;
		if (size >= CLBYTES)
			kbp->kb_elmpercl = 1;
		else
			kbp->kb_elmpercl = CLBYTES / size;
		if (size <= MAXALLOCSAVE)
			kbp->kb_highwat = 5 * kbp->kb_elmpercl;
		else
			kbp->kb_highwat = 0;
	}
	for (indx = 0; indx < M_LAST; indx++)
		kmemstats[indx].ks_limit = VM_KMEM_SIZE * 6 / 10;
#ifndef PROM
	kmemstats[M_BUFFER].ks_limit =  physmem / 5 * NBPG;    /* 2X expected */
#endif
#endif
}
