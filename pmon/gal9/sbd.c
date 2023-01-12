/* $Id: sbd.c,v 1.12 2000/10/16 18:13:56 chris Exp $ */
#include "mips.h"
#include "termio.h"
#include "pmon.h"

#include "mips/prid.h"
#include "sbd.h"
#include "z8530.h"

#include "gt64011.h"

#ifdef SBDDISPLAYPRINT
int	sbddisplayprint;
#endif

extern int z8530 ();
const ConfigEntry ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(Z8530_CHANA), 0, z8530, 256, B9600},
#ifndef DBGSBD
    {(Addr)PHYS_TO_K1(Z8530_CHANB), 1, z8530, 256, B9600},
#endif
    {0}};


/* tacky - used to share information with PCI configuration */
unsigned int	rasbase[4];
unsigned int	rassize[4];
unsigned int	raslo[4];
unsigned int	rashi[4];

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
    return "Galileo-9";
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
	raslo[i] = rlo;
	rashi[i] = rhi;

	if (rhi > rlo) {
	    rasbase[i] = rlo;
	    rassize[i] = rhi - rlo;
	    if (rhi > top)
		top = rhi;
	}
	else
	    rasbase[i] = rassize[i] = 0;
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
    return sr;
}


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}


#ifndef SLOWLED
/* display a message on the l.e.d. (four chars packed into one word) */
void
sbddisplay (unsigned long msg)
{
#ifdef SBDDISPLAYPRINT
    char buf[5];
    if (sbddisplayprint)
	printf ("Status: %c%c%c%c\n", msg>>24, msg>>16, msg>>8, msg);
#endif
}
#endif


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
    return 0;
}

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
    countfreq = getmachtype () == 4100 ? pipefreq : pipefreq/2;

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
    typedef enum {R_X, R1_1, R3_2, R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1} Ratios;
    Ratios *ratios, 
	ratios_4300[] = {R2_1, R3_1, R5_1, R6_1, R5_2, R3_1, R4_1, R3_2},
        ratios_def[] = {R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1, R_X};
	  

    switch (getmachtype ()) {
    case 4300:
	ratios = ratios_4300;
	break;
    default:
	ratios = ratios_def;
    }

    switch (ratios[(Config & CFG_ECMASK) >> CFG_ECSHIFT]) {
    case R2_1: return freq / 2;
    case R3_2: return (2 * freq) / 3;
    case R3_1: return freq / 3;
    case R4_1: return freq / 4;
    case R5_1: return freq / 5;
    case R6_1: return freq / 6;
    case R7_1: return freq / 7;
    case R8_1: return freq / 8;
    }
    return freq;
}


/* print board specific information */
void sbdmachinfo (void)
{
}
