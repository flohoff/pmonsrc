/* 
 * NEC5074/sbdfreq.c: CPU frequency determination for NEC DDB-Vrc5074 board
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifdef IN_PMON
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include <mips/prid.h>
#include "kit_impl.h"
#endif

#include "sbd.h"
#include "ds1386.h"
#include "vrc5074.h"

#ifdef IN_PMON
#define _sbd_pipefreq	sbdpipefreq
#define _sbd_cpufreq	sbdcpufreq
#define mips_getcount()	get_count()
#define r5k_getconfig()	Config

int cpuspeed = MHZ/2;	/* worst case until it gets configured */
#endif

#define RTC_HZ		20
#define RTC_WD_CSECS	0x05

/*
 * Dynamically determine CPU pipeline clock frequency by running the
 * on-chip timer against the Real Time Clock watchdog timer.
 */

long
_sbd_pipefreq (void)
{
    static long pipefreq = 0;
    volatile struct td_clock *td = PA_TO_KVA1 (RTC_BASE);
    unsigned long start, cnt, timeout;
    int i;

    if (pipefreq > 0)
	return pipefreq;

    if (td->td_month & TDMON_DOSC) {
	td->td_command &= ~TDCMD_TE; mips_wbflush();
	td->td_month &= ~TDMON_DOSC; mips_wbflush();
	td->td_command |= TDCMD_TE; mips_wbflush();
    }

    /* enable watchdog i/u and swithch off pulse mode */
    td->td_command &= ~(TDCMD_WAM | TDCMD_PU);
    td->td_wdsecs = 0;
    mips_wbflush();

    /* run more than once to be sure that we're in the cache */
    for (i = 2, cnt = 0; i != 0; i--) {
	start = cnt;
	td->td_wdcsecs = RTC_WD_CSECS;	/* start watchdog */
	/* wait until we see the watchdog flag */
	timeout = 10000000 / RTC_HZ;
	while ((td->td_command & TDCMD_WAF) == 0)
	    if (--timeout == 0) return 0;
	cnt = mips_getcount ();
    }
    cnt -= start;

    /* work out number of counter ticks per sec */
    pipefreq = cnt * RTC_HZ;

    /* pipeline runs at 2x counter rate */
    pipefreq *= 2;

#ifdef IN_PMON
    {
	if (IS_KVA0 (_sbd_pipefreq))
	    cpuspeed = pipefreq / 1000000 / 2;
	else 
	    cpuspeed = ROMUS (1);
    }
#endif

    return pipefreq;
}


/*
 * Return the CPU bus clock frequency (usually same input clock).
 */

long
_sbd_cpufreq (void)
{
    unsigned int ratio = (r5k_getconfig() & CFG_ECMASK) >> CFG_ECSHIFT;
    long freq = _sbd_pipefreq ();

    /* standard r5x00 */
    freq /= (ratio + 2); 
    return freq;
}
