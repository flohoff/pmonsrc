/*
 * eeprom.c: routines to handle serial EEPROM on UBI Pablo
 */

#include <mips.h>
#include <pmon.h>

#ifdef EENVRAM

#include <sbd.h>
#include <eeprom.h>

/*
 * First we have the low-level EEROM access functions
 */

/* 
 * 24C04 has 512x8-bits -> 9 address bits 
 */
#define ABITS		9
#define EESIZE		(1 << ABITS)	/* size in bytes */

#define I2LADDR		(0x04<<1)
#define PGADDR(o)	((o) >> 7) & (((0x1 << ABITS-8) - 1) << 1)
#define WADDR(o)	((o) & 0xff)
#define READ		0x01
#define WRITE		0x00

/*
 * Max chip frequency = 100 kHz, so requires at least 5 usec
 * delay between signal changes.
 */
#define WAIT()	sbddelay(5)

/* clock */
static inline void SCL (int on)
{
    WAIT();
    *(volatile unsigned char *)PA_TO_KVA1 (CSR_I2L_SCL) = on;
}


/* drive SDA with my data bit */
static inline void SDOUT (int on)
{    
    WAIT();
    if (on)
	G10_REG (G10_PIOOUT) |= G10_PIO_EE_D;
    else
	G10_REG (G10_PIOOUT) &= ~G10_PIO_EE_D;
    /* drive the output */
    G10_REG (G10_PIOCONFIG) &= ~G10_PIO_EE_D;
}


/* switch SDA to be an input */
static inline void SDIN (void)	
{
    /* set input mode */
    G10_REG (G10_PIOCONFIG) |= G10_PIO_EE_D;
}


/* read SDA input */
static inline unsigned int SDRD (void)	
{ 
    WAIT();
    return (G10_REG (G10_PIOIN) & G10_PIO_EE_D) ? 1 : 0;
}

/* send a START: SDA high->low when SCL high */
static	void START ()
{
    SDOUT (1);
    SCL (1);
    SDOUT (0);
    SCL (0);
    SDIN ();
}


/* send a STOP: SDA low->high when SCL high */
static	void STOP ()
{ 
    SDOUT (0);
    SCL (1);
    SDOUT (1);
    SCL (0);
    SDIN ();
}


/* receive an ACK: a single 0 bit */
static int GETACK ()
{
    int ack;
    SCL (1);
    ack = SDRD ();
    SCL (0);
    return ack == 0;
}


/* send an ACK: a single 0 bit */
static void SENDACK ()
{
    SDOUT (0);
    SCL (1);
    SCL (0);
    SDIN ();
}


/* send a NACK: a single 1 bit */
static int SENDNACK ()
{
    SDOUT (1);
    SCL (1);
    SCL (0);
    SDIN ();
}


/* send 8 bit word (note: check for ACK externally) */
static void SEND8 (int data)
{
    int i;
    for (i = 7; i >= 0; i--) {
	SDOUT (data & (1 << i));
	SCL (1);
	SCL (0);
    }
    SDIN ();
}


/* receive 8 bit word (note: send ACK/NACK externally) */
static unsigned int GET8 (void)
{
    unsigned int v = 0;
    int i;
    for (i = 7; i >= 0; i--) {
	SCL (1);
	v |= SDRD() << i;
	SCL (0);
    }
    return v;
}


/* send first command byte: device address */
static int DADDR (unsigned int a, unsigned int rw)
{
    START ();
    SEND8 (0xa0 | I2LADDR | PGADDR (a) | rw);
    return GETACK ();
}


/* send command bytes: device address + word address */
static int ADDR (unsigned int a, unsigned int rw)
{
    /* wait for previous write to complete */
    int timeout = 1000;

    /* send first byte: device address */
    while (!DADDR (a, rw)) {
	if (--timeout == 0)
	    return 0;
    }

    /* send second byte: word address */
    SEND8 (WADDR (a));
    return GETACK ();
}


/* reset from unknown state */
static void eereset (void)
{
    int i;
    /* float SDA */
    SDIN ();
    /* 9 clocks to end any input byte and send NACK */
    for (i = 9; i != 0; i--) {
	SCL (1);
	SCL (0);
    }
    /* STOP bit to end command */
    STOP ();
}


/* do a Sequential Read of NB bytes from OFFS into BUF */
static void
eeload (unsigned char *buf, unsigned int offs, unsigned int nb)
{
    if (!ADDR (offs, WRITE))    /* dummy write */
	return;
    DADDR (offs, READ);		/* start read */
    while (nb-- != 0) {
	*buf++ = GET8 ();
	if (nb == 0)
	    SENDNACK ();
	else
	    SENDACK ();
    }
    STOP();
}


/* do a single Byte Write of VAL to OFFS */
static void
eeputbyte (unsigned int offs, unsigned char val)
{
    if (!ADDR (offs, WRITE))
	return;
    SEND8 (val);
    GETACK ();
    STOP ();
}



/*
 * Next the glue that interfaces the hardware independent code to 
 * the low-level access functions.
 *
 * Since reading the EEPROM is very slow, we keep a cached copy of the 
 * whole prom.
 */

static unsigned char envcopy[NVSIZE];

static void nvram_reset (void)
{
    eereset ();
}


static void
nvram_load (void)
{
    eeload (envcopy, NVOFFSET, NVSIZE);
}

static void
nvram_flush (int offs)
{
}


static void
nvram_setbyte(int offs, unsigned char val)
{
    if (envcopy[offs] != val) {
	envcopy[offs] = val;
	eeputbyte (NVOFFSET+offs, val);
    }
}

static void
nvram_setshort(int offs, unsigned short val)
{
    nvram_setbyte (offs, val >> 8);
    nvram_setbyte (offs+1, val & 0xff);
}

static inline unsigned char
nvram_getbyte(int offs)
{
    return envcopy[offs];
}

static inline unsigned short
nvram_getshort(int offs)
{
    return (envcopy[offs] << 8) | envcopy[offs + 1];
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
    nvram_reset ();
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
    (*func) ("dbgport",	"udp"); /* debug via rs232 #2 */
}

sbd_envinit ()
{
}


sbd_envreport ()
{
}

#endif
