#ifdef RTC

#include <mips.h>
#include <pmon.h>
#include "sbd.h"

/*
 * The WinCE driver for the Vr41xx real-time clock chip uses Jan 1 1850 as 
 * its "year 0".  This is:
 *
 * 1) probably a typo, the WinCE HAL manual recommends the range 1950-2050;
 *
 * 2) the HAL manual assumes that your RTC only stores the last two year 
 *    digits, and so isn't relevant for this RTC which holds a simple
 *    incrementing counter - they could have used any nearby Jan 1;
 *
 * 3) it also means that they've used up 32 of the 33 bit seconds
 *    counter, so we'll wraparound in 136 years, not 272.
 *
 * Anyway, to maintain compatibility when cross-booting between WinCE 
 * and SDE-MIPS/POSIX, we include this fudge factor, which is the number
 * of seconds between Jan 1 1850 and Jan 1 1970 (the POSIX year 0).
 */
#define WINCE_TO_UNIX_SECS	3786825600ULL

typedef unsigned long long time48_t;


time_t
sbd_gettime (void)
{
    volatile struct vr41xxrtc1 *rtc = PA_TO_KVA1 (RTC1_BASE);
    time48_t t1, t2;

    /* get consistent time from rtc */
    do {
	t1 = ((time48_t)rtc->rtc1_etimeh << 32)
	     | ((time48_t)rtc->rtc1_etimem << 16)
	     | ((time48_t)rtc->rtc1_etimel << 0);
	t2 = ((time48_t)rtc->rtc1_etimeh << 32)
	     | ((time48_t)rtc->rtc1_etimem << 16)
	     | ((time48_t)rtc->rtc1_etimel << 0);
    } while (t1 != t2);

    /* convert from 32.768kHz count to seconds */
    t1 = (t1 + 16384) / 32768;

    if (t1 >= WINCE_TO_UNIX_SECS)
	/* convert WinCE secs (Jan 1, 1850) to Unix secs (Jan 1, 1970) */
	t1 -= WINCE_TO_UNIX_SECS;

    return (time_t)t1;
}


void
sbd_settime (time_t t)
{
    volatile struct vr41xxrtc1 *rtc = PA_TO_KVA1 (RTC1_BASE);
    time48_t et = t;

    /* force 100us delay between read and write */
    sbddelay (100);

    /* convert Unix secs (Jan 1, 1970) to WinCE secs (Jan 1, 1850) */
    et += WINCE_TO_UNIX_SECS;

    /* convert from seconds to 32.768kHz count */
    et *= 32768;

    /* store in rtc */
    rtc->rtc1_etimel = et >> 0;
    rtc->rtc1_etimem = et >> 16;
    rtc->rtc1_etimeh = et >> 32;
}
#endif
