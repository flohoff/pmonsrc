/* ctech/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include "sbd.h"

extern int ns16550();

ConfigEntry     ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {0}};



static volatile struct sbd_softc * const soft = SBD_SOFTC;

unsigned int 
bcr_bic (unsigned int bits)
{
    unsigned int obcr = soft->s_bcr;
    unsigned int nbcr = obcr & ~bits;
    unsigned int chg = nbcr ^ obcr;

    if (chg != 0) {
	volatile unsigned int *bcr = PA_TO_KVA1 (BCR_BASE);
	soft->s_bcr = nbcr;
	do {
	    if (chg & 1)
		*bcr = nbcr & 1;
	    bcr++;
	    nbcr >>= 1;
	    chg >>= 1;
	} while (chg != 0);
    }
    return obcr;
}


unsigned int 
bcr_bis (unsigned int bits)
{
    unsigned int obcr = soft->s_bcr;
    unsigned int nbcr = obcr | bits;
    unsigned int chg = nbcr ^ obcr;

    if (chg != 0) {
	volatile unsigned int *bcr = PA_TO_KVA1 (BCR_BASE);
	soft->s_bcr = nbcr;
	do {
	    if (chg & 1)
		*bcr = nbcr & 1;
	    bcr++;
	    nbcr >>= 1;
	    chg >>= 1;
	} while (chg != 0);
    }
    return obcr;
}



const char *
sbdgetname ()
{
    return "FNC-6";
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
#define MEG	(1024*1024)

    /* initialise environment from eerom */
    sbd_envinit ();

    /* size memory (look in RTC for sbdreset's computed value) */
    memorysize = soft->s_memsz;
    if (memorysize < 4 || memorysize >  160)
	memorysize = 4 * MEG;	/* minimum possible */
    else
	memorysize *= MEG;
}


sbd_dispinit ()
{
}


/* control the display blank enable */
void
sbdblank (int blankon)
{
    led_flash (!blankon);
}


void
sbddevinit ()
{
}



void
sbdpoll ()
{
}


unsigned int
sbdenable (int machtype)
{
    unsigned int sr;

    /* now is the chance to report any early eerom failures */
    sbd_envreport ();

    /* enable bus error & debug interrupts */
#if 0
    sr = SR_DEBUG | SR_IE;
#else
    sr = SR_IE;
#endif

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
#if 0
    if (cr & CAUSE_DEBUG) {
	/* software debounce debug button */
	do {
	    /* XXX how do we clear the i/u */
	    sbddelay (1000);
	} while (cr & CAUSE_DEBUG);
	return "Debug";
    }
#endif

    return 0;
}



/* display a message (four chars packed into one word),
   for very low-level code. */
void
sbddisplay (unsigned long msg, int x)
{
    char mbuf[4+1]; 
    int i;

    sbdblank (0); /* display on */
    
    /* split up into characters */
    for (i = 3; i >= 0; i--, msg >>= 8)
	mbuf[i] = msg & 0xff;
    mbuf[4] = '\0';
    led_message (mbuf);
}


/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
    int len = strlen (msg);

    sbdblank (0); /* display on */
    if (len <= 4) {
	/* plonk short message straight onto screen */
	led_message (msg);
    }
    else {
	/* scroll long message across screen */
	char mbuf[4];
	int start, i;
	for (start = -3; start <= len - 4; start++) {
	    for (i = start; i < start + 4; i++)
		mbuf[i - start] = (i < 0 || i >= len) ? ' ' : msg[i];
	    mbuf[4] = '\0';
	    led_message (mbuf);
	    sbddelay (250000);		/* 0.25 sec delay */
	}
    }
}


/* display a message and then pause so that it can be read */
static void 
display (const char *msg)
{
    sbdmessage (0, msg);
    sbddelay (1000000);		/* 1 sec delay */
}


/* display a register name and its value, then pause */
static void
xdisplay (char *s, unsigned int x)
{
    char buf[40];
    sprintf (buf, "%s=%04x.%04x", s, x >> 16, x & 0xffff);
    display (buf);
}


char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    static char excbuf[50];
    char *s;
    int i;

    if (!exc) {
	if ((cause & CAUSE_EXCMASK) == CEXC_INT && (s = sbddbgintr (cause)))
	  sprintf (excbuf, "%s interrupt", s);
        else if (cause == CEXC_CACHE) 
	  sprintf (excbuf, "Uncorrectable Cache Error");
	else
	  sprintf (excbuf, "PMON %s exception", 
		   getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }

    display  (exc);
    if (cause != CEXC_CACHE)
	xdisplay ("Cause", cause);
    xdisplay ("EPC", epc);
    xdisplay ("RA", ra);
    switch (cause & CAUSE_EXCMASK) {
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
    case CEXC_ADEL:
    case CEXC_ADES:
	xdisplay ("BadVaddr", extra);
	break;
    case CEXC_CACHE:
	xdisplay ("CacheErr", extra);
	break;
    }
    display ("");
    return exc;
}


_fpstatesz ()
{
    return 0;
}

_fpstate ()
{
    return -1;
}

cop1 ()
{
    printf ("\ncop1 called\n");
    cliexit ();
}


getmachtype ()
{
    switch ((Prid >> 8) & 0xff) {
    case 0x0b:
	return 4300;
    case 0x22:
	return 4640;
    default:
	return 0;
    }
}

#ifndef INET
/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jmp_buf *jb, int sig)
{
    if (jb) longjmp (jb, 1);
}

netopen ()	{return -1;}
netread ()	{return -1;}
netwrite ()	{return -1;}
netclose ()	{return -1;}
netlseek ()	{return -1;}
netioctl ()	{return -1;}
netpoll ()	{}
#endif


static unsigned long pipefreq = MHZ*1000000;
int cpuspeed = MHZ/2;


unsigned long sbdpipefreq (void)
{
    return pipefreq;
}

unsigned long sbdcpufreq (void)
{
    unsigned long freq = sbdpipefreq ();
    switch ((Config & CFG_ECMASK) >> CFG_ECSHIFT) {
    case 0: return freq / 2;
    case 1: return freq / 3;
    case 6: return freq;
    case 7: return (2 * freq) / 3;
    }
    return (freq);
}

/* print board specific information */
void sbdmachinfo (void)
{
}
