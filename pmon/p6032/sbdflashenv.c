/* 
 * p6032/sbdflashenv.c: board-specific environment in Flash
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
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

#ifdef IN_PMON
#include <sys/types.h>
#include <mips/cpu.h>
#include "pmon.h"
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif
#include "flashdev.h"
#include "sbd.h"
#include "bt.h"

#include "i82371eb.h"

flashcookie_t
_sbd_flashenv_open (void)
{
    /* XXX board specific address - wired-down flash prom */
    return flash_open (FLASH_BASE);
}


/*
 * Force environment reset if UART B's CTS & RTS are looped together 
 *
 * But omit if ITROM is present, because it will have already
 * handled this, and may have placed other status values in
 * the environment which we must read.
 */

#include "ns16550.h"

int
_sbd_env_force_reset (void)
{
    int doreset = 1;
    volatile ns16550dev *dp;
    unsigned int omcr;
    int i;

#ifdef BOOTPKG
    struct package *p = ((struct package *)PACKAGEINFO)+PKG_ITROM;
    /* If the ITROM magic mumber is set then don't reset the environment */
    if (p->magic == BTMAGIC)
	return 0;
#endif

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

/*
 * return true if the non-volatile environment should be ignored during PMON initialisation
 */

int
_sbd_env_suppress (void)
{
    int suppress = 0;
    if (sbd_boardtype () == 64 &&
	(sbd_switches () & CPLD_SWOPT_DEFENV))
	suppress = 1;

    return suppress;
}
