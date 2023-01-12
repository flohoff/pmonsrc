/* 
 * p6032/sbdnvram.c: low-level interface to RTC NVRAM
 *
 * Copyright (c) 2000, Algorithmics Ltd.  All rights reserved.
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
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif

#include "sbd.h"
#include "nvenv.h"
#include "rtc.h"


unsigned int 
_sbd_nvram_size (unsigned int *nvoffs, unsigned int *nvsize)
{
    if (nvoffs) *nvoffs = RTC_NVSTART;
    if (nvsize) *nvsize = RTC_NVSIZE;
    return RTC_NVSIZE;
}


void
_sbd_nvram_flush (int offs)
{
}


__inline__ unsigned char
_sbd_nvram_getbyte(unsigned int offs)
{
    return _rtc_get (offs);
}


__inline__ void
_sbd_nvram_setbyte(unsigned int offs, unsigned char val)
{
    _rtc_set (offs, val);
}


/*
 * Always store shorts in little-endian format, so we don't
 * lose our environment when switching endianness.
 */
unsigned short
_sbd_nvram_getshort (unsigned int offs)
{
    return ((_sbd_nvram_getbyte(offs+1) << 8) | _sbd_nvram_getbyte(offs));
}


void
_sbd_nvram_setshort (unsigned int offs, unsigned short val)
{
    _sbd_nvram_setbyte (offs, val);
    _sbd_nvram_setbyte (offs+1, val >> 8);
}


int
_sbd_nvram_hwtest (void)
{
    return _rtc_get (RTC_STATUSD) & RTCSD_VRT ? 1 : 0;
}



#if !defined(BOOTPKG)
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
