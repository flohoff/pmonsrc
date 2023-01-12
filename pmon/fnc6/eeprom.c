/*
 * eeprom.c: routines to handle serial EEPROM on P4032
 */

#include <mips.h>
#include <pmon.h>

#include "sbd.h"
#include "eeprom.h"

/*
 * First we have the low-level EEROM access functions
 */

/* 
 * AT93C66 has 512 bytes, organised as 256x16-bits => 8 address bits 
 */
#define ABITS		8
#define EESIZE		(2 << ABITS)	/* size in bytes */

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
 * EEROM accessed thru Centronics/ECP port
 */
#include "ecp.h"

#define E2_CS		DCR_STROBE
#define E2_CS_HIGH	0

#define E2_SK		DCR_SELECTIN
#define E2_SK_HIGH	DCR_SELECTIN

#define E2_DI		DCR_AUTOFD
#define E2_DI_HIGH	0

#define E2_DO		DSR_NACK
#define E2_DO_HIGH	DSR_NACK

/*
 * Chip requires at least 1 usec delay between signal changes
 */
#define WAIT()	sbddelay(1)

#define TOGGLE(bit,hi,on) \
do { \
	volatile ecpdev *ecp = PA_TO_KVA1 (ECP_BASE); \
	WAIT (); \
	if (((on) != 0) ^ ((hi) == 0)) ecp->dcr |= (bit); \
	else ecp->dcr &= ~(bit); \
} while (0);


/* chip select */
static inline void CS (int on)	{TOGGLE (E2_CS, E2_CS_HIGH, on)}

/* shift clock */
static inline void SK (int on)	{TOGGLE (E2_SK, E2_SK_HIGH, on)}

/* my data out, chip's data in */
static inline void DOUT (int on) {TOGGLE (E2_DI, E2_DI_HIGH, on)}

/* my data in, chip's data out */
static inline unsigned int DIN (void)	
{ 
    volatile ecpdev *ecp = PA_TO_KVA1 (ECP_BASE);
    WAIT (); 
    return ((ecp->dsr & E2_DO) == E2_DO_HIGH) ? 1 : 0;
}

/* clock pulse */
static inline void CLK (void)	
{
    SK (1); SK (0);
}


/*
 * send command
 */
static void CMD (unsigned int c)
{
    int n = CBITS;

    SK(0); DOUT(0); CS (1); CLK ();

    while (n-- > 0) {
	DOUT ((c >> n) & 1); CLK ();
    }
    DOUT (0);
}


static void ECMD (void)
{
    int i;

    DOUT (0); CS (0);

    /* do we really need 5 clocks, probably zero or one would do! */
    for (i = 1; i <= 5; i++)
	CLK ();
}


static void
eereset (void)
{
    int i;

    DOUT (0); CS (0);
    /* clock through enough zero bits to finish any command */
    for (i = 1; i <= CBITS + 16 + 5; i++)
	CLK ();
    ECMD();
}


static unsigned short
eeget (unsigned int offs)
{
    unsigned short v = 0;
    int n;

    CMD (READ (offs));
    n = 16;
    while (n-- > 0) {
	CLK ();
	v = (v << 1) | DIN ();
    }
    ECMD ();
    return v;
}


static void
eeput (unsigned int offs, unsigned short val)
{
    int n;

    /* enable writes */
    CMD (WRTENB); ECMD ();

    /* write command + data */
    CMD (WRITE (offs));
    n = 16;
    while (n-- > 0) {
	DOUT ((val >> n) & 1);
	CLK ();
    }
    ECMD ();

    /* raise CS and poll until DIN is active */
    CS (1); CLK ();
    n = 100;	/* 100ms timeout */
    do {
	sbddelay (1000);	/* 1ms */
	CLK ();
    } while (!DIN () && n-- > 0);
    ECMD ();

    /* disable writes */
    CMD (WRTDIS); ECMD ();
}


/*
 * Next the glue that interfaces the hardware independent code to 
 * the low-level access functions.
 *
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

#if #endian(little)
#define htol(v) (v)
#define ltoh(v) (v)
#endif
#if #endian(big)
#define htol(v) ({ unsigned short _v = v; ((_v & 0xff) << 8) | ((_v >> 8) & 0xff); })
#define ltoh(v) htol(v)
#endif

static int
nvram_hwtest (void)
{
    unsigned short x;
    int ok = 1;

    eeput (NVOFF_TEST/2, 0xaaaa);
    if ((x = eeget (NVOFF_TEST/2)) != 0xaaaa) {
	printf ("EEPROM failure @0x%x, aaaa->%04x\n", NVOFF_TEST/2, x);
	ok = 0;
    }

    eeput (NVOFF_TEST/2, 0x5555);
    if ((x = eeget (NVOFF_TEST/2)) != 0x5555) {
	printf ("EEPROM failure @0x%x, 5555->%04x\n", NVOFF_TEST/2, x);
	ok = 0;
    }

    return ok;
}


static void
nvram_load (void)
{
    int i;
    
    eereset ();
    for (i = 0; i < NVSIZE / 2; i++)
	envcopy.w[i] = htol(eeget (NVOFFSET / 2 + i));
    env_laststore = -1;
}


static void
nvram_flush (int offs)
{
    if (offs > 0)
	offs /= 2;
    if (offs != env_laststore && env_laststore >= 0)
	eeput (NVOFFSET / 2 + env_laststore, ltoh(envcopy.w[env_laststore]));
    env_laststore = offs;
}


static unsigned short
nvram_getshort(int offs)
{
    return ltoh(envcopy.w[offs/2]);
}


static void
nvram_setshort(int offs, unsigned short val)
{
    if (ltoh(envcopy.w[offs/2]) != val) {
	nvram_flush (offs);
	envcopy.w[offs/2] = htol(val);
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
    if (init_state == NVSTATE_OK)
	return;

    /* report any initial failure */
    switch (init_state) {
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

    if (!nvram_hwtest ())
	printf ("WARNING: NVRAM h/w failure\n");
}
