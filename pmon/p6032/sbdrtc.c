/* 
 * p6032/sbdrtc.c: low-level interface to real-time clock chip
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
#include <mips.h>
#include <pmon.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#endif

#include "sbd.h"
#include "rtc.h"

static void
setaddress(int reg)
{
    unsigned char rtca;

    /* PC97307 RTC */
    if (reg < 0x40)
	outb(RTC_ADDR_PORT, reg);	/* accessible from any bank */
    else {
	outb (RTC_ADDR_PORT, RTC_STATUSA);
	rtca = inb (RTC_DATA_PORT);
	if (reg < 0x80) {
	    /* accessible from bank 0 */
	    if ((rtca & RTCSA_DVMASK) != RTC_DV0_OSC_ON)
		outb (RTC_DATA_PORT, (rtca & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);
	    outb (RTC_ADDR_PORT, reg);
	}
	else {
	    /* accessible indirectly via bank 1 */
	    if ((rtca & RTCSA_DVMASK) != RTC_DV1_OSC_ON)
		outb (RTC_DATA_PORT, (rtca & ~RTCSA_DVMASK) | RTC_DV1_OSC_ON);
	    outb (RTC_ADDR_PORT, RTC_BANK1_URADDR);
	    outb (RTC_DATA_PORT, reg - 0x80);
	    outb (RTC_ADDR_PORT, RTC_BANK1_URDATA);
	}
    }
}

unsigned int
_rtc_get (int reg)
{
    setaddress (reg);
    return inb (RTC_DATA_PORT);
}

unsigned int
_rtc_set (int reg, unsigned int val)
{
    unsigned int o;

    setaddress (reg);
    o = inb (RTC_DATA_PORT);
    if (val != o)
	outb (RTC_DATA_PORT, val);
    return (o);
}

unsigned int
_rtc_bis (int reg, unsigned int val)
{
    unsigned int o, n;

    setaddress (reg);
    o = inb (RTC_DATA_PORT);
    n = o | val;
    if (n != o)
	outb (RTC_DATA_PORT, n);
    return (o);
}

unsigned int
_rtc_bic (int reg, unsigned int val)
{
    unsigned int o, n;

    setaddress (reg);
    o = inb (RTC_DATA_PORT);
    n = o & ~val;
    if (n != o)
	outb (RTC_DATA_PORT, n);
    return (o);
}
