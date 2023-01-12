/* $Id: sbdnet.c,v 1.4 1997/09/16 21:15:20 chris Exp $ */
#include <mips.h>
#include <pmon.h>

#include "sbd.h"

#include <sys/time.h>
#include <sys/syslog.h>
#include <machine/param.h>
#define SONICDW 32
#include <drivers/if_sonic.h>

extern struct timeval time;
int cpuspeed = 15;

#ifdef R4000
#ifdef __GNUC__
#define get_count() \
({ \
  unsigned long __r; \
  __asm__ __volatile ("mfc0 %0,$9" : "=r" (__r)); \
  __r; \
})
#else
extern unsigned long get_count();
#endif

static unsigned long pipefreq;
static unsigned long clkpertick;
static unsigned long clkperusec;
static int	     clkenable;
static unsigned long _softcompare;
static int	     blinkhz;

static int
blinker (int blank)
{
    sbdblank (blank);
    timeout (blinker, !blank, blinkhz);
}


startrtclock (int hz)
{
    extern unsigned long sbdpipefreq();
    unsigned long freq = sbdpipefreq () / 2;

    /* get initial value of real time clock */
    time.tv_sec = sbd_gettime ();
    time.tv_usec = 0;

    clkpertick = freq / hz;
    clkperusec = freq / 1000000;
    _softcompare = get_count() + clkpertick;
    clkenable = 0;
    blinkhz = hz / 2;
}


enablertclock ()
{
    untimeout (blinker, 0);
    untimeout (blinker, 1);
    timeout (blinker, 0, blinkhz);
    clkenable = 1;
}


clkpoll ()
{
    register unsigned long count; 
    long cycles, ticks;

    if (!clkenable)
	return;
    
    /* poll the free-running clock */
    count = get_count ();
    cycles = count - _softcompare;

    if (cycles > 0) {

	/* as we are polling, we could have missed a number of ticks */
	ticks = (cycles / clkpertick) + 1;
	_softcompare += ticks * clkpertick;

	/* There is a race between reading count and setting compare
	 * whereby we could set compare to a value "below" the new
	 * clock value.  Check again to avoid an 80 sec. wait 
	 */
	cycles = get_count() - _softcompare;
	while (cycles > 0) {
	    _softcompare += clkpertick; ticks++;
	    cycles = get_count() - _softcompare;
	}

	hardclock (ticks);
    }
}


microtime (tv)
    struct timeval *tv;
{
    static struct timeval lasttime;
    register unsigned long count;
    long cycles;
    long t;

    *tv = time;

    /* work out how far we've progressed since the last "tick" */
    count = get_count();
    cycles = count - (_softcompare - clkpertick);

    if (cycles >= 0)
      tv->tv_usec += cycles / clkperusec;
    else
      log (LOG_INFO, "microtime: count=%u compare=%u\n", count, _softcompare);

    if (tv->tv_usec >= 1000000) {
	tv->tv_sec += tv->tv_usec / 1000000;
	tv->tv_usec %= 1000000;
    }

    if (tv->tv_sec == lasttime.tv_sec && tv->tv_usec <= lasttime.tv_usec &&
	(tv->tv_usec = lasttime.tv_usec + 1) > 1000000) {
	tv->tv_sec++;
	tv->tv_usec -= 1000000;
    }
    lasttime = *tv;
}


#define CLK_FREQ	5000000		/* clock is 5 MHz (half ethernet) */
#define	TEST_PERIOD	1000		/* 1000us test period */

/* hand tune delay */
static void 
config_delay(void)
{
    extern int cpuspeed;
    volatile struct sn_reg *snr = (struct sn_reg *) PHYS_TO_K1 (SONIC_BASE);
    unsigned long start, cnt;
    int i;

    cpuspeed = 1;

    /* stop sonic timer, initialise and start */
    snr->s_cr = CR_STP; wbflush();
    snr->s_wt0 = 0; snr->s_wt1 = 0; wbflush();
    snr->s_cr = CR_ST; wbflush();

    DELAY(TEST_PERIOD);

    /* stop sonic timer and read */
    snr->s_cr = CR_STP; wbflush();
    cnt =  0 - ((snr->s_wt1 << 16) | snr->s_wt0);
    if (cnt == 0)
	cnt = 1;

    /* work out delay factor to give 1 usec delay */
    cpuspeed = TEST_PERIOD * (CLK_FREQ / 1000000);
    cpuspeed = (cpuspeed + cnt) / cnt;

    log (LOG_DEBUG, "cpu delay = %d\n", cpuspeed);

    /* Wait for Sonic timer to reach TEST_PERIOD usecs and
     * and then examine CPU cycle count to determine clock freq. 
     * We run the whole thing twice, to make sure it is in 
     * the cache.  
     */
    for (i = 1; i <= 2; i++) {
	cnt = TEST_PERIOD * (CLK_FREQ / 1000000);
	snr->s_wt1 = (cnt >> 16); snr->s_wt0 = cnt; wbflush();
	snr->s_isr = ISR_TC; wbflush();

	snr->s_cr = CR_ST; wbflush();
	start = get_count ();
	do {
	    cnt = get_count ();
	} while (!(snr->s_isr & ISR_TC));
	cnt -= start;
	snr->s_cr = CR_STP; wbflush();
	snr->s_isr = ISR_TC; wbflush();
    }

    /* work out number of cpu clocks per usec */
    cnt = (cnt + TEST_PERIOD / 2) / TEST_PERIOD;

#if 0
    /* look for match in 5% range of known valid clocks */
    {
	static const int possfreqs[] = {33, 40, 50, 66, 75, 88, 
					100, 133, 150, 0};
	int freq;
	int lo = cnt * 95 / 100,  hi = cnt * 105 / 100;

	for (i = 0; (freq = possfreqs[i]) != 0; i++)
	    if (lo <= freq && freq <= hi)
		break;

	log (LOG_DEBUG, "clk/usec %d => %d MHz\n", cnt, freq);
	if (freq == 0)
	    freq = 50;
    }
    pipefreq = freq * 2 * 1000000;
#else
    log (LOG_DEBUG, "clk/usec = cpumhz = %d\n", cnt);
    pipefreq = 2 * cnt * 1000000;
#endif
}
#else /* R3000 */
#endif

unsigned long
sbdpipefreq ()
{
    if (pipefreq == 0)
	config_delay ();
    return pipefreq;
}


sbdcpufreq ()
{
#ifdef R3000
    return pipefreq;
#else
    unsigned int ratio = (Config & CFG_ECMASK) >> CFG_ECSHIFT;
    unsigned long freq = sbdpipefreq ();
    /* standard r4x00 */
    freq /= (ratio + 2); 
    return (freq);
#endif
}




/*
 * sbdethaddr -- get the ethernet addr
 */
int
sbdethaddr (unsigned char *enaddr)
{
    extern char *getenv();
    char *cp, *ea;
    unsigned i;

    cp = ea = getenv("ethaddr");
    if (!cp || !*cp) {
	log (LOG_ERR, "$ethaddr is not set\n");
	return (-1);
    }

    for (i = 0; i < 6; i++) {
	enaddr[i] = strtoul (cp, &cp, 16);
	if (i != 5) {
	    if (*cp != '-' && *cp != ':')
	      break;
	    cp++;
	}
    }

    if (i != 6 || *cp) {
	log (LOG_ERR, "invalid $ethaddr=%s\n", ea);
	return (-1);
    }

    return (0);
}


sbdnetinit ()
{
    config_delay();
}


sbdnetreset ()
{
}
