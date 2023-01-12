/* 
 * TEMIC/sbdclock.c: timer handling for Algor/Temic module
 * Copyright (c) 1999	Algorithmics Ltd
 *
 * Sadly the R5000 on-chip timer interrupt is disabled by default,
 * so we have to use the timer on the Nile4.  This is not nice,
 * because every time we reprogram the timer we lose time.
 */


#ifdef TEMIC_CPUTIMER

#include "../share/r4kclock.c"

#else 

#include <stddef.h>
#include <sys/time.h>
#include <sys/clock.h>
#include <mips/cpu.h>
#include <mips/xcpt.h>
#include <mips/limits.h>
#include <assert.h>
#include "kit_impl.h"

#include "sbd.h"
#include "vrc5074.h"

/* The interrupt function to call, and the interval (in microseconds) */
static clock_t		(*tickfunc)();
static clock_t		tick;

static clock_t		maxtick;
static unsigned long	cycpertick;
static unsigned long	cycperusec;

static struct intraction oldiact;

clock_t			_clk_tck;

static volatile struct vrc5074 * const n4 = PA_TO_KVA1 (VRC5074_BASE);

void
_sbd_settimer (clock_t newtick)
{
    assert (newtick <= maxtick);
    if (newtick != tick) {
	/* Program the timer to interrupt every NEWTICK microseconds, */
	cycpertick = (tick = newtick) * cycperusec - 1;
	/* restart clock (XXX accuracy is lost) */
	n4->n4_t2ctrl = 0;
	n4->n4_t2cntr = cycpertick;
	n4->n4_t2ctrl = N4_TnCTRL_EN | cycpertick;
    }
}


static int 
clkintr (int arg, struct xcptcontext *xcp)
{
    clock_t newtick;

    if (n4->n4_intstat1 & N4_INTSTAT1_IL5 (N4_INT_GPT)) {

	/* acknowledge gpt interrupt */
	n4->n4_intclr = N4_INTCLR_DEV (N4_INT_GPT);

	/* Call the generic timer code's tick function, passing
	   the number of interrupt times that have elapsed since
	   the last call (usually 1). */
	newtick = (*tickfunc) (1, xcp);
	
	/* You may be asked to vary the interrupt rate now. */
	if (newtick != tick)
	    _sbd_settimer (newtick);
    }

    /* other devices interrupts may be routed to interrupt level 5 */
    if (oldiact.ia_handler != XCPT_DFL)
	return (*oldiact.ia_handler) (oldiact.ia_arg, xcp);

    return 0;
}


clock_t
_sbd_gettimer ()
{
    /* Return how many microseconds have passed since last interrupt,
       as accurately as possible. */
    unsigned long cycles;

    /* get timer */
    cycles = n4->n4_t2cntr;

    /* convert downcount to upcount */
    cycles = (cycpertick - cycles);

    /* check for possible missed interrupt */
    if (cycles < cycpertick / 2 
	&& (n4->n4_intstat1 & N4_INTSTAT1_IL5 (N4_INT_GPT)))
      cycles += cycpertick;

    return cycles / cycperusec;
}


clock_t
_sbd_inittimer (clock_t (*func)())
{
    struct intraction ia;

    /* Initialise the timer, and arrange for it to call FUNC on
       every interrupt.  Install the interrupt handler and
       enable the interrupt. */
    
    if (!_clk_tck) {
	/* get CPU sys clock frequency */
	_clk_tck = _sbd_cpufreq ();
	if (_clk_tck < 1000000)
	    return 0;
    }

    /* work out timing parameters based on that frequency */
    cycperusec = _clk_tck / 1000000;
    maxtick = LONG_MAX / cycperusec;
    tickfunc = func;

    /* stop general purpose timer #2 (started in settimer) */
    n4->n4_t2ctrl = 0;

    /* clear gpt interrupt */
    n4->n4_intclr = N4_INTCLR_DEV (N4_INT_GPT);

    /* connect gpt interrupt to CPU Int5 */
    n4->n4_intctrl &= ~N4_INTCTRL_MASK (N4_INT_GPT);
    n4->n4_intctrl |= (N4_INTCTRL_EN (N4_INT_GPT) 
		       | N4_INTCTRL_PRI (N4_INT_GPT, 5));

    /* enable Int5 output driver */
    n4->n4_intstat1 |= N4_INTSTAT1_OE (5);

    /* install interrupt handler */
    ia.ia_handler = clkintr;
    ia.ia_arg = 0;
    ia.ia_ipl = 8;
    intraction (7, &ia, &oldiact);

    return maxtick;
}

#endif /* !TEMIC_CPUTIMER */
