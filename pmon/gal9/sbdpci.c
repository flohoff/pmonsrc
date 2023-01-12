/*
 * GAL9/pci_machdep.c: Machine-specific functions for PCI autoconfiguration.
 * Copyright (c) 1997	Algorithmics Ltd
 *
 */

#include "mips.h"
#include "pmon.h"
#include "stdio.h"

#include "sbd.h"
#include "gt64011.h"

#include "pci/pcivar.h"
#include "pci/pcireg.h"

#define DBGPCI
#ifdef DBGPCI
#define DBG(fmt, args...) \
	{ if (_pciverbose > 3) printf (fmt, ## args); }
#else
#define DBG(fmt, args...)
#endif

/* reserve bottom 64KB of i/o space for ISA decodes */
static pcireg_t minpciioaddr  = PCI_IO_BASE + 0x10000;
static pcireg_t nextpciioaddr = PCI_IO_BASE + PCI_IO_SIZE;

/* reserve bottom 1MB of mem space for ISA decodes */
static pcireg_t minpcimemaddr =  PCI_MEM_BASE + 0x100000;
static pcireg_t nextpcimemaddr = PCI_MEM_BASE + PCI_MEM_SIZE;

static pcitag_t mytag;
static unsigned int reserveddevices;

static const struct pci_bus gt64011_pci_bus = {
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

extern int _pciverbose;

#define MAXBUS	1
const int _pci_maxbus = MAXBUS;
struct pci_bus _pci_bus[MAXBUS];

/*
 * Called to initialise at the beginning of time
 */
int
_pci_hwinit (int initialise)
{
    pcireg_t reg;
    int master = 0;

    /* find out what devices we should access */
    {
	char *m = getenv ("pcimaster");
	char *s = getenv ("pcislave");

	master = (m != NULL);
	reserveddevices = ((1 << 0) | /* us! */
			   (1 << 31)); /* special function */

	if (m == NULL && s == NULL) {
#if 0
	    printf ("Attempting to initialise PCI bus without $pcimaster or $pcislave variable!\n");
#endif
	    return -1;
	}
	else if (m != NULL && s != NULL) {
	    printf ("Can't initialise PCI bus with both $pcimaster and $pcislave set!\n");
	    return -1;
	}
	else if ((m && *m == 'n') || (s && *s == 'n')) {
	    /*
	     * Scan complete bus with enough feedback to allow pcimaster
	     * to be set correctly
	     */
	    _pciverbose = 2;		/* verbose probe reports... */
	    unsetenv ("pcimaster");	/* in case we hang up during probe */
	    unsetenv ("pcislave");	/* in case we hang up during probe */
	}
	else {
	    char *sp, *ep;
	    long device;

	    /* parse the string to find the devives to be skipped */
	    sp = m ? m : s;
	    while (*sp) {
		if (*sp == ',')
		    sp++;
		device = strtol (sp, &ep, 10);
		if (sp == ep || device < 0 || device > 31) {
		    printf ("Invalid $%s value: %s\n", 
			    m ? "pcimaster" : "pcislave",
			    m ? m : s);
		    return -1;
		}
		reserveddevices |= 1 << device;
		sp = ep;
	    }
#if 0
	    /*
	     * This is a bit naff, but there needs to be some indication
	     * of which slot the board will work in...
	     */
	    printf ("PCI reserved slots:");
	    for (device = 0; device < 32; device++) {
		if (device == 0 || device == 31)
		    continue;
		if (reserveddevices & (1 << device))
		    printf (" %d", device);
	    }
	    printf ("\n");
#endif
	}
    }

    mytag = _pci_make_tag (0, 0, 0);

    if (initialise) {
	/* say we will respond to memory requests and we are a master */
	reg = _pci_conf_read (mytag, PCI_COMMAND_STATUS_REG);
	reg |= PCI_COMMAND_MEM_ENABLE |
	    PCI_COMMAND_MASTER_ENABLE |
	    PCI_COMMAND_SERR_ENABLE;
	_pci_conf_write (mytag, PCI_COMMAND_STATUS_REG, reg);

	/* min latency to 6 or greater */
	reg = _pci_conf_read (mytag, PCI_MISC_REG);
	reg = PCI_MISC_LTIM_SET(reg,7);
	_pci_conf_write (mytag, PCI_MISC_REG, reg);

	{
	    extern unsigned int rassize[];
	    extern unsigned int rasbase[];
	    unsigned int baren;
	    
	    /* allow access to all of our memory */
	    baren = GT_IPCI_BAREN;
	    if (rassize[0]+rassize[1]) {
		baren &= htoll(~GT_IPCI_BAREN_Ras10Dis);
		_pci_conf_write (mytag, PCI_MAP_REG_START + 0, rasbase[0]);
		GT_IPCI_RAS10SIZE = htoll(((rassize[0]+rassize[1])-1) &
					  GT_IPCI_SIZE_BankSizeMASK);
	    }
	    else
		baren |= htoll(GT_IPCI_BAREN_Ras10Dis);
	    if (rassize[2]+rassize[3]) {
		baren &= htoll(~GT_IPCI_BAREN_Ras32Dis);
		_pci_conf_write (mytag, PCI_MAP_REG_START + 4, rasbase[2]);
		GT_IPCI_RAS32SIZE = htoll(((rassize[2]+rassize[3])-1) &
					  GT_IPCI_SIZE_BankSizeMASK);
	    }
	    else
		baren |= htoll(GT_IPCI_BAREN_Ras32Dis);
	    GT_IPCI_BAREN = baren;
	}
    }


#if 0
   /* only allow address matching on RAS[1:0] */
   pciPutCfgDouble (0, 0, 13, pciReadConfigDouble(0, 0, 13) | 0xff); 
#endif

    /* initialise global data */
    minpciioaddr  = PCI_IO_BASE + 0x10000;
    nextpciioaddr = PCI_IO_BASE + PCI_IO_SIZE;

    minpcimemaddr =  PCI_MEM_BASE + 0x100000;
    nextpcimemaddr = PCI_MEM_BASE + PCI_MEM_SIZE;

    _pci_bus[0] = gt64011_pci_bus;

    return initialise && master;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
_pci_hwreinit (void)
{
}


/* Map the CPU virtual address of an area of local memory to a PCI
   address that can be used by a PCI bus master to access it. */
vm_offset_t
_pci_dmamap (vm_offset_t va, unsigned int len)
{
    return 0x00000000 + KVA_TO_PA (va);
}

/* Map the PCI address of an area of local memory to a CPU physical
   address. */
vm_offset_t
_pci_cpumap (vm_offset_t pcia, unsigned int len)
{
    return pcia - 0x00000000;
}


pcitag_t
_pci_make_tag(int bus, int device, int function)
{
    pcitag_t tag;
    tag = GT_IPCI_CFGADDR_ConfigEn |
	GT_IPCI_CFGADDR_BusNum(bus) |
	GT_IPCI_CFGADDR_DevNum(device) |
	GT_IPCI_CFGADDR_FunctNum(function);
    return tag;
}

void
_pci_break_tag(pcitag_t tag, int *busp, int *devicep, int *functionp)
{
    if (busp)
	*busp = (tag & GT_IPCI_CFGADDR_BusNumMASK) >> GT_IPCI_CFGADDR_BusNumSHIFT;
    if (devicep)
	*devicep = (tag & GT_IPCI_CFGADDR_DevNumMASK) >> GT_IPCI_CFGADDR_DevNumSHIFT;
    if (functionp)
	*functionp = (tag & GT_IPCI_CFGADDR_FunctNumMASK) >> GT_IPCI_CFGADDR_FunctNumSHIFT;
}


pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    pcireg_t data;

    DBG ("conf_read tag=0x%x reg=0x%x -> ", tag, reg);

    GT_IPCI_CFGADDR = htoll(tag | reg);
    GT_IPCI_INTRCAUSE = htoll(~(GT_INTR_MASABORT|GT_INTR_TARABORT));
    mips_wbflush ();

    data = (pcireg_t)GT_IPCI_CFGDATA;
    if (((tag & GT_IPCI_CFGADDR_BusNumMASK) == 0) &&
	((tag & GT_IPCI_CFGADDR_DevNumMASK) == 0))
	data = ltohl(data);
    DBG ("0x%x\n", data);

    if (GT_IPCI_INTRCAUSE & htoll(GT_INTR_MASABORT)) {
	GT_IPCI_INTRCAUSE = htoll(~GT_INTR_MASABORT);
#if 0
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: master abort\r\n");
#endif
	data = ~0;
    }

    if (GT_IPCI_INTRCAUSE & htoll(GT_INTR_TARABORT)) {
	GT_IPCI_INTRCAUSE = htoll(~GT_INTR_TARABORT);
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_read: target abort\r\n");
	data = ~0;
    }

    return data;
}


void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
    DBG ("conf_write tag=0x%x reg=0x%x data=0x%x\n", tag, reg, data);

    GT_IPCI_CFGADDR = htoll(tag | reg);
    GT_IPCI_INTRCAUSE = htoll(~(GT_INTR_MASABORT|GT_INTR_TARABORT));
    mips_wbflush ();

    if (((tag & GT_IPCI_CFGADDR_BusNumMASK) == 0) &&
	((tag & GT_IPCI_CFGADDR_DevNumMASK) == 0))
	data = ltohl(data);
    GT_IPCI_CFGDATA = data;
    mips_wbflush ();

    if (GT_IPCI_INTRCAUSE & htoll(GT_INTR_MASABORT)) {
	GT_IPCI_INTRCAUSE = htoll(~GT_INTR_MASABORT);
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: master abort\r\n");
    }

    if (GT_IPCI_INTRCAUSE & htoll(GT_INTR_TARABORT)) {
	GT_IPCI_INTRCAUSE = htoll(~GT_INTR_TARABORT);
	if (_pciverbose >= 1)
	    _pci_tagprintf (tag, "_pci_conf_write: target abort\r\n");
    }
}

int
_pci_canscan (pcitag_t tag)
{
    int bus, device;
    _pci_break_tag(tag, &bus, &device, NULL);

    if (bus == 0 && (reserveddevices & (1 << device)))
	return 0;
    return 1;
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
	    _pci_tagprintf (tag, "_pci_map_io: attempt to memory map a memory region\r\n");
	return -1;
    }

    /* There is a 1<->1 correspondence between PCI and local addresses */
    pa = address & PCI_MAP_IO_ADDRESS_MASK;
    *pap = pa;

    *vap = (vm_offset_t) PA_TO_KVA1 (pa);

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

    *pap = pa;
    if (address & PCI_MAP_MEMORY_CACHABLE) 
	*vap = (vm_offset_t) PA_TO_KVA0 (pa);
    else
	*vap = (vm_offset_t) PA_TO_KVA1 (pa);


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
	    _pci_tagprintf (tag, "_pci_map_int: bad interrupt pin %d\n", pin);
	return NULL;
    }


    _pci_break_tag (tag, &bus, &device, NULL);

    if (bus != 0)
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

