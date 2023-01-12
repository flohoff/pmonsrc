/* 
 * NEC5074/sbdnvram.c: low-level interface to NEC DDB-Vrc5074 NVRAM
 * Copyright (c) 1999	Algorithmics Ltd
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
#include "ds1386.h"


unsigned int 
_sbd_nvram_size (unsigned int *nvoffs, unsigned int *nvsize)
{
    if (nvoffs) *nvoffs = ENVOFFSET;
    if (nvsize) *nvsize = TD_NVRAM_SIZE - ENVOFFSET;
    return TD_MEM_SIZE;
}


void
_sbd_nvram_flush (int offs)
{
}


__inline__ unsigned char
_sbd_nvram_getbyte(unsigned int offs)
{
    register volatile struct td_clock *td = PA_TO_KVA1(RTC_BASE);
    return td->td_mem[offs];
}


__inline__ void
_sbd_nvram_setbyte(unsigned int offs, unsigned char val)
{
    register volatile struct td_clock *td = PA_TO_KVA1(RTC_BASE);
    td->td_mem[offs] = val;
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
    unsigned int offs = ENVOFFSET + ENVOFF_TEST;

    _sbd_nvram_setbyte (offs, 0xaa);
    _sbd_nvram_setbyte (offs+1, 0x55);
    if (_sbd_nvram_getbyte (offs) != 0xaa)
	return 0;

    _sbd_nvram_setbyte (offs, 0x55);
    _sbd_nvram_setbyte (offs + 1, 0xaa);
    if (_sbd_nvram_getbyte (offs) != 0x55)
	return 0;

    return 1;
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
