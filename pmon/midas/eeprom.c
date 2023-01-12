/* $Id: eeprom.c,v 1.1 1996/06/17 15:50:35 chris Exp $ */
/*
 * eeprom.c: routines to handle serial EEPROM on Midas
 */

#include <mips.h>
#include <pmon.h>

#ifdef EENVRAM

#include <sbd.h>
#include <eeprom.h>

/*
 * First we have the low-level EEPROM access functions
 */

/* 
 * AT93C66 configured as 512x8 -> 9 address bits 
 */
#define ABITS		9
#define DBITS		8

#define EESIZE		512	/* size in bytes */

/* 93Cxx command layout */
#define CBITS		(ABITS + 3)
#define AMASK		((1 << ABITS) - 1)
#define SB		(1 << (ABITS + 2))
#define COMD(x)		((x) << (ABITS - 2))

/* 93Cxx command codes */
#define	WRTDIS		(SB | COMD(0x0))
#define	WRITEALL	(SB | COMD(0x1))
#define	ERASEALL	(SB | COMD(0x2))
#define	WRTENB		(SB | COMD(0x3))
#define WRITE(a)	(SB | COMD(0x4) | ((a) & AMASK))
#define READ(a)		(SB | COMD(0x8) | ((a) & AMASK))
#define ERASE(a)	(SB | COMD(0xc) | ((a) & AMASK))

/*
 * Chip requires at least 1 usec delay between signal changes
 */
#define WAIT()	sbddelay(1)

static unsigned int e2ctl;

void
e2_bis (unsigned int bits)
{
    e2ctl |= bits;
    sbd_iowrite32 ((volatile unsigned int *)PA_TO_KVA1(EEPROM_BASE), e2ctl);
}

void
e2_bic (unsigned int bits)
{
    e2ctl &= ~bits;
    sbd_iowrite32 ((volatile unsigned int *)PA_TO_KVA1(EEPROM_BASE), e2ctl);
}

unsigned int
e2_get (void)
{
    return sbd_ioread32((volatile unsigned int *)PA_TO_KVA1(EEPROM_BASE));
}

#define TOGGLE(bit,on) \
do { \
	WAIT (); if (on) e2_bis (bit); else e2_bic (bit); \
} while (0);


/* chip select */
static	void CS (int on)	{TOGGLE (EEPROM_CS, on)}

/* shift clock */
static	void SK (int on)	{TOGGLE (EEPROM_SK, on)}

/* my data out, chip's data in */
static	void DOUT (int on)	{TOGGLE (EEPROM_DIN, on)}

/* my data in, chip's data out */
static unsigned int DIN (void)	
{ 
    WAIT (); 
    return (e2_get () & EEPROM_DOUT) ? 1 : 0;
}

/* clock pulse */
static	void CLK (void)	
{
    SK (1); SK (0);
}


/*
 * send command
 */
static void CMD (unsigned int c)
{
    int n = CBITS;

    SK(0); DOUT(0);

    CS (1); CLK ();
    while (n-- > 0) {
	DOUT ((c >> n) & 1); CLK ();
    }
    DOUT (0);
}


static void ECMD (void)
{
    int i;

    DOUT (0);
    CS (0);

    /* do we really need 5 clocks, probably zero or one would do! */
    for (i = 1; i <= 5; i++)
	CLK ();
}


static unsigned int
eeget (unsigned int offs)
{
    unsigned int v = 0;
    int n;

    CMD (READ (offs));
    n = DBITS;
    while (n-- > 0) {
	CLK ();
	v = (v << 1) | DIN ();
    }
    ECMD ();
    return v;
}


static void
eeput (unsigned int offs, unsigned int val)
{
    int n;

    /* enable writes */
    CMD (WRTENB); ECMD ();

    /* write command + data */
    CMD (WRITE (offs));
    n = DBITS;
    while (n-- > 0) {
	DOUT ((val >> n) & 1);
	CLK ();
    }
    ECMD ();

    /* raise CS and poll until DIN is active */
    CS (1); CLK ();
    do {
	CLK ();
    } while (!DIN ());
    ECMD ();

    /* disable writes */
    CMD (WRTDIS); ECMD ();
}



/*
 * Next the glue that interfaces the hardware independent code to 
 * the low-level access functions.
 */

/*
 * Since reading the EEPROM is very slow, we keep a cached copy of the 
 * whole prom. Since the prom is always accessed in 16-bits words, we
 * buffer stores and only flush out a 16-bit word when a new word is 
 * written to.
 */

static union {
    unsigned char	b[NVSIZE];
    unsigned short	w[NVSIZE/2];
} envcopy;

static int env_laststore;

static void
nvram_load (void)
{
    int i;
#if DBITS == 8
    for (i = 0; i < NVSIZE; i++)
	envcopy.b[i] = eeget (NVOFFSET + i);
#endif
#if DBITS == 16
    for (i = 0; i < NVSIZE / 2; i++)
	envcopy.w[i] = eeget (NVOFFSET / 2 + i);
#endif
    env_laststore = -1;
}


static void
nvram_flush (int offs)
{
#if DBITS == 8
    if (offs != env_laststore && env_laststore >= 0)
	eeput (NVOFFSET + env_laststore, envcopy.b[env_laststore]);
#endif
#if DBITS == 16
    if (offs > 0)
	offs /= 2;
    if (offs != env_laststore && env_laststore >= 0)
	eeput (NVOFFSET / 2 + env_laststore, envcopy.w[env_laststore]);
#endif
    env_laststore = offs;
}

static unsigned short
nvram_getshort(int offs)
{
    return envcopy.w[offs / 2];
}


static void
nvram_setshort(int offs, unsigned short val)
{
#if DBITS == 8
    if (envcopy.w[offs / 2] != val) {
	nvram_flush (offs);
	envcopy.w[offs / 2] = val;
	nvram_flush (offs+1);
    }
#endif
#if DBITS == 16
    if (envcopy.w[offs / 2] != val) {
	nvram_flush (offs);
	envcopy.w[offs / 2] = val;
    }
#endif
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
	printf ("ERROR: EEPROM was uninitialised\n");
	break;
    case NVSTATE_UNRECOVERABLE:
	printf ("ERROR: EEPROM had an unrecoverable checksum error\n");
	break;
    case NVSTATE_RECOVERABLE:
	printf ("WARNING: EEPROM had a recoverable checksum error\n");
	break;
    default:
	printf ("WARNING: EEPROM had bad state (%d)\n", init_state);
	break;
    }
}

#else /* !EENVRAM */

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

void
sbd_mapenv (int (*func)(char *, char *))
{
    (*func) ("hostport","tty2"); /* download via centronics i/f */
    (*func) ("dlproto",	"none"); /* no download protocol */
    (*func) ("dbgport",	"tty1"); /* debug via rs232 #2 */
}

sbd_envinit ()
{
}


sbd_envreport ()
{
}

#endif
