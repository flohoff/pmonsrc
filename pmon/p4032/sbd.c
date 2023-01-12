/* $Id: sbd.c,v 1.25 2000/10/20 00:15:58 chris Exp $ */
/* p4032/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <mips/prid.h>
#include <p4032/sbd.h>
#include <p4032/z80pio.h>
#include <p4032/rtc.h>
#include <p4032/v96xpbc.h>

#ifdef REV_A
#include "icujtag.h"
#endif

#define RTC_HZ		16
#define RTC_RATE	RTC_RATE_16Hz

#define MAXDISPLAYCOLS	16

extern int ns16550();
extern int centronics();

ConfigEntry     ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(ECP_BASE), 0, centronics, 512, B9600},
    {0}};

extern void display_init (void);
extern void display_size (int *, int *);
extern void display_message (int, const char *);

static int displaycols;
static int displayrows;

unsigned int 
bcr_bic (unsigned int bits)
{
    unsigned int obcr = rtc_bic (RTC_BCR, bits);
    unsigned int nbcr = obcr & ~bits;
    unsigned int chg = nbcr ^ obcr;

    if (chg != 0) {
	volatile unsigned int *bcr = PA_TO_KVA1 (BCR_BASE);
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
    unsigned int obcr = rtc_bis (RTC_BCR, bits);
    unsigned int nbcr = obcr | bits;
    unsigned int chg = nbcr ^ obcr;

    if (chg != 0) {
	volatile unsigned int *bcr = PA_TO_KVA1 (BCR_BASE);
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
imask_get (void)
{
    return rtc_get (RTC_IMASK);
}


unsigned int 
imask_set (unsigned int nmask)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    unsigned int omask = rtc_set (RTC_IMASK, nmask);
    icu->ctrl.mask = nmask;
    return omask;
}


unsigned int 
imask_bic (unsigned int bits)
{
    unsigned int omask = rtc_bic (RTC_IMASK, bits);
    unsigned int nmask = omask & ~bits;

    if (nmask != omask) {
	volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
	icu->ctrl.mask = nmask;
    }
    return omask;
}


unsigned int 
imask_bis (unsigned int bits)
{
    unsigned int omask = rtc_bis (RTC_IMASK, bits);
    unsigned int nmask = omask | bits;

    if (nmask != omask) {
	volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
	icu->ctrl.mask = nmask;
    }
    return omask;
}



unsigned int 
zpiob_get (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int val = zpio->b_dat;
    /* two uncached reads to give pio time to get off the i/o bus */
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    return val;
}


unsigned int 
zpiob_bic (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int old = zpio->b_dat;
    /* two uncached reads to give pio time to get off the i/o bus */
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    zpio->b_dat = old & ~bits;
    return old;
}


unsigned int 
zpiob_bis (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int old = zpio->b_dat;
    /* two uncached reads to give pio time to get off the i/o bus */
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    (void) *(volatile unsigned char *) PA_TO_KVA1 (0);
    zpio->b_dat = old | bits;
    return old;
}



const char *
sbdgetname ()
{
    return "P4032";
}


#ifdef REV_A
static void
sbd_icuload()
{
    const unsigned char *bp;
    unsigned char b;
    int i;

    zpiob_bic (ZPIOB_FPGA_TCK);
    for (bp = icujtag; bp < &icujtag[sizeof(icujtag)]; bp++) {
	for (b = *bp, i = 4; i != 0; b >>= 2, i--) {
	    zpiob_bic (ZPIOB_FPGA_DMASK);
	    zpiob_bis ((b << ZPIOB_FPGA_DSHFT) & ZPIOB_FPGA_DMASK);
	    zpiob_bis (ZPIOB_FPGA_TCK);
	    zpiob_bic (ZPIOB_FPGA_TCK);
	}
    }
    
    /* clock 8 lots of zero through to be safe */
    zpiob_bic (ZPIOB_FPGA_DMASK);
    for (i = 8; i != 0; i--) {
	zpiob_bis (ZPIOB_FPGA_TCK);
	zpiob_bic (ZPIOB_FPGA_TCK);
    }
}
#endif

/* early low-level machine initialisation */
void
sbdmachinit ()
{
#define MEG	(1024*1024)

    /* initialise display (again) */
    sbd_dispinit ();

#ifdef REV_A
    /* load ICU FPGA */
    sbd_icuload ();
#endif

    /* initialise environment from eerom */
    sbd_envinit ();

    /* size memory (look in RTC for sbdreset's computed value) */
    memorysize = rtc_get (RTC_MEMSZ);
    if (memorysize < 4 || memorysize >  160)
	memorysize = 4 * MEG;	/* minimum possible */
    else
	memorysize *= MEG;

    /* set RTC periodic interrupt frequency */
    rtc_bic (RTC_STATUSA, RTC_RATE_MASK);
    rtc_bis (RTC_STATUSA, RTC_RATE);

    /* enable periodic interrupts */
    rtc_bis (RTC_STATUSB, RTCSB_PIE);
}


sbd_dispinit ()
{
    /* initialise display */
    display_init ();
    display_size (&displayrows, &displaycols);
    if (displaycols > MAXDISPLAYCOLS)
	displaycols = MAXDISPLAYCOLS;
}


/* control the display blank enable */
void
sbdblank (int blankon)
{
    display_flash (!blankon);
}


void
sbddevinit ()
{
}


/* board-specific environment override */
void
sbd_boardenv (char *(*get)(const char *),
	       int (*set)(const char *, const char *))
{
#ifdef INET
    /* no ethernet serial rom, put default in environment */
    if (!(*get) ("ethaddr"))
	(*set) ("ethaddr", "00:40:bc:03:00:00");
#endif
}


void
sbdpoll ()
{
#ifndef INET
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    /* poll any special devices */
    static int on = 0;
    static int count = 0;

    if (icu->irr.dev & INTR_DEV_RTC) {
	rtc_get (RTC_INTR);	/* clear i/u */
	if (++count >= RTC_HZ/2) {
	    count = 0;
	    display_flash (on = !on);
	}
    }
#endif
}


unsigned int
sbdenable (int machtype)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    unsigned int sr;

    /* now is the chance to report any early eerom failures */
    sbd_envreport ();

    /* clear interrupt latches */
    icu->ctrl.clear = ~0;

    /* all external interrupts masked initially */
    imask_set (0);

    /* enable panic errors + V96/floppy interrupt */
    sr = SR_IBIT7 | SR_IBIT4 | SR_IE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);

    if (cr & CAUSE_IP7) {
	unsigned int irr = icu->irr.err;
	if (irr & INTR_ERR_ACFAIL) {
	    icu->ctrl.clear = INTR_ERR_DEBUG;
	    return "Power Fail";
	}
	if (irr & INTR_ERR_DEBUG) {
	    /* software debounce debug button */
	    do {
		icu->ctrl.clear = INTR_ERR_DEBUG;
		sbddelay (1000);
	    } while (icu->irr.err & INTR_ERR_DEBUG);
	    return "Debug";
	}
	if (irr & INTR_ERR_BUSERR) {
	    icu->ctrl.clear = INTR_ERR_BUSERR;
	    return "Bus Error";
	}
    }

    if (cr & CAUSE_IP4) {
	unsigned int istat = V96X_LB_ISTAT;
	static char unknown[32];
	V96X_LB_ISTAT = 0; /* clear latched interrupts */
	if (V96X_LB_ISTAT & V96X_LB_INTR_PCI_RD)
	    return "PCI read error";
	if (V96X_LB_ISTAT & V96X_LB_INTR_PCI_WR)
	    return "PCI write error";
	sprintf (unknown, "Unknown V962 (0x%x)", istat);
	return unknown;
    }

    return 0;
}



/* display a message (four chars packed into one word),
   for very low-level code. */
void
sbddisplay (unsigned long msg, unsigned int x)
{
    char mbuf[MAXDISPLAYCOLS+1], *mp = mbuf;
    int i;

    sbdblank (0); /* display on */

    /* centre it */
    i = (displaycols - 4) / 2;
    while (i-- > 0)
	*mp++ = ' ';
    
    /* split up into characters */
    for (i = 3; i >= 0; i--, msg >>= 8)
	mp[i] = msg & 0xff;
    mp[4] = '\0';

    display_message (0, mbuf);
}


/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
    int len = strlen (msg);

    sbdblank (0); /* display on */
    if (len <= displaycols) {
	/* plonk short message straight onto screen */
	display_message (line, msg);
    }
    else {
	/* scroll long message across screen */
	char mbuf[MAXDISPLAYCOLS+1];
	int start, i;
	for (start = -displaycols+1; start <= len - displaycols; start++) {
	    for (i = start; i < start + displaycols; i++)
		mbuf[i - start] = (i < 0 || i >= len) ? ' ' : msg[i];
	    mbuf[displaycols] = '\0';
	    display_message (line, mbuf);
	    sbddelay (250000);		/* 0.25 sec delay */
	}
    }
}

/* display a message and then pause so that it can be read */
static void 
display (const char *msg)
{
    sbdmessage (0, msg);
    if (displaycols == 4)
	sbddelay (1000000);		/* 1 sec delay */
    else
	sbddelay (2000000);		/* 2 sec delay */
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
    extern int mips_icache_size;
    switch ((Prid >> 8) & 0xff) {
    case PRID_R4300:
	return 4300;
    case PRID_R4100:
	return 4100;
    case PRID_R4650:
	return 4640;
    case PRID_RM52XX:
	if (mips_icache_size == 32*1024)
	    return 5231;
	else
	    return 5230;
    case PRID_RC6447X:
	return 64474;
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


time_t __time (void)
{
    return (sbd_gettime ());
}
#endif


static unsigned long pipefreq = 0;
int cpuspeed = 15;

/* hand tune delay */
static void 
probefreq(void)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    volatile unsigned long * const rtcaddr = PA_TO_KVA1 (RTC_BASE+RTC_ADDR);
    volatile unsigned long * const rtcdata = PA_TO_KVA1 (RTC_BASE+RTC_DATA);
    unsigned long start, cnt, ohead, timeout;
    int i;

#if 0
    printf ("Memory Size %dMb, Simm0 %dMb SIMM1 %dMb DCR 0x%x\n",
	    rtc_get (RTC_MEMSZ), rtc_get (RTC_SIMM0SZ),
	    rtc_get (RTC_SIMM1SZ),rtc_get (RTC_DCR));
#endif

    SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

    *rtcaddr = RTC_INTR;	/* select interrupt register */

    /* run more than once to be sure that we're in the cache */
    for (i = 2; i != 0; i--) {
	start = get_count ();
	(void) *rtcdata;		/* clear any pending rtc i/u */
	/* wait until we see an RTC i/u */
	timeout = 10000000 / RTC_HZ;
	while ((icu->irr.dev & INTR_DEV_RTC) == 0)
	    if (--timeout == 0) return;
	cnt = get_count ();
    }
    cnt -= start;

    /* discover icu read overhead */

    for (i = 2; i != 0; i--) {
	start = get_count ();
	timeout = 10000000 / RTC_HZ;
	while ((icu->irr.dev & INTR_DEV_RTC) == 0)
	    if (--timeout == 0) return;
	ohead = get_count ();
    }
    ohead -= start;
    ohead /= 2;			/* average */

    /* work out number of cpu ticks per sec, the cpu pipeline */
    pipefreq = cnt * RTC_HZ;
    ohead *= RTC_HZ;		/* XXX is this right? */
    /*printf ("(%d-%d->", pipefreq, ohead);*/
    pipefreq -= ohead;

    /*
     * All processors clock the count register at 1/2 of PClock
     * except the 4100 which clocks at 1/4 of PClock
     */
    if (getmachtype () == 4100)
	pipefreq *= 4;
    else
	pipefreq *= 2;

    /* round frequency to 3 decimal places */
    pipefreq += 500;
    pipefreq -= pipefreq % 1000;
    /*printf ("%d)", pipefreq);*/

    /* how many times round a 2 instruction loop gives 1us delay */
    cpuspeed = (pipefreq/2) / 1000000;
}


unsigned long sbdpipefreq (void)
{
    if (pipefreq == 0)
	probefreq ();
    return pipefreq;
}

unsigned long sbdcpufreq (void)
{
    unsigned long freq;
    unsigned int ratio = (Config & CFG_ECMASK) >> CFG_ECSHIFT;
    typedef enum {R_X, R1_1, R3_2, R2_1, R5_2, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1} Ratios;
    Ratios *ratios, 
	ratios_4300[] = {R2_1, R3_1, R5_1, R6_1, R5_2, R3_1, R4_1, R3_2},
        ratios_def[] = {R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1, R_X};
	  
    freq = sbdpipefreq ();

    switch (getmachtype ()) {
    case 4300:
	ratios = ratios_4300;
	break;
    case 4100:
	/* actually bus clock; input clock = PClock / 4 */
	return (ratio == 0) ? (freq / 2) : freq;
    default:
	ratios = ratios_def;
    }

    switch (ratios[ratio]) {
    case R2_1: return freq / 2;
    case R3_2: return (2 * freq) / 3;
    case R3_1: return freq / 3;
    case R5_2: return (2 * freq) / 5;
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
