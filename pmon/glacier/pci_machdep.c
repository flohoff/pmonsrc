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
#include "lifesaver.h"

#include "pci/pcivar.h"
#include "pci/pcireg.h"

/* PCI regions in PCI space */
#define PCI_MEM_SPACE_PCI_BASE	0x00000000	/* 256Mb */
#define PCI_IO_SPACE_PCI_BASE	0x10000000	/* 256Mb */


static pcireg_t nextpciioaddr;
static pcireg_t nextpcimemaddr;

static const struct pci_bus lsvr_pci_bus = {
	0,		/* minimum grant */
	255,		/* maximum latency */
	0,		/* devsel time = fast */
	0,		/* we support fast back-to-back */
	1,		/* we support prefetch */
	0,		/* we don't support 66 MHz */
	0,		/* we don't support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	1		/* initially one device on bus (i.e. us) */
};

#define MAXBUS	1
const int _pci_maxbus = MAXBUS;
struct pci_bus _pci_bus[MAXBUS];

static unsigned char lsvr_vrev;


/*
 * Called to initialise the bridge at the beginning of time
 */
int
pci_hwinit (int initialise)
{
    pLSVR;
    
    LSVR_BIU_CFG |= LSVR_BIU_CFG_PCI;

    /* check that it responds when we read its revision id */
    lsvr_vrev = LSVR_PCI_CC_REV & LSVR_PCI_CC_REV_VREV;

    /* enable bridge to PCI and PCI memory accesses, plus error handling */
    LSVR_PCI_CMD = LSVR_PCI_CMD_MASTER_EN
	| LSVR_PCI_CMD_MEM_EN
	| LSVR_PCI_CMD_SERR_EN
	| LSVR_PCI_CMD_PAR_EN;

    /* clear errors and say we do fast back-to-back transfers */
    LSVR_PCI_STAT = LSVR_PCI_STAT_PAR_ERR
	| LSVR_PCI_STAT_CONF_DONE	
	| LSVR_PCI_STAT_M_ABORT	
	| LSVR_PCI_STAT_T_ABORT	
	| LSVR_PCI_STAT_PAR_REP	
	| LSVR_PCI_STAT_FAST_BACK;

    /* initialise global data */
    nextpciioaddr = PCI_IO_SPACE_PCI_BASE;
    nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE;
    pci_bus[0] = lsvr_pci_bus;

    return initialise;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
pci_hwreinit (void)
{
    pLSVR;

    if (pci_bus[0].fast_b2b)
	/* fast back-to-back is supported by all devices */
	LSVR_PCI_CMD |= LSVR_PCI_CMD_FBB_EN;

    LSVR_PCI_HDR_CFG = pci_bus[0].def_ltim << LSVR_PCI_HDR_CFG_LT_SHIFT;

    /* FIXME: enable PCI interrupt */

}


pcitag_t
pci_make_tag(int bus, int device, int function)
{
    pcitag_t tag;
    tag = (bus << 16) | (device << 11) | (function << 8);
    return tag;
}

void
pci_break_tag(pcitag_t tag, int *busp, int *devicep, int *functionp)
{
    if (busp) *busp = (tag >> 16) & 7;
    if (devicep) *devicep = (tag >> 11) & 31;
    if (functionp) *functionp = (tag >> 8) & 7;
}


pcireg_t
pci_conf_read(tag, reg)
pcitag_t tag;
int reg;
{
    pLSVR;
    u_int32_t addr;
    pcireg_t data;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x100) {
	printf ("pci_conf_read: bad reg %x\n", reg);
	return ~0;
    }

    pci_break_tag (tag, &bus, &device, &function); 
    if (bus != 0)
	return ~0;	/* device out of range */

    /* clear aborts */
    LSVR_PCI_STAT |= LSVR_PCI_STAT_CONF_DONE
	|LSVR_PCI_STAT_M_ABORT | LSVR_PCI_STAT_T_ABORT; 

    LSVR_PCICONF_ADDR = LSVR_PCICONF_ADDR_E | addr;

    mips_wbflush ();

    while ((LSVR_PCI_STAT & LSVR_PCI_STAT_CONF_DONE) == 0)
	continue;

    data = LSVR_PCICONF_DATA;

    LSVR_PCI_STAT |= LSVR_PCI_STAT_CONF_DONE;

    if (LSVR_PCI_STAT & LSVR_PCI_STAT_M_ABORT) {
	LSVR_PCI_STAT |= LSVR_PCI_STAT_M_ABORT;
#if 0
	printf ("device %d: master abort\n", device);
#endif
	return ~0;
    }

    if (LSVR_PCI_STAT & LSVR_PCI_STAT_T_ABORT) {
	LSVR_PCI_STAT |= LSVR_PCI_STAT_T_ABORT;
	printf ("PCI slot %d: target abort!\n", device);
	return ~0;
    }

    return data;
}


void
pci_conf_write(tag, reg, data)
pcitag_t tag;
int reg;
pcireg_t data;
{
    pLSVR;
    u_int32_t addr;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x100) {
	printf ("pci_conf_read: bad reg %x\n", reg);
	return;
    }

    pci_break_tag (tag, &bus, &device, &function); 
    if (bus != 0)
	return;			/* device out of range */

    /* clear aborts */
    LSVR_PCI_STAT |= LSVR_PCI_STAT_CONF_DONE
	|LSVR_PCI_STAT_M_ABORT | LSVR_PCI_STAT_T_ABORT; 

    LSVR_PCICONF_ADDR = LSVR_PCICONF_ADDR_E | addr;
    LSVR_PCICONF_DATA = data;

    mips_wbflush ();

    while ((LSVR_PCI_STAT & LSVR_PCI_STAT_CONF_DONE) == 0)
	continue;

    LSVR_PCI_STAT |= LSVR_PCI_STAT_CONF_DONE;

    if (LSVR_PCI_STAT & LSVR_PCI_STAT_M_ABORT) {
	LSVR_PCI_STAT |= LSVR_PCI_STAT_M_ABORT;
	printf ("PCI slot %d: conf_write: master abort\n", device);
    }

    if (LSVR_PCI_STAT & LSVR_PCI_STAT_T_ABORT) {
	LSVR_PCI_STAT |= LSVR_PCI_STAT_T_ABORT;
	printf ("PCI slot %d: conf_write: target abort!\n", device);
    }
}


int
pci_map_io(tag, reg, iobasep)
	pcitag_t tag;
	int reg;
	int *iobasep;
{
    panic("pci_map_io: can't map i/o");
}


int
pci_map_mem(tag, reg, vap, pap)
	pcitag_t tag;
	int reg;
	vm_offset_t *vap;
	vm_offset_t *pap;
{
    pcireg_t address;
    vm_offset_t pa;

    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3))
	panic("pci_map_mem: bad request");

    address = pci_conf_read(tag, reg);

    if ((address & PCI_MAP_IO) != 0)
	panic("pci_map_mem: attempt to memory map an I/O region");

    switch (address & PCI_MAP_MEMORY_TYPE_MASK) {
    case PCI_MAP_MEMORY_TYPE_32BIT:
    case PCI_MAP_MEMORY_TYPE_32BIT_1M:
	break;
    case PCI_MAP_MEMORY_TYPE_64BIT:
	printf("pci_map_mem: attempt to map 64-bit region\n");
	return -1;
    default:
	printf("pci_map_mem: reserved mapping type\n");
	return -1;
    }

    pa = (address & PCI_MAP_MEMORY_ADDRESS_MASK) - PCI_MEM_SPACE_PCI_BASE;
    *pap = pa;

    if (address & PCI_MAP_MEMORY_CACHABLE) 
	*vap = (vm_offset_t) PA_TO_KVA0 (PCI_MEM_SPACE + pa);
    else
	*vap = (vm_offset_t) PA_TO_KVA1 (PCI_MEM_SPACE + pa);

#if 0
    printf("pci_map_mem: mapping memory at virtual %08x, physical %08x\n", *vap, *pap);
#endif
    return 0;
}


void *
pci_map_int(tag, level, func, arg)
	pcitag_t tag;
	pci_intrlevel level;
	int (*func) __P((void *));
	void *arg;
{
    pcireg_t data;
    int pin, line, bus, device;

    data = pci_conf_read(tag, PCI_INTERRUPT_REG);

    pin = PCI_INTERRUPT_PIN(data);

    if (pin == 0) {
	/* No IRQ used. */
	return NULL;
    }

    if (pin > 4) {
	printf("pci_map_int: bad interrupt pin %d\n", pin);
	return NULL;
    }


    pci_break_tag (tag, &bus, &device, NULL);

    if (bus != 0 || device > 20)
	return NULL;

#if 0
    /* XXX need to work this out based on device number etc. */
    line = x;
    return x;
#else
    printf("pci_map_int: attempt to map device %d pin %c\n", 
	   device, '@' + pin);
    return NULL;
#endif
}

pcireg_t
pci_allocate_mem (size)
size_t size;
{
    pcireg_t address;

    address = (nextpcimemaddr + size - 1) & ~(size - 1);
    nextpcimemaddr = address + size;
    if (nextpcimemaddr > PCI_MEM_SPACE_PCI_BASE + PCI_MEM_SPACE_SIZE)
	panic ("not enough PCI memory space");
    return address;
}


pcireg_t
pci_allocate_io (size)
size_t size;
{
    pcireg_t address;

    address = (nextpciioaddr + size - 1) & ~(size - 1);
    nextpciioaddr = address + size;
    if (nextpciioaddr > PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE)
	panic ("not enough PCI i/o space");
    return address;
}
