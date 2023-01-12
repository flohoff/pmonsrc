/* $Id: mk48t02.h,v 1.2 1996/01/16 14:24:34 chris Exp $ */
/*
 * algvme/mk48t02.h -- MK48T02 Real Time Clock and Non-Volatile RAM
 */

#ifndef _MK48T02_
#define _MK48T02_

/*
 * General time definitions
 */
#define	SECMIN	((unsigned)60)			/* seconds per minute */
#define	SECHOUR	((unsigned)(60*SECMIN))		/* seconds per hour */
#define	SECDAY	((unsigned)(24*SECHOUR))	/* seconds per day */
#define	SECYR	((unsigned)(365*SECDAY))	/* sec per reg year */

#define	TD_YRREF	1970			/* start of Unix era */
#define	TD_LEAPYEAR(y) (((y) % 4) == 0 && ((y) % 100) != 0 || ((y) % 400) == 0)

/* default time if reprogrammed by ROM */
#define TDSTART	((1991-TD_YRREF)*SECYR + 166*SECDAY + 12*SECHOUR)

/* sensible time: reset if less then this */
#define TDSENSIBLE	(TDSTART + 7*SECDAY)

/*
 * To maintain endianess independence, make all accesses as 32-bit
 * words with appropriate shifting.
 */
#define TD_NVRAM_SIZE	0x7f8	

#ifdef MIPSEB
#define tdreg(x)	unsigned :32; unsigned :24; unsigned char x
#define TDREG(x)	((8*(x))+7)
#endif
#ifdef MIPSEL
#define tdreg(x)	unsigned char x; unsigned :24; unsigned :32
#define TDREG(x)	(8*(x))
#endif

/*
 * Definitions for use MK48T02/12 real time clock
 */
#ifndef __ASSEMBLER__
struct td_clock {
    struct td_mem {
	tdreg(td_value);
    } td_mem[TD_NVRAM_SIZE];	/* Non-volatile ram */
    tdreg(td_control);	/* control register */
    tdreg(td_secs);
    tdreg(td_mins);
    tdreg(td_hours);
    tdreg(td_day);
    tdreg(td_date);
    tdreg(td_month);
    tdreg(td_year);
};
#endif

/*
 * 'nvram' locations for clock registers
 */
#define TD_CONTROL	TDREG(2040)
#define TD_SECS		TDREG(2041)
#define TD_MINS		TDREG(2042)
#define	TD_HOURS	TDREG(2043)
#define TD_DAY		TDREG(2044)
#define TD_DATE		TDREG(2045)
#define TD_MONTH	TDREG(2046)
#define TD_YEAR		TDREG(2047)

/*
 * Control register bit definitions
 */
#define	TDC_WRITE	0x80	/* write lock */
#define	TDC_READ	0x40	/* read lock */
#define TDC_CALIBRATE	0x3f	/* oscillator calibration */

/*
 * Seconds register bit definitions
 */
#define	TDS_STOP	0x80	/* stop oscillator */

/*
 * Hours register bit definitions
 */
#define	TDH_KICK	0x80	/* kick oscillator */

/*
 * Day register bit definitions
 */
#define	TDD_FRQTST	0x40	/* frequency test */
#define TDD_SUNDAY	1

#define THURSDAY	4

#endif /* _MK48T02_ */

