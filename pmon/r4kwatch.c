/*
 * r4kwatch.c: R4000 family watchpoint support for SDE-MIPS kit
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
#include <mips/r4kc0.h>
#include <mips/cpu.h>
#include <mips/prid.h>

#include "watchpoint.h"

static int	r4kwp_init (unsigned int);
static void	r4kwp_setup (struct mips_watchpoint *);
static int 	r4kwp_hit (vaddr_t *, size_t *);
static int	r4kwp_set (struct mips_watchpoint *);
static int	r4kwp_clear (struct mips_watchpoint *);
static void	r4kwp_remove (void);
static void	r4kwp_insert (void);
static void	r4kwp_reset (void);

const struct mips_watchpoint_funcs _r4k_watchpoint_funcs = {
    r4kwp_init,
    r4kwp_setup,
    r4kwp_hit,
    r4kwp_set,
    r4kwp_clear,
    r4kwp_remove,
    r4kwp_insert,
    r4kwp_reset
};

static unsigned long	r4kwp_reg;
static unsigned int	r4kwp_type;

static int	
r4kwp_init (unsigned int prid)
{
    switch ((prid >> 8) & 0xff) {
    case PRID_R4000:
    case PRID_R4100:
    case PRID_R4200:
    case PRID_R4300:
    case PRID_R5400:
	/* r4000 watchpoint supported */
	return 1;
    case PRID_R4600:
    case PRID_R4700:
    case PRID_RC6447X:
	/* recognised, but don't support watchpoints */
	return 0;
    default:
	return -1;
    }
}


static void
r4kwp_setup (struct mips_watchpoint *wp)
{
    wp->capabilities = MIPS_WATCHPOINT_DWORD | MIPS_WATCHPOINT_R
	| MIPS_WATCHPOINT_W;
}


static int
r4kwp_hit (vaddr_t *pva, size_t *plen)
{
    unsigned int cr = DBGREG[R_CAUSE];
    int hit = 0;

    if ((cr & CR_XMASK)  == CR_XCPT(EXC_WATCH))
	hit = _mips_watchpoint_address (r4kwp_type, pva, plen);

    return hit;
}


static int
r4kwp_set (struct mips_watchpoint *wp)
{
    if (((wp->va & 7) + wp->len - 1) > 7)
	return 0;
    wp->mask = 7;

    r4kwp_reg = wp->pa & WATCHLO_PA;
    r4kwp_type = wp->type;
    if (wp->type & MIPS_WATCHPOINT_R)
	r4kwp_reg |= WATCHLO_R;
    if (wp->type & MIPS_WATCHPOINT_W)
	r4kwp_reg |= WATCHLO_W;
    return 1;
}


static int
r4kwp_clear (struct mips_watchpoint *wp)
{
    r4kwp_reg = 0;
    return 1;
}


static void	
r4kwp_remove (void)
{
    if (r4kwp_reg) {
	mips_setwatchlo (0);
	_mips_watchlo = 0;
    }
}


static void
r4kwp_insert (void)
{
    /* exception handler restores watchpoint reg */
    if (r4kwp_reg)
	_mips_watchlo = r4kwp_reg;
}


static void
r4kwp_reset (void)
{
    r4kwp_reg = 0;
    _mips_watchlo = 0;
    mips_setwatchlo (0);
    mips_setwatchhi (0);
}
#endif
