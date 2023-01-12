/* 
 * TEMIC/sbdflashenv.c: board-specific environment in Flash
 * Copyright (c) 1999  Algorithmics Ltd
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif
#include "flashdev.h"
#include "sbd.h"
#include "vrc5074.h"

flashcookie_t
_sbd_flashenv_open (unsigned int *env_offs, unsigned int *env_maxsize)
{
    flashcookie_t cookie;
    struct flashinfo info;
    int offs;
    unsigned int eoffs, esize;

    cookie = flash_open (0);
    if (cookie == FLASH_UNKNOWN || cookie == FLASH_MISSING)
	return FLASH_MISSING;
 
    /* find flash size */
    if (flash_info (cookie, 0, &info) != FLASHDEV_OK)
	return FLASH_MISSING;

    /* find the smallest writable sector from the top of the flash */
    offs = info.size;
    eoffs = esize = 0;
    while (offs > 0) {
	flash_info (cookie, offs - 1, &info);
	offs -= info.ssize;
	if (info.sprot)
	    continue;
	if (eoffs == 0 || info.ssize < esize) {
	    eoffs = info.soffs;
	    esize = info.ssize;
	}
	/* stop when we reach the non-boot sectors */
	if (info.ssize >= info.maxssize)
	    break;
    }
    if (esize == 0)
	return FLASH_MISSING;

    *env_offs = eoffs;
    *env_maxsize = esize;

    return cookie;
}



/*
 * Force environment reset if UART's DTR & DCD are looped together 
 *
 * But omit if ITROM is present, because it will have already
 * handled this, and may have placed other status values in
 * the environment which we must read.
 */

#if !defined(ITBASE)

#include "ns16550.h"

int
_sbd_env_force_reset ()
{
    int doreset = 1;
    volatile ns16550dev *dp;
    unsigned int omcr;
    int i;

    dp = PA_TO_KVA1 (UART0_BASE);
    omcr = dp->mcr;

    /* wiggle DTR four times */
    for (i = 0; i < 4 && doreset; i++) {

	/* check that we can force DCD off by clearing DTR */
	dp->mcr &= ~MCR_DTR; mips_wbflush(); 
	if ((dp->msr & MSR_DCD) != 0)
	    doreset = 0;

	/* check that we can force DCD on by setting DTR */
	dp->mcr |= MCR_DTR; mips_wbflush(); 
	if ((dp->msr & MSR_DCD) == 0)
	    doreset = 0;
    }

    dp->mcr = omcr; mips_wbflush();

    return (doreset);
}
#endif
