#ifdef INET

#include "mips.h"
#include "sbd.h"

#include "sys/types.h"
#include "sys/time.h"
#include "sys/syslog.h"
#include "machine/param.h"

extern struct timeval time;
extern char *getenv();

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

static unsigned long clkpertick;
static unsigned long clkperusec;
static int	     clkenable;
static unsigned long _softcompare;
static int	     blinkhz;


static int
blinker (int unused)
{
    static int blink;
    *(volatile unsigned char *) PA_TO_KVA1 (LEDWR_BASE) = blink++;
    timeout (blinker, 0, blinkhz);
}


startrtclock (int hz)
{
    extern unsigned long sbdpipefreq();
    unsigned long freq;

    /* on Vr41xx the counter runs at the bus clock not
       the pipeline clock. */
    freq = sbdcpufreq ();

    /* get initial value of real time clock */
#ifdef RTC
    time.tv_sec = sbd_gettime ();
#else
    time.tv_sec = 0;
#endif
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


sbdnetinit ()
{
}


sbdnetreset ()
{
}



#define ISA_ADDRW(port) \
	(volatile u_short *)PA_TO_KVA1(ISA_IO_BASE + port)

#define ISA_ADDRB(port) \
	(volatile u_char *)PA_TO_KVA1(ISA_IO_BASE + port)


void isa_outb (unsigned int port, u_char v)
{
    *ISA_ADDRB (port) = v;
    mips_wbflush ();
}

void isa_outsb(unsigned int port, void *addr, int len)
{
    volatile u_char *p = ISA_ADDRB (port);
    u_char *b = addr;
    while (len--)
	*p = *b++;
}

unsigned int isa_inb(unsigned int port)
{
    return *ISA_ADDRB (port);
}

void isa_insb(unsigned int port, void *addr, int len)
{
    volatile u_char *p = ISA_ADDRB (port);
    u_char *b = addr;
    while (len--)
	*b++ = *p;
}

unsigned int isa_inw(unsigned int port)
{
    u_short v = *ISA_ADDRW (port);
    return v;
}

void isa_insw(unsigned int port, void *addr, int len)
{
    volatile u_short *p = ISA_ADDRW (port);
    u_short *w = addr;
    if ((unsigned)addr & 1)
	panic ("unaligned insw");
    while (len--)
	*w++ = *p;
}

void isa_outw(unsigned int port, u_short v)
{
    *ISA_ADDRW (port) = v;
    mips_wbflush ();
}

void isa_outsw(unsigned int port, void *addr, int len)
{
    volatile u_short *p = ISA_ADDRW (port);
    unsigned short *w = addr;
    if ((unsigned)addr & 1)
	panic ("unaligned outsw");
    while (len--)
	*p = *w++;
}
#endif
