/* $Id: sbdnet.c,v 1.1 1996/06/17 15:50:40 chris Exp $ */
#include "mips.h"
#include "sbd.h"

#include "sys/types.h"
#include "sys/time.h"
#include "sys/syslog.h"
#include "machine/param.h"

extern char *getenv();

extern struct timeval time;

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
static int	     blinkticks;

time_t
sbd_gettime ()
{
    return 0;
}


static int
blinker ()
{
    static u_char ledval = 0;

    if (ledval == 0)
	ledval = 1;
    else {
	ledval <<= 1;
	if ((ledval & 0x80) == 0)
	    ledval |= 1;
    }
    sbdsetled (ledval);
    timeout (blinker, 0, blinkticks);
}


startrtclock (int hz)
{
    extern long sbd_gettime();
    extern unsigned long sbdpipefreq();
    unsigned long freq = sbdpipefreq () / 2; /* clock ticks at halfe pipeline frequency */

    /* get initial value of real time clock */
    time.tv_sec = sbd_gettime ();
    time.tv_usec = 0;

    clkpertick = freq / hz;
    clkperusec = freq / 1000000;
    _softcompare = get_count() + clkpertick;
    clkenable = 0;
    blinkticks = hz / 8;
}


enablertclock ()
{
    if (getenv ("ledripple")) {
	untimeout (blinker, 0);
	timeout (blinker, 0, blinkticks);
    }
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


#ifdef MIPSEL
/* The register mapping is a bit unusual :-) */
#define REGOFFSET(reg)	(((reg) & 1) ? (reg)*4+0x40 : (reg)*4+4)
#endif

#define FEREAD(addr)		sbd_ioread8 (addr)
#define FEWRITE(addr,val)	sbd_iowrite8 ((addr), (val))

void sbdfeoutb (unsigned int base, int reg, u_char v)
{
    volatile u_char *p;
    p = (u_char *)(base + REGOFFSET(reg));
    FEWRITE(p, v);
}

void sbdfeoutsb(unsigned int base, int reg, u_char *addr, int len)
{
    volatile u_char *p;
    unsigned char *bp;

    p = (u_char *)(base + REGOFFSET(reg));
    bp = (unsigned char *) addr;
    while (len--) {
	FEWRITE(p, *bp);
	bp++;
    }
}

u_char sbdfeinb(unsigned int base, int reg)
{
    volatile u_char *p;
    p = (u_char *)(base + REGOFFSET(reg));
    return FEREAD(p);
}

void sbdfeinsb(unsigned int base, int reg, u_char *addr, int len)
{
    volatile u_char *p;
    unsigned char *bp;

    p = (u_char *)(base + REGOFFSET(reg));
    bp = (unsigned char *) addr;
    while (len--)
	*bp++ = FEREAD(p);
}


/* Assume this is only used to input 16bit values in littlendian order */
u_short sbdfeinw(unsigned int base, int reg)
{
    volatile u_char *p;
    u_short v;

    /* assert reg == FE_BMPR8 */
    p = (u_char *)(base + REGOFFSET(reg));
    v =  FEREAD(p);
    v |= FEREAD(p) << 8;
    return (v);
}

void sbdfeinsw(unsigned int base, int reg, u_short *addr, int len)
{
    /* assert reg == FE_BMPR8 */
    volatile u_char *p;
    unsigned char *bp;

    p = (u_char *)(base + REGOFFSET(reg));
    bp = (unsigned char *) addr;
    len *= 2;
    while (len--)
	*bp++ = FEREAD(p);
}

/* Assume this is only used to output 16bit values in littlendian order */
void sbdfeoutw(unsigned int base, int reg, u_short v)
{
    volatile u_char *p;

    /* assert reg == FE_BMPR8 */
    p = (u_char *)(base + REGOFFSET(reg));
    FEWRITE(p, v);
    v >>= 8;
    FEWRITE(p, v);
}

void sbdfeoutsw(unsigned int base, int reg, u_short *addr, int len)
{
    /* assert reg == FE_BMPR8 */
    volatile u_char *p;
    unsigned char *bp;

    p = (u_char *)(base + REGOFFSET(reg));
    bp = (unsigned char *) addr;
    len *= 2;
    while (len--) {
	FEWRITE(p, *bp);
	bp++;
    }
}
