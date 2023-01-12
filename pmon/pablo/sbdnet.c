/* $Id: sbdnet.c,v 1.1 1996/12/10 11:58:47 nigel Exp $ */
#include "mips.h"
#include "sbd.h"

#include "sys/types.h"
#include "sys/time.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/syslog.h"

extern struct timeval time;
extern char *getenv();

static unsigned long clkpertick;
static unsigned long clkperusec;
static int	     clkenable;
static unsigned long _softcompare;
static int	     blinkhz;


startrtclock (int hz)
{
    extern unsigned long sbdcpufreq();
    unsigned long freq = sbdcpufreq ();

    /* get initial value of real time clock */
    time.tv_sec = time.tv_usec = 0;

    clkpertick = freq / hz;
    clkperusec = freq / 1000000;

    /* set timer mode */
    G10_REG (G10_CLKCONFIG) = CLK_TIMER;

    /* program counter value */
    G10_REG (G10_COUNTER) = clkpertick;

    /* start clock */
    G10_REG (G10_CLKCONFIG) = CLK_TIMER | CLK_ENABLE;

    clkenable = 0;
    blinkhz = hz / 2;
}


enablertclock ()
{
    clkenable = 1;
}


clkpoll ()
{
    if (!clkenable)
	return;
    
    if (G10_REG (G10_INTCAUSE) & INT_TIMER) {
	G10_REG (G10_INTCAUSE) = ~INT_TIMER;
	hardclock (1);
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
    cycles = clkpertick - G10_REG (G10_COUNTER);

    /* see if we've just missed an interrupt */
    if (G10_REG (G10_INTCAUSE) & INT_TIMER && cycles < clkpertick / 2)
	cycles += clkpertick;

    if (cycles >= 0)
      tv->tv_usec += cycles / clkperusec;

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
    volatile struct csr *csr = (struct csr *) PA_TO_KVA1 (CSR_BASE);
    /* Reset and then enable Ethernet chip */
    csr->csr_eth_enb = 1;
    sbddelay (200);
    csr->csr_eth_enb = 0;
    cpuspeed = CACHEUS;
}


sbdnetreset ()
{
}


#if defined(MIPSEB)+defined(MIPSEL) != 1
#error MIPSEB and MIPSEL incorrectly defined
#endif

#if FE_SYS_BUS_WIDTH == 16
/* word: offset & 1 == 0 -> D15:0
   byte: offset & 1 == 0 -> D7:0
   byte: offset & 1 == 1 -> D15:8 */
#ifdef MIPSEB
#define IOADDRW(base, offset) \
	(volatile u_short *)(base + offset*4 + 2)
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4 + (3 ^ (offset & 1)))
#else /* MIPSEL */
#define IOADDRW(base, offset) \
	(volatile u_short *)(base + offset*4)
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4 + (offset & 1))
#endif /* MIPSEL */
#else /* BUS_WIDTH == 8 */
#ifdef MIPSEB
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4 + 3)
#else /* MIPSEL */
#define IOADDRB(base, offset) \
	(volatile u_char *)(base + offset*4)
#endif /* MIPSEL */
#endif /* BUS_WIDTH == 8 */


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


#if FE_SYS_BUS_WIDTH == 16

u_short sbdfeinw(unsigned int base, int offset)
{
    u_short v = *IOADDRW (base, offset);
#ifdef MIPSEB
    /* register i/o: data in register not memory byte order */
    v = (v << 8) | (v >> 8);
#endif
    return v;
}

void sbdfeinsw(unsigned int base, int offset, u_short *addr, int len)
{
    volatile u_short *p = IOADDRW (base, offset);
    if ((unsigned)addr & 1)
	panic ("unaligned insw");
    while (len--)
	*addr++ = *p;
}

void sbdfeoutw(unsigned int base, int offset, u_short v)
{
#ifdef MIPSEB
    /* register i/o: data in register not memory byte order */
    v = (v << 8) | (v >> 8);
#endif
    *IOADDRW (base, offset) = v;
}

void sbdfeoutsw(unsigned int base, int offset, u_short *addr, int len)
{
    volatile u_short *p = IOADDRW (base, offset);
    if ((unsigned)addr & 1)
	panic ("unaligned outsw");
    while (len--)
	*p = *addr++;
}

#else

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

#endif /* FE_SYS_BUS_WIDTH == 16 */

