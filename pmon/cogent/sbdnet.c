/* $Id: sbdnet.c,v 1.6 2000/10/16 18:13:54 chris Exp $ */

#include <mips.h>
#include <pmon.h>
#include <termio.h>

#include <sys/time.h>
#include <sys/syslog.h>
#include <machine/param.h>

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

static unsigned long cpufreq;
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
    /* get initial value of real time clock */
    time.tv_sec = sbd_gettime ();
    time.tv_usec = 0;

    clkpertick = cpufreq / hz;
    clkperusec = cpufreq / 1000000;
    _softcompare = get_count() + clkpertick;
    clkenable = 0;
    blinkhz = hz / 4;
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

#define CLK_FREQ	XXX		/* known clock */
#define	TEST_PERIOD	1000		/* 1000us test period */

/* hand tune delay */
static void 
config_delay(void)
{
#if 0
    extern int cpuspeed;
    unsigned long start, cnt;
    int i;

    cpuspeed = 1;

    /* stop timer, initialise and start */
    (void) timer_stop ();
    timer_set (0);
    timer_start ();

    DELAY(TEST_PERIOD);

    /* stop timer and read */
    cnt = timer_stop ();
    if (cnt == 0)
	cnt = 1;

    /* work out delay factor to give 1 usec delay */
    cpuspeed = TEST_PERIOD * (CLK_FREQ / 1000000);
    cpuspeed = (cpuspeed + cnt) / cnt;

    log (LOG_DEBUG, "cpu delay = %d\n", cpuspeed);

    /* Wait for timer to reach TEST_PERIOD usecs and
     * and then examine CPU cycle count to determine clock freq. 
     * We run the whole thing twice, to make sure it is in 
     * the cache.  
     */
    for (i = 1; i <= 2; i++) {
	cnt = TEST_PERIOD * (CLK_FREQ / 1000000);
	timer_set (cnt);
	timer_start ();

	start = timer_get ();
	do {
	    cnt = timer_get ();
	} while (!timer_done ());
	cnt -= start;
	timer_stop ();
    }

    /* work out number of cpu clocks per usec */
    cnt = (cnt + TEST_PERIOD / 2) / TEST_PERIOD;

#if 0
    /* look for match in 5% range of known valid clocks */
    {
	static const int possfreqs[] = {33, 40, 50, 66, 75, 88, 
					100, 133, 150, 0};
	int lo = cnt * 95 / 100,  hi = cnt * 105 / 100;

	for (i = 0; (cpufreq = possfreqs[i]) != 0; i++)
	    if (lo <= cpufreq && cpufreq <= hi)
		break;

	log (LOG_DEBUG, "clk/usec %d => %d MHz\n", cnt, cpufreq);
	if (cpufreq == 0)
	    cpufreq = 50;
    }
    cpufreq *= 1000000;
#else
    log (LOG_DEBUG, "clk/usec = cpumhz = %d\n", cnt);
    cpufreq = cnt * 1000000;
#endif

#else
    cpufreq = MHZ * 1000000;
#endif

}
#else /* R3000 */
#endif

sbdcpufreq ()
{
    return cpufreq;
}


sbdpipefreq ()
{
    unsigned long freq;
    unsigned int ratio = (Config & CFG_ECMASK) >> CFG_ECSHIFT;
    typedef enum {R_X, R1_1, R3_2, R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1} Ratios;
    Ratios *ratios, 
	ratios_4300[] = {R2_1, R3_1, R5_1, R6_1, R5_2, R3_1, R4_1, R3_2},
        ratios_def[] = {R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1, R_X};
	  
    freq = sbdcpufreq ();

    switch (getmachtype ()) {
    case 4300:
	ratios = ratios_4300;
	break;
    default:
	ratios = ratios_def;
    }

    switch (ratios[ratio]) {
    case R2_1: return freq * 2;
    case R3_2: return (3 * freq) / 2;
    case R3_1: return freq * 3;
    case R4_1: return freq * 4;
    case R5_1: return freq * 5;
    case R6_1: return freq * 6;
    case R7_1: return freq * 7;
    case R8_1: return freq * 8;
    }
    return freq;
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
    config_delay ();
}


sbdnetreset ()
{
}

void sbdfeoutb (unsigned int base, int offset, unsigned char v)
{
    volatile unsigned char *p = 0;
#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    *p = v;
}

void sbdfeoutsb(unsigned int base, int offset, unsigned char *addr, int len)
{
    volatile unsigned char *p = 0;
    unsigned char *bp;

#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    bp = (unsigned char *) addr;
    while (len--)
	*p = *bp++;
}

unsigned char sbdfeinb(unsigned int base, int offset)
{
    volatile unsigned char *p = 0;
#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    return (*p);
}

void sbdfeinsb(unsigned int base, int offset, unsigned char *addr, int len)
{
    volatile unsigned char *p = 0;
    unsigned char *bp;

#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    bp = (unsigned char *) addr;
    while (len--)
	*bp++ = *p;
}


/* Assume this is only used to input 16bit values in littlendian order */
unsigned short sbdfeinw(unsigned int base, int offset)
{
    volatile unsigned char *p = 0;
    unsigned short v;

    /* assert offset == FE_BMPR8 */
#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    v = *p;
    v |= *p << 8;
    return (v);
}

void sbdfeinsw(unsigned int base, int offset, unsigned short *addr, int len)
{
    /* assert offset == FE_BMPR8 */
    volatile unsigned char *p = 0;
    unsigned char *bp;

#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    bp = (unsigned char *) addr;
    len *= 2;
    while (len--)
	*bp++ = *p;
}

/* Assume this is only used to output 16bit values in littlendian order */
void sbdfeoutw(unsigned int base, int offset, unsigned short v)
{
    volatile unsigned char *p = 0;

    /* assert offset == FE_BMPR8 */
#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    *p = v;
    *p = v >> 8;
}

void sbdfeoutsw(unsigned int base, int offset, unsigned short *addr, int len)
{
    /* assert offset == FE_BMPR8 */
    volatile unsigned char *p = 0;
    unsigned char *bp;

#ifdef MIPSEB
    p = (unsigned char *)(base + offset*8 + 7);
#endif
#ifdef MIPSEL
    p = (unsigned char *)(base + offset*8);
#endif
    bp = (unsigned char *) addr;
    len *= 2;
    while (len--)
	*p = *bp++;
}

