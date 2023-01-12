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

#include "p5064/sbd.h"
#include "p5064/v96xpbc.h"

#include "pci/pcivar.h"
#include "pci/pcireg.h"

#if #endian(little)
#define V96X_SWAP_MEM	V96X_SWAP_NONE
#define V96X_SWAP_IO	V96X_SWAP_NONE
#elif #endian(big)
#define V96X_SWAP_MEM	V96X_SWAP_8BIT
#define V96X_SWAP_IO	V96X_SWAP_AUTO
#endif

/* PCI i/o regions in PCI space */
#define PCI_IO_SPACE_PCI_BASE		0x00000000

/* PCI mem regions in PCI space */
#define PCI_MEM_SPACE_PCI_BASE_NEW	0x00000000
#define PCI_LOCAL_MEM_PCI_BASE_NEW	0x80000000
#define PCI_LOCAL_MEM_ISA_BASE_NEW	0x00800000

/* PCI mem regions in PCI space (old configuration) */
#define PCI_MEM_SPACE_PCI_BASE_OLD	0x10000000
#define PCI_LOCAL_MEM_PCI_BASE_OLD	0x00000000
#define PCI_LOCAL_MEM_ISA_BASE_OLD	0x00000000

/* soft versions of above */
static pcireg_t pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_NEW;
static pcireg_t pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_NEW;
static pcireg_t pci_local_mem_isa_base = PCI_LOCAL_MEM_ISA_BASE_NEW;

/* PCI mem space allocation (note - skip the 16MB ISA mem space) */
static pcireg_t minpcimemaddr =  PCI_MEM_SPACE_PCI_BASE_NEW + 0x1000000;
static pcireg_t nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE_NEW + PCI_MEM_SPACE_SIZE;

/* PCI i/o space allocation (note - bottom 512KB reserved for ISA i/o space) */
static pcireg_t minpciioaddr =  PCI_IO_SPACE_PCI_BASE + 0x80000;
static pcireg_t nextpciioaddr = PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE;

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

static const struct pci_bus secondary_pci_bus = {
	0,		/* minimum grant */
	255,	/* maximum latency */
	0,		/* devsel time = fast */
	0,		/* we don't fast back-to-back */
	0,		/* we don't prefetch */
	0,		/* we don't support 66 MHz */
	0,		/* we don't support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	0		/* initially no devices on bus */
};

extern int _pciverbose;

#define MAXBUS	3
const int _pci_maxbus = MAXBUS;
struct pci_bus _pci_bus[MAXBUS];

static unsigned char	v96x_vrev;


/*
 * Called to initialise the bridge at the beginning of time
 */
int
_pci_hwinit (int initialise)
{
    extern char *_sbd_getenv (const char *);
    unsigned char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    unsigned int pci_rd0, pci_rd1;
    int i;
    char *s;

    /* initialise global data */
    v96x_vrev = V96X_PCI_CC_REV & V96X_PCI_CC_REV_VREV;
    if (v96x_vrev < V96X_VREV_B2) {
	printf ("V96 revisions < B.2 not supported\n");
	return -1;
    }

    _pci_bus[0] = v96x_pci_bus;
    for (i = 1; i < MAXBUS; i++)
	_pci_bus[i] = secondary_pci_bus;

    if (((s = _sbd_getenv ("pcimap")) || (s = _sbd_getenv ("PCIMAP")))
	&& strcmp (s, "old") == 0)
	pcioldmap = 1;
    else
	pcioldmap = 0;

    if (pcioldmap) {
	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_OLD;
	pci_local_mem_isa_base = PCI_LOCAL_MEM_ISA_BASE_OLD;
	pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_OLD;
	/* point to start and end of region */
	minpcimemaddr  = PCI_MEM_SPACE_PCI_BASE_OLD;
	nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE_OLD + PCI_MEM_SPACE_SIZE;
    }
    else {
	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE_NEW;
	pci_local_mem_isa_base = PCI_LOCAL_MEM_ISA_BASE_NEW;
	pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE_NEW;
	/* point to start and end of region (leaving bottom 16M for ISA) */
	minpcimemaddr  = PCI_MEM_SPACE_PCI_BASE_NEW + 0x1000000;
	nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE_NEW + PCI_MEM_SPACE_SIZE;
    }

    /* leave 512K at beginning of PCI i/o space for ISA bridge (it
       actually uses only 64K, but this is needed for a ISA DMA 
       h/w fix which needs a higher address bit to spot ISA cycles). */
    minpciioaddr  = PCI_IO_SPACE_PCI_BASE + 0x80000;
    nextpciioaddr = PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE;

    if (!initialise)
	return 0;

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
    V96X_LB_BASE0 = PCI_CONF_SPACE | V96X_SWAP_IO | V96X_ADR_SIZE_1MB 
	| V96X_LB_BASEx_ENABLE;

    /* Local to PCI aptr 1 - LOCAL:PCI_MEM_SPACE -> PCI:00000000-xxxx */
    V96X_LB_BASE1 = PCI_MEM_SPACE | V96X_SWAP_MEM | V96X_ADR_SIZE_128MB 
	| V96X_LB_BASEx_ENABLE;
    V96X_LB_MAP1 = (pci_mem_space_pci_base >> 16) | V96X_LB_TYPE_MEM;

    /* Local to PCI aptr 2 - LOCAL:PCI_IO_SPACE -> PCI:00000000-xxxx */
    V96X_LB_BASE2 = (PCI_IO_SPACE >> 16) | (V96X_SWAP_IO >> 2)
	| V96X_LB_BASEx_ENABLE;
    V96X_LB_MAP2 = (PCI_IO_SPACE_PCI_BASE >> 16);

    /* PCI to local aptr 1 - PCI:80000000-90000000 -> LOCAL:00000000 */
    /* 256MB window for PCI bus masters to get at our local memory */
    V96X_PCI_BASE1 = pci_local_mem_pci_base | V96X_PCI_BASEx_MEM
	| V96X_PCI_BASEx_PREFETCH;
    V96X_PCI_MAP1 =  0x00000000 | V96X_ADR_SIZE_256MB
	| V96X_SWAP_MEM /*| V96X_PCI_MAPx_RD_POST_INH*/
	| V96X_PCI_MAPx_REG_EN | V96X_PCI_MAPx_ENABLE;
    pci_rd1 = (v96x_vrev >= V96X_VREV_C0)
	? V96X_FIFO_PRI_FLUSHBURST : V96X_FIFO_PRI_FLUSHALL;

    if (pcioldmap) {
	/* PCI to local aptr 0 - PCI:C0000000-D0000000 -> LOCAL:00000000 */
	/* Alternative non-prefetchable window for PCI bus masters */
	V96X_PCI_BASE0 = 0xC0000000 | V96X_PCI_BASEx_MEM;
	V96X_PCI_MAP0 =  0x00000000 | V96X_ADR_SIZE_256MB
	    | V96X_SWAP_MEM /*| V96X_PCI_MAPx_RD_POST_INH*/
	    | V96X_PCI_MAPx_REG_EN | V96X_PCI_MAPx_ENABLE;
	pci_rd0 = V96X_FIFO_PRI_NOFLUSH;
    }
    else {
	/* PCI to local aptr 0 - PCI:00800000-01000000 -> LOCAL:00000000 */
	/* 8MB window for ISA bus masters to get at our local memory */
	V96X_PCI_BASE0 = pci_local_mem_isa_base | V96X_PCI_BASEx_MEM
	    | V96X_PCI_BASEx_PREFETCH;
	V96X_PCI_MAP0 =  0x00000000 | V96X_ADR_SIZE_8MB
	    | V96X_SWAP_MEM /*| V96X_PCI_MAPx_RD_POST_INH*/
	    | V96X_PCI_MAPx_REG_EN | V96X_PCI_MAPx_ENABLE;
	pci_rd0 = (v96x_vrev >= V96X_VREV_C0)
	    ? V96X_FIFO_PRI_FLUSHBURST : V96X_FIFO_PRI_FLUSHALL;
    }

    /* PCI to internal registers - disabled (but avoid address overlap) */
    V96X_PCI_IO_BASE = 0xffffff00 | V96X_PCI_IO_BASE_IO;

    /* Disable PCI_IO_BASE and set optional AD(1:0) to 01b for type1 config */
    V96X_PCI_CFG = V96X_PCI_CFG_IO_DIS | (0x1 << V96X_PCI_CFG_AD_LOW_SHIFT);

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
	| (V96X_FIFO_CFG_BRST_16 << V96X_FIFO_CFG_LBRST_MAX_SHIFT)
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

    /* initial setting, may be updated in pci_hwreinit(), below  */
    V96X_FIFO_PRIORITY = 
	V96X_FIFO_PRIORITY_LOCAL_WR /* local->pci write priority (safe) */
	| V96X_FIFO_PRIORITY_PCI_WR /* pci->local write priority (safe) */ 
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_LB_RD0_SHIFT)
	| (V96X_FIFO_PRI_NOFLUSH << V96X_FIFO_PRIORITY_LB_RD1_SHIFT)
	| (pci_rd0 << V96X_FIFO_PRIORITY_PCI_RD0_SHIFT)
	| (pci_rd1 << V96X_FIFO_PRIORITY_PCI_RD1_SHIFT);
    
#if 0
    /* enable long local bus timeout, followed by ERR/BTERM */
    V96X_LB_CFG = V96X_LB_CFG_TO_256 | V96X_LB_CFG_ERR_EN;
#endif

    /* clear latched PCI interrupts */
    V96X_LB_ISTAT = 0;		

    /* enable V3 general interrupts */
    V96X_LB_IMASK = V96X_LB_INTR_PCI_RD|V96X_LB_INTR_PCI_WR;

    /* finally unreset the PCI bus */
    V96X_SYSTEM |= V96X_SYSTEM_RST_OUT;

    /* ... and the onboard PCI devices */
    {
	volatile p5064bcr1 * const bcr1 = PA_TO_KVA1 (BCR1_BASE);
	bcr1->eth = BCR1_ENABLE;
	bcr1->scsi = BCR1_ENABLE;
	bcr1->isa = BCR1_ENABLE;
	bcr1->pcmcia = BCR1_ENABLE;
	sbddelay (1);
    }

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

    if (_pci_bus[0].fast_b2b)
	/* fast back-to-back is supported by all devices */
	V96X_PCI_CMD |= V96X_PCI_CMD_FBB_EN;

    /* Rev B.1+: can now use variable latency timer */
    V96X_PCI_HDR_CFG = _pci_bus[0].def_ltim << V96X_PCI_HDR_CFG_LT_SHIFT;

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

}

void
_pci_flush (void)
{
    /* flush read-ahead fifos (not necessary on Rev C.0 and above) */
    if (v96x_vrev < V96X_VREV_C0) {
	char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
	V96X_SYSTEM |= 
	    V96X_SYSTEM_LB_RD_PCI1 | V96X_SYSTEM_LB_RD_PCI0 |
	    V96X_SYSTEM_PCI_RD_LB1 | V96X_SYSTEM_PCI_RD_LB0;
    }
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


/* Map the CPU virtual address of an area of local memory to an ISA
   address that can be used by a ISA bus master to access it. */
vm_offset_t
_isa_dmamap (vm_offset_t va, unsigned int len)
{
    unsigned long pa = KVA_TO_PA (va);

    /* restrict ISA DMA access to bottom 8/16MB of local memory */
    if (pa + len > 0x1000000 - pci_local_mem_isa_base)
	return (vm_offset_t)-1;
    return pci_local_mem_isa_base + pa;
}

/* Map the ISA address of an area of local memory to a CPU physical
   address. */
vm_offset_t
_isa_cpumap (vm_offset_t pcia, unsigned int len)
{
    return pcia - pci_local_mem_isa_base;
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
    if (busp) *busp = (tag >> 16) & PCI_BUSMAX;
    if (devicep) *devicep = (tag >> 11) & PCI_DEVMAX;
    if (functionp) *functionp = (tag >> 8) & PCI_FUNCMAX;
}

int
_pci_canscan (pcitag_t tag)
{
    return (1);
}

static pcireg_t
_pci_conf_readn(pcitag_t tag, int reg, int width)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    u_int32_t addr, ad_low;
    void *addrp;
    pcireg_t data;
    int bus, device, function;

    if (reg & (width-1) || reg < 0 || reg >= PCI_REGMAX) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_readn: bad reg 0x%x\r\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 5 || function > 7)
	    return ~0;		/* device out of range */
	addr = (1 << (device+24)) | (function << 8) | reg;
	ad_low = 0;
    }
    else if (v96x_vrev >= V96X_VREV_C0) {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > PCI_BUSMAX || device > PCI_DEVMAX || function > PCI_FUNCMAX)
	    return ~0;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	ad_low = V96X_LB_MAPx_AD_LOW_EN;
    }
    else {
	return ~0;		/* bus out of range */
    }

    /* high 12 bits of address go in map register; set conf space */
    V96X_LB_MAP0 = ((addr >> 16) & V96X_LB_MAPx_MAP_ADR)
	| ad_low | V96X_LB_TYPE_CONF;

    /* clear aborts */
    V96X_PCI_STAT |= V96X_PCI_STAT_M_ABORT | V96X_PCI_STAT_T_ABORT; 

    mips_wbflush ();

    /* low 20 bits of address are in the actual address */
    addrp = PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xfffff));
    switch (width) {
    case 1:
	data = (pcireg_t)*(volatile unsigned char *)addrp;
	break;
    case 2:
	data = (pcireg_t)*(volatile unsigned short *)addrp;
	break;
    default:
    case 4:
	data = (pcireg_t)*(volatile unsigned int *)addrp;
	break;
    }

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

    return data;
}


pcireg_t
_pci_conf_read8(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 1);
}

pcireg_t
_pci_conf_read16(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 2);
}

#ifndef _pci_conf_read32
pcireg_t
_pci_conf_read32(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 4);
}
#endif

pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 4);
}

static void
_pci_conf_writen(pcitag_t tag, int reg, pcireg_t data, int width)
{
    char * const _v96xp = PA_TO_KVA1(V96XPBC_BASE);
    u_int32_t addr, ad_low;
    void *addrp;
    int bus, device, function;

    if (reg & (width-1) || reg < 0 || reg > PCI_REGMAX) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg %x\r\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 5 || function > 7)
	    return;		/* device out of range */
	addr = (1 << (device+24)) | (function << 8) | reg;
	ad_low = 0;
    }
    else if (v96x_vrev >= V96X_VREV_C0) {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > PCI_BUSMAX || device > PCI_DEVMAX || function > PCI_FUNCMAX)
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

    /* low 20 bits of address are in the actual address */
    addrp = PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xfffff));
    switch (width) {
    case 1:
	*(volatile unsigned char *)addrp = data;
	break;
    case 2:
	*(volatile unsigned short *)addrp = data;
	break;
    default:
    case 4:
	*(volatile unsigned int *)addrp = data;
	break;
    }

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

void
_pci_conf_write8(pcitag_t tag, int reg, pcireg_t data)
{
    _pci_conf_writen (tag, reg, data, 1);
}

void
_pci_conf_write16(pcitag_t tag, int reg, pcireg_t data)
{
    _pci_conf_writen (tag, reg, data, 2);
}

#ifndef _pci_conf_write32
void
_pci_conf_write32(pcitag_t tag, int reg, pcireg_t data)
{
    _pci_conf_writen (tag, reg, data, 4);
}
#endif

void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
    _pci_conf_writen (tag, reg, data, 4);
}



int
_pci_map_port(pcitag_t tag, int reg, unsigned int *port)
{
    pcireg_t address;
    
    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	if (_pciverbose >= 1)
	    _pci_tagprintf(tag, "_pci_map_port: bad request\r\n");
	return -1;
    }
    
    address = _pci_conf_read(tag, reg);
    
    if ((address & PCI_MAP_IO) == 0) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_port: attempt to i/o map a memory region\r\n");
	return -1;
    }

    *port = (address & PCI_MAP_IO_ADDRESS_MASK) - PCI_IO_SPACE_PCI_BASE;
    return 0;
}


int
_pci_map_io(pcitag_t tag, int reg, vm_offset_t *vap, vm_offset_t *pap)
{
    pcireg_t address;
    vm_offset_t pa;
    
    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	if (_pciverbose >= 1)
	    _pci_tagprintf(tag, "_pci_map_io: bad request\r\n");
	return -1;
    }
    
    address = _pci_conf_read(tag, reg);
    
    if ((address & PCI_MAP_IO) == 0) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_io: attempt to i/o map a memory region\r\n");
	return -1;
    }

    pa = (address & PCI_MAP_IO_ADDRESS_MASK) - PCI_IO_SPACE_PCI_BASE;
    *pap = pa;
    *vap = (vm_offset_t) PA_TO_KVA1 (PCI_IO_SPACE + pa);
    
    if (_pciverbose >= 3)
	_pci_tagprintf(tag, "_pci_map_io: mapping i/o at virtual %08x, physical %08x\n", 
		       *vap, *pap);

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
		_pci_tagprintf(tag, "_pci_map_mem: bad request\r\n");
	    return -1;
	}
	
	address = _pci_conf_read(tag, reg);
	
	if ((address & PCI_MAP_IO) != 0) {
	    if (_pciverbose >= 1)
		_pci_tagprintf(tag, "pci_map_mem: attempt to memory map an I/O region\r\n");
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
	_pci_tagprintf (tag, "_pci_map_mem: mapping memory at virtual 0x%x, physical 0x%x\r\n", 
			*vap, *pap);
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

    if (bus != 0 || device > 5)
	return NULL;

    /* XXX need to work this out based on device number etc. */
    _pci_tagprintf(tag, "_pci_map_int: attempt to map device %d pin %c\n", 
		   device, '@' + pin);
    return NULL;
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


void *
iodev_map (unsigned int port)
{
    return PA_TO_KVA1 (ISAPORT_BASE(port));
}

unsigned char
inb (unsigned int port)
{
    return *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port));
}

unsigned short
inw (unsigned int port)
{
    return *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port));
}

unsigned long
inl (unsigned int port)
{
    return *(volatile unsigned long *) PA_TO_KVA1 (ISAPORT_BASE(port));
}

void
outb (unsigned int port, unsigned char val)
{
    *(volatile unsigned char *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
    mips_wbflush ();
}

void
outw (unsigned int port, unsigned short val)
{
    *(volatile unsigned short *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
    mips_wbflush ();
}

void
outl (unsigned int port, unsigned long val)
{
    *(volatile unsigned long *) PA_TO_KVA1 (ISAPORT_BASE(port)) = val;
    mips_wbflush ();
}
