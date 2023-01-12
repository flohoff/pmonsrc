/*
 * rtc.c: interface to the PC-style real-time clock chip
 */

#include <mips.h>
#include <pmon.h>
#include <p5064/sbd.h>
#include <p5064/rtc.h>


static void
setaddress(int reg)
{
    if (BOARD_REVISION >= 'C') {
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
    else
	outb (RTC_ADDR_PORT, reg);
}

unsigned int
rtc_get (int reg)
{
    setaddress (reg);
    return inb (RTC_DATA_PORT);
}

unsigned int
rtc_set (int reg, unsigned int val)
{
    unsigned int o;
    setaddress (reg);
    o =  inb (RTC_DATA_PORT);
    if (val != o)
	outb (RTC_DATA_PORT, val);
    return (o);
}

unsigned int
rtc_bis (int reg, unsigned int val)
{
    unsigned int o, n;

    setaddress (reg);
    o =  inb (RTC_DATA_PORT);
    n = o | val;
    if (n != o)
	outb (RTC_DATA_PORT, n);
    return (o);
}

unsigned int
rtc_bic (int reg, unsigned int val)
{
    unsigned int o, n;

    setaddress (reg);
    o =  inb (RTC_DATA_PORT);
    n = o & ~val;
    if (n != o)
	outb (RTC_DATA_PORT, n);
    return (o);
}

unsigned int
apc_get (int reg)
{
    unsigned int rtcsa, v;

    if (BOARD_REVISION < 'C')
	return ~0;

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
apc_set (int reg, unsigned int val)
{
    unsigned int rtcsa;

    if (BOARD_REVISION < 'C')
	return ~0;

    outb (RTC_ADDR_PORT, RTC_STATUSA);
    rtcsa = inb (RTC_DATA_PORT);
    if ((rtcsa & RTCSA_DVMASK) != RTC_DV2_OSC_ON)
	outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV2_OSC_ON);

    outb (RTC_ADDR_PORT, reg);
    outb (RTC_DATA_PORT, val);

    /* paranoia - switch back to bank 0 */
    outb (RTC_ADDR_PORT, RTC_STATUSA);
    outb (RTC_DATA_PORT, (rtcsa & ~RTCSA_DVMASK) | RTC_DV0_OSC_ON);
}


unsigned int
apc_bis (int reg, unsigned int val)
{
    unsigned int rtcsa, o, n;

    if (BOARD_REVISION < 'C')
	return ~0;

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
apc_bic (int reg, unsigned int val)
{
    unsigned int rtcsa, o, n;

    if (BOARD_REVISION < 'C')
	return ~0;

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


static int rtcload (unsigned char *rtclk)
{
    int i;

    memset (rtclk, 0, RTC_NTODREGS);

    /* set 24 hour clock, binary mode */
    rtc_bis (RTC_STATUSB, RTCSB_BINARY | RTCSB_24HR);

    /* check for valid battery */
    if ((rtc_get (RTC_STATUSD) & RTCSD_VRT) == 0)
	return (0);
    
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
    rtclk[RTC_WDAY]	= tm->tm_wday + 1; /* 0-11 -> 1-12 */
    rtclk[RTC_YEAR]	= tm->tm_year % 100;
    rtclk[RTC_CENTURY]	= (tm->tm_year / 100) + 19;
    rtclk[RTC_MONTH]	= tm->tm_mon + 1;
    rtclk[RTC_DAY]	= tm->tm_mday;

    /* stop updates while setting */
    rtc_bis (RTC_STATUSB, RTCSB_UTI);

    /* write all of the tod/alarm regs */
    for (i = 0; i < RTC_NTODREGS; i++)
	rtc_set (i, rtclk[i]);

    /* reenable updates */
    rtc_bic (RTC_STATUSB, RTCSB_UTI);
}
