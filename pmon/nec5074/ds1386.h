/*
 * ds1386.h -- Dallas 1386  Real Time Clock and Non-Volatile RAM
 * Copyright (c) 1999	Algorithmics Ltd
 */

#ifndef __DS1386_H__
#define __DS1386_H__

#define TD_MEM_SIZE	(32768 - 14)	/* actual memory size */
#define TD_NVRAM_SIZE	0x0f00		/* as per Risq PMON */

#ifndef __ASSEMBLER__
struct td_clock {
  unsigned char	td_csecs;	/* 0: bcd seconds/100 */
  unsigned char	td_secs;	/* 1: bcd seconds */
  unsigned char	td_mins;	/* 2: bcd minutes */
  unsigned char	td_minalrm;	/* 3: bcd minutes */
  unsigned char	td_hours;	/* 4: bcd hours */
  unsigned char	td_houralarm;	/* 5: bcd hours */
  unsigned char	td_day;		/* 6: bcd day (sunday = 1) */
  unsigned char	td_dayalarm;	/* 7: bcd day (sunday = 1) */
  unsigned char	td_date;	/* 8: bcd date (1-31) */
  unsigned char	td_month;	/* 9: bcd month (1-12) */
  unsigned char	td_year;	/* a: bcd year (0-99) */
  unsigned char	td_command;	/* b: command register */
  unsigned char	td_wdcsecs;	/* c: bcd watchdog secs/100 */
  unsigned char	td_wdsecs;	/* d: bcd watchdog secs */
  unsigned char	td_mem[TD_MEM_SIZE];	/* e: non-volatile ram */
};
#endif /* !__ASSEMBLER__ */

/*
 * 'nvram' locations for clock registers
 */
#define	TD_CSECS	0x0
#define	TD_SECS		0x1
#define	TD_MINS		0x2
#define	TD_MINALRM	0x3
#define	TD_HOURS	0x4
#define	TD_HOURALARM	0x5
#define	TD_DAY		0x6
#define	TD_DAYALARM	0x7
#define	TD_DATE		0x8
#define	TD_MONTH	0x9
#define	TD_YEAR		0xa
#define	TD_COMMAND	0xb
#define	TD_WDCSECS	0xc
#define	TD_WDSECS	0xd
#define	TD_MEMX(x)	(0xe + (x))

/*
 * Command register bit definitions
 */
#define TDCMD_TDF	0x01	/* time of day alarm flag */
#define TDCMD_WAF	0x02	/* watchdog alarm flag */
#define TDCMD_TDM	0x04	/* time of day alarm mask (disable) */
#define TDCMD_WAM	0x08	/* watchdog alarm mask (disable) */
#define TDCMD_PU	0x10	/* pulse interrupt */
#define TDCMD_IBH	0x20	/* intb source/sink */
#define TDCMD_IPSW	0x40	/* interrupt switch */
#define TDCMD_TE	0x80	/* transfer enable */

#define TDMON_DOSC	0x80	/* disable oscillator */
#define TDMON_DSQW	0x40	/* disable square wave */

/* Generic alarm register mask bit */
#define	TDALARM_MASK	0x80

/* Hour register bit definitions */
#define	TDHOUR_12	0x40
#define	TDHOUR_24	0x00
#define	TDHOUR_PM	0x20

#endif /* __DS1386_H__ */
