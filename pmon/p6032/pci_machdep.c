/*
 * p6032/pci_machdep.c: Machine-specific functions for PCI autoconfiguration.
 *
 * Copyright (c) 2000-2001, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */

#if defined(IN_PMON)
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#else
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <mips/xcpt.h>
#include <mips/cpu.h>
#include "kit_impl.h"

#define printf _mon_printf
#endif

#include "pcivar.h"
#include "pcireg.h"

#include "sbd.h"

/* default PCI mem regions in PCI space */
#define PCI_MEM_SPACE_PCI_BASE		0x00000000
#define PCI_LOCAL_MEM_PCI_BASE		0x80000000
#define PCI_LOCAL_MEM_ISA_BASE		0x00800000

/* soft versions of above */
static pcireg_t const pci_mem_space = PCI_MEM_SPACE;	/* CPU base for access to mapped PCI memory */
static pcireg_t pci_mem_space_pci_base;			/* PCI mapped memory base */
static pcireg_t pci_local_mem_pci_base;			/* PCI address of local memory */
static pcireg_t pci_local_mem_isa_base;			/* PCI address of ISA accessible local memory */

static pcireg_t const pci_io_space = PCI_IO_SPACE;	/* CPU base for access to mapped PCI IO */
static pcireg_t const pci_io_space_pci_base = 0;	/* PCI mapped IO base */

/* PCI mem space allocation */
static pcireg_t minpcimemaddr;
static pcireg_t nextpcimemaddr;

/* PCI i/o space allocation */
static pcireg_t minpciioaddr;
static pcireg_t nextpciioaddr;

static const struct pci_bus bonito_pci_bus = {
	0,		/* minimum grant */
	255,		/* maximum latency */
	0,		/* devsel time = fast */
	0,		/* we don't support fast back-to-back */
	0,		/* we don't support prefetch */
	0,		/* we don't support 66 MHz */
	0,		/* we don't support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	1		/* initially one device on bus (i.e. us) */
};

extern int _pciverbose;

#define MAXBUS	3
const int _pci_maxbus = MAXBUS;		/* maximum # buses we support */
struct pci_bus _pci_bus[MAXBUS];

static char * const _bonito = PA_TO_KVA1(BONITO_BASE);

static int pcimaster;
static int pcireserved;

#if #endian(little)

#define ltohl(x) (x)
#define ltohs(x) (x)
#define htoll(x) (x)
#define htols(x) (x)
#endif

#if #endian(big)

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

/*
 * Called to initialise the bridge at the beginning of time
 */
int
_pci_hwinit (int initialise)
{
    /* find out what devices we should access */
    {
	extern char *_sbd_getenv (const char *s);
	char *m = _sbd_getenv ("pcimaster");
	char *s = _sbd_getenv ("pcislave");

	if (m == NULL && s == NULL)
	    m = "";

	pcimaster = (m != NULL);
	pcireserved = 1 << PCI_DEV_BONITO;

	if (m != NULL && s != NULL) {
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

    _pci_bus[0] = bonito_pci_bus;

    if (initialise && pcimaster) {
	/*
	 * We are initialising and we are the bus master
	 * so we get to define the mappings
	 */
	pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE;

	pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE;
	pci_local_mem_isa_base = PCI_LOCAL_MEM_ISA_BASE;

	/* point to start and end of region (leaving bottom 16M for ISA) */
	minpcimemaddr  = pci_mem_space_pci_base;
	if (minpcimemaddr < 0x1000000)
	    minpcimemaddr = 0x1000000;
	nextpcimemaddr = pci_mem_space_pci_base + PCI_MEM_SPACE_SIZE;

	/* leave 64KB at beginning of PCI i/o space for ISA bridge */
	minpciioaddr  = pci_io_space_pci_base;
	if (minpciioaddr < 0x10000)
	    minpciioaddr = 0x10000;
	nextpciioaddr = pci_io_space_pci_base + PCI_IO_SPACE_SIZE;
    }
    else {
	/* If we are not the master then wait for somebody to configure us */
	if (!pcimaster) {
	    /* wait for up to 3s until we get configured */
	    int i;
	    for (i = 0; i < 300; i++) {
		msdelay (10);
		if (BONITO_PCICMD & PCI_COMMAND_MEM_ENABLE)
		    break;
	    }
	    /* wait a little bit longer for the master scan to complete */
	    msdelay (100);
	}

	/* This assumes the address space is contiguous starting at PCIMAP_LO0 */
	pci_mem_space_pci_base = (BONITO_PCIMAP & BONITO_PCIMAP_PCIMAP_LO0) << (26 - BONITO_PCIMAP_PCIMAP_LO0_SHIFT);

	/* FIXME: If BARs get an enable bit then they should be checked here */
	pci_local_mem_pci_base = BONITO_PCIBASE0 & PCI_MAP_MEMORY_ADDRESS_MASK;

	/* assume BAR1 is suitable for 24 bit accesses */
	pci_local_mem_isa_base = BONITO_PCIBASE1 & PCI_MAP_MEMORY_ADDRESS_MASK;
    }


    if (initialise) {
	/* set up Local->PCI mappings */
	/* LOCAL:PCI_MEM_SPACE+00000000 -> PCI:pci_mem_space_pci_base#0x0c000000 */
	/* LOCAL:80000000	        -> PCI:80000000#0x80000000 */
	BONITO_PCIMAP =
	    BONITO_PCIMAP_WIN(0, pci_mem_space_pci_base+0x00000000) |	
	    BONITO_PCIMAP_WIN(1, pci_mem_space_pci_base+0x04000000) |
	    BONITO_PCIMAP_WIN(2, pci_mem_space_pci_base+0x08000000) |
	    BONITO_PCIMAP_PCIMAP_2;

	/* LOCAL:PCI_IO_SPACE -> PCI:00000000-00100000 */
	/* hardwired */
    }

    if (pcimaster) {
	/* set up PCI->Local mappings */

	/* pcimembasecfg has been set up by low-level code */

	/* Initialise BARs for access to our memory */
	/* PCI:pci_local_mem_pci_base -> LOCAL:0 */
	BONITO_PCIBASE0 = pci_local_mem_pci_base;

	/* PCI:pci_local_mem_isa_base -> LOCAL:0 */
	BONITO_PCIBASE1 = pci_local_mem_isa_base;

	/* PCI:PCI_LOCAL_MEM_PCI_BASE+10000000 -> LOCAL bonito registers (512b) */
	BONITO_PCIBASE2 = pci_local_mem_pci_base + 0x10000000;

	if (!initialise)
	    return 0;

#ifdef BONITO_PCICFG_PCIRESET
	/* reset the PCI bus */
	BONITO_PCICFG &= ~BONITO_PCICFG_PCIRESET;
	usdelay (1);

	/* unreset the PCI bus */
	BONITO_PCICFG |= BONITO_PCICFG_PCIRESET;
#endif
    }

    return initialise && pcimaster;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
_pci_hwreinit (void)
{
    if (pcimaster) {
	/* set variable latency timer */
	PCI_MISC_LTIM_SET(BONITO_PCILTIMER, _pci_bus[0].def_ltim);/* FIXME correct?*/

	if (_pci_bus[0].prefetch) {
	    /* we can safely prefetch from all pci mem devices */
	    /* FIXME how? */
	}
    }
}


void
_pci_flush (void)
{
    /* flush read-ahead fifos (!) */
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

void __inline__
_pci_break_tag(pcitag_t tag, int *busp, int *devicep, int *functionp)
{
    if (busp) *busp = (tag >> 16) & 255;
    if (devicep) *devicep = (tag >> 11) & 31;
    if (functionp) *functionp = (tag >> 8) & 7;
}

int
_pci_canscan (pcitag_t tag)
{
    int bus, dev;
    _pci_break_tag (tag, &bus, &dev, NULL);
    return !(bus == 0 && (pcireserved & (1 << dev)));
}

#if !defined(IN_VXWORKS) && !defined(IN_PMON)
#include <pthread.h>

static pthread_mutex_t	pci_cfg_mx = PTHREAD_MUTEX_INITIALIZER;

#define pci_cfg_lock() \
  (void) pthread_mutex_lock (&pci_cfg_mx)

#define pci_cfg_unlock() \
  (void) pthread_mutex_unlock (&pci_cfg_mx)

#else
#define pci_cfg_lock() 
#define pci_cfg_unlock() 
#endif

/*
 * flush Bonito register writes
 */
static void __inline__
bflush (void)
{
    (void)BONITO_PCIMAP_CFG;	/* register address is arbitrary */
}

pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    u_int32_t addr, type;
    pcireg_t data, stat;
    int bus, device, function;

    if ((reg & 3) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: bad reg 0x%x\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return ~0;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
	type = 0x00000;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return ~0;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }

    pci_cfg_lock ();
    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16) | type;
    bflush ();

    data = *(volatile pcireg_t *)PA_TO_KVA1(BONITO_PCICFG_BASE | (addr & 0xfffc));

    stat = BONITO_PCICMD;
    pci_cfg_unlock ();

    if (stat & PCI_STATUS_MASTER_ABORT) {
#if 0
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: reg=%x master abort\n", reg);
#endif
	return ~0;
    }

    if (stat & PCI_STATUS_MASTER_TARGET_ABORT) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: target abort\n");
	return ~0;
    }

#if defined(P6064)
    /* byte swapper enabled - data returned in little-endian order */
    return ltohl (data);
#else
    /* byte swapper disabled - data returned in host order */
    return data;
#endif
}


void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
    u_int32_t addr, type;
    pcireg_t stat;
    int bus, device, function;

    if ((reg & 3) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: bad reg %x\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function);

    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 20 || function > 7)
	    return;		/* device out of range */
	addr = (1 << (device+11)) | (function << 8) | reg;
	type = 0x00000;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 31 || function > 7)
	    return;	/* device out of range */
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
	type = 0x10000;
    }

    pci_cfg_lock ();
    /* clear aborts */
    BONITO_PCICMD |= PCI_STATUS_MASTER_ABORT | PCI_STATUS_MASTER_TARGET_ABORT;

    BONITO_PCIMAP_CFG = (addr >> 16) | type;
    bflush ();
#if defined(P6064)
    /* byte swapper enabled - data required in little-endian order */
    *(volatile pcireg_t *)PA_TO_KVA1(BONITO_PCICFG_BASE | (addr & 0xfffc)) = htoll (data);
#else
    /* byte swapper disabled - data required in host order */
    *(volatile pcireg_t *)PA_TO_KVA1(BONITO_PCICFG_BASE | (addr & 0xfffc)) = data;
#endif
    stat = BONITO_PCICMD;
    pci_cfg_unlock ();

    if (stat & PCI_STATUS_MASTER_ABORT) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: master abort\n");
    }

    if (stat & PCI_STATUS_MASTER_TARGET_ABORT) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: target abort\n");
    }
}

void
_pci_wbflush (void)
{
    (void)BONITO_PCICMD;
}

pcireg_t
_pci_statusread (void)
{
    return BONITO_PCICMD;
}

void
_pci_statuswrite (pcireg_t r)
{
    BONITO_PCICMD = r;
    (void)BONITO_PCICMD;
    
}


int
_pci_map_port(pcitag_t tag, int reg, unsigned int *port)
{
    pcireg_t address;
    
    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	if (_pciverbose >= 1)
	    _pci_tagprintf(tag, "_pci_map_port: bad request\n");
	return -1;
    }
    
    address = _pci_conf_read(tag, reg);
    
    if ((address & PCI_MAP_IO) == 0) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_port: attempt to i/o map a memory region\n");
	return -1;
    }

    *port = (address & PCI_MAP_IO_ADDRESS_MASK) - pci_io_space_pci_base;
    return 0;
}


int
_pci_map_io(pcitag_t tag, int reg, vm_offset_t *vap, vm_offset_t *pap)
{
    pcireg_t address;
    vm_offset_t pa;
    
    if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	if (_pciverbose >= 1)
	    _pci_tagprintf(tag, "_pci_map_io: bad request\n");
	return -1;
    }
    
    address = _pci_conf_read(tag, reg);
    
    if ((address & PCI_MAP_IO) == 0) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_io: attempt to i/o map a memory region\n");
	return -1;
    }

    pa = (address & PCI_MAP_IO_ADDRESS_MASK) - pci_io_space_pci_base;
    *pap = pa;
    *vap = (vm_offset_t) PA_TO_KVA1 (pci_io_space + pa);
    
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
	    _pci_tagprintf (tag, "_pci_map_mem: attempt to map missing rom\n");
	    return -1;
	}
	pa = address & PCI_MAP_ROM_ADDRESS_MASK;
    }
    else {
	if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3)) {
	    if (_pciverbose >= 1)
		_pci_tagprintf(tag, "_pci_map_mem: bad request\n");
	    return -1;
	}
	
	address = _pci_conf_read(tag, reg);
	
	if ((address & PCI_MAP_IO) != 0) {
	    if (_pciverbose >= 1)
		_pci_tagprintf(tag, "pci_map_mem: attempt to memory map an I/O region\n");
	    return -1;
	}
	
	switch (address & PCI_MAP_MEMORY_TYPE_MASK) {
	case PCI_MAP_MEMORY_TYPE_32BIT:
	case PCI_MAP_MEMORY_TYPE_32BIT_1M:
	    break;
	case PCI_MAP_MEMORY_TYPE_64BIT:
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: attempt to map 64-bit region\n");
	    return -1;
	default:
	    if (_pciverbose >= 1)
		_pci_tagprintf (tag, "_pci_map_mem: reserved mapping type\n");
	    return -1;
	}
	pa = address & PCI_MAP_MEMORY_ADDRESS_MASK;
    }

    
    pa -= pci_mem_space_pci_base;
    *pap = pa;
    if (address & PCI_MAP_MEMORY_CACHABLE) 
	*vap = (vm_offset_t) PA_TO_KVA0 (pci_mem_space + pa);
    else
	*vap = (vm_offset_t) PA_TO_KVA1 (pci_mem_space + pa);

    if (_pciverbose >= 3)
	_pci_tagprintf (tag, "_pci_map_mem: mapping memory at virtual 0x%x, physical 0x%x\n", 
			*vap, *pap);
    return 0;
}

#if !defined(IN_VXWORKS) && !defined(IN_PMON)
/* Return interrupt number */
int
_pci_map_int (pcitag_t tag)
{
    static const int sbdintrmap[4] = {INTR_PCI_A, INTR_PCI_B,
				      INTR_PCI_C, INTR_PCI_D};
    pcireg_t data;
    int pin, bus, device;

    data = _pci_conf_read(tag, PCI_INTERRUPT_REG);
    pin = PCI_INTERRUPT_PIN(data);
    if (pin == 0)
	/* No IRQ used. */
	return 0;

    if (--pin > 3) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_map_int: bad interrupt pin %d\r\n", 
			    pin);
	return -1;
    }

    /* ascend bus hierarchy until we get back to our (host) controller */
    _pci_break_tag (tag, &bus, &device, NULL);
    while (bus != 0) {
	/* XXX assume interrupt pins just pass straight through */
	tag = _pci_bus[bus].tag;
	_pci_break_tag (tag, &bus, &device, NULL);
    }

    switch (device) {
    case 16:	/* onboard ethernet */
	return INTR_ETH;
    case 13:	/* slot 2 */
    case 14:	/* slot 3 */
    case 15:	/* slot 4 */
    case 17:	/* south bridge */
	pin += device - 13;
	break;
    case 18:	/* slot 1 */
	pin += 3;
	break;
    default:
	return -1;
    }

    return sbdintrmap[pin % 4];
}
#endif

/*
 * allocate PCI address spaces
 * only applicable if we are the PCI master
 */

pcireg_t
_pci_allocate_mem (pcitag_t tag, size_t size)
{
    pcireg_t address;

    if (!pcimaster) {
	printf ("PCI: _pci_allocate_mem: not pcimaster\n");
	return -1;
    }

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

    if (!pcimaster) {
	printf ("PCI: _pci_allocate_io: not pcimaster\n");
	return -1;
    }

    /* allocate downwards, then round to size boundary */
    address = (nextpciioaddr - size) & ~(size - 1);
    if (address > nextpciioaddr || address < minpciioaddr)
	return -1;
    nextpciioaddr = address;
    return address;
}



/* 
 * Handle mapping of fixed ISA addresses 
 */

void *
_isa_map_io (unsigned int port)
{
    if (pci_io_space_pci_base != 0
	|| port > 0xffff)
	return 0;
   return PA_TO_KVA1 (pci_io_space + port);
}

void *
_isa_map_mem (vm_offset_t addr)
{
    if (pci_mem_space_pci_base != 0
	|| pci_local_mem_pci_base < 0x100000 
	|| addr > 0xfffff)
	return 0;
    return PA_TO_KVA1 (pci_mem_space + addr);
}
