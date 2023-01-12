/* $Id: sbd.c,v 1.6 2000/03/28 00:21:24 nigel Exp $ */
#include "mips.h"
#include "termio.h"
#include "pmon.h"

#include "mips/prid.h"
#include "sbd.h"
#include "ns16550.h"

#include "gt64011.h"

extern int ns16550 ();
const ConfigEntry ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {0}};


/* return the current CPU type */
unsigned int 
getmachtype (void)
{
    extern unsigned int mips_icache_size;
    switch (Prid >> 8) {
    case PRID_R4650:
	return 4640;
    case PRID_RM52XX:
	if (mips_icache_size == 32*1024)
	    return 5231;
	return 5230;
    default:
	return 0;
    }
}


/* return name of this board */
const char *
sbdgetname (void)
{
    return "CUDIG";
}


/* very early low-level machine initialisation */

void
sbdmachinit (void)
{
    unsigned int top = 0;
    unsigned int rbase, rlo, rhi;
    int i;

    /* memory has been configured by sbdreset */
    /* we assume that the highest mapped address is the top of memory */

    for (i = 0; i < 4; i++) {
	switch (i) {
	case 0:
	    rbase = ltohl(GT_PAS_RAS10LO);
	    rlo = ltohl(GT_DDAS_RAS0LO);
	    rhi = ltohl(GT_DDAS_RAS0HI);
	    break;
	case 1:
	    rbase = ltohl(GT_PAS_RAS10LO);
	    rlo = ltohl(GT_DDAS_RAS1LO);
	    rhi = ltohl(GT_DDAS_RAS1HI);
	    break;
	case 2:
	    rbase = ltohl(GT_PAS_RAS32LO);
	    rlo = ltohl(GT_DDAS_RAS2LO);
	    rhi = ltohl(GT_DDAS_RAS2HI);
	    break;
	case 3:
	    rbase = ltohl(GT_PAS_RAS32LO);
	    rlo = ltohl(GT_DDAS_RAS3LO);
	    rhi = ltohl(GT_DDAS_RAS3HI);
	    break;
	}
#if 1
	rbase = ((rbase & GT_PAS_LOMASK_Low) >> 7) << 28;
	rlo = rbase | ((rlo & GT_DDAS_LOMASK_Low) << 20);
	rhi = rbase | ((rhi & GT_DDAS_HIMASK_High) << 20) + (1 << 20);
#else
	rlo = ((rlo & GT_DDAS_LOMASK_Low) << 20);
	rhi = ((rhi & GT_DDAS_HIMASK_High) << 20) + (1 << 20);
#endif
	if (rhi > rlo && rhi > top)
	    top = rhi;
    }
    memorysize = top;
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit (void)
{
}


void
sbdpoll (void)
{
    /* poll any special devices */
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (unsigned int machtype)
{
    unsigned int sr = 0;

#ifdef FLASH
    /* now's a good time to tell the poor loser about his zapped flash */
    extern const char *_sbd_envinit (void);
    const char *err;
    if (err = _sbd_envinit ())
	printf ("WARNING: flash environment: %s\n", err);
#endif

    return sr;
}


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
    volatile unsigned short *d1 = (unsigned short *) PA_TO_KVA1(DISPLAY1_BASE);
    if (blankon)
	*d1 = DISPLAY1_IDLE & ~DISPLAY1_NBL;
    else
	*d1 = DISPLAY1_IDLE;
}


#if 0
/* XXX in assembler */
/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg)
{
}
#endif


static void 
display (s)
    char *s;
{
    int l  = strlen (s);
    int i, d;

    /* scroll message across display */
    for (i = -4; i < l; i++) {
	unsigned long v = 0;
	for (d = i; d < i + 4; d++) {
	    v <<= 8;
	    v |= (d < 0 || d >= l) ? ' ' : (s[d] & 0xff);
	}
	sbddisplay (v, 0);
	sbddelay (250000);
    }
}


static void
xdisplay (s, x)
    const char *s;
    unsigned int x;
{
    char buf[40];
    sprintf (buf, "%s=%04x.%04x", s, x >> 16, x & 0xffff);
    display (buf);
}


/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    return 0;
}


/* called on a fatal prom exception: display it on l.e.d, (if present) */
char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long epc, cause, ra, extra;
    char *exc;
{
    static char excbuf[50];
    char *s;
    int i;

    /* write exception to PROM, encoding data in address */
    register volatile unsigned char *prom = (void *) 0xbfdff00;
#define pByte(x) prom[(x) & 0xff] = 0
#define pWord(x) {pByte(x); pByte(x>>8); pByte(x>>16); pByte(x>>24);}

    pWord(cause);
    pWord(epc);
    pWord(extra);

    if (!exc) {
	if (cause == CEXC_CACHE) 
	  sprintf (excbuf, "Uncorrectable Cache Error");
	else
	  sprintf (excbuf, "PMON %s exception", getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }

    display  (exc);
    if (cause != EXC_CACHE)
	xdisplay ("Cause", cause);
    xdisplay ("EPC", epc);
    xdisplay ("RA", ra);
    switch (cause & CAUSE_EXCMASK) {
#ifdef CEXC_TLBL
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
#endif
    case CEXC_ADEL:
    case CEXC_ADES:
	xdisplay ("BadVaddr", extra);
	break;
    case CEXC_CACHE:
	xdisplay ("CacheErr", extra);
	break;
    }
    sbddisplay ("    ", 0);
    return exc;
}



#ifndef FLASH

/*
 * dummy nvram handler for systems without nvram.
 */

char *
sbd_getenv (name)
    char *name;
{
    return 0;
}


int
sbd_setenv (char *name, char *value)
{
    return 1;
}


int
sbd_unsetenv (char *name)
{
    return 1;
}


void
sbd_mapenv (int (*func)(char *, char *))
{
#ifdef INET
    (*func) ("bootp", "pri");
#endif
}


void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
}


int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset)
{
    printf ("This port does not support FLASH\n");
    return -1;
}

#endif



/*
 * floating point emulator stubs
 */

int
_fpstatesz (void)
{
    return 0;
}

int
_fpstate (void)
{
    return -1;
}

int
cop1 (void)
{
    printf ("\ncop1 called\n");
    cliexit ();
}


#ifndef INET
/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jb, sig)
    jmp_buf *jb;
    int sig;
{
    if (jb)
      longjmp (jb, 1);
}


netopen ()	{return -1;}
netread ()	{return -1;}
netwrite ()	{return -1;}
netclose ()	{return -1;}
netlseek ()	{return -1;}
netioctl ()	{return -1;}
netpoll ()	{}


#ifdef RTC
time_t __time (void)
{
    return (sbd_gettime ());
}
#endif
#endif

static unsigned long pipefreq;
static unsigned long countfreq;
/*
 * cpuspeed is used as a multiplier for the DELAY macro
 * it's the number of times round a 2 instruction loop to
 * give a 1us delay
 */
/* we should probably just include net/sys/sys/param.h */
#ifndef DELAY
#define	DELAY(n)	{ register int N = cpuspeed * (n); while (--N > 0); }
#endif

int cpuspeed = MHZ/2;	/* worst case until it gets configured */

static void
probefreq (void)
{
    int i;

    /* should be calculated dynamically using an external timer */
    pipefreq = MHZ * 1000000;

    /* calculate the counter frequency */
    countfreq = pipefreq/2;

    /* simple minded calibration of cpuspeed */
    for (i = 1; i < MHZ/2; i++) {
	unsigned long start, stop, delta, ohead;
	cpuspeed = i;
	start = get_count ();
	DELAY (10);
	stop = get_count ();
	ohead = get_count ();
	ohead -= stop;
	delta = stop - start - ohead;
	if (delta == 0) {
	    /* complete paranoia! */
	    cpuspeed = 1;
	    printf ("!!!countfreq %d start 0x%x stop 0x%x ohead = %d delta %d cpuspeed=%d\n",
		    countfreq, start, stop, ohead, delta, cpuspeed);
	    break;
	}
	if (countfreq / delta < 1000000) {
#if 0
	    printf ("countfreq %d start 0x%x stop 0x%x ohead = %d delta %d cpuspeed=%d\n",
		    countfreq, start, stop, ohead, delta, cpuspeed);
#endif
	    break;
	}
    }
}


unsigned long
sbdpipefreq (void)
{
    if (pipefreq == 0)
	probefreq ();
    return pipefreq;
}

unsigned long
sbdcpufreq (void)
{
    unsigned long freq = sbdpipefreq ();
    typedef struct {char mult; char div;} Ratio;
    static const Ratio	ratios[8] = {{2,1}, {3,1}, {4,1}, {5,1},
				     {6,1}, {7,1}, {8,1}, {1,1}};
    const Ratio *r = &ratios[(Config & CFG_ECMASK) >> CFG_ECSHIFT];
    return freq * r->div / r->mult;
}

/* print board specific information */
void sbdmachinfo (void)
{
}
