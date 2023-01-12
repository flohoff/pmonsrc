/* $Id: sbd.c,v 1.16 2001/08/12 12:08:07 chris Exp $ */
/* p5064/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <p5064/sbd.h>
#include <p5064/p2681.h>
#include <p5064/z80pio.h>
#include <p5064/v96xpbc.h>
#include <mips/prid.h>

#ifdef RTC
#include <p5064/rtc.h>
#define RTC_HZ		16
#define RTC_RATE	RTC_RATE_16Hz
#endif

#define MAXDISPLAYCOLS	16

extern int ns16550();
extern int centronics();

static struct p2681info p2681info = {PA_TO_KVA1 (DBGUART_BASE)};
extern int p2681();

ConfigEntry     ConfigTable[6];

ConfigEntry	ConfigTableDbg[] =
{
    {(Addr)&p2681info, 0, p2681, 256, B9600},
    {(Addr)&p2681info, 1, p2681, 256, B9600},
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(ECP_BASE), 0, centronics, 512, B9600},

    {0}};

ConfigEntry	ConfigTableStd[] =
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

static volatile p5064softc * const softc = PA_TO_KVA1 (SOFTC_BASE);
static volatile p5064icu * const icu = PA_TO_KVA1 (ICU_BASE);
static volatile unsigned int * const option = PA_TO_KVA1 (OPTION_BASE);



unsigned int 
imask_get (void)
{
    return softc->imask;
}


inline unsigned int 
imask_set (unsigned int nmask)
{
    unsigned int omask = softc->imask;
    unsigned int diff = nmask ^ omask;

    if (diff != 0) {
	softc->imask = nmask;
	if (diff & ICU_DEV_MASK)
	    icu->ctrl.devmask = nmask >> ICU_DEV_SHIFT;
	if (diff & ICU_PCI_MASK)
	    icu->ctrl.pcimask = nmask >> ICU_PCI_SHIFT;
	if (diff & ICU_ISA_MASK)
	    icu->ctrl.isamask = nmask >> ICU_ISA_SHIFT;
    }
    return omask;
}


unsigned int 
imask_bic (unsigned int bits)
{
    return imask_set (softc->imask & ~bits);
}


unsigned int 
imask_bis (unsigned int bits)
{
    return imask_set (softc->imask | bits);
}



unsigned int 
zpiob_get (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int val = zpio->b_dat;
    return val;
}


unsigned int 
zpiob_bic (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int old = zpio->b_dat;
    zpio->b_dat = old & ~bits;
    return old;
}


unsigned int 
zpiob_bis (unsigned int bits)
{
    volatile zpiodev * const zpio = PA_TO_KVA1 (ZPIO_BASE);
    unsigned int old = zpio->b_dat;
    zpio->b_dat = old | bits;
    return old;
}



const char *
sbdgetname ()
{
    return "P5064";
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
#define MEG	(1024*1024)

    /* initialise display (again) */
    sbd_dispinit ();

    /* size memory (look for sbdreset's computed value) */
    memorysize = softc->memsz * 4 * MEG;
    if (memorysize < 4*MEG || memorysize > 256*MEG)
	memorysize = 4 * MEG;	/* minimum possible */

#ifdef RTC
    /* set RTC periodic interrupt frequency */
    rtc_bic (RTC_STATUSA, RTC_RATE_MASK);
    rtc_bis (RTC_STATUSA, RTC_RATE);
    /* enable periodic interrupts */
    rtc_bis (RTC_STATUSB, RTCSB_PIE);
#endif
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


#define WDELAY() \
{\
   register int i; \
   wbflush(); \
   for (i = DELAY; i != 0; i--) continue; \
}

void
sbddevinit ()
{
    volatile struct p2681dev *dp = p2681info.dev;
    unsigned char a, b;

    dp->channel[0].cr = 0x0a; WDELAY(); /* disable tx & rx */
    dp->channel[0].cr = 0x10; WDELAY();	/* reset mr ptr */
    dp->channel[0].mr = 0x55; WDELAY();	/* mr1a pattern 1 */
    dp->channel[0].mr = 0xaa; WDELAY();	/* mr1b pattern 2 */

    dp->channel[0].cr = 0x10; WDELAY();	/* reset mr ptr */
    a = dp->channel[0].mr; WDELAY();	/* read mr1a */
    b = dp->channel[0].mr; WDELAY();	/* read mr1b */

    /* set up the config table */
    if (a == 0x55 && b == 0xaa)
	memcpy (ConfigTable, ConfigTableDbg, sizeof(ConfigTableDbg));
    else
	memcpy (ConfigTable, ConfigTableStd, sizeof(ConfigTableStd));
}


/* board-specific environment override */
void
sbd_boardenv (char *(*get)(const char *),
	       int (*set)(const char *, const char *))
{
#ifdef INET
    /* no ethernet serial rom, put default in environment */
    if (!(*get) ("ethaddr"))
	(*set) ("ethaddr", "00:40:bc:04:00:00");
#endif
    /* set memory size unless it has already been defined */
    if (!(*get)("memsize")) {
	char buf[11];
	int memsize = softc->memsz * 4 * MEG; 
	if (memsize < 4*MEG || memsize > 256*MEG) 
	    memsize = 4 * MEG;  /* minimum possible */ 
	sprintf (buf, "0x%08x", memsize);
	(*set) ("memsize", buf);
    }
}


void
sbdpoll ()
{
#if !defined(INET) && defined(RTC)
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
    unsigned int sr;

#ifdef FLASH
    {
	/* now is the chance to report any early eerom failures */
	extern const char *_sbd_envinit (void);
	const char * err = _sbd_envinit ();
	if (err)
	    printf ("WARNING: flash environment: %s\n", err);
    }
#endif

    /* clear interrupt latches */
    icu->ctrl.clear = ~0;

    /* all external interrupts masked initially */
    imask_set (0);

    /* enable panic errors */
    sr = SR_IBIT7 | SR_IE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    if (cr & CAUSE_IP7) {
	unsigned int irr = icu->irr.err;
	icu->ctrl.clear = irr;
	if (irr & ICU_ERR_ACFAIL) {
	    icu->ctrl.clear = ICU_ERR_ACFAIL;
	    return "Power Fail";
	}
	if (irr & ICU_ERR_DEBUG) {
	    /* software debounce debug button */
	    do {
		icu->ctrl.clear = ICU_ERR_DEBUG;
		sbddelay (1000);
	    } while (icu->irr.err & ICU_ERR_DEBUG);
	    return "Debug";
	}
	if (irr & ICU_ERR_BUSERR) {
	    icu->ctrl.clear = ICU_ERR_BUSERR;
	    return "Bus Timeout";
	}
	if (irr & ICU_ERR_V96XPE) {
	    icu->ctrl.clear = ICU_ERR_V96XPE;
	    return "PCI Parity";
	}
	if (irr & ICU_ERR_ISANMI) {
	    icu->ctrl.clear = ICU_ERR_ISANMI;
	    return "ISA bus NMI";
	}
    }

#if 0
    if (cr & CAUSE_IP4) {
	unsigned char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
	unsigned int istat = V96X_LB_ISTAT;
	static char unknown[32];
	V96X_LB_ISTAT = 0; /* clear latched interrupts */
	if (istat & V96X_LB_INTR_PCI_RD)
	    return "PCI read error";
	if (istat & V96X_LB_INTR_PCI_WR)
	    return "PCI write error";
	sprintf (unknown, "Unknown V962 (0x%x)", istat);
	return unknown;
    }
#endif

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
	  sprintf (excbuf, "Cache exception");
	else
	  sprintf (excbuf, "%s exception", 
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
	xdisplay ("BadVa", extra);
	break;
    case CEXC_CACHE:
	xdisplay ("CErr", extra);
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
    extern int mips_scache_size;
    extern int mips_icache_size;
    switch ((Prid >> 8) & 0xff) {
    case PRID_R4000:
	return 4000;
    case PRID_R4600:
	return 4600;
    case PRID_R4700:
	return 4700;
    case PRID_R5000:
	return 5000;
    case PRID_RM52XX:
	/* blech */
	if (mips_scache_size > 0)
	    return 5270;
	if (mips_icache_size == 32*1024)
	    return (5261);
	return 5260;
    case PRID_RM7000:
	return 7000;
    case PRID_R5400:
	return 5464;
    case PRID_JADE:
	return 4032;		/* XXXX think of a number */
    case PRID_RC6457X:
	return 64574;
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


unsigned long __time (void)
{
#ifdef RTC
    return (sbd_gettime ());
#else
    return 0;
#endif
}
#endif


static unsigned long pipefreq = 0;
int cpuspeed = 100;

/* hand tune delay */
static void 
probefreq(void)
{
#ifdef RTC
    unsigned long start, cnt, ohead, timeout;
    int i;

    SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

    pipefreq = 133333333;	/* default value */

    /* select interrupt register */
    outb (RTC_ADDR_PORT, RTC_INTR);

    /* run more than once to be sure that we're in the cache */
    for (i = 2; i != 0; i--) {
	start = get_count ();
	/* clear any pending rtc i/u */
	(void) inb (RTC_DATA_PORT);
	/* wait until we see an RTC i/u */
	timeout = 10000000 / RTC_HZ;
	while ((icu->irr.dev & ICU_DEV_RTC) == 0)
	    if (--timeout == 0) goto done;
	cnt = get_count ();
    }
    cnt -= start;

    /* discover icu read overhead */

    for (i = 2; i != 0; i--) {
	start = get_count ();
	timeout = 10000000 / RTC_HZ;
	while ((icu->irr.dev & ICU_DEV_RTC) == 0)
	    if (--timeout == 0) goto done;
	ohead = get_count ();
    }
    ohead -= start;
    ohead /= 2;			/* average */

    /* work out number of cpu ticks per sec, the cpu pipeline */
    pipefreq = cnt * RTC_HZ;
    ohead *= RTC_HZ;		/* XXX is this right? */
    /*printf ("(%d-%d->", pipefreq, ohead);*/
    pipefreq -= ohead;

    /* All R5000 processors clock the count register at 1/2 of PClock */
    pipefreq *= 2;

    /* round frequency to 3 decimal places */
    pipefreq += 500;
    pipefreq -= pipefreq % 1000;
    /*printf ("%d)", pipefreq);*/
#else
    pipefreq = 133333333;
#endif

    /* how many times round a 2 instruction loop gives 1us delay */
 done:
    if (IS_KVA0 (&probefreq))
	cpuspeed = (pipefreq/2) / 1000000;
    else
	cpuspeed = 1;
}


unsigned long sbdpipefreq (void)
{
    if (pipefreq == 0)
	probefreq ();
    return pipefreq;
}


/* SysClk to PipeClk ratio table */
struct ratio {char mult; char div;};
static const struct ratio ratios_norm[8] = {{2,1}, {3,1}, {4,1}, {5,1},
					    {6,1}, {7,1}, {8,1}, {1,1}};
static const struct ratio ratios_rm7k[8] = {{2,1}, {3,1}, {4,1}, {5,2},
					    {6,1}, {7,2}, {8,1}, {9,2}};

unsigned long sbdcpufreq (void)
{
    unsigned int ec = (Config & CFG_ECMASK) >> CFG_ECSHIFT;	  
    const struct ratio *ratio;

    switch (getmachtype ()) {
    case 7000:
    case 7010:
	ratio = &ratios_rm7k[ec];
	break;
    default:
	ratio = &ratios_norm[ec];
	break;
    }

    /* For PipeClk to SysClk reverse the ratio calculation */
    return sbdpipefreq () * ratio->div / ratio->mult;
}

void
sbdmachinfo (void)
{
    printf ("Board Rev: %c", 'A' + ((*option & OPTION_REV) >> OPTION_REV_SHIFT));
    printf ("; FPGA Rev: %02x", icu->fpga.rev & 0xff);
    printf ("; User Options: %d", *option & OPTION_USER);
    printf ("\n");
}

void
sbd_off (int argc, char **argv)
{
    if (BOARD_REVISION >= 'C')
	apc_bis (RTC_BANK2_APCR1, APCR1_SOC);
    else
	*(volatile unsigned int *)BCR0(BCR0_PSU) = BCR0_PSU_OFF;
    wbflush ();
    sbddelay (100000);		/* 100ms */
    printf ("power-off failed\n");
}

#ifndef RTC
long
sbd_gettime (void)
{
    return 0;
}
#endif


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
}

void
sbd_flashinfo (void **buf, int *size)
{
    buf = 0;
    size = 0;
}


int
sbd_flashprogram (void *buf, int size)
{
    return 0;
}
#endif
