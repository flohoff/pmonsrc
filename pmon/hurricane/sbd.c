/* 				/alg4/pmon/pmon/p4032/
 *	 p4032/sbd.c
 * $Id: sbd.c,v 1.4 2000/10/16 18:13:57 chris Exp $ 
 *
 * a7 23/dec/1998  5:30p rh -sbddbgintr(),rid of V96X references, rid of #include <v96xpbc.h>.
 * a6 15/dec/1998  6:00p rh -sbdmachinit() debug memorysize.
 * a5 15/dec/1998  4:30p rh -sbdmachinit() update memorysize from USER EEPROM
 * a4 14/dec/1998 11:30a rh -sbdmachinit() set memorysize = 16,for RAM testing.
 * a3 14/dec/1998 10:00a rh -sbdmachinit() set memorysize =  4,for RAM testing.
 * a2 30/nov/1998  3:30P rh -sbdmachinit() read USER EEPROM for memory size.
 * a1 08/sep/1998 10:30a rh -sbdmachinit() use #ifdef HRTC, complement data in sbdv3display()
 * a0 07/sep/1998 12:00p rh	-edit sbdmachinit(),added sbdv3display(),change SBD_DISPLAY.
 */


#include <mips.h>
#include <pmon.h>
#include <termio.h>
#include <mips/prid.h>
#include <sbd.h>

#define I2CSLAVEADR		4
#define NONCONFIGCYCLE 	0
#define HZBYTELOCATION 	10
#define MEMBYTELOCATION 	12

extern int ns16550();

ConfigEntry     ConfigTable[] =
{
    {(Addr)PHYS_TO_K1(UART0_BASE), 0, ns16550, 256, B9600},
    {(Addr)PHYS_TO_K1(UART1_BASE), 0, ns16550, 256, B9600},
    {0}};

const char *
sbdgetname ()
{
    return "Hurricane";
}


/* early low-level machine initialisation */
void
sbdmachinit ()
{
#define MEG	(1024*1024)
    long *hV3 = (long *)V3USC_BASE;

    /* retrieve memory size from USER EEPROM */
    I2C_EEPROMRead(hV3,I2CSLAVEADR,MEMBYTELOCATION,&memorysize,NONCONFIGCYCLE);

    memorysize &= 0xff;
    if (memorysize < 4 || memorysize >  160)
	memorysize = 4 * MEG;	/* minimum possible */
    else
	memorysize *= MEG;

}


sbd_dispinit ()
{
    /* initialise display */
}


/* control the display blank enable */
void
sbdblank (int blankon)
{
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

    sr = 0;

    (void) bis_sr (sr);
    return sr;
}


char *
sbddbgintr (unsigned int cr)
{
    return 0;
}



/* display a message (four chars packed into one word),
   for very low-level code. */
void
sbddisplay (unsigned long msg, int x)
{
    volatile unsigned int *led = PA_TO_KVA1 (LED_BASE);
    x ^= 0xff;			/* complement data first */
    led[0] = x;			/* write to LEDs */
    
}

/* display a long message on the display, scrolling if necessary. */
void
sbdmessage (int line, const char *msg)
{
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

/*
time_t __time (void)
{
    return (sbd_gettime ());
} 
*/
#endif


static unsigned long cpufreq;
static unsigned long pipefreq;
int cpuspeed = 15;

static void
probefreq (void)
{
    long *hV3 = (long *)V3USC_BASE;

    /* retrieve operating frequency from USER EEPROM */
    I2C_EEPROMRead(hV3,I2CSLAVEADR,HZBYTELOCATION,&cpufreq,NONCONFIGCYCLE);
    if (cpufreq)
	cpufreq *= 1000000;
    else
	cpufreq = 50000000;
}



unsigned long sbdcpufreq (void)
{
    if (cpufreq == 0)
	probefreq ();
    return cpufreq;
}

unsigned long sbdpipefreq (void)
{
    unsigned long freq;
    unsigned int ratio = (Config & CFG_ECMASK) >> CFG_ECSHIFT;
    typedef enum {R_X, R1_1, R3_2, R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1} Ratios;
    Ratios *ratios, 
	ratios_4300[] = {R2_1, R3_1, R5_1, R6_1, R5_2, R3_1, R4_1, R3_2},
        ratios_def[] = {R2_1, R3_1, R4_1, R5_1, R6_1, R7_1, R8_1, R_X};
	  
    if (pipefreq == 0) {
	
	freq = sbdcpufreq ();

	switch (getmachtype ()) {
	case 4100:
	    /* frig the ratio to get the right multiplier from the 4300 table */
	    ratio = (ratio == 0) ? 3 : 6;
	    /* fall through */
	case 4300:
	    ratios = ratios_4300;
	    break;
	default:
	    ratios = ratios_def;
	}

	switch (ratios[ratio]) {
	case R2_1: freq *= 2; break;
	case R3_2: freq = (3 * freq) / 2; break;
	case R3_1: freq *= 3; break;
	case R4_1: freq *= 4; break;
	case R5_1: freq *= 5; break;
	case R6_1: freq *= 6; break;
	case R7_1: freq *= 7; break;

	case R8_1: freq *= 8; break;
	}

	pipefreq = freq;

	/* how many times round a 2 instruction loop gives 1us delay */
	cpuspeed = (freq/2) / 1000000;
    }

    return pipefreq;
}

#if 0
PCICONFIG()
{
/* PCI initializations */
/*   	Normally this is done earlier in this function in call to "init_net()" */

    int uscstep;

    printf("Delay 25 sec before doing PCI enumeration.\n\n");
    sbddelay(8000000);		/* 25000000 ,25 sec delay */
    /* a value of 1000000, more effect */

    uscstep = V3USC_PCI_CC_REV & PCI_CC_REV_VREV_MASK;

    /* printf ("uscstep = %08x   ", uscstep);  */

    if(uscstep == 0)
	uscstepstg[0] = 'A';
    else if(uscstep == 1)
	uscstepstg[0] = 'B';
 
    printf ("	PCI Bridge: 	USC320 (%s) \n\n", uscstepstg);


    if(V3USC_PCI_I2O_BASE == 0 )
    {
	printf("Start PCI Configuration\n");

	_pci_configure (1);

	printf("Completed PCI Configuration");
		
    }
    else
	printf("PCI enumeration done by External Master.\n");

  
    printf ("\n\n");

}
#endif

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

void
sbdmachinfo ()
{
}
