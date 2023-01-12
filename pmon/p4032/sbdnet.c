/* $Id: sbdnet.c,v 1.7 1998/09/11 16:44:00 nigel Exp $ */
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

/* NetBSD style autoconf data */
#include "pci/pcivar.h"
#include "pci/device.h"

extern struct cfdriver decd;

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

struct cfdata _pci_cfdata[] = {
	/* driver     unit state    loc     flags parents ivstubs */
/*  0: de0 */
	{&decd,	 	0, NORM,    0,      0, 	  0, 	  0},
	{0}
};



static int
blinker (int blank)
{
    sbdblank (blank);
    timeout (blinker, !blank, blinkhz);
}


startrtclock (int hz)
{
    extern long sbd_gettime();
    extern unsigned long sbdpipefreq();
    unsigned long freq;

    freq = sbdpipefreq ();
    if (getmachtype () == 4100)
	freq /= 4;
    else
	freq /= 2;

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


/* "ethernet" probe and attach */
eninit ()
{
    extern int _pmon_in_ram;
    int s = splhigh ();
    int i;
    
    for (i = 0; _pci_cfdata[i].cf_driver; i++)
	_pci_cfdata[i].cf_fstate = FSTATE_NOTFOUND;
    _pci_configure (!_pmon_in_ram);
    (void) splx (s);
}
