/* $Id: vicvac.c,v 1.2 1996/01/16 14:24:15 chris Exp $ */
/*
 * How to reset an Algorithmics VME system board to a 
 * known (operational) state.
 */

#include <mips.h>
#include <pmon.h>
#include <algvme/sbd.h>
#include <algvme/vac068.h>
#include <algvme/vic068.h?

/* SysV-style typedefs */
typedef unsigned char 	unchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;

/* BSD-style typedefs */
typedef unsigned char 	u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;

char *vmefair, *vmebr, *vmearb, *vmerel;

/* these have GOT to be standardised
*/
#define VITABBASE	64
int vic_errgrpvec = 0+VITABBASE;	/* 8 of */
int vic_locgrpvec = 8+VITABBASE;	/* 8 of */
int vic_icgsvec   = 16+VITABBASE;	/* 4 of */
int vic_icmsvec   = 24+VITABBASE;	/* 4 of */

static void vistray(int);

static struct {
    void 	(*fun)(int);
    int		arg;
} vitab[256];

static void config_delay(void);
static int intrhandler(int xcptno,struct xcptcontext *xcp);

/*
 * VIC interrupter
 */
void
vic_interrupter (int ipl)
{
  *VIC_VIICR = VIC_INT_DIS | (ipl & 7);
  wbflush ();
}

void
vic_interrupter_enable  ()
{
  *VIC_VIICR |= VIC_INT_DIS;
  wbflush ();
}

void
vic_interrupter_disable  ()
{
  *VIC_VIICR &= ~VIC_INT_DIS;
  wbflush ();
}

/*
 * VME interrupts
 */
static volatile u_int *vmeicr[] = {
  VIC_VICR1,
  VIC_VICR2,
  VIC_VICR3,
  VIC_VICR4,
  VIC_VICR5,
  VIC_VICR6,
  VIC_VICR7
  };

void
vic_vmeinterrupt (int VME, int ipl)
{
  VME = (VME & 7) - 1;
  *vmeicr[VME] = VIC_INT_DIS | (ipl & 7);
  wbflush ();
}

void
vic_vmeinterrupt_enable (int VME)
{
  VME = (VME & 7) - 1;
  *vmeicr[VME] &= ~VIC_INT_DIS;
  wbflush ();
}

void
vic_vmeinterrupt_disable (int VME)
{
  VME = (VME & 7) - 1;
  *vmeicr[VME] |= VIC_INT_DIS;
  wbflush ();
}

/*
 * DMA status interrupt
 */
void
vic_dmainterrupt (int ipl)
{
  *VIC_DMAICR = VIC_INT_DIS | (ipl & 7);
  wbflush ();
}

void
vic_dmainterrupt_enable ()
{
  *VIC_DMAICR &= ~VIC_INT_DIS;
  wbflush ();
}

void
vic_dmainterrupt_disable ()
{
  *VIC_DMAICR |= VIC_INT_DIS;
  wbflush ();
}


/*
 * local interrupts
 */
static volatile u_int *locicr[7] = {
  VIC_LICR1,
  VIC_LICR2,
  VIC_LICR3,
  VIC_LICR4,
  VIC_LICR5,
  VIC_LICR6,
  VIC_LICR7,
};

void
vic_locinterrupt (int local, int activehigh,
		  int edgesense, int vector, int ipl)
{
  u_int val;
  
  local = (local & 7) - 1;

  val = ipl & 7;
  val |= activehigh ? VIC_LACTIVEHI : VIC_LACTIVELO;
  val |= edgesense ? VIC_LEDGE : VIC_LLEVEL;
  if (vector)
      val |= VIC_LVEC;
  *locicr[local] = VIC_INT_DIS | val;
  wbflush ();
}

void
vic_locinterrupt_enable (int local)
{
  local = (local & 7) - 1;
  *locicr[local] &= ~VIC_INT_DIS;
  wbflush ();
}

void
vic_locinterrupt_disable (int local)
{
  local = (local & 7) - 1;
  *locicr[local] |= VIC_INT_DIS;
  wbflush ();
}


/*
 * Interprocessor communications global interrupt
 */
void
vic_icgsinterrupt (int ipl)
{
  u_int val;
  val = VIC_ICGS3_DIS | VIC_ICGS2_DIS | VIC_ICGS1_DIS | VIC_ICGS0_DIS;
  *VIC_ICGSICR = val | (ipl & 7);
  wbflush ();
}

void
vic_icgsinterrupt_enable (int s3, int s2, int s1, int s0)
{
  u_int val;
  val = s3 ? VIC_ICGS3_DIS : 0;
  val |= s2 ? VIC_ICGS2_DIS : 0;
  val |= s1 ? VIC_ICGS1_DIS : 0;
  val |= s0 ? VIC_ICGS0_DIS : 0;
  *VIC_ICGSICR &= ~val;
  wbflush ();
}

void
vic_icgsinterrupt_disable (int s3, int s2, int s1, int s0)
{
  u_int val;
  val = s3 ? VIC_ICGS3_DIS : 0;
  val |= s2 ? VIC_ICGS2_DIS : 0;
  val |= s1 ? VIC_ICGS1_DIS : 0;
  val |= s0 ? VIC_ICGS0_DIS : 0;
  *VIC_ICGSICR |= val;
  wbflush ();
}

/*
 * Interprocessor communications module interrupt
 */
void
vic_icmsinterrupt (int ipl)
{
  u_int val;
  val = VIC_ICMS3_DIS | VIC_ICMS2_DIS | VIC_ICMS1_DIS | VIC_ICMS0_DIS;
  *VIC_ICMSICR = val | (ipl & 7);
  wbflush ();
}

void
vic_icmsinterrupt_enable (int s3, int s2, int s1, int s0)
{
  u_int val;
  val = s3 ? VIC_ICMS3_DIS : 0;
  val |= s2 ? VIC_ICMS2_DIS : 0;
  val |= s1 ? VIC_ICMS1_DIS : 0;
  val |= s0 ? VIC_ICMS0_DIS : 0;
  *VIC_ICMSICR &= ~val;
  wbflush ();
}

void
vic_icmsinterrupt_disable (int s3, int s2, int s1, int s0)
{
  u_int val;
  val = s3 ? VIC_ICMS3_DIS : 0;
  val |= s2 ? VIC_ICMS2_DIS : 0;
  val |= s1 ? VIC_ICMS1_DIS : 0;
  val |= s0 ? VIC_ICMS0_DIS : 0;
  *VIC_ICMSICR |= val;
  wbflush ();
}

/*
 * Error group interrupt
 */
void
vic_errorinterrupt (int ipl)
{
  u_int val;
  val = VIC_ACFAIL_DIS | VIC_WPOST_DIS | VIC_ARBF_DIS | VIC_SYSF_DIS;
  *VIC_EGRPICR = val | (ipl & 7);  
  wbflush ();
}

void
vic_errorinterrupt_enable (int acfail, int wpost, int arbf, int sysf)
{
  u_int val;
  val = acfail ? VIC_ACFAIL_DIS : 0;
  val |= wpost ? VIC_WPOST_DIS : 0;
  val |= arbf ? VIC_ARBF_DIS : 0;
  val |= sysf ? VIC_SYSF_DIS : 0;
  *VIC_EGRPICR &= ~val;
  wbflush ();
}

void
vic_errorinterrupt_disable (int acfail, int wpost, int arbf, int sysf)
{
  u_int val;
  val = acfail ? VIC_ACFAIL_DIS : 0;
  val |= wpost ? VIC_WPOST_DIS : 0;
  val |= arbf ? VIC_ARBF_DIS : 0;
  val |= sysf ? VIC_SYSF_DIS : 0;
  *VIC_EGRPICR |= val;
  wbflush ();
}

/*
 * interrupt vectors
 */
void
vic_icgsvector (int n)
{
  *VIC_ICGSVEC = n & VIC_ICGSUMASK;
  wbflush ();
}

void
vic_icmsvector (int n)
{
  *VIC_ICMSVEC = n & VIC_ICMSUMASK;
  wbflush ();
}

void
vic_locvector (int n)
{
  *VIC_LOCVEC = n & VIC_LIRQUMASK;
  wbflush ();
}

void
vic_errorvector (int n)
{
  *VIC_EGRPVEC = n & VIC_EGRPUMASK;
  wbflush ();
}


u_char
vic_icsr ()
{
  return (*VIC_ICSR);
}

u_char
vic_icms ()
{
  return (*VIC_ICSR);
}

void
vic_icms_set (int n)
{
  *VIC_ICSR |= n;
  wbflush ();
}

int
vic_errorintr (src)
int src;
{
  switch (src) {
  case VIC_ACFAIL:
    cmn_err(CE_PANIC, "vicerr: ACFAIL*");
    break;
  case VIC_WPOSTVEC:
    cmn_err(CE_PANIC, "vicerr: failed writepost");
    break;
  case VIC_ARBFVEC:
    cmn_err(CE_PANIC, "vicerr: arbitration timeout");
    break;
  case VIC_SYSFVEC:
    cmn_err(CE_PANIC, "vicerr: SYSFAIL*");
    break;
  case VIC_VIIVEC:
    cmn_err(CE_PANIC, "vicerr: interrupter acknowledge");
    break;
  case VIC_DMAVEC:
    cmn_err(CE_PANIC, "vicerr: DMA completion");
    break;
  default:
    cmn_err(CE_PANIC, "vicerr: unrecognised interrupt %d", src);
    break;
  }
}

int
vic_icgsintr (swtch)
int swtch;
{
  cmn_err (CE_NOTE, "ICGS%d set", swtch);
  *VIC_ICSR &= ~(0x10 << swtch);
}

int
vic_icmsintr (swtch)
int swtch;
{
  cmn_err (CE_NOTE, "ICMS%d set", swtch);
  *VIC_ICSR &= ~(0x01 << swtch);
}

#ifndef _SDE_SOURCE
/*
 * initialise local interrupts
 */
void
vic_setup_local_interrupts ()
{
  extern int vic_locgrpvec;
  extern int vac_panicintr ();

  if (vic_locgrpvec & 7) {
    vic_locgrpvec &= ~7;
    cmn_err (CE_WARN, "Setting vic_locgrpvec to 0x%x", vic_locgrpvec);
  }

  vic_locinterrupt_disable(1);	/* Ethernet */

  vic_locinterrupt (2, 0, 1, 1, 4); /* timer */
  vic_locinterrupt_enable (2);

  vic_locinterrupt_disable (3);	/* SCSI  */
  vic_locinterrupt_disable (4);	/* vacser */
  vic_locinterrupt_disable (5);	/* not used */

  vic_locinterrupt (6, 0, 0, 1, 7); /* debug/parity */
  intrVecSet (vic_locgrpvec+6, vac_panicintr, 0);
  vic_locinterrupt_enable (6);

  vic_locinterrupt (7, 0, 0, 1, 6); /* profiler/mailbox */
  vic_locinterrupt_enable (7);

  vic_locvector (vic_locgrpvec);
}

void
vic_setup_error_interrupts ()
{
  extern int vic_errgrpvec;
  int i;
  if (vic_errgrpvec & 7) {
    vic_errgrpvec &= ~7;
    cmn_err (CE_WARN, "Setting vic_errgrpvec to 0x%x", vic_errgrpvec);
  }
  vic_errorvector (vic_errgrpvec);
  vic_errorinterrupt (7);
  for (i = 0; i < 6; i++) {
    intrVecSet (vic_errgrpvec+i, vic_errorintr, i);
  }

#ifdef TODO
  vic_errorinterrupt_enable (1, 1, 1, 1);
#else
  vic_errorinterrupt_enable (1, 1, 1, 0);
#endif
}
#endif

void
vic_setup_icms_interrupts ()
{
  extern int vic_icmsvec;
  int i;

  if (vic_icmsvec & 3) {
    vic_icmsvec &= ~3;
    cmn_err (CE_WARN, "Setting vic_icmsvec to 0x%x", vic_icmsvec);
  }
  vic_icmsvector (vic_icmsvec);
  vic_icmsinterrupt (4);
  for (i = 0; i < 4; i++) {
    intrVecSet (vic_icmsvec+i, vic_icmsintr, i);
  }
  vic_icmsinterrupt_enable (1, 1, 1, 1);
}


void
vic_setup_icgs_interrupts ()
{
  extern int vic_icgsvec;
  int i;
  if (vic_icgsvec & 3) {
    vic_icgsvec &= ~3;
    cmn_err (CE_WARN, "Setting vic_icgsvec to 0x%x", vic_icgsvec);
  }
  vic_icgsvector (vic_icgsvec);
  vic_icgsinterrupt (4);	/* spltty */
  for (i = 0; i < 4; i++) {
    intrVecSet (vic_icgsvec+i, vic_icgsintr, i);
  }
  vic_icgsinterrupt_enable (1, 1, 1, 1);
}

void
vic_setup_vme_interrupts ()
{
  int i;
  for (i = 1; i <= 7; i++) {
    vic_vmeinterrupt (i, i);
  }
}


void
vic_initrtclock ()
{
}

void
vic_startrtclock(hz)
int hz;
{
  int o;
  o = *VIC_SEL0CR0 & 0x3f;
  switch (hz) {
  default:
    cmn_err (CE_WARN,"unsupported HZ value %d",hz);
  case 50:
    *VIC_SEL0CR0 = VIC_SL0T50 | o;
    break;
  case 100:
    *VIC_SEL0CR0 = VIC_SL0T100 | o;
    break;
  case 1000:
    *VIC_SEL0CR0 = VIC_SL0T1000 | o;
    break;
  }
  wbflush ();
}

#ifndef _SDE_SOURCE
void
vic_ackrtclock ()
{
  extern int vic_locgrpvec;
  unsigned id;
  
  id = vicack4();
  ASSERT (id == (vic_locgrpvec + 2));
}
#endif

void
vic_stoprtclock ()
{
  *VIC_SEL0CR0 &= 0x3f;
  wbflush ();
}

/* initialise VMEbus etc */
#define streq(s1,s2) (strcmp((s1),(s2)) == 0)

void
vic_initialise ()
{
  int arbrcr, rcr;
  extern char *vmefair, *vmebr, *vmearb, *vmerel;

  if (vmefair) {
    if (streq (vmefair, "unfair"))
      arbrcr = VIC_UNFAIR;
    else if (streq (vmefair, "nofair"))
      arbrcr = VIC_NOFAIR;
    else {
      if (*atob (vmefair, &arbrcr) || arbrcr <= VIC_UNFAIR || arbrcr >= VIC_NOFAIR)
	cmn_err (CE_WARN, "bad argument for vmefair %s", vmefair);
      arbrcr = VIC_NOFAIR;
    }
  }
  else {
    arbrcr = VIC_NOFAIR;
  }

  if (vmebr) {
    if (streq (vmebr, "br0"))
      arbrcr |= VIC_BR0;
    else if (streq (vmebr, "br1"))
      arbrcr |= VIC_BR1;
    else if (streq (vmebr, "br2"))
      arbrcr |= VIC_BR2;
    else if (streq (vmebr, "br3"))
      arbrcr |= VIC_BR3;
    else {
      cmn_err (CE_WARN, "bad argument for vmefair %s", vmefair);
      arbrcr |= VIC_BR3;
    }
  }
  else
    arbrcr |= VIC_BR3;

  if (vmearb) {
    if (streq (vmearb, "pri"))
      arbrcr |= VIC_PRI;
    else if (streq (vmearb, "rrs"))
      arbrcr |= VIC_RRS;
    else if (streq (vmearb, "sgl")) {
      arbrcr |= VIC_PRI;
      arbrcr |= VIC_BR3;
    }
    else {
      cmn_err (CE_WARN, "bad argument for vmepri %s", vmearb);
      arbrcr |= VIC_PRI;
    }
  }
  else {
    arbrcr |= VIC_PRI;
  }
  *VIC_ARBREQ = arbrcr;

  if (vmerel) {
    if (streq (vmerel, "rwd"))
      rcr = VIC_RWD;
    else if (streq (vmerel, "ror"))
      rcr = VIC_ROR;
    else if (streq (vmerel, "roc"))
      rcr = VIC_ROC;
    else {
      cmn_err (CE_WARN, "bad argument for vmerel %s", vmerel);
      rcr = VIC_ROR;
    }
  }
  else {
    rcr = VIC_ROR;
  }
  *VIC_RCR = rcr;
}

#ifndef _SDE_SOURCE
int
sl3000_interrupt_init ()
{
}

int
sl3000_init ()
{
    extern void vac_perrctl(int);
    
  vac_initialise ();
  vic_initialise ();
  vic_setup_icms_interrupts ();
  vic_setup_icgs_interrupts ();
  vic_setup_local_interrupts ();
  vic_setup_error_interrupts ();
  vic_setup_vme_interrupts ();
#ifdef NVADDR_PARITY
  if (nvram_getbyte (NVADDR_PARITY) == 'y') {
    printf("Parity ENABLED\n");
    vac_perrctl (1);
  }
  else {
    printf("Parity DISABLED\n");
    vac_perrctl (0);
  }
#else
  vac_perrctl (1);
#endif
}
#endif

void sbd_reset(int debug)
{
  /* called from sbd_main with arg 0 */

  *(volatile unsigned int *)PHYS_TO_K1(BCRR) = 
#ifndef SL3000
    BCRR_AUBLK | BCRR_AUCLR | 
#endif
      BCRR_USART | BCRR_ETH | BCRR_SCSI;

  /* disable ALL interrupts*/
  (void) splhi ();

  /* clear out soft ints */
  mips_biccr(CR_SINT0|CR_SINT1);

  /* clear out the fpu (what if it doesn't exist?) */
  {
    unsigned s = mips_bissr(SR_CU1);
    fpa_setsr(0);
    mips_setsr(s);
  }
    
  vmefair = "nofair";
  vmebr   = "br3";
  vmearb  = "pri";
  vmerel = "ror";
    
  {
    vac_lowreset();
	
    config_delay();
#ifdef notdef
    /* serious: this takes us to the reset vector */
    *VIC_SYSRESET = VIC_RESETSYS;
#endif

    vic_initialise ();
    vic_stoprtclock();
    vic_setup_icms_interrupts ();
    vic_setup_icgs_interrupts ();
    /* local interrupts */
    {
      int i;

      for (i=1; i<8; i++)
	vic_locinterrupt_disable (i);	
	    
      vic_locinterrupt (1, 0, 0, 1, 3); /* Ethernet */
      vic_locinterrupt (2, 0, 1, 1, 4); /* timer */
      vic_locinterrupt (3, 0, 0, 1, 3); /* SCSI */
      vic_locinterrupt (4, 0, 0, 1, 2); /* VAC068 serial */
      vic_locinterrupt (5, 0, 0, 1, 2); /* MPSC serial */
      vic_locinterrupt (6, 0, 0, 1, 7); /* debug/parity */
      vic_locinterrupt (7, 0, 0, 1, 6); /* profiler/mailbox */
      vic_locvector (vic_locgrpvec);
	    
      vic_locinterrupt_enable (2); /* timer */
      vic_locinterrupt_enable (6); /* debug */
#ifdef notdef
      vic_locinterrupt_enable (7); /* prof/mailbox */
#endif
    }

    /* error interrupts */
    vic_errorvector (vic_errgrpvec);
    vic_errorinterrupt (7);
    vic_errorinterrupt_enable (0, 0, 0, 0);

    /* VME interrupts */
    vic_setup_vme_interrupts ();
	
    /* enable parity */
    vac_perrctl(1);
  }
    
  if(debug) {
    /* if not debug then we aint got a useful log device
     */
    syslog(LOG_DEBUG,"VME Board Reset\n");
    syslog(LOG_DEBUG,"VIC_ID=%x VAC_ID=%x VIC_VSTATUS=%x\n",
	*VIC_ID & 0xff, (*VACID >> 16)&0xffff, *VIC_VSTATUS&0xff);
  }

  /*
   * initialise vitab
   */
  {
    int i;
	
    for(i=0; i < 256; i++) {
      vitab[i].fun = vistray;
      vitab[i].arg = i;
    }
  }
    
  /* 
   * install interrupt exception handler for XCPT_INTR
   */
  xcption(XCPTINTR,intrhandler);
    
  /*
   * enable all interrupts
   */
  (void) spl0 ();
}


void sbd_unreset(void)
{
  /* put everything back in reset (except USART) */

  *(volatile unsigned int *)PHYS_TO_K1(BCRR) = BCRR_USART;

#ifdef CPU_R3000
  r3k_bicsr(SR_IMASK|SR_IEC);	/* disable ALL interrupts*/
#else
  r4k_bicsr(SR_IMASK|SR_IE);	/* disable ALL interrupts*/
#endif
  mips_biccr(CR_SINT0|CR_SINT1);  /* clear out soft ints */

  /* stop the clock */
  vic_stoprtclock();

  /* disable all local interrupts at VIC */
  {
    int i;
    for(i=1;i<8;i++)
      vic_locinterrupt_disable (i);
  }
	    
  /* disable all error interrupts at VIC */
  vic_errorinterrupt_enable (0, 0, 0, 0);

  /* disable parity */
  vac_perrctl(0);
}

/*
 * should probably go and disable it 
 * otherwise it wont go away by itself :-)
 */
static void vistray(int id)
{
    syslog(LOG_ERR,"Vic stray interrupt 0x%x\n", id);
}

void sbd_setivec(int ivec,void (*fun)(),void *arg)
{
    spl_t s;
    
    if(ivec > sizeof(vitab)/sizeof(vitab[0]) || ivec < VITABBASE) {
	syslog(LOG_ERR,"sbd_setivec: out of range ivec %d\n",ivec);
	return;
    }
    
    s = splhi ();
    if(fun) {
	vitab[ivec].arg = (int)arg;
	vitab[ivec].fun = fun;
    } else {
	vitab[ivec].fun = vistray;
	vitab[ivec].arg = ivec;
    }
    
    if(ivec >= vic_locgrpvec && ivec < vic_locgrpvec+8)
      if(fun)
	vic_locinterrupt_enable(ivec-vic_locgrpvec);	
      else
	vic_locinterrupt_disable(ivec-vic_locgrpvec);	
#ifdef notdef
    else if(ivec >= vic_errgrpvec && ivec < vic_errgrpvec+8)
    else if(ivec >= vic_icgsvec && ivec < vic_icgesvec+4)
    else if(ivec >= vic_icgsvec && ivec < vic_icgesvec+4)
#endif
    else
      syslog(LOG_WARNING,"sbd_setivec: dont grok ivec %d\n",ivec);
    
    (void) splx (s);
}

/*
 * Acknowledge VIC interrupt
 */
int
viciack (n)
int n;
{
  static void *iackaddrs[] = {
    0,
    PA_TO_KVA1(IACK_BASE2+2),
    PA_TO_KVA1(IACK_BASE2+4),
    PA_TO_KVA1(IACK_BASE2+6),
    PA_TO_KVA1(IACK_BASE2+8),
    PA_TO_KVA1(IACK_BASE2+10),
    PA_TO_KVA1(IACK_BASE2+12),
    PA_TO_KVA1(IACK_BASE2+14),
  };

#if BYTE_ORDER==BIG_ENDIAN
  if (n & 1)
    return (0xff & *(volatile unsigned short *)iackaddrs[n]);
  else
    return (0xff & *(volatile unsigned int *)iackaddrs[n]);
#endif
#if BYTE_ORDER==LITTLE_ENDIAN
  /* bit order preserved */
  if (n & 1)
    return (0xff & ((*(volatile unsigned short *)iackaddrs[n]) >> 8));
  else
    return (0xff & ((*(volatile unsigned int *)iackaddrs[n]) >> 24));
#endif
}

extern void softclkintr(int arg, struct xcptcontext *xcp);

static void vac_panicintr(struct xcptcontext *xcp);

static int intrhandler(int xcptno,struct xcptcontext *xcp)
{
    unsigned cr = xcp->cr;
    unsigned sr = xcp->sr;
    unsigned id;
    unsigned im;

    /* lop off the ones disabled at time of exception
     */
    cr &= (sr & SR_IMASK);
    
    if(cr & CR_HINT5) {
	id = viciack(7);		/* "panic" interrupt */
	vac_panicintr(xcp);
	return 0;
    }
    else if(cr & CR_HINT4) {		/* vic "prof"/vmeerr int */
	id = viciack(6);
	im = CR_HINT4|CR_HINT2|CR_HINT1|CR_HINT0|CR_SINT1|CR_SINT0;
    }
#if CPU_R3000
    else if(cr & CR_HINT3) {		/* cop1 fpu */
	unsigned s, fsr;    
	s = r3k_bissr(SR_CU1); 	/* ensure access to cop1 */
	fsr = fpa_xchsr(0); 	/* shut it up */
	r3k_setsr(s);		/* restore current access */
	syslog(LOG_WARNING,"fpa: interrupt fsr=%08x\n",fsr);
	return SIGFPE;
    }
#endif
    else if(cr & CR_HINT2) {		/* vic "clk" intr */
	id = viciack(4);
	im = CR_HINT2|CR_HINT1|CR_HINT0|CR_SINT1|CR_SINT0;
    }
    else if(cr & CR_HINT1) {		/* vic "bio" int */
	id = viciack(3);
	im = CR_HINT1|CR_HINT0|CR_SINT1|CR_SINT0;
    }
    else if(cr & CR_HINT0) {		/* vic "tty" int */
	id = viciack(2);
	im = CR_HINT0|CR_SINT1|CR_SINT0;
    }
    else if(cr & CR_SINT1) {		/* softintr 1 */
	mips_biccr(CR_SINT1);
	return SIGUSR2;
    }
    else if(cr & CR_SINT0) {		/* softintr 0 */
	mips_bissr(SR_IMASK ^ SR_SINT0);
	softclkintr (0, xcp);
	return 0;
    }
#if CPU_R4000
    else {
      /*
       * On the R4000 it is possible to get an interrupt exception in the
       * two instructions following the mtc0 that has disabled the interrupt
       */
      return (0);
    }
#endif
    
    /* id should be valid interrupt number 
     * im is the NEW interrupt mask which needs to be set
     */
    assert(id);

    /* enable all interrupts except those specified */
    mips_bissr(im ^ SR_IMASK);
    (vitab[id].fun)(vitab[id].arg);
    
    return 0;
}

/* cause remote debug to fire */
void debugButton(struct xcptcontext *xcp)
{
  __asm("break 0");
}

static void vac_panicintr(struct xcptcontext *xcp)
{
  uint vacisr;
    
  vacisr = *VACISR;
  /* VACRDELAY(); */
    
  if (vacisr & VAC_ISR_DEBUG) {
    vac_ackdebug ();
    syslog(LOG_ALERT,"Debug Button epc=%p\n",xcp->epc);
    debugButton(xcp);
    /*
     * any pending parity error will get handled on next interrupt
     * and this saves confusion
     */
    return;
  }   
  if (vacisr & VAC_ISR_PERR) {
    int errstat;

    errstat = *(volatile unsigned *)PHYS_TO_K1(ERRREG);
    vac_ackperr ();
    syslog(LOG_EMERG, "Parity error: 0x%08x\n", errstat);
    abort();
  } else {
    syslog(LOG_EMERG, "unexpected VAC068 panic interrupt 0x%08x\n", vacisr);
    abort();
  }
}


int itc_setup(int hz,void (*fun)())
{
    mips_bicsr(SR_HINT2);
    
    vic_stoprtclock();

    /* set the itc vic interrupt vector */
    sbd_setivec(vic_locgrpvec+2,fun,0);

    vic_startrtclock(100);

    /* and enable cpu interrupt
     * (NOTE locgrp+2 and SR_HINT2 is a coincidence)
     */
    mips_bissr(SR_HINT2);
    return 100;
}

/* few things which shouldnt be here */
/*
 * this has got to be fixed 
 * - depend on cpu MHZ
 */
static unsigned usDelayMult = 1;

/* hand tune delay */
static void config_delay(void)
{
  /* FIXME */
  if(IS_KVA0(mips_cycle))
#if CPU_R4000
    usDelayMult = 30;
#else
    usDelayMult = 18;
#endif
  else
    usDelayMult = 1;
}

void usdelay(register unsigned us)
{
  mips_cycle(us*usDelayMult);
}

void msdelay(register unsigned ms)
{
    while(ms--)
      usdelay(1000);
}

#ifdef notdef
/* XXX should have xcptsetjmp,xcptlongjmp 
*/
static xcptjmp_buf	memjb;

static int sbd_memcatch(int xcptno,struct xcptcontext *xcp)
{
    xcptlongjmp(memjb,1);
    return 0;
}

/* DESTRUCTIVE memory sizer
 * starting at sva try to extend memory size up return new limit
 */
void *sbd_memsize(void *sva)
{
    volatile struct xcptaction oact,nact;
    /* volatile so preserved across setjmp/longjmp */
    void	* volatile va;		
    
    /* catch a bus error on data accesses */
    nact.xa_handler 	= sbd_memcatch;
    nact.xa_flags 	= 0;
    xcptaction(XCPTDBE,&nact,&oact);
    
    if( xcptsetjmp(memjb) ) {
	xcptaction(XCPTDBE,&oact,0);
	va -= VMPGSIZE;
	return va;
    }
    
    for(va=sva; 1; va+=VMPGSIZE) {
	*(volatile unsigned*)va = 0;
	*(volatile unsigned*)va;
    }
}
#endif

void sbd_flashleds (int arg)
{
  if (arg & 0x08)
    *(volatile unsigned int *)PA_TO_KVA1(BCRR) |= BCRR_LBLK;
  else
    *(volatile unsigned int *)PA_TO_KVA1(BCRR) &= ~BCRR_LBLK;
}


static void myclktick ()
{
  extern void clktick(void);

  /* handle hardware clock */
  clktick ();

  /* schedule a soft clock interrupt */
  siron (CR_SINT0);
}

void softclkintr (int n, struct xcptcontext *xcp)
{
    extern int signalsblocked;
    int osignalsblocked;
    osignalsblocked = signalsblocked;
    signalsblocked = 1;

    /* disable soft clock interrupt */
    siroff (CR_SINT0);

    /* poll devices and schedule streams & timer events */
    sbd_extioproc ();

    signalsblocked = osignalsblocked;
}

/*
 * get called from standard rom crt0 never returns
 * do what needs to be done 
 */
void _sbd_main(int argc,char **argv,char **envp)
{
    extern int main(int argc,char **argv,char **envp);
    int nostreams;
    char *s;

    /* enable but mask interupts */
#if CPU_R4000
    r4k_bicsr(SR_IMASK|SR_ERL);
    r4k_bissr(SR_IE);
#else
    r3k_bicsr(SR_IMASK);
    r3k_bissr(SR_IEC);
#endif

    /* enables all interrupt CPU 's */
    sbd_reset(0);

    /* initialise environment */
    {
      extern char **environ;
      if (nvram_isvalid () == 0)
	nvram_initialise ();
      sbd_initenv ();
      /* environ may have moved so reset envp */
      envp = environ;
    }
    
    /* initialize timer */
    {
	extern void clkinit(unsigned);
	extern void clktick(void);
	extern void itc_setup(int,void (*fun)(void));
	
#define ITC_HZ		100
#define ITC_MSPTICK	1000/ITC_HZ
	
	clkinit(ITC_MSPTICK);
	itc_setup(ITC_HZ,myclktick);
    }
    
    /* get initial dumb console */
    if(open("/dev/console",O_RDWR) != 0) 
      abort();
    dup(0);
    dup(0);
    
    {
	int level = LOG_ERR;
	if (s = getenv ("loglevel")) {
	    level = strtoul (s, &s, 0);
	    if (*s || (level < LOG_EMERG) || (LOG_DEBUG < level))
	      level = LOG_DEBUG;
	}
	sysloginit (level);
    }

    /* initialise streams buffers */
    nostreams = dbinit();

#ifdef SCSI
    /* initialise scsi controller */
    _iscinit();
#endif

    /* get a tty (instead of a console) on stdin/stdout/stderr */
    /* FIXME: there shoudl be a better way of doing this
     *		maybe multiplex ttys like SPP ?
     */
    if (!nostreams) {
	int fd;
	char buf[10], c;
	
#ifdef MPSCCONSOLE	
	c = '2';
#else
	c = '0';
#endif
	if (s = getenv ("console"))
	  switch (*s) {
	  case '0':
	  case '1':
	  case '2':
	  case '3':
	      c = *s;
	      break;
	  }

	sprintf (buf, "/dev/tty%c", c);
	if ((fd=open(buf,O_RDWR)) < 0 &&
#ifdef MPSC
	    (fd=open("/dev/tty2",O_RDWR)) < 0 &&
#endif
	    (fd=open("/dev/tty0",O_RDWR)) < 0) {
	    perror("/dev/tty?");
	} else {
	    close(0); dup(fd);
	    close(1); dup(fd);
	    close(2); dup(fd);
	    close(fd);
	}
    }

    /* fix backspace handling */
    {
	struct termios tio;
	ioctl (0, TCGETA, &tio);
	tio.c_cc[VERASE] = 'H'&0x1f; /* CTRL(H) */
	tio.c_cc[VKILL] = 'U'&0x1f;	/* CTRL(U); */
	ioctl (0, TCSETA, &tio);
    }

    exit(main(argc,argv,envp));
}

