/* 
 * p6032/sbdtod.c: read/write time-of-day from real-time clock chip
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
#include <time.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif

#include "sbd.h"
#include "rtc.h"

/*
 * "Simple" time conversion (to avoid dragging in large C library versions),
 * these work in GMT only.
 */

#define	SECS_PER_MIN	60
#define	MINS_PER_HOUR	60
#define	HOURS_PER_DAY	24
#define	DAYS_PER_WEEK	7
#define	DAYS_PER_NYEAR	365
#define	DAYS_PER_LYEAR	366
#define	SECS_PER_HOUR	(SECS_PER_MIN * MINS_PER_HOUR)
#define	SECS_PER_DAY	((long) SECS_PER_HOUR * HOURS_PER_DAY)
#define	MONS_PER_YEAR	12

#define	TM_SUNDAY	0
#define	TM_MONDAY	1
#define	TM_TUESDAY	2
#define	TM_WEDNESDAY	3
#define	TM_THURSDAY	4
#define	TM_FRIDAY	5
#define	TM_SATURDAY	6

#define	TM_YEAR_BASE	1900

#define	EPOCH_YEAR	1970
#define	EPOCH_WDAY	TM_THURSDAY

#define	isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

static const int	mon_lengths[2][MONS_PER_YEAR] = {
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static const int	year_lengths[2] = {
	DAYS_PER_NYEAR, DAYS_PER_LYEAR
};


static time_t
rtc_mktime (const struct tm *tm)
{
    int yr;
    int mn;
    time_t secs;

    if (tm->tm_sec > 61 || 
	tm->tm_min > 59 || 
	tm->tm_hour > 23 || 
	tm->tm_mday > 31 || 
	tm->tm_mon > 12 ||
	tm->tm_year < 70) {
	return (time_t)-1;
    }
    
    /*
     * Sum up seconds from beginning of year
     */
    secs = tm->tm_sec;
    secs += tm->tm_min * SECS_PER_MIN;
    secs += tm->tm_hour * SECS_PER_HOUR;
    secs += (tm->tm_mday-1) * SECS_PER_DAY;
    
    for (mn = 0; mn < tm->tm_mon; mn++)
      secs += mon_lengths[isleap(tm->tm_year+TM_YEAR_BASE)][mn] * SECS_PER_DAY;
    
    for(yr=EPOCH_YEAR; yr < tm->tm_year + TM_YEAR_BASE; yr++)
      secs += year_lengths[isleap(yr)]*SECS_PER_DAY;
    
    return secs;
}


static struct tm *
rtc_gmtime (const time_t *clock)
{
    struct tm *	tmp;
    long	days;
    long	rem;
    int		y;
    int		yleap;
    const int *		ip;
    static struct tm	tm;

    tmp = &tm;
    days = *clock / SECS_PER_DAY;
    rem = *clock % SECS_PER_DAY;
    while (rem < 0) {
	rem += SECS_PER_DAY;
	--days;
    }
    while (rem >= SECS_PER_DAY) {
	rem -= SECS_PER_DAY;
	++days;
    }
    tmp->tm_hour = (int) (rem / SECS_PER_HOUR);
    rem = rem % SECS_PER_HOUR;
    tmp->tm_min = (int) (rem / SECS_PER_MIN);
    tmp->tm_sec = (int) (rem % SECS_PER_MIN);
    tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYS_PER_WEEK);
    if (tmp->tm_wday < 0)
      tmp->tm_wday += DAYS_PER_WEEK;
    y = EPOCH_YEAR;
    if (days >= 0)
      for ( ; ; ) {
	  yleap = isleap(y);
	  if (days < (long) year_lengths[yleap])
	    break;
	  ++y;
	  days = days - (long) year_lengths[yleap];
      }
    else do {
	--y;
	yleap = isleap(y);
	days = days + (long) year_lengths[yleap];
    } while (days < 0);
    tmp->tm_year = y - TM_YEAR_BASE;
    tmp->tm_yday = (int) days;
    ip = mon_lengths[yleap];
    for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
      days = days - (long) ip[tmp->tm_mon];
    tmp->tm_mday = (int) (days + 1);
    return tmp;
}



static int
rtcload (unsigned char *rtclk)
{
    int i;

    /* set 24 hour clock, binary mode */
    _rtc_bis (RTC_STATUSB, RTCSB_BINARY | RTCSB_24HR);

    /* check for valid battery */
    if ((_rtc_get (RTC_STATUSD) & RTCSD_VRT) == 0)
	return 0;
    
    /* uip spin */
    while (_rtc_get (RTC_STATUSA) & RTCSA_UIP)
	continue;

    /* quickly read all of the tod/alarm regs */
    for (i = 0; i < RTC_NTODREGS; i++)
	rtclk[i] = _rtc_get (i);

    return 1;
}


time_t
_sbd_gettod (void)
{
    unsigned char rtclk[RTC_NTODREGS];
    struct tm tm;
    int cent;
    
    if (!rtcload (rtclk))
	return (time_t)-1;

    cent  = rtclk[RTC_CENTURY];
    if (cent < 19 || cent > 22)
	return (time_t)-1;

    tm.tm_sec	= rtclk[RTC_SEC];
    tm.tm_min	= rtclk[RTC_MIN];
    tm.tm_hour	= rtclk[RTC_HRS];
    tm.tm_mday	= rtclk[RTC_DAY];
    tm.tm_mon	= rtclk[RTC_MONTH] - 1; /* 1-12 -> 0-11 */
    tm.tm_year	= rtclk[RTC_YEAR] + cent * 100;
    tm.tm_isdst = tm.tm_gmtoff = 0;

    /* deal with the millenium rollover... */
    if (tm.tm_year < EPOCH_YEAR)
	tm.tm_year += 100;
    tm.tm_year -= TM_YEAR_BASE;

    return rtc_mktime (&tm);
}


void
_sbd_settod (time_t t)
{
    unsigned char rtclk[RTC_NTODREGS];
    struct tm *tm;
    int i;

    tm = rtc_gmtime (&t);
    tm->tm_year += TM_YEAR_BASE;

    rtcload (rtclk);
    rtclk[RTC_SEC]	= tm->tm_sec;
    rtclk[RTC_MIN]	= tm->tm_min;
    rtclk[RTC_HRS]	= tm->tm_hour;
    rtclk[RTC_WDAY]	= tm->tm_wday + 1; /* 0-11 -> 1-12 */
    rtclk[RTC_YEAR]	= tm->tm_year % 100;
    rtclk[RTC_CENTURY]	= tm->tm_year / 100;
    rtclk[RTC_MONTH]	= tm->tm_mon + 1;
    rtclk[RTC_DAY]	= tm->tm_mday;

    /* stop updates while setting */
    _rtc_bis (RTC_STATUSB, RTCSB_UTI);

    /* write all of the tod/alarm regs */
    for (i = 0; i < RTC_NTODREGS; i++)
	_rtc_set (i, rtclk[i]);

    /* reenable updates */
    _rtc_bic (RTC_STATUSB, RTCSB_UTI);
}

#if defined(IN_PMON)
void
sbd_settime (time_t t)
{
    _sbd_settod (t);
}

time_t
sbd_gettime ()
{
    return _sbd_gettod ();
}
#endif

