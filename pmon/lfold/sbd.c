/*
 *	
 *	Copyright (c) 1996 ALGORITHMICS LIMITED 
 *	ALL RIGHTS RESERVED 
 *	
 *	THIS SOFTWARE PRODUCT CONTAINS THE UNPUBLISHED SOURCE
 *	CODE OF ALGORITHMICS LIMITED
 *	
 *	The copyright notices above do not evidence any actual 
 *	or intended publication of such source code.
 *	
 */

/*
 * sbd.c: UBI Pablo SBD C code
 */

#include "mips.h"
#include "termio.h"
#include "pmon.h"
#include "sbd.h"

#include "ns16550.h"
extern int ns16550 ();

/* 8-bit IOCHAN0 - UART: 300ns cycle time */
#define DEVTIME0	IOCONFIG_TIME(300)

/* 8-bit IOCHAN1 - board registers */
#define DEVTIME1	IOCONFIG_TIME(120)

/* 16-bit IOGPCHAN0 - Ethernet */
#define  DEVTIME3	IOCONFIG_TIME(40)

/* 16-bit IOGPCHAN1 - ASIC ROM */
#define DEVTIME4	IOCONFIG_TIME(150)


extern int centronics ();

/*
 * this initialisation table is used during basic initialisation
 */

struct vrent {
    volatile unsigned int 	*regp;
    unsigned int		value;
};

#define REGP(p) &G10_REG(p)

const struct vrent sbditab[] = {
  {REGP(G10_ROMCONFIG),	ROMCONFIG_2MB|
			(ROM_ACK<<ROMCONFIG_ACK_SHIFT)|
			(ROM_GAP3<<ROMCONFIG_GAP3_SHIFT)|
			(ROM_GAP2<<ROMCONFIG_GAP2_SHIFT)|
			(ROM_GAP1<<ROMCONFIG_GAP1_SHIFT)|
			(ROM_FIRST<<ROMCONFIG_FIRST_SHIFT)
  },

  {REGP(G10_DRAMCONFIG),DRAMCONFIG_REF|
#if MHZ >= 20
			DRAMCONFIG_TECAS|
#endif
     			(DRAMCONFIG_4Mb<<DRAMCONFIG_BANK0_SHIFT)|
     			(DRAMCONFIG_16Mb<<DRAMCONFIG_BANK12_SHIFT)},

  {REGP(G10_INTCAUSE),	0},

  {REGP(G10_INTMASK),	0},

  {REGP(G10_IOCONFIG),	IOCONFIG_TOEN|
#ifdef MIPSEB
   			IOCONFIG_IOBE1|IOCONFIG_IOBE0|
#else
   			IOCONFIG_IOLE1|IOCONFIG_IOLE0|
#endif
     			(DEVTIME4<<IOCONFIG_DEVTIME4_SHIFT)|
			(DEVTIME3<<IOCONFIG_DEVTIME3_SHIFT)|
			(IOCONFIG_TIME(120)<<IOCONFIG_CENTIME_SHIFT)|
     			(DEVTIME1<<IOCONFIG_DEVTIME1_SHIFT)|
			(DEVTIME0<<IOCONFIG_DEVTIME0_SHIFT)},

  /* Initialise PIO outputs */
  {REGP(G10_PIOOUT),	0},

  /* Identify the PIO inputs */
  {REGP(G10_PIOCONFIG),	0x3f},

  /* Centronics port initially OFFLINE, BUSY, PERROR, FAULT, no ACK */
  {REGP(G10_CENCTLOUT),	CENOUT_BUSY | CENOUT_PERROR | CENOUT_NOTACK},

  {0,			0},	/* terminator */
};



const ConfigEntry ConfigTable[] =
{
    {(Addr)NS16550_CHANA, 0, ns16550, 256, B9600},
    {(Addr)1, 0, centronics, 256, B9600},
    {0}};


/* return the current CPU type */
getmachtype ()
{
    switch ((Prid >> 8) & 0xff) {
    case 0x02:
    case 0x03:
	/* have we got an FPU? */
	return (initial_sr & SR_CU1) ? 3081 : 3051;
    case 0x07:
	return 3041;
    default:
	return 0;
    }
}


/* return name of this board */
const char *
sbdgetname ()
{
    return "PABLO";
}


/* return CPU clock frequency */
int
sbdcpufreq ()
{
    /* should be calculated dynamically using timer */
    return MHZ*1000000;
}

int
sbdpipefreq ()
{
    /* should be calculated dynamically using timer */
    return MHZ*1000000;
}


static void
sbdmemsize (start)
    void *start;
{
#define MEG (1024*1024/sizeof(int))
    volatile unsigned int *dramc = &G10_REG (G10_DRAMCONFIG);
    volatile unsigned int *base = (unsigned int *) K0_TO_K1 (start);
    unsigned int save[8];
    unsigned int aces = 0xaaaaaaaa;
    unsigned int fives = 0x55555555;
    unsigned long max = 0;
    unsigned int cfg; 
    int i;

    /* Set maximum bank sizes */ 
    *dramc = (*dramc & ~(DRAMCONFIG_BANK0_MASK | DRAMCONFIG_BANK12_MASK)) |
	 (DRAMCONFIG_8Mb << DRAMCONFIG_BANK0_SHIFT) |
	 (DRAMCONFIG_16Mb << DRAMCONFIG_BANK12_SHIFT);

    save[7] = base[0*MEG];

    /* Check to see if there is any memory at all in bank 0 */
    save[6] = base[1];
    base[0] = aces;
    base[1] = fives;
    if (base[0] != aces)
	goto error;
    base[1] = save[6];

    /* Figure out size of bank 0: 1 or 4 Mbyte */
    base[0] = DRAMCONFIG_4Mb;
    save[DRAMCONFIG_1Mb] = base[1*MEG];
    base[1*MEG] = DRAMCONFIG_1Mb;

    cfg = base[0];
    if (cfg > DRAMCONFIG_4Mb)
	goto error;

    base[1*MEG] = save[DRAMCONFIG_1Mb];
    base[0*MEG] = save[7];
    
    /* We now know bank 0 size, so program it */ 
    *dramc = (*dramc & ~DRAMCONFIG_BANK0_MASK) |
	 (cfg << DRAMCONFIG_BANK0_SHIFT);

    max  += MEG << cfg;
    base += MEG << cfg;

    /* Check to see if there is any memory at all in bank 1 */
    save[7] = base[0*MEG];
    save[6] = base[1];
    base[0] = aces;
    base[1] = fives;
    if (base[0] != aces)
	goto done;
    base[1] = save[6];

    /* Figure out size of bank 1 */
    base[0*MEG] = DRAMCONFIG_16Mb;
    for (i = DRAMCONFIG_4Mb; i >= DRAMCONFIG_1Mb; i -= 2) {
	save[i] = base[MEG << i];
	base[MEG << i] = i;
    }

    cfg = base[0];
    if (cfg > DRAMCONFIG_16Mb)
	goto error;

    for (i = cfg - 2; i >= DRAMCONFIG_1Mb; i -= 2)
	base[MEG << i] = save[i];

    base[0*MEG] = save[7];

    /* We now know bank 1 size, so program it (and bank 2)  */ 
    *dramc = (*dramc & ~DRAMCONFIG_BANK12_MASK) |
	 (cfg << DRAMCONFIG_BANK12_SHIFT);

    base += MEG << cfg;
    max  += MEG << cfg;

    /* Check to see if there is any memory at all in bank 2 */
    save[7] = base[0];
    save[6] = base[1];
    base[0] = aces;
    base[1] = fives;
    if (base[0] != aces)
	goto done;
    base[1] = save[6];
    base[0] = save[7];

    max  += (MEG << cfg);

done:
    memorysize = max * sizeof(int);
    return;

error:
    while(1) continue;
}


/* very early low-level machine initialisation */
void
sbdmachinit ()
{
    volatile struct csr *csr = (struct csr *) PA_TO_KVA1 (CSR_BASE);
    extern int _pmon_in_ram;

    /* initialise low level devices */
    if (_pmon_in_ram) {
	G10_REG (G10_IOCONFIG) = 
	    IOCONFIG_TOEN|
#ifdef MIPSEB
	    IOCONFIG_IOBE1|IOCONFIG_IOBE0|
#else
	    IOCONFIG_IOLE1|IOCONFIG_IOLE0|
#endif
	    (DEVTIME4<<IOCONFIG_DEVTIME4_SHIFT)|
	    (DEVTIME3<<IOCONFIG_DEVTIME3_SHIFT)|
	    (IOCONFIG_TIME(120)<<IOCONFIG_CENTIME_SHIFT)|
	    (DEVTIME1<<IOCONFIG_DEVTIME1_SHIFT)|
	    (DEVTIME0<<IOCONFIG_DEVTIME0_SHIFT);
    }

    /* Enable COMBI i/o chip */
    csr->csr_combi_enb = 1;

    /* Disable PAGESYNC to 3710 */
    csr->csr_pagesync = 1;

    /* initialise memory controller and size memory */
    sbdmemsize (CLIENTPC);

    /* initialise eeprom non-volatile environment store */
    sbd_envinit ();
}


int
sbdberrenb (int on)
{
    return 0;
}

int
sbdberrcnt (void)
{
    return 0;
}


/* initialise any local devices (except uarts, which are handled via ConfigTable) */
void
sbddevinit ()
{
}


/* enable any local interrupts which we will handle */
unsigned int
sbdenable (machtype)
{
    unsigned int sr = 0;

    sbd_envreport ();

    if (initial_sr & SR_CU1)
	/* enable FPU interrupts */
	sr |= (SR_IBIT1 << sbd_getfpaintr()) | SR_IEC;

#if 0
    /* use VSYNC input interrupt as debug interrupt */
    G10_REG (G10_INTMASK) = INT_VSYNCIN;
    sr |= SR_IBIT3;
#endif

    (void) bis_sr (sr);
    return sr;
}


/* poll any local devices */
sbdpoll ()
{
}	


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}



void
sbdmessage (int line, const char *msg)
{
}


/* Low-level function to display a message on a display (4 chars packed
 * into one word); may be called before memory and .data have been
 * initialised. 
 */
void
sbddisplay (unsigned long msg)
{
}


sbdled (int on)
{
}



/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
#if 0
    if ((cr & CAUSE_IP3) & (G10_REG (G10_INTCAUSE) & INT_VSYNCIN)) {
	G10_REG (G10_INTMASK) &= ~INT_VSYNCIN; /* mask interrupt */
	return "Debug";
    }
#endif
    return 0;
}




/* called on a fatal prom exception: display it on l.e.d, (if present) */
char *
sbdexception (epc, cause, ra, extra, exc)
    unsigned long epc, cause, ra, extra;
    char *exc;
{
    return NULL;
}


/*
 * floating point emulator stubs
 */

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
#endif

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
