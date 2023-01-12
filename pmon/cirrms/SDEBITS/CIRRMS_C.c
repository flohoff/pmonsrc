/* $Id: CIRRMS_C.c,v 1.2 1996/01/16 14:24:29 chris Exp $ */
/* 
 * CIRRMS_C.c: C support functions for CIR's RMS board
 * Copyright (c) 1994	Algorithmics Ltd
 */

#include <stddef.h>
#include <time.h>
#include <errno.h>

#include <mips/xcpt.h>
#include <mips/cpu.h>

#include <idt/r3081.h>

/*
 * On the CIR board we could try to use one of the M82510 timer channels
 * to do timing, but I can't be bothered at this stage.
 */

#ifdef notdef

/* 
 * Define HZ to be timer input clock frequency.
 */
#define HZ		25000000 			/* clock = 25Mhz */

/*
 * Define COUNTER as the repeat rate of the timer interrupt as
 * multiples of the above.  Try to compute a value that will give
 * be an exact result when passed to TICKS_TO_USECS below.
 */
#define COUNTER		500000				/* 20ms */

/* convert number of counter ticks to microseconds */
#if HZ > 1000000
/* assumes HZ is a multiple of 1000000 */
#define	TICKS_TO_USECS(t)	((t) / (HZ / 1000000))
#else
/* assumes HZ is a factor of 1000000 */
#define	TICKS_TO_USECS(t)	((t) * (1000000 / HZ))
#endif


/* used by sys/clock.h header file */
clock_t _clk_tck = HZ;

/* advanced at each timer interrupt */
static clock_t clk_base;


static int
timerintr (int arg, struct xcptcontext *xcp)
{
    /* acknowledge the timer interrupt */

    /* advance current time */
    clk_base += TICKS_TO_USECS (COUNTER);

    return 0;
}


static void
timerstart (void)
{
    /* set initial counter value and repeat rate */

    /* enable counter and interrupt */
    
    /* install interrupt handler */
    intrupt (INTRNO, timerintr, 0);
    spl (0);
}


static unsigned long
timerread (void)
{
    unsigned long timer;

    /* get timer */
    timer = xx;

    /* work out ticks since last interrupt */
    /* e.g. convert downcount to upcount */
    timer = (COUNTER - timer);

    /* check for possible missed interrupt */
    if (timer < COUNTER / 2 && intrpending (INTRNO))
      timer += COUNTER;

    return timer;
}


clock_t clock (void)
{
    static int first = 1;
    clock_t result;
    int s;

    if (first) {
	timerstart ();
	first = 0;
	return 0;
    }

    s = spl (8);
    result = clk_base + TICKS_TO_USECS (timerread ());
    (void) splx (s);

    return result;
}
#endif


clock_t clock (void)
{
    static int clk = 0;

    return clk++;
}



time_t
time (time_t *tp)
{
    /* default is unknown time */
    errno = ENOENT;
    return -1;
}
