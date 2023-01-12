/* $Id: mk48t02.h,v 1.2 1996/01/16 14:24:05 chris Exp $ */
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

/*
 * Definitions for use MK48T02/12 real time clock
 */
#ifndef __ASSEMBLER__
struct td_clock {
  struct td_mem {
    unsigned int	td_value;
  } td_mem[TD_NVRAM_SIZE];	/* Non-volatile ram */
  unsigned int	td_control;	/* control register */
  unsigned int	td_secs;
  unsigned int	td_mins;
  unsigned int	td_hours;
  unsigned int	td_day;
  unsigned int	td_date;
  unsigned int	td_month;
  unsigned int	td_year;
};
#endif

/*
 * 'nvram' locations for clock registers
 */
#define TD_CONTROL	0x7f8
#define TD_SECS		0x7f9
#define TD_MINS		0x7fa
#define	TD_HOURS	0x7fb
#define TD_DAY		0x7fc
#define TD_DATE		0x7fd
#define TD_MONTH	0x7fe
#define TD_YEAR		0x7ff

/*
 * Control register bit definitions
 */
#define	TDC_WRITE	0x80000000 /* write lock */
#define	TDC_READ	0x40000000 /* read lock */
#define TDC_CALIBRATE	0x3f000000 /* oscillator calibration */

/*
 * Seconds register bit definitions
 */
#define	TDS_STOP	0x80000000 /* stop oscillator */

/*
 * Hours register bit definitions
 */
#define	TDH_KICK	0x80000000 /* kick oscillator */

/*
 * Day register bit definitions
 */
#define	TDD_FRQTST	0x40000000 /* frequency test */
#define TDD_SUNDAY	1

#define THURSDAY	4

#endif /* _MK48T02_ */
