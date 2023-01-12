/* 
 * p6032/sbdapc.: low-level interface to real-time clock chip APC
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


#include <sys/types.h>

#include "sbd.h"
#include "rtc.h"

unsigned int
_apc_get (int reg)
{
    unsigned int rtcsa, v;

    outb (RTC_ADDR_PORT, RTC_STATUSA);
    rtcsa = inb (RTC_DATA_PORT);
    if ((rtcsa & RTCSA_DVMASK) != RTC_DV2_OSC_ON)
	outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV2_OSC_ON);

    outb (RTC_ADDR_PORT, reg);
    v = inb (RTC_DATA_PORT);

    /* paranoia - switch back to bank 0 */
    outb (RTC_ADDR_PORT, RTC_STATUSA);
    outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);

    return (v);
}

unsigned int
_apc_set (int reg, unsigned int val)
{
    unsigned int rtcsa;
    unsigned int o;

    outb (RTC_ADDR_PORT, RTC_STATUSA);
    rtcsa = inb (RTC_DATA_PORT);
    if ((rtcsa & RTCSA_DVMASK) != RTC_DV2_OSC_ON)
	outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV2_OSC_ON);

    outb (RTC_ADDR_PORT, reg);
    o = inb (RTC_DATA_PORT);
    if (val != o)
	outb (RTC_DATA_PORT, val);

    /* paranoia - switch back to bank 0 */
    outb (RTC_ADDR_PORT, RTC_STATUSA);
    outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);

    return o;
}


unsigned int
_apc_bis (int reg, unsigned int val)
{
    unsigned int rtcsa, o, n;

    outb (RTC_ADDR_PORT, RTC_STATUSA);
    rtcsa = inb (RTC_DATA_PORT);
    if ((rtcsa & RTCSA_DVMASK) != RTC_DV2_OSC_ON)
	outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV2_OSC_ON);

    outb (RTC_ADDR_PORT, reg);
    o = inb (RTC_DATA_PORT);
    n = o | val;
    if (o != n)
	outb (RTC_DATA_PORT, n);

    /* paranoia - switch back to bank 0 */
    outb (RTC_ADDR_PORT, RTC_STATUSA);
    outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);

    return (o);
}

unsigned int
_apc_bic (int reg, unsigned int val)
{
    unsigned int rtcsa, o, n;

    outb (RTC_ADDR_PORT, RTC_STATUSA);
    rtcsa = inb (RTC_DATA_PORT);
    if ((rtcsa & RTCSA_DVMASK) != RTC_DV2_OSC_ON)
	outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV2_OSC_ON);

    outb (RTC_ADDR_PORT, reg);
    o = inb (RTC_DATA_PORT);
    n = o & ~val;
    if (o != n)
	outb (RTC_DATA_PORT, n);

    /* paranoia - switch back to bank 0 */
    outb (RTC_ADDR_PORT, RTC_STATUSA);
    outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);

    return (o);
}

