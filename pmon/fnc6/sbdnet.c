/* $Id: sbdnet.c,v 1.1 1997/02/20 15:02:19 nigel Exp $ */
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
    time.tv_sec = 0;
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


/*
 * sbdethaddr -- get the ethernet addr
 */
int
sbdethaddr (unsigned char *enaddr)
{
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
}


sbdnetreset ()
{
}


#if defined(MIPSEB)+defined(MIPSEL) != 1
#error MIPSEB and MIPSEL incorrectly defined
#endif

#ifdef MIPSEB
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4 + 3)
#else /* MIPSEL */
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4)
#endif /* MIPSEL */


void sbdfeoutb (unsigned int base, int offset, u_char v)
{
    *IOADDRB (base, offset) = v;
}

void sbdfeoutsb(unsigned int base, int offset, u_char *addr, int len)
{
    volatile u_char *p = IOADDRB (base, offset);
    while (len--)
	*p = *addr++;
}

u_char sbdfeinb(unsigned int base, int offset)
{
    return *IOADDRB (base, offset);
}

void sbdfeinsb(unsigned int base, int offset, u_char *addr, int len)
{
    volatile u_char *p = IOADDRB (base, offset);
    while (len--)
	*addr++ = *p;
}

u_short sbdfeinw(unsigned int base, int offset)
{
    volatile u_char *p = IOADDRB (base, offset);
    u_short v;
    v = *p;			/* low */
    v |= *p << 8;		/* high */
    return v;
}

void sbdfeoutw(unsigned int base, int offset, u_short v)
{
    volatile u_char *p = IOADDRB (base, offset);
    *p = v;			/* low */
    *p = v >> 8;		/* high */
}
