/*
 * rtc.c: interface to the PC-style real-time clock chip
 */

#include <mips.h>
#include <pmon.h>
#include <p4032/sbd.h>
#include <p4032/rtc.h>

static volatile unsigned int * const rtcaddr = PA_TO_KVA1 (RTC_BASE+RTC_ADDR);
static volatile unsigned int * const rtcdata = PA_TO_KVA1 (RTC_BASE+RTC_DATA);

unsigned int
rtc_get (int reg)
{
    *rtcaddr = reg;
    return (*rtcdata & 0xff);
}

unsigned int
rtc_set (int reg, unsigned int val)
{
    unsigned int o;
    *rtcaddr = reg;
    o = *rtcdata & 0xff;
    if (val != o)
	*rtcdata = val;
    return (o);
}

unsigned int
rtc_bis (int reg, unsigned int val)
{
    unsigned int o, n;

    *rtcaddr = reg;
    o = *rtcdata & 0xff;
    n = o | val;
    if (n != o)
	*rtcdata = n;
    return (o);
}

unsigned int
rtc_bic (int reg, unsigned int val)
{
    unsigned int o, n;

    *rtcaddr = reg;
    o = *rtcdata & 0xff;
    n = o & ~val;
    if (n != o)
	*rtcdata = n;
    return (o);
}




static int rtcload (unsigned char *rtclk)
{
    int i;

    /* set 24 hour clock, binary mode */
    rtc_bis (RTC_STATUSB, RTCSB_BINARY | RTCSB_24HR);

    /* check for valid battery */
    if (rtc_get (RTC_STATUSD) & RTCSD_VRT == 0) {
	memset (rtclk, 0, RTC_NTODREGS);
	return (0);
    }
    
    /* uip spin */
    i = 1000;
    while (rtc_get (RTC_STATUSA) & RTCSA_UIP) {
	if (--i == 0)
	    return (0);
    }

    /* read all of the tod/alarm regs */
    for (i = 0; i < RTC_NTODREGS; i++)
	rtclk[i] = rtc_get (i);

    return (1);
}


time_t
sbd_gettime (void)
{
    unsigned char rtclk[RTC_NTODREGS];
    struct tm tm;
    time_t t;
    int cent;
    
    if (!rtcload (rtclk))
	return (time_t)-1;

    cent  = rtclk[RTC_CENTURY];
    if (cent < 19 || cent > 22)
	sbd_settime (t = 0);

    tm.tm_sec	= rtclk[RTC_SEC];
    tm.tm_min	= rtclk[RTC_MIN];
    tm.tm_hour	= rtclk[RTC_HRS];
    tm.tm_wday  = rtclk[RTC_WDAY] - 1; /* 1-7 -> 0-6 */
    tm.tm_mday	= rtclk[RTC_DAY];
    tm.tm_mon	= rtclk[RTC_MONTH] - 1; /* 1-12 -> 0-11 */
    tm.tm_year	= rtclk[RTC_YEAR];
    tm.tm_isdst = tm.tm_gmtoff = 0;

    /*
     * this deals with the millenium -
     * after that someone else can worry...
     */
    if (cent == 19 && tm.tm_year < 70)
	rtclk[RTC_CENTURY] = ++cent;
    tm.tm_year += (cent - 19) * 100;

    t = gmmktime (&tm);
    if (t == -1)
	sbd_settime (t = 0);

    return t;
}


void
sbd_settime (time_t t)
{
    struct tm *tm;
    unsigned char rtclk[RTC_NTODREGS];
    int i;

    tm = gmtime (&t);
    rtcload (rtclk);
    rtclk[RTC_SEC]	= tm->tm_sec;
    rtclk[RTC_MIN]	= tm->tm_min;
    rtclk[RTC_HRS]	= tm->tm_hour;
    rtclk[RTC_WDAY]	= tm->tm_wday + 1; /* 0-6 -> 1-7 */
    rtclk[RTC_YEAR]	= tm->tm_year % 100;
    rtclk[RTC_CENTURY]	= (tm->tm_year / 100) + 19;
    rtclk[RTC_MONTH]	= tm->tm_mon + 1; /* 0-11 -> 1->12 */
    rtclk[RTC_DAY]	= tm->tm_mday;

    /* stop updates while setting */
    rtc_bis (RTC_STATUSB, RTCSB_UTI);

    /* write all of the tod/alarm regs */
    for (i = 0; i < RTC_NTODREGS; i++)
	rtc_set (i, rtclk[i]);

    /* reenable updates */
    rtc_bic (RTC_STATUSB, RTCSB_UTI);
}
