/*
 * r4650watch.c: R4650 family watchpoint support for SDE-MIPS kit
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
#include <mips/r4650.h>
#include <mips/cpu.h>
#include <mips/prid.h>

#include "watchpoint.h"

static int	r4650wp_init (unsigned int);
static void	r4650wp_setup (struct mips_watchpoint *);
static int 	r4650wp_hit (vaddr_t *, size_t *);
static int	r4650wp_set (struct mips_watchpoint *);
static int	r4650wp_clear (struct mips_watchpoint *);
static void	r4650wp_remove (void);
static void	r4650wp_insert (void);
static void	r4650wp_reset (void);

const struct mips_watchpoint_funcs _r4650_watchpoint_funcs = {
    r4650wp_init,
    r4650wp_setup,
    r4650wp_hit,
    r4650wp_set,
    r4650wp_clear,
    r4650wp_remove,
    r4650wp_insert,
    r4650wp_reset
};


struct r4650wp {
    unsigned long	reg;
    unsigned int	mask;
};


#define IWATCH	0
#define DWATCH	1
static struct r4650wp r4650wp[2];


static int	
r4650wp_init (unsigned int prid)
{
    switch ((prid >> 8) & 0xff) {
    case PRID_R4650:
    case PRID_RC3236X:
	/* r4650 iwatch/dwatch don't care bits */
	r4650wp[IWATCH].mask = 3;
	r4650wp[DWATCH].mask = 7;
	return 2;
    default:
	/* unrecognised */
	return -1;
    }
}

static void
r4650wp_setup (struct mips_watchpoint *wp)
{
    wp->private = &r4650wp[wp->num];
    wp->capabilities =  MIPS_WATCHPOINT_VADDR | MIPS_WATCHPOINT_DWORD;
    if (wp->num == IWATCH)
	wp->capabilities |= MIPS_WATCHPOINT_X;
    else
	wp->capabilities |= MIPS_WATCHPOINT_R | MIPS_WATCHPOINT_W;
}


static int
r4650wp_hit (vaddr_t *pva, size_t *plen)
{
    unsigned int cr = DBGREG[R_CAUSE];
    int hit = 0;

    if ((cr & CR_XMASK)  == CR_XCPT(EXC_WATCH) 
	&& (r4650wp[IWATCH].reg || r4650wp[DWATCH].reg)) {
	hit = (cr & CR_IW) ? MIPS_WATCHPOINT_X : MIPS_WATCHPOINT_W;
	hit = _mips_watchpoint_address (hit, pva, plen);
    }

    return hit;
}


static int
r4650wp_set (struct mips_watchpoint *wp)
{
    struct r4650wp *pwp = wp->private;

    if (((wp->va & pwp->mask) + wp->len - 1) > pwp->mask)
	return 0;
    wp->mask = pwp->mask;

    pwp->reg = wp->va & ~pwp->mask;
    if (wp->type & MIPS_WATCHPOINT_X)
	pwp->reg |= IWATCH_EN;
    if (wp->type & MIPS_WATCHPOINT_R)
	pwp->reg |= DWATCH_R;
    if (wp->type & MIPS_WATCHPOINT_W)
	pwp->reg |= DWATCH_W;

    return 1;
}


static int
r4650wp_clear (struct mips_watchpoint *wp)
{
    struct r4650wp *pwp = wp->private;
    pwp->reg = 0;
    return 1;
}


static void	
r4650wp_remove (void)
{
    if (r4650wp[IWATCH].reg || r4650wp[DWATCH].reg) {
	_mips_watchlo = _mips_watchhi = 0;
	r4650_setiwatch (0);
	r4650_setdwatch (0);
    }
}


static void
r4650wp_insert (void)
{
    if (r4650wp[IWATCH].reg || r4650wp[DWATCH].reg) {
	/* exception handler reloads */
	_mips_watchlo = r4650wp[IWATCH].reg;
	_mips_watchhi = r4650wp[DWATCH].reg;
    }
}


static void
r4650wp_reset (void)
{
    r4650wp[IWATCH].reg = r4650wp[DWATCH].reg = 0;
    _mips_watchlo = _mips_watchhi = 0;
    r4650_setiwatch (0);
    r4650_setdwatch (0);
}
#endif
