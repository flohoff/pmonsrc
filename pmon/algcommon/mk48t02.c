/* $Id: mk48t02.c,v 1.5 1996/03/18 15:37:13 nigel Exp $ */
/*
 * mk48t02.c: Algorithmics RTC/NVRAM support code for PMON
 */

#include <mips.h>
#include <pmon.h>
#include <mk48t02.h>
#include <nvram.h>
#include <sbd.h>

static unsigned int
bcdtobin(bcd)
unsigned int bcd;
{
  bcd >>= 24;
  return ((bcd >> 4) & 0x0f) * 10 + (bcd & 0x0f);
}

static unsigned int
bintobcd(bin)
unsigned int bin;
{
  return (((bin / 10) << 4) + bin % 10) << 24;
}


/*
 * set real time clock value
 * no checks...
 */
void
sbd_settime (time_t t)
{
  register volatile struct td_clock *td = 
    (struct td_clock *) PHYS_TO_K1 (RTCLOCK_BASE);
  struct tm *tm;

  tm = gmtime (&t);
  td->td_control |= TDC_WRITE; wbflush();
  td->td_secs = bintobcd(tm->tm_sec);
  td->td_mins = bintobcd(tm->tm_min);
  td->td_hours = bintobcd(tm->tm_hour);
  td->td_day = bintobcd(tm->tm_wday);
  td->td_date = bintobcd(tm->tm_mday);
  td->td_month = bintobcd(tm->tm_mon + 1);
  td->td_year = bintobcd(tm->tm_year % 100);
  wbflush();
  td->td_control &= ~(TDC_READ|TDC_WRITE);
}


time_t
sbd_gettime (void)
{
  register volatile struct td_clock *td = 
    (struct td_clock *) PHYS_TO_K1(RTCLOCK_BASE);
  struct tm tm;
  time_t t;

  td->td_control |= TDC_READ; wbflush();
  tm.tm_sec = bcdtobin(td->td_secs);
  tm.tm_min = bcdtobin(td->td_mins);
  tm.tm_hour = bcdtobin(td->td_hours);
  tm.tm_mday = bcdtobin(td->td_date);
  tm.tm_mon = bcdtobin(td->td_month) - 1;
  tm.tm_year = bcdtobin(td->td_year);
  td->td_control &= ~(TDC_READ|TDC_WRITE);

  /* Have we entered the next century?? */
  if (tm.tm_year < TD_YRREF % 100)
    tm.tm_year += 100;

  t = gmmktime (&tm);
  if (t == -1)
    sbd_settime (t = 0);

  return t;
}


/*
 * check rtc to make sure it is running
 */
static int
rtc_start ()
{
  register volatile struct td_clock *td = 
	  (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  unsigned int secs;

  td->td_control |= TDC_READ; wbflush ();
  secs = td->td_secs;
  td->td_control &= ~(TDC_READ|TDC_WRITE);
  if (secs & TDS_STOP) {
    /*
     * clock isn't running so start it up
     */
    td->td_control |= TDC_WRITE;
    td->td_secs = 0;
    td->td_hours = TDH_KICK;
    td->td_control &= ~(TDC_READ|TDC_WRITE);
    /* 2s delay required here while oscillator gets going */
    {
      int i;
      for (i = 0; i < 2*1000000; i++)
	continue;
    }
    td->td_control |= TDC_WRITE;
    td->td_hours = 0;
    td->td_control &= ~(TDC_READ|TDC_WRITE);
    sbd_settime (0);
  }
  /* Now it should be running */
  td->td_control |= TDC_READ; wbflush ();
  secs = td->td_secs;
  td->td_control &= ~(TDC_READ|TDC_WRITE); 
  return (0);
}



#define NVPOS(x)	(td->td_mem[x].td_value)
#define NVRD(x)		(NVPOS(x) >> 24)
#define NVWR(x,v)	NVPOS(x) = ((v)<<24)

static unsigned char
nvram_getbyte(int offs)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);

  return (NVRD(offs));
}

static void
nvram_setbyte(int offs, unsigned char val)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);

  NVWR(offs, val);
}

/*
 * BigEndian!
 */
static unsigned short
nvram_getshort(int offs)
{
  return ((nvram_getbyte(offs) << 8) | nvram_getbyte(offs+1));
}

static void
nvram_setshort(int offs, unsigned short val)
{
  nvram_setbyte (offs, (unsigned char)((val >> 8) & 0xff));
  nvram_setbyte (offs+1, (unsigned char)(val & 0xff));
}


/*
 * set nvram state flag to RECOVERABLE
 */
static void
nvram_setrecoverable ()
{
  nvram_setbyte (NVOFF_STATE, NVSTATE_RECOVERABLE);
};


/*
 * set nvram state flag to UNRECOVERABLE
 */
static void
nvram_setunrecoverable ()
{
  nvram_setbyte (NVOFF_STATE, NVSTATE_UNRECOVERABLE);
};


/*
 * calculate NVRAM checksum
 */
static unsigned short
nvram_calcsum (void)
{
  unsigned short sum = NV_MAGIC;
  int	    i;

  for (i = ENV_BASE; i < ENV_TOP; i += 2)
    sum += nvram_getshort(i);
  return (sum);
}

/*
 * update the nvram checksum
 */
static void
nvram_updatesum (void)
{
  nvram_setshort (NVOFF_CSUM, nvram_calcsum());
  nvram_setbyte (NVOFF_STATE, NVSTATE_OK);
}


/*
 * test validity of nvram by checksumming it
 */
static int
nvram_invalid (void)
{
  unsigned short sum = NV_MAGIC;

  if (nvram_getshort(NVOFF_MAGIC) != NV_MAGIC)
    return (NVSTATE_UNINITIALISED);
  sum = nvram_calcsum ();
  if (sum != nvram_getshort(NVOFF_CSUM))
    return (NVSTATE_BADCHECKSUM);
  return (nvram_getbyte (NVOFF_STATE));
}


/*
 *
 */
static int
nvram_recover ()
{
  if (nvram_getbyte (NVOFF_STATE) == NVSTATE_RECOVERABLE) {
    /* check that the structure makes vague sense */
    volatile struct td_clock *td =
	(struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
    int envsize, envp, n, i;
      
    envsize = nvram_getshort(NVOFF_ENVSIZE);
    envp = ENV_BASE;
    while (envsize > 0) {
	n = NVRD(envp);
	if (n == 0 || (envp + n) > ENV_TOP)
	    return (0);		/* sanity test */
	for (i = envp + 1; i < envp + n; i++) {
	    unsigned char c = NVRD(i);
	    if (c == '=')
		break;		/* end of name */
	    if (c < ' ' || c >= 0x7f)
		return (0);	/* name must be printable */
	}
	envsize -= n;
	envp += n;
    }

    /* ok, just update the checksum to make it valid */
    nvram_updatesum ();
    return (1);
  }
  return (0);
}


/* return nvram address of environment string */
static int
nvram_matchenv(char *s)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  int envsize, envp, n, i, varsize;
  char *var;

  envsize = nvram_getshort(NVOFF_ENVSIZE);
  if (envsize > ENV_AVAIL)
    return (0);			/* sanity */
  envp = ENV_BASE;

  if ((n = strlen (s)) > 255)
    return (0);
  while (envsize > 0) {
    varsize = NVRD(envp);
    if (varsize == 0 || (envp + varsize) > ENV_TOP)
      return (0);		/* sanity */
    if (NVRD(envp+1) == '=') {	/* empty var in the process of being deleted */
      envsize -= varsize;
      envp += varsize;
      continue;
    }
    for (i = envp+1, var = s; i <= envp+n; i++, var++)
      if (NVRD(i) != (unsigned char) *var)
	break;
    if (i > envp+n) {		/* match so far */
      if (n == varsize-1)	/* match on boolean */
	return(envp);
      if (NVRD(i) == '=')	/* exact match on variable */
	return(envp);
    }
    envsize -= varsize;
    envp += varsize;
  }
  return (0);
}

int
nvram_delenv(char *s)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  int nenvp, envp, envsize, nbytes;

  envp = nvram_matchenv (s);
  if (envp == 0)
    return 0;
  nenvp = envp + NVRD(envp);
  envsize = nvram_getshort(NVOFF_ENVSIZE); 
  nbytes = envsize - (nenvp - ENV_BASE);
  nvram_setunrecoverable ();
  nvram_setshort(NVOFF_ENVSIZE, envsize - (nenvp-envp));
  while (nbytes--) {
    NVWR (envp, NVRD(nenvp));
    envp++;
    nenvp++;
  }

  nvram_updatesum ();
  return 1;
}

static void
nvram_resetenv(void)
{
  nvram_setshort(NVOFF_MAGIC, NV_MAGIC);
  nvram_setshort(NVOFF_ENVSIZE, 0);
  nvram_updatesum();
}



int
sbd_setenv(char *s, char *v)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  int ns, nv, total;
  int envp;

  if (nvram_invalid ())
    return (0);

  ns = strlen (s);
  if (ns == 0)
    return 0;

  if (v && *v) {
    nv = strlen(v);
    total = ns + nv + 2;
  }
  else {
    nv = 0;
    total = ns + 1;
  }
  if (total > 255)
    return;

  if ((envp = nvram_matchenv (s)) && NVRD(envp) == total) {
    /* replace in place */
    if (nv) {
      nvram_setrecoverable ();
      envp += 1 + ns + 1;
      while (nv--) {
	NVWR(envp, *v); envp++; v++;
      }
      nvram_updatesum ();
    }
    return 1;
  }

  nvram_delenv (s);
  if (total > ENV_AVAIL - nvram_getshort(NVOFF_ENVSIZE))
    return 0;
  envp = ENV_BASE + nvram_getshort(NVOFF_ENVSIZE);
  nvram_setrecoverable ();
  NVWR(envp, (unsigned char) total); envp++;
  while (ns--) {
    NVWR (envp, *s); envp++; s++;
  }
  if (nv) {
    NVWR (envp, '='); envp++;
    while (nv--) {
      NVWR (envp, *v); envp++; v++;
    }
  }
  nvram_setshort (NVOFF_ENVSIZE, envp-ENV_BASE);
  nvram_updatesum ();
  return 1;
}


char *
sbd_getenv(char *s)
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  static char buf[256];		/* FIXME: this cannot be static */
  int envp, ns, nbytes, i;

  if (nvram_invalid ())
    return ((char *)0);

  envp = nvram_matchenv (s);
  if (envp == 0)
    return ((char *)0);

  ns = strlen(s);
  if (NVRD(envp) == ns+1)	/* boolean */
    buf[0] = '\0';
  else {
    nbytes = NVRD(envp)-(ns+2);
    envp += ns+2;
    for (i = 0; i < nbytes; i++)
      buf[i] = NVRD(envp++);
    buf[i] = '\0';
  }
  return (buf);
}


int
sbd_unsetenv(char *s)
{
  if (nvram_invalid ())
    return 0;
  return nvram_delenv (s);
}


/*
 * apply func to each string in environment
 */
void
sbd_mapenv (int (*func)(char *, char *))
{
  register volatile struct td_clock *td =
    (struct td_clock *)PHYS_TO_K1(RTCLOCK_BASE);
  int envsize, envp, n, i, seeneql;
  char name[256], value[256];
  char c, *s;

  if (nvram_invalid ())
    return;

  envsize = nvram_getshort(NVOFF_ENVSIZE);
  envp = ENV_BASE;

  while (envsize > 0) {
    value[0] = '\0';
    seeneql = 0;
    s = name;
    n = NVRD(envp);
    if (n == 0 || (envp + n) > ENV_TOP)
      return;		/* sanity test */
    for (i = envp + 1; i < envp + n; i++) {
      c = NVRD(i);
      if ((c == '=') && !seeneql){
	*s = '\0';
	s = value;
	seeneql = 1;
	continue;
      }
      *s++ = c;
    }
    *s = '\0';
    (*func) (name, value);
    envsize -= n;
    envp += n;
  }
}

static int env_state = NVSTATE_OK;


sbd_rtcinit ()
{
    rtc_start ();

    env_state = nvram_invalid ();
    switch (env_state) {
    default:
	/* probably an OLD nvram with garbage state, reset it to OK */
	nvram_setbyte (NVOFF_STATE, NVSTATE_OK);
	return;
    case NVSTATE_OK:
	return;
    case NVSTATE_UNINITIALISED:
	break;
    case NVSTATE_BADCHECKSUM:
    case NVSTATE_RECOVERABLE:
	if (nvram_recover ()) {
	    env_state = NVSTATE_RECOVERABLE; 
	    return;
	}
	env_state = NVSTATE_BADCHECKSUM; 
	break;
    }
    nvram_resetenv ();
}


sbd_envreport ()
{
    /* report previous NVRAM failure */
    switch (env_state) {
    case NVSTATE_OK:
	break;
    case NVSTATE_UNINITIALISED:
	printf ("ERROR: NVRAM was uninitialised\n");
	break;
    case NVSTATE_BADCHECKSUM:
	printf ("ERROR: NVRAM had an unrecoverable checksum error\n");
	break;
    case NVSTATE_RECOVERABLE:
	printf ("WARNING: NVRAM had a recoverable checksum error\n");
	break;
    default:
	printf ("WARNING: NVRAM had bad state (%d)\n", env_state);
	break;
    }
    env_state = NVSTATE_OK;
}
