/* $Id: sbd.c,v 1.4 1999/04/22 16:35:39 chris Exp $ */
/* mann/sbd.c */

#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <mann/sbd.h>

#ifdef Z8530
extern int z8530 ();
#endif
#ifdef M82510
extern int m82510 ();
#endif
extern int pp ();

ConfigEntry     ConfigTable[] =
{
#ifdef Z8530
    {(Addr)PHYS_TO_K1(Z8530_BASE + 8), 0, z8530, 256, B9600},
#endif
#ifdef M82510
    {(Addr)PHYS_TO_K1(M82510_BASE), 0, m82510, 256, B9600},
#endif
    {(Addr)-1, 0, pp, 256, B9600},
    {0}};


const char *
sbdgetname ()
{
    return "RIP-CPU 2.1";
}



/* early low-level machine initialisation */
void
sbdmachinit ()
{
    /* clear any pending interrupts */
    *(volatile unsigned int *)PHYS_TO_K1(DBG_BASE) = DBG_FIFOINT|DBG_DBGINT;
    memorysize = sizemem (CLIENTPC, LOCAL_MEM+LOCAL_MEM_SIZE);
}


void
sbddevinit ()
{
    /* program any local devices */
}


unsigned int
sbdenable (int machtype)
{
    unsigned int sr;

    /* clear left-over panics */
    *(volatile unsigned int *)PHYS_TO_K1(DBG_BASE) = DBG_FIFOINT|DBG_DBGINT;

    /* enable debug/fifo interrupts */
    sr = SR_IBITDBG | SR_IE;

    if (Prid == 0x0a11)
      /* Rev 1.1 R4200: disable parity errors */
      sr |= SR_DE;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    register int i;

    if (cr & CAUSE_IPDBG) {
	unsigned int dbg = *(volatile unsigned int *) PHYS_TO_K1 (DBG_BASE);
	*(volatile unsigned int *) PHYS_TO_K1 (DBG_BASE) = dbg;
	if (dbg & DBG_FIFOINT)
	  return "FIFO interrupt";
	if (dbg & DBG_DBGINT) {
	    /* software debounce debug button
	     * (hope no other FIFO interrupt occurs in the meantime)
	     */
	    do {
		*(volatile unsigned int *) PHYS_TO_K1 (DBG_BASE) = dbg;
		/* rather arbitrary delay here as we don't tune a delay loop */
		for (i = 15 * 1000; i > 0; i--) continue;
		dbg = *(volatile unsigned int *) PHYS_TO_K1 (DBG_BASE);
	    } while (dbg & DBG_DBGINT);
	    return "Debug";
	}
    }
    return 0;
}


void
sbdpoll ()
{
    /* poll any special devices */
}


static void 
display (s)
    char *s;
{
}


static void
xdisplay (s, x)
    char *s;
    unsigned int x;
{
}


char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
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
    extern int icache_size, dcache_size;
    switch ((Prid >> 8) & 0xff) {
    case 0x04:
	return (icache_size == 16384) ? 4400 : 4000;
    case 0x0a:
	return 4200;
    case 0x20:
	return 4600;
    default:
	return 0;
    }
}

static const struct {
    unsigned int iclock;	/* Input clock */
    unsigned int cclock;	/* CPU clock */
    unsigned int bclock;	/* Bus clock */
} clkinfo[8] = {
    {50000000,	100000000,	50000000},
    {67000000,	133000000,	67000000},
    {75000000,	150000000,	50000000},
    {83000000,	167000000,	55700000},
    {100000000,	200000000,	67000000},
    {125000000,	250000000,	62500000},
    {50000000,	100000000,	50000000},
    {67000000,	133000000,	67000000},
};

int
sbdcpufreq ()
{
    unsigned int cnf;

    cnf = *(volatile unsigned int *)PHYS_TO_K1(CNF_BASE);
    cnf = (cnf & CNF_CLKRATE) >> CNF_CLKSHIFT;
    return (clkinfo[cnf].iclock);
}

int
sbdbclock ()
{
    unsigned int cnf;

    cnf = *(volatile unsigned int *)PHYS_TO_K1(CNF_BASE);
    cnf = (cnf & CNF_CLKRATE) >> CNF_CLKSHIFT;
    return (clkinfo[cnf].bclock);
}

#ifdef SROM
void
sbd_softromcopy (from, to, n)
uword from, to, n;
{
    if (from == 0) {
	from = PHYS_TO_K0(LOCAL_PROM+0x80000);
	to = PHYS_TO_K0(SOFTROM+0x400000);
	n = SOFTROM_SIZE-0x400000-0x80000;
    }
    memcpy ((void *)to, (void *)from, n);
    r4k_flush ();
}

volatile void
sbd_softromgo ()
{
    extern volatile void sbdsoftromgo ();
    sbdsoftromgo ();
}
#endif

/*
 * dummy nvram handler for systems without nvram.
 */

char *
sbd_getenv (name)
    char *name;
{
    return (NULL);
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
#ifdef SROM
    (*func)("autoboot", "srom");
    (*func)("bootdelay", "5");
    (*func)("AUTO", "srom");
#endif
    (*func)("hostport", "tty0");
}


/* gsignal() is called when a keyboard interrupt (^C) is detected */
gsignal (jb, sig)
    jmp_buf *jb;
    int sig;
{
    if (jb)
      longjmp (jb, 1);
}


/* stubs for unused network support */
netopen ()	{return -1;}
netread ()	{return -1;}
netwrite ()	{return -1;}
netclose ()	{return -1;}
netlseek ()	{return -1;}
netioctl ()     {return -1;}
netpoll ()	{}

/*
 * flash support stub
 */

void
sbd_flashinfo (void **flashbuf, int *flashsize)
{
    *flashbuf = (void *)0;
    *flashsize = 0;
    return 0;
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
