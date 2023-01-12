/* 
 * devaz/sbdflashenv.c: board-specific environment in Flash
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

flashcookie_t
_sbd_flashenv_open (unsigned int *env_offs, unsigned int *env_maxsize)
{
    flashcookie_t cookie;
    struct flashinfo info;
    unsigned int eoffs, esize;

    /* XXX board specific address - wired-down flash prom */
    cookie = flash_open (FLASH_BASE);
    if (cookie == FLASH_UNKNOWN || cookie == FLASH_MISSING)
	return FLASH_MISSING;

    /* find flash size */
    if (flash_info (cookie, 0, &info) != FLASHDEV_OK)
	return FLASH_MISSING;

    /* find at least 512 bytes at top of flash */
    eoffs = info.size; esize = 0; 
    while (esize < 512) {
	flash_info (cookie, eoffs - 1, &info);
	eoffs = info.soffs;
	if (info.sprot) /* must be writable */
	    esize = 0;
	else
	    esize += info.ssize;
    }
    
    *env_offs = eoffs;
    *env_maxsize = esize;
    return cookie;
}


#if !defined(ITBASE)
/*
 * Force environment reset if UART B's CTS & RTS are looped together 
 *
 * But omit if ITROM is present, because it will have already
 * handled this, and may have placed other status values in
 * the environment which we must read.
 */

#include "ns16550.h"

int
_sbd_env_force_reset ()
{
    int doreset = 1;
    volatile ns16550dev *dp;
    unsigned int omcr;
    int i;

    dp = (volatile ns16550dev *) PA_TO_KVA1 (UART1_BASE);
    omcr = dp->mcr;

    /* wiggle RTS four times */
    for (i = 0; i < 4 && doreset; i++) {

	/* check that we can force CTS off by clearing RTS */
	dp->mcr &= ~MCR_RTS; mips_wbflush(); 
	if ((dp->msr & MSR_CTS) != 0)
	    doreset = 0;

	/* check that we can force CTS on by setting RTS */
	dp->mcr |= MCR_RTS; mips_wbflush(); 
	if ((dp->msr & MSR_CTS) == 0)
	    doreset = 0;
    }

    dp->mcr = omcr; mips_wbflush();

    return (doreset);
}
#endif
