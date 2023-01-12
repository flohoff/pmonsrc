/* 
 * TEMIC/sbdfreq.c: CPU frequency determination for Algor/Temic module
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
#include "vrc5074.h"
#include "ics9148.h"

#ifdef IN_PMON
# define _sbd_pipefreq	sbdpipefreq
# define _sbd_cpufreq	sbdcpufreq
# define mips_getprid()	Prid
# define mips_getconfig() Config
  int cpuspeed = MHZ/2;	/* worst case until it gets configured */
#else
# if #cpu(r4000)
#  define mips_getconfig() r4k_getconfig()
# else
#  define mips_getconfig() r5k_getconfig()
# endif
#endif

/*
 * Bus clock to pipeline clock ratio tables
 */

struct ratio {char mult; char div;};
static const struct ratio ratios_norm[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					    {6,1}, {7,1}, {8,1}, {1,1}};

/*
 * Return the CPU system clock frequency, determined by reading
 * the ICS clock synthesiser chip.
 */

long
_sbd_cpufreq (void)
{
    static long cpufreq;

    if (cpufreq == 0) {
	struct ics9148reg ics;
	memset (&ics, 0, sizeof(ics));
	if (_sbd_icsread (&ics, sizeof (ics)) < 1) {
	    cpufreq = 200000000; /* default to ridiculous value */
	}
	else switch (ics.freq & ICS_FREQ_MASK) {
	case ICS_FREQ_100_333_666:
	    cpufreq = 100000000;
	    break;
	case ICS_FREQ_95_318_635:
	    cpufreq = 95250000;
	    break;
	case ICS_FREQ_83_333_666:
	    cpufreq = 83300000;
	    break;
	case ICS_FREQ_75_300_600:
	case ICS_FREQ_75_375_750:
	    cpufreq = 75000000;
	    break;
	case ICS_FREQ_69_343_685:
	    cpufreq = 68500000;
	    break;
	case ICS_FREQ_67_334_668:
	    cpufreq = 66800000;
	    break;
	case ICS_FREQ_60_300_600:
	    cpufreq = 60000000;
	    break;
	}
    }
    return cpufreq;
}


/*
 * Determine CPU pipeline clock frequency as a multiplier from
 * the CPU system clock.
 */

long
_sbd_pipefreq (void)
{
    unsigned int ec = (mips_getconfig() & CFG_ECMASK) >> CFG_ECSHIFT;
    const struct ratio *ratio;
    static long pipefreq;

    if (pipefreq == 0) {
	ratio = &ratios_norm[ec];
	pipefreq = _sbd_cpufreq () * ratio->mult / ratio->div;
#ifdef IN_PMON	
	cpuspeed = IS_KVA0(_sbd_pipefreq) ? pipefreq/2000000 : ROMUS(1);
#endif
    }

    return pipefreq;
}
