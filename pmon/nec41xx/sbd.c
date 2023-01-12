#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <nec41xx/sbd.h>

extern int ns16550();

ConfigEntry     ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B38400},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B38400},
#if 0
    {(Addr)PHYS_TO_K1(UART2_BASE), 0, ns16550, 256, B38400},
#endif
    {0}
};



const char *
sbdgetname ()
{
    return "NEC Vr41XX UEB";
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
    memorysize = sizemem (CLIENTPC, MEM_SIZE);
    if (memorysize < 0x100000) {
	SBD_DISPLAY ("EMEM", 0xef);
	memorysize = 0x100000;
    }

#ifdef FLASH
    /* initialise environment from flash */
    sbd_envinit ();
#endif
}


/* initialise any local devices (except uarts which are handled 
   via ConfigTable) */
void
sbddevinit ()
{
}


void
sbdpoll ()
{
    /* poll any special devices */
#ifdef INET
#endif
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (int machtype)
{
    unsigned int sr = 0;
#ifdef FLASH
    /* now is the chance to report any early environment failures */
    sbd_envreport ();
#endif
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}



#if 0
/* display a message (four chars packed into one word),
   for very low-level code. */
void
sbddisplay (unsigned long msg, int chk)
{
    *(volatile unsigned char *)PA_TO_KVA1(LEDWR_BASE) = chk;
}
#endif

/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
}


static void
xdisplay (int which, unsigned long reg)
{
    int i;
    
    /* do something exciting to show a new register is about to appear */
    for (i = 0; i < 100; i++) {
	*(volatile unsigned char *)PA_TO_KVA1(LEDWR_BASE) = (i << 4) | which;
	sbddelay (10000);	/* 0.01 secs */
    }

    /* display 1 byte at a time */
    for (i = 0; i < 4; i++) {
	*(volatile unsigned char *)PA_TO_KVA1(LEDWR_BASE) = reg >> 24;
	reg <<= 8;
	sbddelay (500000);	/* 0.5 sec */
    }
}


char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long   epc, cause, ra, extra;
    char *exc;
{
    xdisplay (1, epc);
    xdisplay (2, ra);
    xdisplay (3, cause);
    return 0;
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
    volatile struct vr41xxbcu *bcu = PA_TO_KVA1 (BCU_BASE);
    unsigned int revid = ((bcu->bcu_revid & BCUREVID_RID_MASK)
			  >> BCUREVID_RID_SHIFT);
    
    /* XXX these are guesses, apart from 2=4111 */
    switch (revid) {
    case 1:
	return 4102;
    case 2:
	return 4111;
    case 3:
	return 4121;
    case 4:
	return 4122;
    default:
	return 4100;
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


/* 
 * The Vr41xx pipeline clock is derived from 18.432MHz multiplied
 * by a value selected by the CLKSEL pins, which is reported in the
 * BCU clock speed register.
 */

/* return pipeline clock frequency */
unsigned long sbdpipefreq (void)
{
    volatile struct vr41xxbcu *bcu = PA_TO_KVA1 (BCU_BASE);
    unsigned int div;

    div = (bcu->bcu_clkspeed & BCUCLKSPEED_CLKSP_MASK) 
	>> BCUCLKSPEED_CLKSP_SHIFT;
    if (div == 0)
	div = 1;

    switch (getmachtype ()) {
    case 4102:
	return (18432000 * 32) / div;
    case 4111:
    case 4121:
    case 4122:	/* guess */
    default:
	return (18432000 * 64) / div;
    }
}


/* 
 * The Vr41xx internal Tclock run at a divisor of the pipeline 
 * clock (Pclock).  Return that divisor.
 */
long
_sbd_tclkdiv (void)
{
    volatile struct vr41xxbcu *bcu = PA_TO_KVA1 (BCU_BASE);

    switch (getmachtype ()) {
    case 4102:
    case 4111:
	if ((bcu->bcu_clkspeed & BCUCLKSPEED_DIV2B) == 0)
	    return 2;
	if ((bcu->bcu_clkspeed & BCUCLKSPEED_DIV3B) == 0)
	    return 3;
	if ((bcu->bcu_clkspeed & BCUCLKSPEED_DIV4B) == 0)
	    return 4;
	/* ??? what if none are set to zero */
	return 1;
    case 4121:
    case 4122:
    default:
	return ((bcu->bcu_clkspeed & BCUCLKSPEED_DIVT_MASK) >> 
		BCUCLKSPEED_DIVT_SHIFT);
    }
}


/* 
 * The bus clock runs at TClock / 4.
 */
unsigned long sbdcpufreq (void)
{
    unsigned long pipe = sbdpipefreq ();
    return sbdpipefreq () / _sbd_tclkdiv () / 4;
}

/* print board specific information */
void sbdmachinfo (void)
{
}
