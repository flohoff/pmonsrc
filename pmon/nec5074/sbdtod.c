/* 
 * NEC5074/sbdtod.c: time-of-day for NEC Vr5074 real-time clock chip
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifdef IN_PMON
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <sys/clock.h>
#include <mips/cpu.h>
#include <time.h>
#include "kit_impl.h"
#endif
#include <errno.h>

#include "sbd.h"
#include "ds1386.h"
#include "nvenv.h"

#define td_century	td_mem[ENVOFF_CENTURY]

extern int errno;

/*
 * "Simple" time conversion (to avoid dragging in large C library versions),
 * these work in UTC/GMT only. Now Y2K safe!
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

#ifndef IN_PMON
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

    if (tm->tm_sec > 59 || 
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
#endif


static unsigned int
bcdtobin (unsigned int bcd)
{
    return ((bcd >> 4) & 0x0f) * 10 + (bcd & 0x0f);
}


static unsigned int
bintobcd (unsigned int bin)
{
    return (((bin / 10) << 4) + bin % 10);
}


#ifdef IN_PMON
#define _sbd_gettod	sbd_gettime
#define _sbd_settod	sbd_settime
#endif

time_t
_sbd_gettod ()
{
    volatile struct td_clock *td = PA_TO_KVA1(RTC_BASE);
    struct tm tm;
    int cent;
    
    td->td_command &= ~TDCMD_TE;
    mips_wbflush();

    tm.tm_sec	= bcdtobin(td->td_secs & 0x7f);
    tm.tm_min	= bcdtobin(td->td_mins & 0x7f);
    tm.tm_hour	= bcdtobin(td->td_hours & 0x3f);
    tm.tm_mday	= bcdtobin(td->td_date & 0x3f);
    tm.tm_mon	= bcdtobin(td->td_month & 0x1f) - 1;
    tm.tm_year	= bcdtobin(td->td_year);
    cent  	= bcdtobin(td->td_century);
    tm.tm_wday	= 0;		/* ignored by mktime */
    tm.tm_yday	= 0;		/* ignored by mktime */

    /* make sure century is sensible */
    if (cent < 19 || cent > 20)
	td->td_century = bintobcd (cent = 19);

    /* deal with the millenium rollover... */
    if (cent == EPOCH_YEAR/100 && tm.tm_year < EPOCH_YEAR%100)
	td->td_century = bintobcd (++cent);

    td->td_command |= TDCMD_TE;
    mips_wbflush();
    
    if (cent < 19 || cent > 22)
	return (time_t)-1;

    tm.tm_year += cent * 100;
    tm.tm_year -= TM_YEAR_BASE;

    /* return time in seconds */
#ifdef IN_PMON
    return gmmktime (&tm);
#else
    return rtc_mktime (&tm);
#endif
}


void
_sbd_settod (time_t t)
{
    volatile struct td_clock *td = PA_TO_KVA1(RTC_BASE);
    struct tm *tm;

#ifdef IN_PMON
    tm = gmtime (&t);
#else
    tm = rtc_gmtime (&t);
#endif
    tm->tm_year += TM_YEAR_BASE;

    td->td_command &= ~TDCMD_TE;
    mips_wbflush();

    td->td_secs		= bintobcd(tm->tm_sec);
    td->td_mins		= bintobcd(tm->tm_min);
    td->td_hours	= bintobcd(tm->tm_hour);
    td->td_date 	= bintobcd(tm->tm_mday);
    td->td_day  	= bintobcd(tm->tm_wday);
    td->td_month	= bintobcd(tm->tm_mon + 1);
    td->td_year		= bintobcd(tm->tm_year % 100);
    td->td_century	= bintobcd(tm->tm_year / 100);

    td->td_command |= TDCMD_TE;
    mips_wbflush();
}

