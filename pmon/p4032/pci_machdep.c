/* Very loosely based on: */
/*	$NetBSD: pci_machdep.c,v 1.17 1995/07/27 21:39:59 cgd Exp $	*/

/*
 * Copyright (c) 1994 Charles Hannum.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Charles Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Machine-specific functions for PCI autoconfiguration.
 */

#include "mips.h"
#include "pmon.h"
#include "stdio.h"

#include "sbd.h"
#include "v96xpbc.h"

#include "pci/pcivar.h"
#include "pci/pcireg.h"

#if #endian(little)
#define V96X_SWAP	V96X_SWAP_NONE
#define V96X_SWAP_IO	V96X_SWAP_NONE

#define ltohl(x) (x)
#define ltohs(x) (x)
#define htoll(x) (x)
#define htols(x) (x)
#endif

#if #endian(big)
#define V96X_SWAP	V96X_SWAP_8BIT
#define V96X_SWAP_IO	V96X_SWAP_AUTO

#define ltohl(x)		\
    ({				\
    unsigned long v = x;	\
    ((v & 0x000000ff) << 24) |	\
     ((v & 0x0000ff00) <<  8) |	\
     ((v >> 8)  & 0x0000ff00) |	\
     ((v >> 24) & 0x000000ff);	\
    })

#define ltohs(x)		\
    ({				\
    unsigned short v = x;	\
    (((v << 8) & 0xff00) |	\
     ((v >> 8) & 0x00ff));	\
    })

#define htoll(x) ltohl(x)
#define htols(x) ltohs(x)
#endif

/* PCI i/o region in PCI space */
#define PCI_IO_SPACE_PCI_BASE		0x00000000

/* PCI mem regions in PCI space (new configuration) */
#define PCI_MEM_SPACE_PCI_BASE_NEW	0x00000000
#define PCI_LOCAL_MEM_PCI_BASE_NEW	0x80000000

/* PCI mem regions in PCI space (old configuration) */
#define PCI_MEM_SPACE_PCI_BASE_OLD	0x10000000
#define PCI_LOCAL_MEM_PCI_BASE_OLD	0x00000000

/* soft versions of above */
static pcireg_t pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_NEW;
static pcireg_t pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_NEW;

/* PCI mem space allocation (note - skip the 1MB ISA mem space) */
static pcireg_t minpcimemaddr =  PCI_MEM_SPACE_PCI_BASE_NEW + 0x100000;
static pcireg_t nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE_NEW + PCI_MEM_SPACE_SIZE;

/* PCI i/o space allocation (note - bottom 64KB reserved for ISA i/o space) */
static pcireg_t minpciioaddr =  PCI_IO_SPACE_PCI_BASE + 0x10000;
static pcireg_t nextpciioaddr = PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE_NEW;

static int pcioldmap = 0;

static const struct pci_bus v96x_pci_bus = {
	0,		/* minimum grant */
	255,		/* maximum latency */
	0,		/* devsel time = fast */
	1,		/* we support fast back-to-back */
	1,		/* we support prefetch */
	0,		/* we don't support 66 MHz */
	0,		/* we don't support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	1		/* initially one device on bus (i.e. us) */
};

struct pci_bus _pci_bus[1];
int _pci_maxbus = 1;		/* maximum # buses we support */
extern int _pciverbose;

static unsigned char	v96x_vrev;
static int		v96x_bswap;

static int pcimaster;
static int pcireserved;

#ifdef IN_PMON
#define _sbd_getenv	sbd_getenv
#endif


/*
 * Called to initialise the bridge at the beginning of time
 */
int
_pci_hwinit (int initialise)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    unsigned char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    char *s;
    int tries;

    /* find out what devices we should access */
    {
	extern char *_sbd_getenv (const char *s);
	char *m = _sbd_getenv ("pcimaster");
	char *s = _sbd_getenv ("pcislave");

	pcimaster = (m != NULL);
	pcireserved = ~0x1fffff;	/* slots 0-20 accessible */

	/* for now... */
	if (m == NULL && s == NULL)
	    m = "";

	if (m == NULL && s == NULL) {
	    if (_pciverbose)
		printf ("PCI: set $pcimaster or $pcislave to enable\n");
	    return -1;
	}
	else if (m != NULL && s != NULL) {
	    printf ("PCI: $pcimaster and $pcislave can't both set!\n");
	    return -1;
	}
	else {
	    /* parse the string to find the devices to be skipped */
	    char *sp, *ep;
	    long device;
	    sp = m ? m : s;
	    while (*sp) {
		if (*sp == ',')
		    sp++;
		device = strtol (sp, &ep, 10);
		if (sp == ep || device < 0 || device > 31) {
		    printf ("Invalid $%s value %s\n", 
			    m ? "pcimaster" : "pcislave",
			    m ? m : s);
		    return -1;
		}
		pcireserved |= 1 << device;
		sp = ep;
	    }
	}
    }

    /* initialise data structures */
    _pci_bus[0] = v96x_pci_bus;

    if (((s = getenv ("pcimap")) || (s = getenv ("PCIMAP")))
	&& strcmp (s, "old") == 0)
	pcioldmap = 1;
    else
	pcioldmap = 0;

    if (pcioldmap) {
	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_OLD;
	pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_OLD;
    }
    else {
	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_NEW;
	pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_NEW;
    }

    /* point to start and end of region (leaving bottom 1M for ISA) */
    minpcimemaddr  = pci_mem_space_pci_base + 0x100000;
    nextpcimemaddr = pci_mem_space_pci_base + PCI_MEM_SPACE_SIZE;

    /* leave 64K at beginning of PCI i/o space for ISA devices (e.g. VGA) */
    minpciioaddr  = PCI_IO_SPACE_PCI_BASE + 0x10000;
    nextpciioaddr = PCI_IO_SPACE_PCI_BASE + 
	(pcioldmap ? PCI_IO_SPACE_SIZE_OLD : PCI_IO_SPACE_SIZE_NEW);

    if (!initialise) {
	/* get chip revision */
	v96x_vrev = V96X_PCI_CC_REV & V96X_PCI_CC_REV_VREV;
	/* probe for PROM byte swap setting */
	v96x_bswap = (V96X_LB_BASE0 & V96X_LB_BASEx_SWAP) != V96X_SWAP_NONE;
	/* no h/w initialisation required */
	return 0;
    }

    /* we define the setting */
    v96x_bswap = V96X_SWAP != V96X_SWAP_NONE;

    /* forcibly reset the V96x chip */
    bcr_bis (BCR_V96X_ENABLE);
    sbddelay (1);
    bcr_bic (BCR_V96X_ENABLE | BCR_ETH_ENABLE | BCR_SCSI_ENABLE);
    sbddelay (1);
    icu->ctrl.clear = INTR_ERR_BUSERR;
    bcr_bis (BCR_V96X_ENABLE);
    sbddelay (20000);	/* 20ms delay while it could be reading EEROM */

    /* tell it its base address */
    V96X_LB_IO_BASE = V96XPBC_BASE+0x6c;
    mips_wbflush ();

    /* check that it responds when we read its revision id */
    v96x_vrev = V96X_PCI_CC_REV & V96X_PCI_CC_REV_VREV;
    if (icu->irr.err & INTR_ERR_BUSERR) {
	icu->ctrl.clear = INTR_ERR_BUSERR;
	printf ("ERROR: V962PBC not responding after reset\n");
	return -1;
    }

    v96x_vrev = V96X_PCI_CC_REV & V96X_PCI_CC_REV_VREV;

    /* stop the V3 chip from servicing any further PCI requests */
    V96X_PCI_CMD = 0;
    mips_wbflush ();

    /* reset the PCI bus */
    V96X_SYSTEM &= ~V96X_SYSTEM_RST_OUT;
    
    /* enable bridge to PCI and PCI memory accesses, plus error handling */
    V96X_PCI_CMD = V96X_PCI_CMD_MASTER_EN
	| V96X_PCI_CMD_MEM_EN
	| V96X_PCI_CMD_SERR_EN
	| V96X_PCI_CMD_PAR_EN;

    /* clear errors and say we do fast back-to-back transfers */
    V96X_PCI_STAT = V96X_PCI_STAT_PAR_ERR
	| V96X_PCI_STAT_SYS_ERR	
	| V96X_PCI_STAT_M_ABORT	
	| V96X_PCI_STAT_T_ABORT	
	| V96X_PCI_STAT_PAR_REP	
	| V96X_PCI_STAT_FAST_BACK;

    /* Local to PCI aptr 0 - LOCAL:PCI_CONF_SPACE -> PCI:config (1MB) */
    V96X_LB_BASE0 = PCI_CONF_SPACE | V96X_SWAP | V96X_ADR_SIZE_1MB 
	| V96X_LB_BASEx_ENABLE;

    /* Local to PCI aptr 1 - LOCAL:PCI_MEM_SPACE -> PCI:10000000-xxxx */
    V96X_LB_BASE1 = PCI_MEM_SPACE | V96X_SWAP | V96X_ADR_SIZE_128MB 
	| V96X_LB_BASEx_ENABLE;
    V96X_LB_MAP1 = (pci_mem_space_pci_base >> 16) | V96X_LB_TYPE_MEM;

    /* PCI to local aptr 1 - PCI:80000000-90000000 -> LOCAL:00000000 */
    V96X_PCI_BASE1 = pci_local_mem_pci_base | V96X_PCI_BASEx_MEM;
    V96X_PCI_MAP1 =  0x00000000 | V96X_ADR_SIZE_256MB
	| V96X_SWAP /*| V96X_PCI_MAPx_RD_POST_INH*/
	| V96X_PCI_MAPx_REG_EN | V96X_PCI_MAPx_ENABLE;

    /* PCI to local aptr 0 - PCI:C0000000-D0000000 -> LOCAL:00000000 */
    V96X_PCI_BASE0 = 0xC0000000 | V96X_PCI_BASEx_MEM;
    V96X_PCI_MAP0 =  0x00000000 | V96X_ADR_SIZE_256MB
	| V96X_SWAP /*| V96X_PCI_MAPx_RD_POST_INH*/
	| V96X_PCI_MAPx_REG_EN | V96X_PCI_MAPx_ENABLE;

    if (v96x_vrev >= V96X_VREV_B2 && !pcioldmap) {
	/* Local to PCI aptr 2 - LOCAL:PCI_IO_SPACE -> PCI:00000000-xxxx */
	V96X_LB_BASE2 = (PCI_IO_SPACE_NEW >> 16) | (V96X_SWAP_IO >> 2)
	    | V96X_LB_BASEx_ENABLE;
	V96X_LB_MAP2 = (PCI_IO_SPACE_PCI_BASE >> 16);
    }

    /* PCI to internal registers - disabled (but avoid address overlap) */
    V96X_PCI_IO_BASE = 0xffffff00 | V96X_PCI_IO_BASE_IO;

    if (v96x_vrev >= V96X_VREV_B2)
	/* Disable PCI_IO_BASE and set optional AD(1:0) to 01b for 
	   type1 config */
	V96X_PCI_CFG = V96X_PCI_CFG_IO_DIS 
	    | (0x1 << V96X_PCI_CFG_AD_LOW_SHIFT);

    /* setup fifo config:
       empty write fifos at end of burst;
       refill read fifos when not full (any space);
       local bus max burst 8 words (XXX could be >, & be controlled by lb);
       pci bus max burst 256 words */
    V96X_FIFO_CFG = 
	(V96X_FIFO_CFG_BRST_256 << V96X_FIFO_CFG_PBRST_MAX_SHIFT)
	| (V96X_FIFO_CFG_WR_ENDBRST << V96X_FIFO_CFG_WR_LB_SHIFT)
	| (V96X_FIFO_CFG_RD_NOTFULL << V96X_FIFO_CFG_RD_LB1_SHIFT)
	| (V96X_FIFO_CFG_RD_NOTFULL << V96X_FIFO_CFG_RD_LB0_SHIFT)
	| (V96X_FIFO_CFG_BRST_8 << V96X_FIFO_CFG_LBRST_MAX_SHIFT)
	| (V96X_FIFO_CFG_WR_ENDBRST << V96X_FIFO_CFG_WR_PCI_SHIFT)
	| (V96X_FIFO_CFG_RD_NOTFULL << V96X_FIFO_CFG_RD_PCI1_SHIFT)
	| (V96X_FIFO_CFG_RD_NOTFULL << V96X_FIFO_CFG_RD_PCI0_SHIFT);
    
    /* Set fifo priorities: note that on Rev C.0 and above we set the
       read prefetch fifos to flush at the end of a burst, and not to
       retain data like a cache (which causes coherency problems).  For
       Rev B.2 and below we can't do this, so we set them to be
       flushed by any write cycle (inefficient but safer), and we also
       require explicit software flushing of the fifos to maintain
       full coherency (i.e. call pci_flush() from the cache flush
       routines or after modifying uncached descriptors). */

    /* initial setting, may be reprogrammed in pci_hwreinit(), below  */
    V96X_FIFO_PRIORITY = 
	V96X_FIFO_PRIORITY_LOCAL_WR /* local->pci write priority (safe) */
	| V96X_FIFO_PRIORITY_PCI_WR /* pci->local write priority (safe) */ 
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_LB_RD0_SHIFT)
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_LB_RD1_SHIFT)
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_PCI_RD0_SHIFT)
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_PCI_RD1_SHIFT);
    
#ifdef notyet
    /* enable long local bus timeout, followed by ERR/BTERM */
    V96X_LB_CFG = V96X_LB_CFG_TO_256 | V96X_LB_CFG_ERR_EN;
#endif

    /* finally unreset the PCI bus and the onboard PCI devices */
    V96X_SYSTEM |= V96X_SYSTEM_RST_OUT;
    bcr_bis (BCR_ETH_ENABLE | BCR_SCSI_ENABLE);
    sbddelay (1);

    return 1;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
_pci_hwreinit (void)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    unsigned int flush;

    if (_pci_bus[0].fast_b2b)
	/* fast back-to-back is supported by all devices */
	V96X_PCI_CMD |= V96X_PCI_CMD_FBB_EN;

    /* Rev B.0 bug: set maximum bus latency timer */
    V96X_PCI_HDR_CFG = 0xff << V96X_PCI_HDR_CFG_LT_SHIFT;

    if (v96x_vrev >= V96X_VREV_B1) {
	/* Rev B.1+: can now use variable latency timer */
	V96X_PCI_HDR_CFG = _pci_bus[0].def_ltim << V96X_PCI_HDR_CFG_LT_SHIFT;
    }

    if (v96x_vrev >= V96X_VREV_C0) {
	/* Rev C.0+: pci devices can now prefetch from local bus */
	V96X_PCI_BASE1 |= V96X_PCI_BASEx_PREFETCH;
	flush = (v96x_vrev >= V96X_VREV_C0)
	    ? V96X_FIFO_PRI_FLUSHBURST : V96X_FIFO_PRI_FLUSHALL;
	V96X_FIFO_PRIORITY = (V96X_FIFO_PRIORITY & ~V96X_FIFO_PRIORITY_PCI_RD1)
	    | (flush << V96X_FIFO_PRIORITY_PCI_RD1_SHIFT);
    }

    if (_pci_bus[0].prefetch && v96x_vrev >= V96X_VREV_C0) {
	/* Rev C.0+: we can safely prefetch from all pci mem devices */
	V96X_LB_BASE1 |= V96X_LB_BASEx_PREFETCH;
	V96X_FIFO_PRIORITY = (V96X_FIFO_PRIORITY & ~V96X_FIFO_PRIORITY_LB_RD1)
	    | (V96X_FIFO_PRI_FLUSHBURST << V96X_FIFO_PRIORITY_LB_RD1_SHIFT);
    }

    /* clear latched PCI interrupts */
    V96X_LB_ISTAT = 0;		

    /* enable PCI read/write error interrupts */
    V96X_LB_IMASK = V96X_LB_INTR_PCI_RD | V96X_LB_INTR_PCI_WR;

    /* enable V96 interrupt */
    imask_bis (INTR_DEV_V96);
}


void
_pci_flush (void)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);

    /* flush read-ahead fifos (not necessary on Rev C.0 and above) */
    if (v96x_vrev < V96X_VREV_C0)
	V96X_SYSTEM |= 
	    V96X_SYSTEM_LB_RD_PCI1 | V96X_SYSTEM_LB_RD_PCI0 |
	    V96X_SYSTEM_PCI_RD_LB1 | V96X_SYSTEM_PCI_RD_LB0;
}


/* Map the CPU virtual address of an area of local memory to a PCI
   address that can be used by a PCI bus master to access it. */
vm_offset_t
_pci_dmamap (vm_offset_t va, unsigned int len)
{
    return pci_local_mem_pci_base + KVA_TO_PA (va);
}

/* Map the PCI address of an area of local memory to a CPU physical
   address. */
vm_offset_t
_pci_cpumap (vm_offset_t pcia, unsigned int len)
{
    return pcia - pci_local_mem_pci_base;
}


pcitag_t
_pci_make_tag(int bus, int device, int function)
{
    pcitag_t tag;
    tag = (bus << 16) | (device << 11) | (function << 8);
    return tag;
}

void
_pci_break_tag(pcitag_t tag, int *busp, int *devicep, int *functionp)
{
    if (busp) *busp = (tag >> 16) & 255;
    if (devicep) *devicep = (tag >> 11) & 31;
    if (functionp) *functionp = (tag >> 8) & 7;
}

int
_pci_canscan(pcitag_t tag)
{
    int bus, dev;
    _pci_break_tag (tag, &bus, &dev, NULL);
    return !(bus == 0 && (pcireserved & (1 << dev)));
}

pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    u_int32_t addr, ad_low;
    pcireg_t data;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg 0x%x\r\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return ~0;	/* device out of range */
	addr = (0x800 << device) | (function << 8) | reg;
	ad_low = 0;
    }
    else if (v96x_vrev >= V96X_VREV_C0) {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return ~0;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	ad_low = V96X_LB_MAPx_AD_LOW_EN;
    }
    else {
	return ~0;		/* bus out of range */
    }

    /* high 12 bits of address go in map register, and set for conf space */
    V96X_LB_MAP0 = ((addr >> 16) & V96X_LB_MAPx_MAP_ADR) 
	| ad_low | V96X_LB_TYPE_CONF;

    /* clear aborts */
    V96X_PCI_STAT |= V96X_PCI_STAT_M_ABORT | V96X_PCI_STAT_T_ABORT; 

    mips_wbflush ();

    /* low 20 bits of address are in the actual address */
    data = *(volatile pcireg_t *) PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xfffff));

    if (V96X_PCI_STAT & V96X_PCI_STAT_M_ABORT) {
	V96X_PCI_STAT |= V96X_PCI_STAT_M_ABORT;
#if 0
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: master abort\r\n");
#endif
	return ~0;
    }

    if (V96X_PCI_STAT & V96X_PCI_STAT_T_ABORT) {
	V96X_PCI_STAT |= V96X_PCI_STAT_T_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: target abort\r\n");
	return ~0;
    }

    if (v96x_bswap)
	data = ltohl (data);

    return data;
}


void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    u_int32_t addr, ad_low;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg %x\r\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return;	/* device out of range */
	addr = (0x800 << device) | (function << 8) | reg;
	ad_low = 0;
    }
    else if (v96x_vrev >= V96X_VREV_C0) {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	ad_low = V96X_LB_MAPx_AD_LOW_EN;
    }
    else
	return;			/* bus out of range */

    /* high 12 bits of address go in map register; set conf space */
    V96X_LB_MAP0 = ((addr >> 16) & V96X_LB_MAPx_MAP_ADR)
	| ad_low | V96X_LB_TYPE_CONF;

    /* clear aborts */
    V96X_PCI_STAT |= V96X_PCI_STAT_M_ABORT | V96X_PCI_STAT_T_ABORT; 

    mips_wbflush ();

    if (v96x_bswap)
	data = htoll (data);

    /* low 20 bits of address are in the actual address */
    *(volatile pcireg_t *) PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xfffff)) = data;

    mips_wbflush ();

    /* wait for write FIFO to empty */
    while (V96X_FIFO_STAT & V96X_FIFO_STAT_L2P_WR)
	continue;

    if (V96X_PCI_STAT & V96X_PCI_STAT_M_ABORT) {
	V96X_PCI_STAT |= V96X_PCI_STAT_M_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: master abort\r\n");
    }

    if (V96X_PCI_STAT & V96X_PCI_STAT_T_ABORT) {
	V96X_PCI_STAT |= V96X_PCI_STAT_T_ABORT;
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: target abort\r\n");
    }
}


int
_pci_map_io(pcitag_t tag, int reg, vm_offset_t *vap, vm_offset_t *pap)
{
    pcireg_t address;
    vm_offset_t pa;

    if (v96x_vrev < V96X_VREV_B2 || pcioldmap) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_io: can't map i/o\r\n");
	return -1;
    }

    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	if (_pciverbose >= 1)
	    _pci_tagprintf(tag, "_pci_map_io: bad request\r\n");
	return -1;
    }

    address = _pci_conf_read(tag, reg);

    if ((address & PCI_MAP_IO) == 0) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_io: attempt to memory map a memory region\r\n");
	return -1;
    }

    pa = (address & PCI_MAP_IO_ADDRESS_MASK) - PCI_IO_SPACE_PCI_BASE;
    *pap = pa;

    *vap = (vm_offset_t) PA_TO_KVA1 (PCI_IO_SPACE_NEW + pa);

    if (_pciverbose >= 3)
	_pci_tagprintf (tag, "_pci_map_io: mapping io at virtual 0x%x, physical 0x%x\r\n", *vap, *pap);

    return 0;
}


int
_pci_map_mem(pcitag_t tag, int reg, vm_offset_t *vap, vm_offset_t *pap)
{
    pcireg_t address;
    vm_offset_t pa;

    if (reg == PCI_MAP_REG_ROM) {
	/* expansion ROM */
	address = _pci_conf_read(tag, reg);
	if (!(address & PCI_MAP_ROM)) {
	    _pci_tagprintf (tag, "_pci_map_mem: attempt to map missing rom\r\n");
	    return -1;
	}
	pa = address & PCI_MAP_ROM_ADDRESS_MASK;
    }
    else {
	if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: bad request\r\n");
	    return -1;
	}
	
	address = _pci_conf_read(tag, reg);

	if ((address & PCI_MAP_IO) != 0) {
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: attempt to memory map an I/O region\r\n");
	    return -1;
	}
	
	switch (address & PCI_MAP_MEMORY_TYPE_MASK) {
	case PCI_MAP_MEMORY_TYPE_32BIT:
	case PCI_MAP_MEMORY_TYPE_32BIT_1M:
	    break;
	case PCI_MAP_MEMORY_TYPE_64BIT:
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: attempt to map 64-bit region\r\n");
	    return -1;
	default:
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: reserved mapping type\r\n");
	    return -1;
	}
	pa = address & PCI_MAP_MEMORY_ADDRESS_MASK;
    }

    pa -= pci_mem_space_pci_base;
    *pap = pa;
    if (address & PCI_MAP_MEMORY_CACHABLE) 
	*vap = (vm_offset_t) PA_TO_KVA0 (PCI_MEM_SPACE + pa);
    else
	*vap = (vm_offset_t) PA_TO_KVA1 (PCI_MEM_SPACE + pa);

    if (_pciverbose >= 3)
	_pci_tagprintf (tag, "_pci_map_mem: mapping memory at virtual 0x%x, physical 0x%x\r\n", *vap, *pap);
    return 0;
}


void *
_pci_map_int(pcitag_t tag, pci_intrlevel level, int (*func) __P((void *)), void *arg)
{
    pcireg_t data;
    int pin, bus, device;

    data = _pci_conf_read(tag, PCI_INTERRUPT_REG);

    pin = PCI_INTERRUPT_PIN(data);

    if (pin == 0) {
	/* No IRQ used. */
	return NULL;
    }

    if (pin > 4) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_int: bad interrupt pin %d\r\n", pin);
	return NULL;
    }

    _pci_break_tag (tag, &bus, &device, NULL);

    if (bus != 0 || device > 20)
	return NULL;

#if 0
    /* XXX need to work this out based on device number etc. */
    line = x;
    return x;
#else
    _pci_tagprintf (tag, "_pci_map_int: attempt to map device %d pin %c\r\n", 
		     device, '@' + pin);
    return NULL;
#endif
}

pcireg_t
_pci_allocate_mem (pcitag_t tag, size_t size)
{
    pcireg_t address;

    /* allocate downwards, then round to size boundary */
    address = (nextpcimemaddr - size) & ~(size - 1);
    if (address > nextpcimemaddr || address < minpcimemaddr)
	return -1;
    nextpcimemaddr = address;
    return address;
}


pcireg_t
_pci_allocate_io (pcitag_t tag, size_t size)
{
    pcireg_t address;

    /* allocate downwards, then round to size boundary */
    address = (nextpciioaddr - size) & ~(size - 1);
    if (address > nextpciioaddr || address < minpciioaddr)
	return -1;
    nextpciioaddr = address;
    return address;
}
