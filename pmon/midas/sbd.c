/* $Id: sbd.c,v 1.6 2000/03/28 00:21:49 nigel Exp $ */
/* p4032/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <midas/sbd.h>

extern int ns16550();

#ifdef SBDDISPLAYPRINT
int	sbddisplayprint;
#endif

ConfigEntry     ConfigTable[] =
{
    {(Addr)PA_TO_KVA1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PA_TO_KVA1(UART1_BASE), 0, ns16550, 256, B9600},
    {0}};



const char *
sbdgetname ()
{
    return "MIDAS";
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
    /* initialise environment from eerom */
    sbd_envinit ();

    if (memorysize == 0)
	memorysize = sizemem (CLIENTPC, LOCAL_MEM_SIZE);
}


sbd_dispinit ()
{
}


/* control the display blank enable */
void
sbdblank (int blankon)
{
}


void
sbddevinit ()
{
    /* program any local devices */
}



void
sbdpoll ()
{
    /* poll any special devices */
}


unsigned int
sbdenable (int machtype)
{
    unsigned int sr;

    /* now is the chance to report any early eerom failures */
    sbd_envreport ();

    /* no interrupts enabled */
    sr = SR_IE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}



/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
}

/* display a message and then pause so that it can be read */
static void 
display (const char *msg)
{
}


/* display a register name and its value, then pause */
static void
xdisplay (char *s, unsigned int x)
{
}


/* display exception information on debug display */
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
    extern int icache_size;
    switch ((Prid >> 8) & 0xff) {
    case 35: return 5000;	/* IDT R5000			ISA IV  */
    default: return 0;
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


static unsigned long pipefreq = 0;
int cpuspeed = 15;

#define TIMER_NUM	25175LL
#define TIMER_DEN	420LL
#define TIMER_HZ (TIMER_NUM/TIMER_DEN)

void
sbd_timerconf ()
{
    struct {
	volatile unsigned int *addr;
	unsigned int val;
    } *vp, vreg[] = {
#define VREG(r) ((volatile unsigned int *)PA_TO_KVA1(TSP_BASE+(r)))
#define FBSDCTRL1 VREG(0x0110)		/* FBSDCTRL1 */
	{FBSDCTRL1, 0x52224228},
#define FBSDCTRL2 VREG(0x0114)		/* FBSDCTRL2 */
	{FBSDCTRL2, 0xCB027732},
#define VO_STARTX VREG(0x0060)		/* VO_STARTX */
	{VO_STARTX, 0x00000094},
#define VO_STARTF1 VREG(0x0064)		/* VO_STARTF1 */
	{VO_STARTF1, 0x0000001B},
#define VO_STARTF2 VREG(0x0068)		/* VO_STARTF2 */
	{VO_STARTF2, 0x00000002},
#define VO_CONTROL VREG(0x007C)		/* VO_CONTROL */
	{VO_CONTROL, 0x0000000A},
#define KEY VREG(0x0048)		/* KEY */
	{KEY, 0x000000FF},
#define MEMMODE VREG(0x0040)		/* MEMMODE */
	{MEMMODE, 0x00000008},
#define AG_YSIZE VREG(0x00CC)		/* AG_YSIZE */
	{AG_YSIZE, 0x000001E0},
#define AG_XSIZE3D VREG(0x00D0)		/* AG_XSIZE3D */
	{AG_XSIZE3D, 0x000001E0},
#define AG_XSIZE2D VREG(0x00D4)		/* AG_XSIZE2D */
	{AG_XSIZE2D, 0x000000A0},
#define OVERLAYYSIZE VREG(0x00DC)		/* OVERLAYYSIZE */
	{OVERLAYYSIZE, 0x000001E0},
#define OVERLAYXSIZE VREG(0x00D8)		/* OVERLAYXSIZE */
	{OVERLAYXSIZE, 0x000000A0},
#define AG_XOFFSET VREG(0x00E0)		/* AG_XOFFSET */
	{AG_XOFFSET, 0x00000000},
#define AG_YOFFSET VREG(0x00E4)		/* AG_YOFFSET */
	{AG_YOFFSET, 0x00000000},
#define AG_LOAD2D VREG(0x00E8)		/* AG_LOAD2D */
	{AG_LOAD2D, 0x00000000},
#define ARB_3DW VREG(0x00EC)		/* ARB_3DW */
	{ARB_3DW, 0x00001C08},
#define ARB_2DW VREG(0x00F0)		/* ARB_2DW */
	{ARB_2DW, 0x00000C04},
#define ARB_3DR VREG(0x00F4)		/* ARB_3DR */
	{ARB_3DR, 0x00000814},
#define ARB_2DR VREG(0x00F8)		/* ARB_2DR */
	{ARB_2DR, 0x0000040A},
#define ARB_BURSTMAX VREG(0x00FC)		/* ARB_BURSTMAX */
	{ARB_BURSTMAX, 0x00000016},
#define SPG_CONTROL VREG(0x00A8)		/* SPG_CONTROL */
	{SPG_CONTROL, 0x00000700},
#define SPG_HLOAD VREG(0x0080)		/* SPG_HLOAD */
	{SPG_HLOAD, 0x0000031F},
#define SPG_VLOAD VREG(0x0084)		/* SPG_VLOAD */
	{SPG_VLOAD, 0x0000020C},
#define SPG_HBSTART VREG(0x008C)		/* SPG_HBSTART */
	{SPG_HBSTART, 0x000002D0},
#define SPG_HBEND VREG(0x0088)		/* SPG_HBEND */
	{SPG_HBEND, 0x00000050},
#define SPG_VBSTART VREG(0x0094)		/* SPG_VBSTART */
	{SPG_VBSTART, 0x000001F7},
#define SPG_VBEND VREG(0x0090)		/* SPG_VBEND */
	{SPG_VBEND, 0x00000016},
#define AG_CONTROL VREG(0x00C0)		/* AG_CONTROL */
	{AG_CONTROL, 0x00000401},
#define SPG_HSWIDTH VREG(0x0098)		/* SPG_HSWIDTH */
	{SPG_HSWIDTH, 0x00000040},
#define SPG_VSWIDTH VREG(0x009C)		/* SPG_VSWIDTH */
	{SPG_VSWIDTH, 0x00000008},
#define INTMASK VREG(0x0010)		/* SPG_VSWIDTH */
	{INTMASK, 0x00000001},
	{0, 0}
    };

    for (vp = vreg; vp->addr; vp++)
	sbd_iowrite32 (vp->addr, vp->val);

}

static int
sbd_timersync (unsigned long *count)
{
    volatile unsigned int *intstatus = VREG(0x000c);
    int timeout;
    /* synchronise with the interrupt */
    timeout = 10000000 / TIMER_HZ;
    
    while ((sbd_ioread32 (intstatus) & 1) == 0) {
	if (--timeout == 0) return 0;
    }
    *count = get_count ();
    return 1;
}

/* hand tune delay */
static void 
probefreq(void)
{
#ifdef HAVETIMESOURCE
    unsigned long start, cnt, timeout;
    int i;

    SBD_DISPLAY ("FREQ", CHKPNT_FREQ);

    /* configure time source */
    sbd_timerconf ();

    /* run more than once to be sure that we're in the cache */
    for (i = 3; i != 0; i--) {
	(void) sbd_timersync (&start);
	/* wait until we see timer tick */
	if (!sbd_timersync (&cnt))
	    return;
    }
    cnt -= start;		/* just take final value */

    /* work out number of cpu ticks per sec, the cpu pipeline 
       actually runs at twice this speed */
    pipefreq = (2 * cnt * TIMER_NUM) / TIMER_DEN;

    /* round frequency to 3 decimal places */
    pipefreq += 500;
    pipefreq -= pipefreq % 1000;

    /* how many times round a 2 instruction loop gives 1us delay */
    cpuspeed = (pipefreq/2) / 1000000;
#else
    pipefreq = MHZ*1000000;
    cpuspeed = MHZ;
#endif
}

unsigned long sbdpipefreq (void)
{
    if (pipefreq == 0) {
	probefreq ();
    }
    return pipefreq;
}

unsigned long sbdcpufreq (void)
{
    unsigned long freq;
    freq = sbdpipefreq ();
    /* R5000 specific */
    switch ((Config & CFG_ECMASK) >> CFG_ECSHIFT) {
    case 0: return freq / 2;
    case 1: return freq / 3;
    case 2: return freq / 4;
    case 3: return freq / 5;
    case 4: return freq / 6;
    case 5: return freq / 7;
    case 6: return freq / 8;
    }
    return (freq);
}

void
sbdsetled (unsigned char ledval)
{
    sbd_iowrite8 (PA_TO_KVA1(DBGLED_BASE), ledval);
}

#ifdef MIPSEB
#error sbd_io functions unimplemented for bigendian systems
#endif

#ifdef MIPSEL

unsigned char
sbd_ioread8 (volatile unsigned char *addr)
{
    volatile unsigned int *genctlp = MC_REG(MC_GEN_CTL);
    unsigned int genctl = *genctlp;
    unsigned char v;

    /* disable readahead on SCIO space */
    *genctlp = genctl | MC_GEN_CTL_RABYPASS_SCIO;

#ifdef MIDASPABUG
    if ((unsigned int)addr & 0x04) {
	v = *addr;
    }
    else {
	unsigned int iv;
	iv = *(volatile unsigned long long *)((unsigned int)addr & ~7);
	iv >>= ((unsigned int)addr & 3)*8;
	v = iv;
    }
#else
    v = *addr;
#endif
    *genctlp = genctl;

    return v;
}

unsigned short
sbd_ioread16 (volatile unsigned short *addr)
{
    volatile unsigned int *genctlp = MC_REG(MC_GEN_CTL);
    unsigned int genctl = *genctlp;
    unsigned short v;

    /* disable readahead on SCIO space */
    *genctlp = genctl | MC_GEN_CTL_RABYPASS_SCIO;

#ifdef MIDASPABUG
    if ((unsigned int)addr & 0x04) {
	v = *addr;
    }
    else {
	unsigned int iv;
	iv = *(volatile unsigned long long *)((unsigned int)addr & ~7);
	iv >>= ((unsigned int)addr & 3)*8;
	v = iv;
    }
#else
    v = *addr;
#endif
    *genctlp = genctl;

    return v;
}

unsigned int
sbd_ioread32 (volatile unsigned int *addr)
{
    volatile unsigned int *genctlp = MC_REG(MC_GEN_CTL);
    unsigned int genctl = *genctlp;
    unsigned int v;

    /* disable readahead on SCIO space */
    *genctlp = genctl | MC_GEN_CTL_RABYPASS_SCIO;

#ifdef MIDASPABUG
    if ((unsigned int)addr & 0x04) {
	v = *addr;
    }
    else {
	v = *(volatile unsigned long long *)((unsigned int)addr & ~7);
    }
#else
    v = *addr;
#endif
    *genctlp = genctl;

    return v;
}


void
sbd_iowrite8 (volatile unsigned char *addr, unsigned char v)
{
    *addr = v;
    mips_wbflush ();
#if defined(MIDASWRBUG)
    if ((((unsigned int) addr) & 0xffff0000) == (unsigned int)PA_TO_KVA1(NIC_BASE))
	*(volatile unsigned int *)PA_TO_KVA1(TSP_BASE) = 0;
#endif
}

void
sbd_iowrite16 (volatile unsigned short *addr, unsigned short v)
{
    *addr = v;
    mips_wbflush ();
#if defined(MIDASWRBUG) && 0
    if (((unsigned int addr) & 0xffff0000) == PA_TO_KVA1(NIC_BASE))
	*(volatile unsigned int *)PA_TO_KVA1(TSP_BASE) = 0;
#endif
}

void
sbd_iowrite32 (volatile unsigned int *addr, unsigned int v)
{
    *addr = v;
    mips_wbflush ();
#if defined(MIDASWRBUG) && 0
    if (((unsigned int addr) & 0xffff0000) == PA_TO_KVA1(NIC_BASE))
	*(volatile unsigned int *)PA_TO_KVA1(TSP_BASE) = 0;
#endif
}

#endif

/*
 * flash support stub
 */

void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
}


int
sbd_flashprogram (unsigned char *data, unsigned int size, unsigned int offset)
{
    printf ("This target does not support FLASH devices\n");
    return -1;
}

/* print board specific information */
void sbdmachinfo (void)
{
}
