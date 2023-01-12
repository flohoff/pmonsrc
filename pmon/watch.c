/*
 * watch.c: switchable watchpoint support for PMON
 *
 * Copyright (c) 1998-2000 Algorithmics Ltd - all rights reserved.
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

#include <sys/types.h>
#include <mips/cpu.h>
#include <mips/instrdef.h>
#include <watchpoint.h>

#ifdef R4000
#if 0
extern struct mips_watchpoint_funcs	_m32_watchpoint_funcs;
extern struct mips_watchpoint_funcs	_r54_watchpoint_funcs;
#endif
extern struct mips_watchpoint_funcs	_r4650_watchpoint_funcs;
extern struct mips_watchpoint_funcs	_r4k_watchpoint_funcs;
extern struct mips_watchpoint_funcs	_r5k_watchpoint_funcs;
extern struct mips_watchpoint_funcs	_rm7k_watchpoint_funcs;
#endif
#ifdef LR33000
extern struct mips_watchpoint_funcs	_lr33k_watchpoint_funcs;
#endif
#ifdef CW40XX
extern struct mips_watchpoint_funcs	_cw4_watchpoint_funcs;
#endif

static const struct mips_watchpoint_funcs *watchpoint_funcs[] = {
#ifdef R4000
#if 0
    &_m32_watchpoint_funcs,
    &_r54_watchpoint_funcs,
#endif
    &_r4650_watchpoint_funcs,
    &_r4k_watchpoint_funcs,
    &_r5k_watchpoint_funcs,
    &_rm7k_watchpoint_funcs,
#endif
#ifdef LR33000
    &_lr33k_watchpoint_funcs,
#endif
#ifdef CW40XX
    &_cw4_watchpoint_funcs,
#endif
    0
};


/* current cpu functions */
static const struct mips_watchpoint_funcs *wfn;

/* list of current watchpoints */
static struct mips_watchpoint watchpoints[WATCHPOINT_MAX];
static int nwatchpoints = -1;

/*
 * Initialise the watchpoint subsystem
 */
int
_mips_watchpoint_init (void)
{
    const struct mips_watchpoint_funcs * const *wfnp;
    unsigned int prid;
    int found = 0;

    if (wfn)
	/* already initialised */
	return nwatchpoints;

    /* scan for cpu-specific support code */
    prid = DBGREG[R_PRID];
    for (wfnp = watchpoint_funcs; *wfnp; wfnp++)
	if ((found = (*wfnp)->wp_init (prid)) >= 0)
	    break;

    if (found > 0) {
	int i;
	wfn = *wfnp;

	/* initialise each watchpoint resource */
	nwatchpoints = found;
	for (i = 0; i < found; i++) {
	    watchpoints[i].num = i;
	    wfn->wp_setup (&watchpoints[i]);
	}

	/* reset cpu-specific facilities */
	wfn->wp_reset ();
    }

    /* return number of resources */
    return nwatchpoints;
}


/*
 * Return the number of watchpoint resources.
 * XXX redundant function
 */
int
_mips_watchpoint_howmany (void)
{
    return nwatchpoints;
}


/*
 * Return the capabilities of the numbered watchpoint resource (0 .. n-1)
 */
int
_mips_watchpoint_capabilities (int wpnum)
{
    if (wpnum < 0 || wpnum >= nwatchpoints)
	return 0;
    return watchpoints[wpnum].capabilities;
}


/*
 * Return non-zero set of "hit" flags if we've just hit a watchpoint
 * and optionally return the virtual address and byte lenfth of 
 * the access which triggered it.
 */
int
_mips_watchpoint_hit (vaddr_t *pva, size_t *plen)
{
    int hit;
    vaddr_t va;
    size_t len;

    if (!wfn)
	return 0;

    if (!pva)
	pva = &va;

    if (!plen)
	plen = &len;

    hit = wfn->wp_hit (pva, plen);

    if (hit) {
	struct mips_watchpoint *wp;
	for (wp = watchpoints; wp < &watchpoints[nwatchpoints]; wp++) {
	    if ((wp->type & hit) && wp->len) {
		vaddr_t wpa, hita;

		wpa = wp->va;
		hita = *pva;
		if (!(wp->capabilities & MIPS_WATCHPOINT_VADDR)) {
		    wpa = KVA_TO_PA (wpa);
		    hita = KVA_TO_PA (hita);
		}

		/* do address ranges overlap */
		if (wpa < hita + *plen && hita < wpa + wp->len)
		    return hit | (wp->type & MIPS_WATCHPOINT_VADDR);
	    }
	}
	hit |= MIPS_WATCHPOINT_INEXACT;
    }

    return hit;
}


/*
 * Remove watchpoints after a debug exception
 */
void
_mips_watchpoint_remove (void)
{
    if (wfn)
	wfn->wp_remove ();
}


/*
 * Insert watchpoints before returning from febug exception
 */
void
_mips_watchpoint_insert (void)
{
    if (wfn)
	wfn->wp_insert ();
}


/*
 * Chance for debug host to complain if the address range
 * covered by the watchpoint would overlap its own data.
 */
#pragma weak _mips_watchpoint_set_callback
int
_mips_watchpoint_set_callback (int asid, vaddr_t va, size_t len)
{
    return MIPS_WP_OK;
}


/*
 * Add a new watchpoint 
 */
int
_mips_watchpoint_set (int type, int asid, vaddr_t va, paddr_t pa, size_t len)
{
    struct mips_watchpoint *wp;
    int res = MIPS_WP_NOTSUP;

    if (nwatchpoints <= 0)
	return MIPS_WP_NONE;

    for (wp = watchpoints; wp < &watchpoints[nwatchpoints]; wp++) {
	if ((wp->capabilities & type) == type) {
	    if (wp->len != 0)
		res = MIPS_WP_INUSE;
	    else {
		wp->type = type;
		wp->asid = asid;
		wp->va = va;
		if ((wp->capabilities & MIPS_WATCHPOINT_VADDR) || pa)
		    wp->pa = pa;
		else if (IS_KVA01 (va))
		    wp->pa = KVA_TO_PA (va);
		else
		    return MIPS_WP_BADADDR;
		wp->len = len;
		wp->mask = _mips_watchpoint_calc_mask (va, len);
		if (!wfn->wp_set (wp))
		    res = MIPS_WP_NOTSUP;
		else
		    res = _mips_watchpoint_set_callback (wp->asid, 
							 wp->va & ~wp->mask, 
							 wp->mask + 1);
		if (res != MIPS_WP_OK)
		    wp->len = 0;
		break;
	    }
	}
    }

    return res;
}


/*
 * Remove an existing watchpoint 
 */
int
_mips_watchpoint_clear (int type, int asid, vaddr_t va, size_t len)
{
    struct mips_watchpoint *wp;

    for (wp = watchpoints; wp < &watchpoints[nwatchpoints]; wp++) {
	if (wp->type == type && wp->asid == asid
	    && wp->va == va && wp->len == len) {
	    wp->len = 0;
	    wfn->wp_clear (wp);
	    return MIPS_WP_OK;
	}
    }

    return MIPS_WP_NOMATCH;
}


/*
 * Remove all watchpoints
 */
void
_mips_watchpoint_reset (void)
{
    if (wfn) {
	struct mips_watchpoint *wp;
	for (wp = watchpoints; wp < &watchpoints[nwatchpoints]; wp++)
	    wp->len = 0;
	wfn->wp_reset ();
    }
}



/* 
 * Compute a don't care mask for the region bounding ADDR 
 * and ADDR + LEN - 1. 
 */

unsigned long
_mips_watchpoint_calc_mask (addr, len)
     vaddr_t addr;
     size_t len;
{
  unsigned long mask;
  int i;

  mask = addr ^ (addr + len - 1);

  for (i = 32; i >= 0; i--)
    if (mask == 0)
      break;
    else
      mask >>= 1;

  mask = (unsigned long) 0xffffffff >> i;

  return mask;
}


/* Size of data access for all load/store instructions */
static const unsigned char datalen[64] = {
    0, 0, 0, 0,		/* 00: */
    0, 0, 0, 0,		/* 04: */
    0, 0, 0, 0,		/* 08: */
    0, 0, 0, 0,		/* 12: */
    0, 0, 0, 0,		/* 16: */
    0, 0, 0, 0,		/* 20: */
    0, 0, 7, 7,		/* 24: xx  xx    ldl  ldr */
    0, 0, 0, 0,		/* 28: */
    1, 2, 3, 4,		/* 32: lb   lh   lwl  lw   */
    1, 2, 3, 4,		/* 36: lbu  lhu  lwr  lwu  */
    1, 2, 3, 4,		/* 40: sb   sh   swl  sw   */
    7, 7, 3, 4,		/* 44: sdl  sdr  swr  cache*/
    4, 4, 4, 4,		/* 48: ll   lwc1 lwc3 lwc3 */
    8, 8, 8, 8,		/* 52: lld  ldc1 ldc2 ld   */
    4, 4, 4, 4,		/* 56: sc   swc1 swc2 swc3 */
    8, 8, 8, 8		/* 60: scd  sdc1 sdc2 sd   */
};


/* 
 * Return the address referenced by the trapped instruction, and the
 * size of the access.
 */
int
_mips_watchpoint_address (int type, vaddr_t *pva, size_t *plen)
{
    vaddr_t pc = (vaddr_t) DBGREG[R_EPC];
    vaddr_t va = 0;
    int len = 0;

    if ((pc & 3) == 0) {
	if (DBGREG[R_CAUSE] & CR_BD)
	    pc += 4;
	if (type & (MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W)) {
	    union mipsn_instr i;
	    int err = 0;
	    type &= ~(MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W);
	    i.value = load_word (pc);
	    if (err == 0 && (len = datalen[i.itype.op])) {
		/* calculate virtual address */
		va = (vaddr_t) DBGREG[R_ZERO + i.itype.rs] + i.itype.imm;

		/* handle left/right magic */
		if (len == 3 || len == 7) {
		    size_t plen;
		    switch (i.itype.op) {
		    case ITYPE_lwl:
		    case ITYPE_swl:
		    case ITYPE_ldl:
		    case ITYPE_sdl:
#ifdef __MIPSEB__
			plen = len + 1 - (va & len);
#else
			va &= ~len;
			plen = (va & len) + 1;
#endif
			break;
		    case ITYPE_ldr:
		    case ITYPE_sdr:
		    case ITYPE_lwr:
		    case ITYPE_swr:
#ifdef __MIPSEB__
			va &= ~len;
			plen = (va & len) + 1;
#else
			plen = len + 1 - (va & len);
#endif
			break;
		    default:
			abort();
		    }
		    len = plen;
		}

		if (i.itype.op == ITYPE_ldl || i.itype.op == ITYPE_ldr)
		    /* special cases */
		    type |= MIPS_WATCHPOINT_R;
		else
		    /* normal case */
		    type |= ((i.itype.op & 8) ? MIPS_WATCHPOINT_W
			     : MIPS_WATCHPOINT_R);
	    }
	}
	if (type == MIPS_WATCHPOINT_X) {
	    /* not a load/store, must be an instruction fetch breakpoint */
	    va = pc;
	    len = 4;
	}
    }

    if (plen)
	*plen = len;
    if (pva)
	*pva = va;
    return type;
}
