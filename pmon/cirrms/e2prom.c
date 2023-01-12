/* $Id: e2prom.c,v 1.2 1996/01/16 14:24:23 chris Exp $ */

/*
 * e2prom.c: routines to handle E2PROM
 */

#include <mips.h>
#include <pmon.h>
#include <e2env.h>
#include <sbd.h>

/*
 * First of all the low-level functions for writing to E2PROM
 */

typedef void (*fnptr)();

static void
e2store (unsigned char *data, unsigned int e2offs, unsigned int size)
{
    volatile int *e2;
    unsigned int pack;
    int ndone;
    
    e2 = (int *) (E2BASE + e2offs);

    ndone = 0;

    while (ndone != size) {
#ifdef __MIPSEB__
	pack = (data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];
#else
	pack = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
#endif
	*e2++ = pack;
         data += 4;
         ndone += 4;
	
	if (((unsigned long)e2 & 511) == 0 || ndone == size) {
	    /* end of transfer, or page boundary: wait for write to complete */
	    while (e2[-1] != pack)
	      continue;
	}
    }

/* Reboot if we may have overwritten PMON, which lives in the bottom 
   128 KBytes 
*/
    if (e2offs < 0x20000) { 
	fnptr rvec = (fnptr) 0xbfc00000;
	(*rvec)();
    }
}


/* XXX this function simply marks the end of e2store() */
static void
e2end() {}


/*
 * This public function is used to overwrite and restart the PROM
 */
void
sbd_e2program (unsigned char *data, unsigned int size, unsigned int offset)
{
    char *fnbuf;
    fnptr storefn;

    printf ("Address = %x, Size = %x, Offset = %x\n",
	    data, size, offset);

    /* Four E2PROMS used, so must use word multiples */
    if (size & 3) {
	printf ("size must be a 4 byte multiple\n");
	return;
    }

#ifdef E2NVRAM
#define MAXROM (E2SIZE - ENV_TOP)
#else
#define MAXROM E2SIZE
#endif
    if ((size + offset) > MAXROM) {
	printf ("size too large (max 0x%x)\n", E2SIZE - ENV_TOP);
	return;
    }

    /* put a  copy of the e2store() function in RAM */
    fnbuf = (char *) malloc ((void *)e2end - (void *)e2store);
    if (!fnbuf) {
	printf ("cannot allocate function buffer\n");
	return;
    }
    bcopy ((void *)e2store, fnbuf, (void *)e2end - (void *)e2store);
    flush_cache (ICACHE, 0);

    /* call the RAM based function, and tell it to reboot when finished */
    printf ("Writing to E2PROM ....\n");
    storefn = (fnptr) fnbuf;
    (*storefn) (data, offset, size);
}

#ifdef E2NVRAM
/*
 * The remainder of this file stores the environment at the top of the E2PROM.
 */

static unsigned char envcopy[ENV_TOP];

#define NVPOS(x)			envcopy[x]
#define NVRD(x)				NVPOS(x)
#define NVWR(x,v)			NVPOS(x) = (v)
#define nvram_getbyte(offs)		NVRD(offs)
#define nvram_setbyte(offs, val)	NVWR(offs, val)

static void
nvram_load (void)
{
    bcopy (E2BASE + E2SIZE - ENV_TOP, envcopy, ENV_TOP);
}


static void
nvram_store (void)
{
    e2store (envcopy, E2SIZE - ENV_TOP, ENV_TOP);
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
  nvram_store ();
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
    nvram_updatesum ();
    return (1);
  }
  return (0);
}


/* return nvram address of environment string */
static int
nvram_matchenv(char *s)
{
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


sbd_envinit ()
{
    nvram_load ();
    env_state = nvram_invalid ();
    switch (env_state) {
    default:
	/* probably an OLD nvram with garbage state, reset it to OK */
	nvram_setbyte (NVOFF_STATE, NVSTATE_OK);
	nvram_store ();
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
#else /* !E2NVRAM */

int
sbd_setenv(char *s, char *v)
{
    return 1;
}


char *
sbd_getenv(char *s)
{
  return ((char *)0);
}


int
sbd_unsetenv(char *s)
{
  return 1;
}


/*
 * set our board-specific defaults
 */
void
sbd_mapenv (int (*func)(char *, char *))
{
  (*func) ("tty1", "38400");
}

sbd_envinit ()
{
}


sbd_envreport ()
{
}

#endif



