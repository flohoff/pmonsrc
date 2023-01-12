/* $Id: sbdenv.c,v 1.2 1996/01/16 14:25:22 chris Exp $ */
/*
 * sbdenv.c: routines to handle environment accesses
 */

#include <mips.h>
#include <pmon.h>

#ifdef SWENV

#include <swenv.h>
#include <sbd.h>

/*
 * First we have the low-level access functions
 */

static unsigned int
swenvget (unsigned int offs)
{
    volatile unsigned int *swenv = (unsigned int *)PHYS_TO_K1(SRAM_ENV);
    return (swenv[offs]);
}


static void
swenvput (unsigned int offs, unsigned int val)
{
    volatile unsigned int *swenv = (unsigned int *)PHYS_TO_K1(SRAM_ENV);
    swenv[offs] = val;
}



/*
 * Next the glue that interfaces the hardware independent code to 
 * the low-level access functions.
 *
 * Since the environment memory must be accessed a word at a time,
 * we cache the whole area here 
 */

static union {
    unsigned char	b[NVSIZE];
    unsigned short	h[NVSIZE/2];
    unsigned int	w[NVSIZE/4];
} envcopy;

static int env_laststore;

static void
nvram_load (void)
{
    int i;

    for (i = 0; i < NVSIZE / 4; i++)
	envcopy.w[i] = swenvget (NVOFFSET/4+i);
    env_laststore = -1;
}


static void
nvram_flush (int offs)
{
    if (offs > 0)
	offs /= 4;
    if (offs != env_laststore && env_laststore >= 0)
	swenvput (NVOFFSET+env_laststore, envcopy.w[env_laststore]);
    env_laststore = offs;
}


static unsigned short
nvram_getshort(int offs)
{
    return envcopy.h[offs / 2];
}


static void
nvram_setshort(int offs, unsigned short val)
{
    if (envcopy.h[offs / 2] != val) {
	nvram_flush (offs);
	envcopy.h[offs / 2] = val;
    }
}


static unsigned char
nvram_getbyte(int offs)
{
    return envcopy.b[offs];
}


static void
nvram_setbyte(int offs, unsigned char val)
{
    if (envcopy.b[offs] != val) {
	nvram_flush (offs);
	envcopy.b[offs] = val;
    }
}



/*
 * From here on down is hardware independent.
 */

static int nv_state = NVSTATE_UNKNOWN;
static int init_state;

static void
nvram_setstate (int state)
{
    nv_state = state;
    nvram_setbyte (NVOFF_STATE, state);
}

#define nvram_setrecoverable()		nvram_setstate(NVSTATE_RECOVERABLE)
#define nvram_setunrecoverable()	nvram_setstate(NVSTATE_UNRECOVERABLE)


/*
 * calculate NVRAM checksum
 */
static unsigned short
nvram_calcsum (void)
{
    unsigned short sum = NV_MAGIC;
    int i;
    
    for (i = NVOFF_ENVBASE; i < NVOFF_ENVTOP; i += 2)
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
    nvram_setstate (NVSTATE_OK);
    nvram_flush (-1);
}


/*
 * check that the nvram structure is valid
 */
static int
nvram_structcheck ()
{
    int envsize, envp, top;
    
    envsize = nvram_getshort (NVOFF_ENVSIZE);
    if (envsize > ENV_AVAIL)
	return (0);

    envp = NVOFF_ENVBASE; top = NVOFF_ENVBASE + envsize;
    while (envp < top) {
	int varsize = nvram_getbyte (envp);
	int i;
	
	if (varsize == 0 || envp + varsize > top)
	    return (0);		/* sanity test */
	
	for (i = envp + 1; i < envp + varsize; i++) {
	    unsigned char c = nvram_getbyte (i);
	    if (c == '=')
		break;		/* end of name */
	    if (c < ' ' || c >= 0x7f)
		return (0);	/* name must be printable */
	}

	envp += varsize;
    }

    return (1);
}



static void
nvram_resetenv (void)
{
    nvram_setshort (NVOFF_MAGIC, NV_MAGIC);
    nvram_setshort (NVOFF_ENVSIZE, 0);
    nvram_updatesum ();
}


/*
 * load nvram and check its validity.
 */
static int
nvram_invalid (void)
{
    if (nv_state == NVSTATE_UNKNOWN) {

	/* initialise nvram and get initial state */
	nvram_load ();
	init_state = nvram_getbyte (NVOFF_STATE);
	if (nvram_getshort (NVOFF_MAGIC) != NV_MAGIC)
	    /* bad magic: must be uninitialised */
	    init_state = NVSTATE_UNINITIALISED;
	else if (nvram_getshort (NVOFF_ENVSIZE) > ENV_AVAIL)
	    /* bad magic: must be wrong */
	    init_state = NVSTATE_UNINITIALISED;
	else if (nvram_calcsum () != nvram_getshort (NVOFF_CSUM)) {
	    if (init_state != NVSTATE_RECOVERABLE)
		init_state = NVSTATE_UNRECOVERABLE;
	}
	nv_state = init_state;
      
	/* take recovery action if necessary */
	switch (init_state) {

	default:
	    /* may be an old nvram with uninitialised state, reset to OK */
	    init_state = NVSTATE_OK;
	    /* drop through */

	case NVSTATE_RECOVERABLE:
	    /* may be a bad checksum, just recalculate it */
	    nvram_updatesum ();
	    /* drop through */

	case NVSTATE_OK:
	    /* seems ok, but do a final check of the structure */
	    if (nvram_structcheck ())
		/* ok, we pass */
		break;
	    init_state = NVSTATE_UNRECOVERABLE;
	    /* drop through */

	case NVSTATE_UNRECOVERABLE:
	case NVSTATE_UNINITIALISED:
	    /* reset to a blank environment */
	    nvram_resetenv ();
	    break;
	}
    }

    return nv_state;
}



/* 
 * return nvram address of environment string 
 */
static int
nvram_matchenv (char *name)
{
    int envsize, envp, top, namelen, varsize, i;

    envsize = nvram_getshort (NVOFF_ENVSIZE);
    if (envsize > ENV_AVAIL)
	return (0);			/* sanity */

    namelen = strlen (name);
    if (name == 0 || namelen >= 255)
	return (0);

    envp = NVOFF_ENVBASE; top = NVOFF_ENVBASE + envsize;
    while (envp < top) {

	varsize = nvram_getbyte (envp);
	if (varsize == 0 || envp + varsize > top)
	    break;		/* sanity */

	--varsize; ++envp;
	if (nvram_getbyte (envp) != '=' && varsize >= namelen) {

	    /* find length of match */
	    for (i = 0; i < namelen; i++)
		if (nvram_getbyte (envp + i) != (unsigned char)name[i])
		    break;

	    if (i == namelen &&
		(i == varsize ||			/* match on boolean */
		 nvram_getbyte (envp + i) == '='))	/* match on variable */
		return (envp - 1);	
	}

	envp += varsize;
    }

    return (0);
}



/*
 * delete nvram entry
 */
static int
nvram_delenv (char *name)
{
    int nenvp, envp, envsize, varsize, nremain;
  
    envp = nvram_matchenv (name);
    if (envp == 0)
	return 0;

    envsize = nvram_getshort (NVOFF_ENVSIZE); 
    varsize = nvram_getbyte (envp);

    /* unrecoverable because we're changing the layout */
    nvram_setunrecoverable ();

    /* splice out this entry by moving higher entries down */
    nenvp = envp + varsize;			 /* next entry */
    nremain = envsize - (nenvp - NVOFF_ENVBASE); /* # bytes after this */
    while (nremain--)
	nvram_setbyte (envp++, nvram_getbyte (nenvp++));

    nvram_setshort (NVOFF_ENVSIZE, envp - NVOFF_ENVBASE);
    nvram_updatesum ();
    return 1;
}



int
sbd_setenv (char *name, char *val)
{
    int namelen, vallen, total, envsize;
    int envp;

    if (nvram_invalid ())
	return (0);

    namelen = strlen (name);
    if (namelen == 0)
	return 0;

    total = 1 + namelen;		/* size . "name" */

    vallen = val ? strlen (val) : 0; 
    if (vallen)
	total += 1 + vallen;		/* . '=' . "value" */

    if (total > 255)			/* max variable size */
	return (0);

    envp = nvram_matchenv (name);
    if (envp && nvram_getbyte (envp) == total) {
	/* same size: replace value in place */
	if (vallen) {
	    /* recoverable because we do not move any other variables */
	    nvram_setrecoverable ();
	    envp += 1 + namelen + 1;
	    while (vallen--)
		nvram_setbyte (envp++, *val++);
	    nvram_updatesum ();
	}
	return 1;
    }

    if (envp)
	/* delete old entry */
	nvram_delenv (name);

    envsize = nvram_getshort (NVOFF_ENVSIZE);
    if (envsize + total > ENV_AVAIL)
	/* not enough room */
	return 0;

    /* recoverable because we don't update the size until we finish */
    nvram_setrecoverable ();
    envp = NVOFF_ENVBASE + envsize;
    nvram_setbyte (envp++, (unsigned char) total);
    while (namelen--)
	nvram_setbyte (envp++, *name++);
    if (vallen) {
	nvram_setbyte (envp++, '=');
	while (vallen--)
	    nvram_setbyte (envp++, *val++);
    }

    nvram_setshort (NVOFF_ENVSIZE, envp - NVOFF_ENVBASE);
    nvram_updatesum ();
    return 1;
}


char *
sbd_getenv (char *name)
{
    static char buf[256];		/* FIXME: this cannot be static */
    char *s = buf;
    int envp, namelen, varlen;

    if (nvram_invalid ())
	return ((char *)0);

    envp = nvram_matchenv (name);
    if (envp == 0)
	return ((char *)0);

    namelen = strlen (name);
    varlen = nvram_getbyte (envp); 
    if (varlen > namelen + 2) { 	/* !boolean */
	int n = varlen - (namelen + 2);
	envp += namelen + 2;
	while (n--)
	    *s++ = nvram_getbyte (envp++);
    }
    *s = '\0';
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
    int envp, top;
    char buf[256];
    
    if (nvram_invalid ())
	return;

    envp = NVOFF_ENVBASE;
    top = NVOFF_ENVBASE + nvram_getshort(NVOFF_ENVSIZE);

    while (envp < top) {
	char *s = buf; 
	char *val = 0;
	int varlen = nvram_getbyte (envp);

	if (varlen == 0 || envp + varlen > top)
	    break;		/* sanity clause */

	++envp;
	while (--varlen) {
	    char c = nvram_getbyte (envp++);
	    if ((c == '=') && !val) {
		*s++ = '\0';
		val = s;
	    }
	    else
		*s++ = c;
	}
	*s = '\0';

	(*func) (buf, val);
    }
}



sbd_envinit ()
{
    /* load and check nvram */
#if 0
    nv_state = NVSTATE_UNKNOWN;
#endif
    (void) nvram_invalid ();
}


sbd_envreport ()
{
    /* report any initial failure */
    switch (init_state) {
    case NVSTATE_OK:
	break;
    case NVSTATE_UNINITIALISED:
	printf ("ERROR: environment was uninitialised\n");
	break;
    case NVSTATE_UNRECOVERABLE:
	printf ("ERROR: environment had an unrecoverable checksum error\n");
	break;
    case NVSTATE_RECOVERABLE:
	printf ("WARNING: environment had a recoverable checksum error\n");
	break;
    default:
	printf ("WARNING: environment had bad state (%d)\n", init_state);
	break;
    }
}

#else /* !SWENV */

int
sbd_setenv(char *s, char *v)
{
    return 1;
}


char *
sbd_getenv(char *s)
{
sbdmessage ("sbd_getenv");
  return ((char *)0);
}


int
sbd_unsetenv(char *s)
{
  return 1;
}

void
sbd_mapenv (int (*func)(char *, char *))
{
    (*func) ("netaddr", "193.117.190.250");
    (*func) ("loglevel", "debug");
}

sbd_envinit ()
{
}


sbd_envreport ()
{
}

#endif /* !SWENV */
