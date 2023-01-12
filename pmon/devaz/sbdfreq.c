/* 
 * devaz/sbdfreq.c: CPU frequency determination for DEVA-0 board
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifdef IN_PMON
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif
#include <mips/prid.h>
#include "sbd.h"
#include "ics9148.h"

#ifdef IN_PMON
#define _sbd_pipefreq	sbdpipefreq
#define _sbd_cpufreq	sbdcpufreq
#define mips_getprid()	Prid
#define mips_getconfig() Config
int cpuspeed = MHZ/2;	/* worst case until it gets configured */
#else
#if #cpu(r4000)
#define mips_getconfig() r4k_getconfig()
#else
#define mips_getconfig() r5k_getconfig()
#endif
#endif

/*
 * Bus clock to pipeline clock ratio tables
 */

struct ratio {char mult; char div;};
static const struct ratio ratios_norml[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					     {6,1}, {7,1}, {8,1}, {1,1}};
static const struct ratio ratios_r4300[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					     {6,1}, {7,1}, {1,1}, {3,2}};
static const struct ratio ratios_r54xx[8] = {{2,1}, {5,2}, {3,1}, {4,1},
					     {5,1}, {6,1}, {7,1}, {1,1}};

/*
 * Return the CPU system clock frequency, determined by reading
 * the ICS clock synthesiser chip.
 */

long
_sbd_cpufreq (void)
{
    static long cpufreq;

    if (cpufreq == 0) {
#if defined(DEVAZ_SET_FREQ)
	struct ics9148reg ics;
	memset (&ics, 0xff, sizeof(ics));
#if SYSCLK_MHZ == 100	
	ics.freq = ICS_FREQ_100_333_666 | ICS_FREQ_USEREG;
	cpufreq = 100000000;
#elif SYSCLK_MHZ == 95
	ics.freq = ICS_FREQ_95_318_635 | ICS_FREQ_USEREG;
	cpufreq = 95250000;
#elif SYSCLK_MHZ == 83
	ics.freq = ICS_FREQ_83_333_666 | ICS_FREQ_USEREG;
	cpufreq = 83300000;
#elsif SYSCLK_MHZ == 75
	ics.freq = ICS_FREQ_75_300_600 | ICS_FREQ_USEREG;
	cpufreq = 75000000;
#elif SYSCLK_MHZ == 68 || SYSCLK_MHZ == 69
	ics.freq = ICS_FREQ_69_343_685 | ICS_FREQ_USEREG;
	cpufreq = 68500000;
#elif SYSCLK_MHZ == 66 || SYSCLK_MHZ == 67
	ics.freq = ICS_FREQ_67_334_668 | ICS_FREQ_USEREG;
	cpufreq = 66800000;
#elif SYSCLK_MHZ == 60
	ics.freq = ICS_FREQ_60_300_600 | ICS_FREQ_USEREG;
	cpufreq = 60000000;
#else
# error Unknown value for SYSCLK_MHZ
#endif
	if (_sbd_icswrite (&ics, sizeof (ics)) != sizeof(ics))
	    cpufreq = 200000000;
#elif defined(DEVA_GET_FREQ)
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
#else
	char * const _bonito = PA_TO_KVA1 (BONITO_BASE);
	/* guess the maximum likely frequency for the version of Bonito */
	if (BONITO_PCICLASS & 0x80)
	    cpufreq = 68500000;		/* FPGA: max = 68.5 MHz */
	else 
	    cpufreq = 100000000;	/* ASIC: max = 100 MHz */
	
#endif
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
    const struct ratio *ratios;
    static long pipefreq;

    if (pipefreq == 0) {
	switch (mips_getprid() >> 8) {
	case PRID_R4300:
	    ratios = ratios_r4300;
	    break;
	case PRID_R5400:
	    ratios = ratios_r54xx;
	    break;
	default:
	    ratios = ratios_norml;
	    break;
	}
	ratios += (mips_getconfig() & CFG_ECMASK) >> CFG_ECSHIFT;
	pipefreq = _sbd_cpufreq () * ratios->mult / ratios->div;
#ifdef IN_PMON	
	{
	    /*  1 microsend delay loop counter */
	    char * const _bonito = PA_TO_KVA1 (BONITO_BASE);
	    cpuspeed = IS_KVA0(_sbd_pipefreq) ? pipefreq/2000000 
		: ((BONITO_BONPONCFG & BONITO_BONPONCFG_ROMBOOT)
		   == BONITO_BONPONCFG_ROMBOOT_SDRAM) ? RAMUS(1)
		: ROMUS(1);
	}
#endif
    }

    return pipefreq;
}
