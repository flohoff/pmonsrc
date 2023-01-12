/*
 * NEC5074/pci_machdep.c: Machine-specific functions for PCI autoconfiguration.
 * Copyright (c) 1999	Algorithmics Ltd
 */


#ifdef IN_PMON
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#else
#include <stdlib.h>
#include <sys/types.h>
#include <mips/xcpt.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif

#include "../pci/pcivar.h"
#include "../pci/pcireg.h"
#include "sbd.h"
#include "vrc5074.h"

static volatile struct vrc5074 * const n4 
	= PA_TO_KVA1(VRC5074_BASE);

static volatile struct vrc5074_pcicfg * const n4p 
	= PA_TO_KVA1(VRC5074_BASE + N4_PCICFG);

#define PCIWIND0_CONF	(PCI_CONF_SPACE | N4_PDAR_64BIT | N4_PDAR_64MB)
#define PCIWIND0_IO	(PCI_IO_SPACE   | N4_PDAR_64BIT | N4_PDAR_32MB)
#define PCIWIND1_MEM	(PCI_MEM_SPACE  | N4_PDAR_64BIT | N4_PDAR_128MB)
#define PCIINIT0_CONF	(N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_CONF)
#define PCIINIT0_IO	(N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_IO)
#define PCIINIT1_MEM	(N4_PCIINIT_32BIT | N4_PCIINIT_TYPE_MEM)

/* PCI i/o regions in PCI space */
#define PCI_IO_SPACE_PCI_BASE		0x00000000

/* PCI mem regions in PCI space */
#define PCI_MEM_SPACE_PCI_BASE		0x00000000	/* 128MB pci mem */
#define PCI_LOCAL_MEM_PCI_BASE		0x08000000	/* 64MB local mem */

/* soft versions of above */
static pcireg_t pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE;
static pcireg_t pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE;

/* PCI mem space allocation (note - bottom 16MB resvd for ISA mem space) */
static pcireg_t minpcimemaddr =  PCI_MEM_SPACE_PCI_BASE + 0x1000000;
static pcireg_t nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE + PCI_MEM_SPACE_SIZE;

/* PCI i/o space allocation (note - bottom 64KB resvd for ISA i/o space) */
static pcireg_t minpciioaddr =  PCI_IO_SPACE_PCI_BASE + 0x10000;
static pcireg_t nextpciioaddr = PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE;

/* Devices to skip */
static unsigned long reserveddevices;

static const struct pci_bus n4_pci_bus = {
	0,		/* minimum grant */
	255,		/* maximum latency */
	1,		/* devsel time = medium */
	1,		/* we support fast back-to-back */
	1,		/* we support prefetch */
	0,		/* we don't (yet!) support 66 MHz */
	0,		/* we don't (yet!) support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	1		/* initially one device on bus (i.e. us) */
};

extern int _pciverbose;

#define MAXBUS	3
const int _pci_maxbus = MAXBUS;
struct pci_bus _pci_bus[MAXBUS];

static int n4_rev;


int
_pci_cacheline_log2 (void)
{
    /* 8 words == 2^3 */
    return 3;
}


int
_pci_maxburst_log2 (void)
{
    /* 8 words == 2^3 */
    return 3;
}


/*
 * Called to initialise the bridge at the beginning of time
 */
int
_pci_hwinit (int initialise)
{
    char *m;

    /* find out what devices we should skip, if any */
    reserveddevices = ((1 << 7) 	/* skip ourselves! */
		       | (~0 << 10));	/* skip devices >= 10 */

    if (m = getenv ("pcimaster")) {
	char *sp = m;
	while (*sp) {
	    long device;
	    char *ep;
	    if (*sp == ',')
		sp++;
	    device = strtol (sp, &ep, 10);
	    if (sp == ep || device < 0 || device > 31) {
#ifdef IN_PMON
		printf ("Invalid $pcimaster value: %s\n", m);
#else
		_mon_printf ("Invalid $pcimaster value: %s\n", m);
#endif
		return -1;
	    }
	    reserveddevices |= 1 << device;
	    sp = ep;
	}
    }

    /* initialise global data */
    _pci_bus[0] = n4_pci_bus;

    n4_rev = (n4p->n4p_class >> PCI_REVISION_SHIFT) & PCI_REVISION_MASK;
    if (n4_rev < 3)
	_pci_bus[0].fast_b2b = _pci_bus[0].prefetch = 0;

    pci_local_mem_pci_base = PCI_LOCAL_MEM_PCI_BASE;
    pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE;

    /* point to base and end of pci mem space (bottom 16M resvd for ISA) */
    minpcimemaddr  = PCI_MEM_SPACE_PCI_BASE + 0x1000000;
    nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE + PCI_MEM_SPACE_SIZE;

    /* point to base and end of pci i/o space (bottom 64K resvd for ISA) */
    minpciioaddr  = PCI_IO_SPACE_PCI_BASE + 0x10000;
    nextpciioaddr = PCI_IO_SPACE_PCI_BASE + PCI_IO_SPACE_SIZE;

    if (!initialise)
	return 0;

#if 0
    /* all this should have been done already by sbdreset.S */
    /* Local to PCI aptr 0 - LOCAL:PCI_IO_SPACE -> PCI mem:00000000-xxx */
    n4->n4_pciw0 	= PCIWIND0_IO;
    n4->n4_pciinit0 	= PCIINIT0_IO;

    /* Local to PCI aptr 1 - LOCAL:PCI_MEM_SPACE -> PCI io:00000000-xxxx */
    n4->n4_pciw1 	= PCIWIND1_MEM;
    n4->n4_pciinit1 	= PCIINIT1_MEM;
#endif

    /* enable PCI prefetch from local memory, if working */
    if (n4_rev >= 3)
	n4p->n4p_bar0 |= PCI_MAP_MEMORY_CACHABLE;

    /* set "cache line" (prefetch) size */
    n4p->n4p_clsiz = 1 << _pci_cacheline_log2();

    /* disable PCI read/write error capture & interrupts */
    n4->n4_pcictrl &= ~(N4_PCICTRL_TAIN | N4_PCICTRL_MAIN
	| N4_PCICTRL_RTYIN | N4_PCICTRL_PERIN
	| N4_PCICTRL_DTIMIN | N4_PCICTRL_AERIN
    	| N4_PCICTRL_TACH | N4_PCICTRL_MACH
	| N4_PCICTRL_RTYCH | N4_PCICTRL_PERCH
	| N4_PCICTRL_DTIMCH);
    
    /* disable PCI Parity & SERR error checking */
    n4p->n4p_cmd &= ~(PCI_COMMAND_SERR_ENABLE | PCI_COMMAND_PARITY_ENABLE);

    /* clear errors */
    n4->n4_pcierr = 0;

    return initialise;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
_pci_hwreinit (void)
{

    if (_pci_bus[0].fast_b2b)
	/* fast back-to-back is supported by all devices */
	n4p->n4p_cmd |= PCI_COMMAND_BACKTOBACK_ENABLE;

    if (_pci_bus[0].prefetch)
	/* if we can safely prefetch from all pci mem devices (unlikely) */
	n4->n4_pciinit1 |= N4_PCIINIT_PREFETCHABLE 
	    | N4_PCIINIT_MERGING | N4_PCIINIT_COMBINING;

    /* clear latched PCI interrupts */
    n4->n4_pcierr = 0;

    /* enable PCI read/write error capture & interrupts */
    n4->n4_pcictrl |= N4_PCICTRL_TAIN | N4_PCICTRL_MAIN
	| N4_PCICTRL_RTYIN | N4_PCICTRL_PERIN
	| N4_PCICTRL_DTIMIN | N4_PCICTRL_AERIN
    	| N4_PCICTRL_TACH | N4_PCICTRL_MACH
	| N4_PCICTRL_RTYCH | N4_PCICTRL_PERCH
	| N4_PCICTRL_DTIMCH;
    
    /* enable PCI Parity & SERR error checking */
    n4p->n4p_cmd |= PCI_COMMAND_SERR_ENABLE | PCI_COMMAND_PARITY_ENABLE;

    mips_wbflush ();
}

void
_pci_flush (void)
{
    /* flush read-ahead fifos */
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
    return (vm_offset_t)-1;
}

/* Map the ISA address of an area of local memory to a CPU physical
   address. */
vm_offset_t
_isa_cpumap (vm_offset_t pcia, unsigned int len)
{
    return (vm_offset_t)-1;
}


pcitag_t
_pci_make_tag(int bus, int device, int function)
{
    pcitag_t tag;
    tag = (bus << 16) | (device << 11) | (function << 8);
    return tag;
}

__inline__ void
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
    return !(bus == 0 && (reserveddevices & (1 << dev)));
}

static pcireg_t
_pci_conf_readn(pcitag_t tag, int reg, int width)
{
    u_int64_t type, err;
    u_int32_t addr;
    void *addrp;
    pcireg_t data;
    int bus, device, function;

    if (reg & (width-1) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_readn: bad reg 0x%x\r\n", reg);
	return ~0;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 9 || function > 7)
	    return ~0;		/* device out of range */
	type = N4_PCIINIT_CONFIGTYPE0;
	addr = (1 << (device+16)) | (function << 8) | reg;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 15 || function > 7)
	    return ~0;	/* device out of range */
	type = N4_PCIINIT_CONFIGTYPE1;
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
    }

    n4->n4_pciw0 = PCIWIND0_CONF;
    n4->n4_pciinit0 = PCIINIT0_CONF | type;
    n4->n4_pcierr = 0;
    mips_wbflush ();

    addrp = PA_TO_KVA1(PCI_CONF_SPACE + addr);
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

    n4->n4_pciw0 = PCIWIND0_IO;
    n4->n4_pciinit0 = PCIINIT0_IO;
    mips_wbflush ();

    if (err = (n4->n4_pcictrl & N4_PCICTRL_ERRTYPE_MASK)) {
	n4->n4_pcierr = 0;
	mips_wbflush ();
	if (_pciverbose >= 1) {
	    const char *msg;
	    switch (err) {
	    case N4_PCICTRL_ERRTYPE_MABORT:
#if 0
		msg = "master abort";
#else
		msg = NULL;
#endif
		break;
	    case N4_PCICTRL_ERRTYPE_TABORT:
		msg = "target abort";
		break;
	    case N4_PCICTRL_ERRTYPE_RETRYLIM:
		msg = "retry limit exceeded";
		break;
	    case N4_PCICTRL_ERRTYPE_RDPERR:
		msg = "read parity error";
		break;
	    default:
		msg = "unknown error";
		break;
	    }
	    if (msg)
		_pci_tagprintf (tag, "_pci_conf_read: %s\r\n", msg);
	}
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

pcireg_t
_pci_conf_read32(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 4);
}

pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    return _pci_conf_readn (tag, reg, 4);
}

static void
_pci_conf_writen(pcitag_t tag, int reg, pcireg_t data, int width)
{
    u_int64_t type, err;
    u_int32_t addr;
    void *addrp;
    int bus, device, function;

    if (reg & (width-1) || reg < 0 || reg >= 0x100) {
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_readn: bad reg 0x%x\r\n", reg);
	return;
    }

    _pci_break_tag (tag, &bus, &device, &function); 
    if (bus == 0) {
	/* Type 0 configuration on onboard PCI bus */
	if (device > 9 || function > 7)
	    return;		/* device out of range */
	type = N4_PCIINIT_CONFIGTYPE0;
	addr = (1 << (device+16)) | (function << 8) | reg;
    }
    else {
	/* Type 1 configuration on offboard PCI bus */
	if (bus > 255 || device > 15 || function > 7)
	    return;	/* device out of range */
	type = N4_PCIINIT_CONFIGTYPE1;
	addr = (bus << 16) | (device << 11) | (function << 8) | reg;
    }

    n4->n4_pciw0 = PCIWIND0_CONF;
    n4->n4_pciinit0 = PCIINIT0_CONF | type;
    n4->n4_pcierr = 0;
    mips_wbflush ();

    addrp = PA_TO_KVA1(PCI_CONF_SPACE + addr);
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

    /* wait for write FIFO to empty */
    mips_wbflush ();
    while (n4->n4_pcictrl & N4_PCICTRL_FIFOSTALL)
	continue;

    n4->n4_pciw0 = PCIWIND0_IO;
    n4->n4_pciinit0 = PCIINIT0_IO;
    mips_wbflush ();

    if (err = (n4->n4_pcictrl & N4_PCICTRL_ERRTYPE_MASK)) {
	n4->n4_pcierr = 0;
	mips_wbflush ();
	if (_pciverbose >= 1) {
	    const char *msg;
	    switch (err) {
	    case N4_PCICTRL_ERRTYPE_MABORT:
		msg = "master abort";
		break;
	    case N4_PCICTRL_ERRTYPE_TABORT:
		msg = "target abort";
		break;
	    case N4_PCICTRL_ERRTYPE_RETRYLIM:
		msg = "retry limit exceeded";
		break;
	    case N4_PCICTRL_ERRTYPE_WRPERR:
		msg = "write parity error";
		break;
	    default:
		msg = "unknown error";
		break;
	    }
	    if (msg)
		_pci_tagprintf (tag, "_pci_conf_write: %s\r\n", msg);
	}
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

void
_pci_conf_write32(pcitag_t tag, int reg, pcireg_t data)
{
    _pci_conf_writen (tag, reg, data, 4);
}

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
	case PCI_MAP_MEMORY_TYPE_64BIT:
	    break;
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



/* 
 * Handle mapping of fixed ISA addresses 
 */

void *
_isa_map_io (unsigned int port)
{
    if (port > 0xffff)
	return 0;
   return PA_TO_KVA1 (ISAPORT_BASE (port));
}

void *
_isa_map_mem (vm_offset_t addr)
{
    if (addr > 0xfffff)
	return 0;
    return PA_TO_KVA1 (ISAMEM_BASE (addr));
}
