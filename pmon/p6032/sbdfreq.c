/* 
 * p6032/sbdfreq.c: CPU frequency determination for P-6032 board
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
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

#include "rtc.h"
#ifndef RTC_HZ
#define RTC_HZ		16
#define RTC_RATE	RTC_RATE_16Hz
#endif

#ifdef IN_PMON
#define _sbd_pipefreq	sbdpipefreq
#define _sbd_cpufreq	sbdcpufreq
#define mips_getprid()	Prid
#define mips_getconfig() Config
#define mips_getcount() get_count()
int cpuspeed = MHZ/2;	/* worst case until it gets configured */
#endif

long _sbd_pipefreq (void);

/*
 * Bus clock to pipeline clock ratio tables
 */

struct ratio {char mult; char div;};
static const struct ratio ratios_norml[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					     {6,1}, {7,1}, {8,1}, {1,1}};
static const struct ratio ratios_qedint[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					     {6,1}, {7,1}, {8,1}, {9,1}};
static const struct ratio ratios_qedhalfint[8] = {{1,1}, {1,1}, {1,1},{5,2},
					     {1,1}, {7,2}, {1,1}, {9,2}};
static const struct ratio ratios_r4300[8] = {{2,1}, {3,1}, {5,1}, {6,1},
					     {5,2}, {3,1}, {4,1}, {3,2}};
static const struct ratio ratios_r54xx[8] = {{2,1}, {5,2}, {3,1}, {4,1},
					     {5,1}, {6,1}, {7,1}, {1,1}};
static const struct ratio ratios_r55xx[8] = {{2,1}, {5,2}, {3,1}, {7,2},
					     {4,1}, {9,2}, {5,1}, {11,2}};

/*
 * Return the CPU system clock frequency, determined by reading
 * the ICS clock synthesiser chip.
 */

long
_sbd_cpufreq (void)
{
    static long cpufreq;

    if (cpufreq == 0) {
#if defined(P60XX_SET_FREQ)
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
#elif defined(P60XX_GET_FREQ)
	/* FIXME we cannot actually read the power-on frequency */
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
	const struct ratio *ratios;
	switch (mips_getprid() >> 8) {
	case PRID_RM52XX:
	case PRID_RM7000:
	    /* using 1/2 rate multipliers? */
	    ratios = ratios_qedhalfint;
	    if (sbd_boardtype () == 32 &&
		(sbd_switches () & /* FIXME */ 2))
		break;
	    if (sbd_boardtype () == 64 &&
		(((sbd_switches () & CPLD_CLKMULT) >> CPLD_CLKMULT_SHIFT) & 1))
		break;
	    ratios = ratios_qedint;
	    break;
	case PRID_R4300:
	    ratios = ratios_r4300;
	    break;
	case PRID_R5400:
	    ratios = ratios_r54xx;
	    break;
	case PRID_R5500:
	    ratios = ratios_r55xx;
	    break;
	default:
	    ratios = ratios_norml;
	    break;
	}
	ratios += (mips_getconfig() & CFG_ECMASK) >> CFG_ECSHIFT;
	cpufreq = _sbd_pipefreq () * ratios->div / ratios->mult;
#endif
    }
    return cpufreq;
}


#if !(defined(P60XX_SET_FREQ) || defined(P60XX_GET_FREQ))

/* hand tune delay */
static int
probefreq(void)
{
    int pipefreq;
    unsigned long start, cnt, timeout;
    unsigned int osta, ostb;
    int i;

#ifdef IN_PMON
    SBD_DISPLAY ("FREQ", CHKPNT_FREQ);
#endif

    /* set RTC periodic interrupt frequency */
    osta = _rtc_bic (RTC_STATUSA, RTC_RATE_MASK);
    _rtc_bis (RTC_STATUSA, RTC_RATE);

    /* enable periodic interrupts */
    ostb = _rtc_bic (RTC_STATUSB, RTCSB_AIE | RTCSB_UIE);
    _rtc_bis (RTC_STATUSB, RTCSB_PIE);

    /* select interrupt register */
    outb (RTC_ADDR_PORT, RTC_INTR);

    /* run more than once to be sure that we're in the cache */
    for (i = 3; i != 0; i--) {
	start = mips_getcount ();
	/* wait until we see an RTC i/u */
	timeout = 10000000 / RTC_HZ;
	while ((inb (RTC_DATA_PORT) & RTCIR_INTF) == 0)
	    if (--timeout == 0) goto done;
	cnt = mips_getcount ();
    }
    cnt -= start;
#if 0
printf ("cnt = %d, start = %d, delta=%d\n", cnt, start, cnt-start);
#endif
    /* restore old rate and interrupt enables */
    _rtc_set (RTC_STATUSA, osta);
    _rtc_set (RTC_STATUSB, ostb);

    /* work out number of cpu ticks per sec, the cpu pipeline */
    pipefreq = cnt * RTC_HZ;

    /* All R5000 processors clock the count register at 1/2 of PClock */
    pipefreq *= 2;

    /* round frequency to 3 decimal places */
    pipefreq += 500;
    pipefreq -= pipefreq % 1000;

    /* how many times round a 2 instruction loop gives 1us delay */
 done:
#ifdef IN_PMON
    if (IS_KVA0 (&probefreq))
	cpuspeed = (pipefreq/2) / 1000000;
    else
	cpuspeed = 1;
#endif

    if (pipefreq < 10000000)	/* 10MHz (!) */
      pipefreq = 133333333;	/* default value */

    return (pipefreq);
}
#endif

/*
 * Determine CPU pipeline clock frequency as a multiplier from
 * the CPU system clock.
 */

long
_sbd_pipefreq (void)
{
    static long pipefreq;

#if defined(P60XX_SET_FREQ) || defined(P60XX_GET_FREQ)
    const struct ratio *ratios;

    if (pipefreq == 0) {
	switch (mips_getprid() >> 8) {
	case PRID_RM52XX:
	case PRID_RM7000:
	    /* using 1/2 rate multipliers? */
	    ratios = ratios_qedhalfint;
	    if (sbd_boardtype () == 32 &&
		(sbd_switches () & /* FIXME */ 2))
		break;
	    if (sbd_boardtype () == 64 &&
		(((sbd_switches () & CPLD_CLKMULT) >> CPLD_CLKMULT_SHIFT) & 1))
		break;
	    ratios = ratios_qedint;
	    break;
	case PRID_R4300:
	    ratios = ratios_r4300;
	    break;
	case PRID_R5400:
	    ratios = ratios_r54xx;
	    break;
	case PRID_R5500:
	    ratios = ratios_r55xx;
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
#else
    if (pipefreq == 0)
	pipefreq = probefreq();
#endif

    return pipefreq;
}
