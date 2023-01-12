/* $Id: sbd.c,v 1.4 2000/03/28 00:21:45 nigel Exp $ */
/*
 *	
 *	Copyright (c) 1992 ALGORITHMICS LIMITED 
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
 * sbd.c: Libra SBD C code
 */

#include "mips.h"
#include "termio.h"
#include "pmon.h"
#include "sbd.h"
#include "pio.h"

#include "ns16550.h"
extern int ns16550 ();

/* UART: 300ns cycle time */
#define DEVTIME0	IOCONFIG_TIME(300)
#define DEVTIME1	IOCONFIG_TIME(300)

/* 8255A-2 parallel i/o */
#define DEVTIME3	IOCONFIG_TIME(250)

/* HP font cartridge */
#define DEVTIME4	IOCONFIG_TIME(250)


extern int centronics ();

/*
 * this initialisation table is used during basic initialisation
 */

struct vrent {
    volatile unsigned int 	*regp;
    unsigned int		value;
};

#define REGP(p) &G10_REG(p)

#define USEREALTIMINGS

const struct vrent sbditab[] = {
  {REGP(G10_ROMCONFIG),	ROMCONFIG_2MB|
#ifdef USEREALTIMINGS
			(ROM_ACK<<ROMCONFIG_ACK_SHIFT)|
			(ROM_GAP3<<ROMCONFIG_GAP3_SHIFT)|
			(ROM_GAP2<<ROMCONFIG_GAP2_SHIFT)|
			(ROM_GAP1<<ROMCONFIG_GAP1_SHIFT)|
			(ROM_FIRST<<ROMCONFIG_FIRST_SHIFT)
#else
			(31<<ROMCONFIG_ACK_SHIFT)|
			(15<<ROMCONFIG_GAP3_SHIFT)|
			(15<<ROMCONFIG_GAP2_SHIFT)|
			(15<<ROMCONFIG_GAP1_SHIFT)|
			(15<<ROMCONFIG_FIRST_SHIFT)
#endif
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
   			IOCONFIG_IOBE1|IOCONFIG_IOBE0|
     			(DEVTIME4<<IOCONFIG_DEVTIME4_SHIFT)|
			(DEVTIME3<<IOCONFIG_DEVTIME3_SHIFT)|
			(IOCONFIG_TIME(120)<<IOCONFIG_CENTIME_SHIFT)|
     			(DEVTIME1<<IOCONFIG_DEVTIME1_SHIFT)|
			(DEVTIME0<<IOCONFIG_DEVTIME0_SHIFT)},

  /* Initialise PIO outputs (LSYNC & VSYNC disabled; Centronics BUSY) */
  {REGP(G10_PIOOUT),	G10_PIO_CENBUSY | G10_PIO_VSYNCOUT | G10_PIO_LSYNC},

  /* Identify the PIO inputs */
  {REGP(G10_PIOCONFIG),	G10_PIO_UARTINT1 | G10_PIO_UARTINT0 | G10_PIO_VSYNCIN},

  /* Centronics port initially OFFLINE, BUSY, PERROR, FAULT, no ACK */
  {REGP(G10_CENCTLOUT),	CENOUT_BUSY | CENOUT_PERROR | CENOUT_NOTACK},

  {0,			0},	/* terminator */
};



const ConfigEntry ConfigTable[] =
{
    /* XXX Note console is on channel B */
    {(Addr)NS16550_CHANB, 0, ns16550, 256, B9600},
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
    if (pio_get (PIO_ENGSEL))
	return "LASERFOLD-24";
    else
	return "LASERFOLD-16";
}


/* return CPU clock frequency */
int
sbdcpufreq ()
{
    /* should be calculated dynamically using timer */
    return MHZ*1000000;
}

/* calculated dynamically using timer below */
static int pipefreq = MHZ*1000000;

int
sbdpipefreq ()
{
    return pipefreq;
}


void
sbddelay (unsigned long us)
{
    static unsigned long cycperusec;

    if (cycperusec == 0) {
	if (IS_KVA0 (sbddelay))
	    /* CPU pipeline freq */
	    cycperusec = pipefreq / 1000000 / 2;
	else
	    /* ROM cycle time */
	    cycperusec = ROMUS(1);
    }

    mips_cycle (cycperusec * us);
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
    extern int _pmon_in_ram;

    /* initialise low level devices */
    if (_pmon_in_ram) {
	pio_init ();
	fpanel_init ();
    }

    /* initialise memory controller and size memory */
    sbdmemsize (CLIENTPC);

    sbdstarttimer ();

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

#define HZ		(1000000 * MHZ)
#define SBDTIMER	(500000 * MHZ) 		/* 0.5 seconds */

sbdstarttimer ()
{
    unsigned long actual;
    int mult;

    /* set timer mode */
    G10_REG (G10_CLKCONFIG) = CLK_TIMER;

    /* program counter value */
    G10_REG (G10_COUNTER) = SBDTIMER;

    /* start timer */
    G10_REG (G10_CLKCONFIG) = CLK_TIMER | CLK_ENABLE;
	    
    /*
     * Try to work out the pipeline frequency of this board.
     */

    /* attempt 1ms delay */
    mips_cycle (CACHEMS (1));
    
    /* see how long we actually took */
    actual = SBDTIMER - G10_REG (G10_COUNTER);
    
    /* try various multiples of the bus clock */
    for (mult = 8; mult > 1; mult--) {
	if (actual >= (HZ/1100)/mult && actual <= (HZ/900)/mult)
	    /* within 10%: pipe clock = bus clock * mult */
	    break;
    }
    
    pipefreq = HZ*mult;
}


/* poll any local devices */
sbdpoll ()
{
    static int ledon = 0;

    /* poll the timer and flash led on every "interrupt" */
    if (G10_REG (G10_INTCAUSE) & INT_TIMER) {
	G10_REG (G10_INTCAUSE) = ~INT_TIMER;
	sbdled (ledon = !ledon);
    }
}	


/* control the l.e.d. blank enable */
void
sbdblank (int blankon)
{
}



void
sbdmessage (int line, const char *msg)
{
    extern void lcd_display (int, const char *);
    int len = strlen (msg);

    if (len <= 16) {
	/* plonk short message straight onto screen */
	lcd_display (line, msg);
    }
    else {
	/* scroll long message across screen */
	char mbuf[17];
	int start, i;
	for (start = -15; start <= len - 16; start++) {
	    for (i = start; i < start + 16; i++)
		mbuf[i - start] = (i < 0 || i >= len) ? ' ' : msg[i];
	    mbuf[17] = '\0';
	    lcd_display (line, mbuf);
	    sbddelay (250000);		/* 0.25 sec delay */
	}
    }
}


/* Low-level function to display a message on a display (4 chars packed
 * into one word); may be called before memory and .data have been
 * initialised. 
 */
void
sbddisplay (unsigned long msg)
{
    char mbuf[16];
    int i;

    for (i = 0; i < 6; i++)
	mbuf[i] = ' ';
    for (i = 9; i >= 6; i--, msg >>= 8)
	mbuf[i] = msg;
    mbuf[10] = '\0';
    sbdmessage (0, mbuf);
}


static unsigned short engn_ctl = (ENGN_OUT_NSTLED
				  | ENGN_OUT_NCMD
				  | ENGN_OUT_NSCLK
				  | ENGN_OUT_NCMBSY
				  | ENGN_OUT_NPRINT
				  | ENGN_OUT_NVSYNC);

sbdled (int on)
{
    if (on)
	engn_ctl &= ~ENGN_OUT_NSTLED;
    else
	engn_ctl |= ENGN_OUT_NSTLED;
    *(volatile unsigned short *) PHYS_TO_K1 (ENGNCTL_BASE) = engn_ctl;
}



/* return a string describing the current debug/panic interrupt, if any */
char *
sbddbgintr (unsigned int cr)
{
    if ((cr & CAUSE_IP3) & (G10_REG (G10_INTCAUSE) & INT_VSYNCIN)) {
	G10_REG (G10_INTMASK) &= ~INT_VSYNCIN; /* mask interrupt */
	return "Debug";
    }
    return 0;
}


static void
display (const char *msg)
{
    sbdmessage (0, msg);
    sbddelay (1000000);		/* 1 sec delay */
}


static void
xdisplay (s, x)
    char *s;
    unsigned int x;
{
    char buf[40];
    sprintf (buf, "%s %08x", s, x);
    display (buf);
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

    if (!exc) {
	sprintf (excbuf, "PMON %s exception",
		 getexcname (cause & CAUSE_EXCMASK));
	exc = excbuf;
    }
    display (exc);

    xdisplay ("Cause", cause);
    xdisplay ("EPC  ", epc);
    xdisplay ("RA   ", ra);
    switch (cause & CAUSE_EXCMASK) {
    case CEXC_MOD:
    case CEXC_TLBL:
    case CEXC_TLBS:
    case CEXC_ADEL:
    case CEXC_ADES:
	xdisplay ("BadVa", extra);
	break;
    }

    if ((epc & 3) == 0 && epc >= K0BASE && epc < K2BASE) {
	char ibuf[128];
	disasm (ibuf, epc, load_word (epc));
	display (ibuf);
    }
	
    display ("");
    return exc;
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
