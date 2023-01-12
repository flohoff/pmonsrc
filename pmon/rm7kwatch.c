/*
 * rm7kwatch.c: RM7000 family watchpoint support for SDE-MIPS kit
 *
 * Copyright (c) 1998-2000 Algorithmics Ltd - all rights reserved.
 * 
 * This program is NOT free software, it is supplied under the terms
 * of the SDE-MIPS License Agreement, a copy of which is available at:
 *
 *  http://www.algor.co.uk/algor/info/sde-license.pdf
 *
 * Any company which has obtained and signed a valid SDE-MIPS license
 * may use and modify this software internally and use (without
 * restrictions) any derived binary.  You may not, however,
 * redistribute this in whole or in part as source code, nor may you
 * modify or remove any part of this copyright message.
 */

#ifdef R4000
#include <sys/types.h>
#include <mips/rm7kc0.h>
#include <mips/cpu.h>
#include <mips/prid.h>

#include "watchpoint.h"

static int	rm7kwp_init (unsigned int);
static void	rm7kwp_setup (struct mips_watchpoint *);
static int 	rm7kwp_hit (vaddr_t *, size_t *);
static int	rm7kwp_set (struct mips_watchpoint *);
static int	rm7kwp_clear (struct mips_watchpoint *);
static void	rm7kwp_remove (void);
static void	rm7kwp_insert (void);
static void	rm7kwp_reset (void);

const struct mips_watchpoint_funcs _rm7k_watchpoint_funcs = {
    rm7kwp_init,
    rm7kwp_setup,
    rm7kwp_hit,
    rm7kwp_set,
    rm7kwp_clear,
    rm7kwp_remove,
    rm7kwp_insert,
    rm7kwp_reset
};


struct rm7kwp {
    unsigned long long	reg;
    unsigned long 	mask;
    unsigned int 	type;
};


#define W1	0
#define W2	1
static struct rm7kwp rm7kwp[2];


static int	
rm7kwp_init (unsigned int prid)
{
    switch ((prid >> 8) & 0xff) {
    case PRID_RM7000:
	/* rm7k w1/w2 supported */
	return 2;
    default:
	/* unrecognised */
	return -1;
    }
}


static void
rm7kwp_setup (struct mips_watchpoint *wp)
{
    wp->private = &rm7kwp[wp->num];
    wp->capabilities = MIPS_WATCHPOINT_MASK | MIPS_WATCHPOINT_WORD |
	MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W | MIPS_WATCHPOINT_X;
}


static int
rm7kwp_hit (vaddr_t *pva, size_t *plen)
{
    unsigned int cr = DBGREG[R_CAUSE];
    int hit = 0;

    if ((cr & CR_XMASK)  == CR_XCPT(EXC_DWE)
	|| (cr & CR_XMASK)  == CR_XCPT(EXC_IWE)) {
	if (cr & CR_W1)
	    hit |= rm7kwp[W1].type;
	if (cr & CR_W2)
	    hit |= rm7kwp[W2].type;
	if (hit)
	    hit = _mips_watchpoint_address (hit, pva, plen);
    }

    return hit;
}


static int
rm7kwp_set (struct mips_watchpoint *wp)
{
    struct rm7kwp *pwp = wp->private;
    
    pwp->type = wp->type;
    pwp->reg = wp->pa & ~wp->mask & WATCH_PA;
    pwp->mask = wp->mask & WATCHMASK_MASK;
    if (wp->type & MIPS_WATCHPOINT_X)
	pwp->reg |= WATCH_IN;
    if (wp->type & MIPS_WATCHPOINT_R)
	pwp->reg |= WATCH_LD;
    if (wp->type & MIPS_WATCHPOINT_W)
	pwp->reg |= WATCH_ST;

    return 1;
}


static int
rm7kwp_clear (struct mips_watchpoint *wp)
{
    struct rm7kwp *pwp = wp->private;
    if (pwp->type != 0)
	pwp->reg = pwp->mask = pwp->type = 0;
    return 1;
}


static void	
rm7kwp_remove (void)
{
    if (rm7kwp[W1].type || rm7kwp[W2].type) {
	_mips_watchlo = _mips_watchhi = 0;
	rm7k_setwatch1 (0);
	rm7k_setwatch2 (0);
    }
}


static void
rm7kwp_insert (void)
{
    unsigned long mask = 0;

    if (!rm7kwp[W1].type && !rm7kwp[W2].type)
	return;

    /* exception handler reloads lo/hi */
    _mips_watchlo = rm7kwp[W1].reg;
    _mips_watchhi = rm7kwp[W2].reg;

    /* accumulate widest don't-care mask */
    mask = rm7kwp[W1].mask | rm7kwp[W2].mask;

    if (mask != 0) {
	unsigned long long docare = ~(unsigned long long)mask;

	/* propagate widest mask and enable masking */
	if (rm7kwp[W1].mask != 0) {
	    _mips_watchlo &= docare;
	    mask |= WATCHMASK_W1;
	}
	if (rm7kwp[W2].mask != 0) {
	    _mips_watchhi &= docare;
	    mask |= WATCHMASK_W2;
	}
    }

#ifdef IN_PMON
    _mips_watchmask = mask;
#else
    /* we have to set the mask manually */
    rm7k_setwatchmask (mask);
#endif
}


static void
rm7kwp_reset (void)
{
    _mips_watchlo = _mips_watchhi = 0;
    rm7k_setwatch1 (0);
    rm7k_setwatch2 (0);
    memset (rm7kwp, 0, sizeof (rm7kwp));
}
#endif /* cache(rm7k) */
