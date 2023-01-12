/*
 * pciconf.c: generic PCI bus configuration
 * Copyright (c) 1999 Algorithmics Ltd
 */

#ifdef IN_PMON
#include <stdio.h>
#include <stdarg.h>
#include "mips.h"
#include "pmon.h"
#include "pci/pcivar.h"
#include "pci/pcireg.h"
#include "pci/device.h"	/* NetBSD style autoconfig data structures */
#else
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <mips/cpu.h>

#include "kit_impl.h"
#include "pcivar.h"
#include "pcireg.h"
#include "device.h"
#endif


#ifndef dimof
#define dimof(arr)	(sizeof(arr)/sizeof(arr[0]))
#endif

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifdef IN_PMON
#define PRINTF printf
#define VPRINTF vprintf
#define MALLOC malloc
#define FREE free
#define _PCI_VERBOSE 1
#else
#define PRINTF _mon_printf
#define VPRINTF _mon_vprintf
#define MALLOC _pci_malloc
#define FREE _pci_free
#endif

#if _PCI_VERBOSE > 0
int _pciverbose = _PCI_VERBOSE;
#else
int _pciverbose;
#endif

#ifndef SBD_DISPLAY
#define SBD_DISPLAY(msg, chkpnt) ((void)0)
#endif

/* pci_devinfo uses sprintf(), and we don't necessarily want to drag in 
   all those tables for a minimal SDE-MIPS build, so set this function
   pointer if it is required. */
void	(*_pci_devinfo_func) (pcireg_t, pcireg_t, char *, int *);

int	_pci_nbus;		/* start off with one bus */
int	_pci_enumerated;

#define PCIMAX_DEV	16	/* arbitrary */
#define PCIMAX_MEMWIN	3	/* arbitrary per device */
#define PCIMAX_IOWIN	1	/* arbitrary per device */

struct pcidev {
    struct pci_attach_args pa;
    unsigned char	min_gnt;
    unsigned char	max_lat;
    short		nmemwin;
    short		niowin;
};

struct pciwin {
    struct pcidev *dev;
    int		reg;
    vm_size_t	size;
    pcireg_t	address;
};

static struct pcidev pcidev[PCIMAX_DEV];
static int pcindev;

static struct pciwin pcimemwin[PCIMAX_DEV * PCIMAX_MEMWIN];
static int pcinmemwin;

static struct pciwin pciiowin[PCIMAX_DEV * PCIMAX_IOWIN];
static int pciniowin;
    

#ifndef IN_PMON
/*
 * This code may run before the runtime system is properly 
 * initialised so...
 *	o no mallocs
 *	o no stdio (printf, sprintf etc)
 *
 * The probe and attach routines need to be aware of this as well...
 */

/* rudimentary malloc/free for early life SDE-MIPS */
static void *
_pci_malloc (size_t size)
{
    void *p = sbrk (size);
    if (p == (void *)-1)
	return 0;
    return (p);
}

static void
_pci_free (void *p)
{
    return;
}
#endif


static void
print_bdf (int bus, int device, int function)
{
    PRINTF ("PCI");
    if (_pci_nbus > 1 && bus >= 0)
	PRINTF (" bus %d", bus);
    if (device >= 0)
	PRINTF (" slot %d", device);
    if (function >= 0)
	PRINTF ("/%d", function);
    PRINTF (": ");
}

void
_pci_bdfprintf (int bus, int device, int function, const char *fmt, ...)
{
    va_list arg;

    print_bdf (bus, device, function);
#ifdef __VARARGS_H
    va_start(arg);
#else
    va_start(arg, fmt);
#endif
    VPRINTF (fmt, arg);
    va_end(arg);
}

void
_pci_tagprintf (pcitag_t tag, const char *fmt, ...)
{
    va_list arg;
    int bus, device, function;

    _pci_break_tag (tag, &bus, &device, &function); 
    print_bdf (bus, device, function);

#ifdef __VARARGS_H
    va_start(arg);
#else
    va_start(arg, fmt);
#endif
    VPRINTF (fmt, arg);
    va_end(arg);
}


/*
 * Scan each PCI device on the system and record its configuration
 * requirements.
 */


static void
_pci_query_dev_func (pcitag_t tag, int initialise)
{
    pcireg_t id, class;
    pcireg_t old, mask;
    pcireg_t stat;
    pcireg_t bparam;
    unsigned int x;
    int reg;
    struct pci_bus *pb;
    struct pcidev *pd;
    struct pciwin *pm, *pi;
    int bus, device, function;

    class = _pci_conf_read(tag, PCI_CLASS_REG);
    id = _pci_conf_read(tag, PCI_ID_REG);

    if (_pciverbose && _pci_devinfo_func) {
	char devinfo[256];
	int supported;
	(*_pci_devinfo_func) (id, class, devinfo, &supported);
	_pci_tagprintf (tag, "%s\n", devinfo);
    }

    if (pcindev >= dimof(pcidev)) {
	PRINTF ("pci: too many devices\n");
	return;
    }

    pd = &pcidev[pcindev++];
    
    _pci_break_tag (tag, &bus, &device, &function);

    pd->pa.pa_bus = bus;
    pd->pa.pa_device = device;
    pd->pa.pa_function = function;
    pd->pa.pa_tag = tag;
    pd->pa.pa_id = id;
    pd->pa.pa_class = class;
    pd->nmemwin = 0;
    pd->niowin = 0;
    
    stat = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);

    pb = &_pci_bus[bus];
    pb->ndev++;

    /* do all devices support fast back-to-back */
    if ((stat & PCI_STATUS_BACKTOBACK_OKAY) == 0)
	pb->fast_b2b = 0;		/* no, sorry */

    /* do all devices run at 66 MHz */
    if ((stat & PCI_STATUS_66MHZ_OKAY) == 0)
	pb->freq66 = 0; 		/* no, sorry */

    /* find slowest devsel */
    x = (stat & PCI_STATUS_DEVSEL_MASK) >> PCI_STATUS_DEVSEL_SHIFT;
    if (x > pb->devsel)
	pb->devsel = x;

    if (!initialise)
	/*
	 * don't need to calculate bus latency or
	 * scan base registers if not initialising
	 */
	return;

    bparam = _pci_conf_read(tag, PCI_BPARAM_REG);

    pd->min_gnt = PCI_BPARAM_MIN_GNT (bparam);
    pd->max_lat = PCI_BPARAM_MAX_LAT (bparam);

    if (pd->min_gnt != 0 || pd->max_lat != 0) {
	/* find largest minimum grant time of all devices */
	if (pd->min_gnt != 0 && pd->min_gnt > pb->min_gnt)
	    pb->min_gnt = pd->min_gnt;
	
	/* find smallest maximum latency time of all devices */
	if (pd->max_lat != 0 && pd->max_lat < pb->max_lat)
	    pb->max_lat = pd->max_lat;
	
	if (pd->max_lat != 0)
	    /* subtract our minimum on-bus time per sec from bus bandwidth */
	    pb->bandwidth -= pd->min_gnt * 4000000 /
		(pd->min_gnt + pd->max_lat);
    }

    if (PCI_CLASS(class) == PCI_CLASS_BRIDGE &&
	PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI)
	return;

    for (reg = PCI_MAP_REG_START; reg < PCI_MAP_REG_END; reg += 4) {
	old = _pci_conf_read(tag, reg);
	_pci_conf_write(tag, reg, 0xfffffffe);
	mask = _pci_conf_read(tag, reg);
	_pci_conf_write(tag, reg, old);

	if (mask == 0 || mask == 0xffffffff)
	    continue;

	if (_pciverbose >= 3)
	    _pci_tagprintf (tag, "reg 0x%x = 0x%x\n", reg, mask);

	if ((mask & PCI_MAP_IO) != 0) {
	    mask |= 0xffff0000; /* must be ones */
		
	    if (pciniowin >= dimof(pciiowin)) {
		PRINTF ("pci: too many i/o windows\n");
		continue;
	    }
	    pi = &pciiowin[pciniowin++];
	    
	    pi->dev = pd;
	    pi->reg = reg;
	    pi->size = -(mask & PCI_MAP_IO_ADDRESS_MASK);
	    pd->niowin++;
	} 
	else {
	    switch (mask & PCI_MAP_MEMORY_TYPE_MASK) {
	    case PCI_MAP_MEMORY_TYPE_32BIT:
	    case PCI_MAP_MEMORY_TYPE_32BIT_1M:
		break;
	    case PCI_MAP_MEMORY_TYPE_64BIT:
		_pci_tagprintf (tag, "64-bit region ignored\n");
		continue;
	    default:
		_pci_tagprintf (tag, "reserved mapping type 0x%x\n",
				 mask & PCI_MAP_MEMORY_TYPE_MASK);
		continue;
	    }
		
	    if  ((mask & PCI_MAP_MEMORY_PREFETCHABLE) == 0)
		_pci_bus[bus].prefetch = 0;

	    if (pcinmemwin >= dimof(pcimemwin)) {
		PRINTF ("pci: too many memory windows\n");
		continue;
	    }
	    pm = &pcimemwin[pcinmemwin++];
	    
	    pm->dev = pd;
	    pm->reg = reg;
	    pm->size = -(mask & PCI_MAP_MEMORY_ADDRESS_MASK);
	    pd->nmemwin++;
	}
    }

    /* Finally check for Expansion ROM */
    reg = PCI_MAP_REG_ROM;
    old = _pci_conf_read(tag, reg);
    _pci_conf_write(tag, reg, 0xfffffffe);
    mask = _pci_conf_read(tag, reg);
    _pci_conf_write(tag, reg, old);

    if (mask != 0 && mask != 0xffffffff) {
	if (_pciverbose >= 3)
	    _pci_tagprintf (tag, "reg 0x%x = 0x%x\n", reg, mask);

	if (pcinmemwin >= dimof(pcimemwin)) {
	    PRINTF ("pci: too many memory windows\n");
	    return;
	}

	pm = &pcimemwin[pcinmemwin++];
	pm->dev = pd;
	pm->reg = reg;
	pm->size = -(mask & PCI_MAP_ROM_ADDRESS_MASK);
	pd->nmemwin++;
    }
}

static void
_pci_query_dev (int bus, int device, int initialise)
{
    pcitag_t tag;
    pcireg_t id;
    pcireg_t misc;
    int hdrtype;

    tag = _pci_make_tag(bus, device, 0);
    if (!_pci_canscan (tag))
	return;

    if (_pciverbose >= 2)
	_pci_bdfprintf (bus, device, -1, "probe...");

    id = _pci_conf_read(tag, PCI_ID_REG);

    if (_pciverbose >= 2)
	PRINTF ("completed\n");

    if (id == 0 || id == 0xffffffff)
	return;

    misc = _pci_conf_read(tag, PCI_MISC_REG);

    hdrtype = PCI_MISC_HDRTYPE(misc);

    if (hdrtype & PCI_MISC_HDRTYPE_MULTI) {
	int function;
	for (function = 0; function < PCI_FUNCMAX; function++) {
	    tag = _pci_make_tag(bus, device, function);
	    id = _pci_conf_read(tag, PCI_ID_REG);
	    if (id == 0 || id == 0xffffffff)
		return;
	    _pci_query_dev_func (tag, initialise);
	}
    }
    else
	_pci_query_dev_func (tag, initialise);
}


static int 
wincompare (const void *a, const void *b)
{
    const struct pciwin *wa = a, *wb = b;
    if (wa->dev->pa.pa_bus != wb->dev->pa.pa_bus)
	/* sort into ascending order of bus number */
	return (int)(wa->dev->pa.pa_bus - wb->dev->pa.pa_bus);
    else
	/* sort into descending order of size */
	return (int)(wb->size - wa->size);
}


static void
_pci_setup_iowins (void)
{
    struct pciwin *pi;
    pcireg_t base;

    qsort(pciiowin, pciniowin, sizeof (struct pciwin), wincompare);
    for (pi = pciiowin; pi < &pciiowin[pciniowin]; pi++) {
	struct pcidev *pd = pi->dev;
	struct pci_bus *pb;

	if (pd->niowin < 0 || pi->size == 0)
	    continue;
	pi->address = _pci_allocate_io (pd->pa.pa_tag, pi->size);
	if (pi->address == -1) {
	    _pci_tagprintf (pd->pa.pa_tag, 
			    "not enough PCI i/o space (%d requested)\n", 
			    pi->size);
	    pd->nmemwin = pd->niowin = -1;
	    continue;
	}

	pb = &_pci_bus[pd->pa.pa_bus];
	if (pi->address < pb->min_io_addr)
	    pb->min_io_addr = pi->address;
	if (pi->address + pi->size - 1 > pb->max_io_addr)
	    pb->max_io_addr = pi->address + pi->size - 1;

	if (_pciverbose >= 2)
	    _pci_tagprintf (pd->pa.pa_tag, "gets %d bytes of i/o @0x%x\n", 
			    pi->size, pi->address);
	base = _pci_conf_read(pd->pa.pa_tag, pi->reg);
	base = (base & ~PCI_MAP_IO_ADDRESS_MASK) | pi->address;
	_pci_conf_write(pd->pa.pa_tag, pi->reg, base);
    }
}

static void
_pci_setup_memwins (void)
{
    struct pciwin *pm;
    struct pcidev *pd;

    qsort(pcimemwin, pcinmemwin, sizeof (struct pciwin), wincompare);
    for (pm = pcimemwin; pm < &pcimemwin[pcinmemwin]; pm++) {
	struct pci_bus *pb;

	pd = pm->dev;
	if (pd->nmemwin < 0 || pm->size == 0)
	    continue;
	pm->address = _pci_allocate_mem (pd->pa.pa_tag, pm->size);
	if (pm->address == -1) {
	    _pci_tagprintf (pd->pa.pa_tag, 
			    "not enough PCI mem space (%d requested)\n", 
			    pm->size);
	    pd->nmemwin = pd->niowin = -1;
	    continue;
	}
	if (_pciverbose >= 2)
	    _pci_tagprintf (pd->pa.pa_tag, "gets %d bytes of memory @0x%x\n", 
			    pm->size, pm->address);

	pb = &_pci_bus[pd->pa.pa_bus];
	if (pm->address < pb->min_mem_addr)
	    pb->min_mem_addr = pm->address;
	if (pm->address + pm->size - 1 > pb->max_mem_addr)
	    pb->max_mem_addr = pm->address + pm->size - 1;

	if (pm->reg != PCI_MAP_REG_ROM) {
	    /* normal memory - expansion rom done below */
	    pcireg_t base = _pci_conf_read(pd->pa.pa_tag, pm->reg);
	    base = pm->address | (base & ~PCI_MAP_MEMORY_ADDRESS_MASK);
	    _pci_conf_write(pd->pa.pa_tag, pm->reg, base);
	}
    }

    /* Program expansion rom address base after normal memory base, 
       to keep DEC ethernet chip happy */
    for (pm = pcimemwin; pm < &pcimemwin[pcinmemwin]; pm++) {
	if (pm->reg == PCI_MAP_REG_ROM && pm->address != -1) {
	    /* expansion rom */
	    pd = pm->dev;
	    _pci_conf_write(pd->pa.pa_tag, pm->reg, pm->address | PCI_MAP_ROM);
	}
    }
}


static void
_pci_setup_ppb(void)
{
    int i;

    for (i = _pci_nbus - 1; i > 0; i--) {
	struct pci_bus *psec = &_pci_bus[i];
	struct pci_bus *ppri = &_pci_bus[psec->primary];
	if (ppri->min_io_addr > psec->min_io_addr)
	    ppri->min_io_addr = psec->min_io_addr;
	if (ppri->max_io_addr < psec->max_io_addr)
	    ppri->max_io_addr = psec->max_io_addr;
	if (ppri->min_mem_addr > psec->min_mem_addr)
	    ppri->min_mem_addr = psec->min_mem_addr;
	if (ppri->max_mem_addr < psec->max_mem_addr)
	    ppri->max_mem_addr = psec->max_mem_addr;
    }

    if (_pciverbose >= 2) {
	struct pci_bus *pb = &_pci_bus[0];
	if (pb->min_io_addr < pb->max_io_addr)
	    _pci_bdfprintf (0, -1, -1, "io  0x%08x-0x%08x\n", 
			    pb->min_io_addr, pb->max_io_addr);
	if (pb->min_mem_addr < pb->max_mem_addr)
	    _pci_bdfprintf (0, -1, -1, "mem 0x%08x-0x%08x\n",
			    pb->min_mem_addr, pb->max_mem_addr);
    }

    for (i = 1; i < _pci_nbus; i++) {
	struct pci_bus *pb = &_pci_bus[i];
	pcireg_t cmd;

	cmd = _pci_conf_read(pb->tag, PCI_COMMAND_STATUS_REG);
	if (_pciverbose >= 2)
	    _pci_bdfprintf (i, -1, -1, "subordinate to bus %d\n", pb->primary);

	if (pb->min_io_addr < pb->max_io_addr) {
	    pcireg_t iodata;

	    cmd |= PCI_COMMAND_IO_ENABLE;
	    if (_pciverbose >= 2)
		_pci_bdfprintf (i, -1, -1, "io  0x%08x-0x%08x\n",
				pb->min_io_addr, pb->max_io_addr);
	    iodata = _pci_conf_read(pb->tag, PPB_IO_REG);
	    iodata = (iodata & ~PPB_IO_BASE_MASK)
		| (pb->min_io_addr >> 8);
	    iodata = (iodata & ~PPB_IO_LIMIT_MASK)
		| (pb->max_io_addr & PPB_IO_LIMIT_MASK);
	    _pci_conf_write(pb->tag, PPB_IO_REG, iodata);
	}

	if (pb->min_mem_addr < pb->max_mem_addr) {
	    pcireg_t memdata;

	    cmd |= PCI_COMMAND_MEM_ENABLE;
	    if (_pciverbose >= 2)
		_pci_bdfprintf (i, -1, -1, "mem  0x%08x-0x%08x\n",
				pb->min_mem_addr, pb->max_mem_addr);
	    memdata = _pci_conf_read(pb->tag, PPB_MEM_REG);
	    memdata = (memdata & ~PPB_MEM_BASE_MASK)
		| (pb->min_mem_addr >> 16);
	    memdata = (memdata & ~PPB_MEM_LIMIT_MASK)
		| (pb->max_mem_addr & PPB_MEM_LIMIT_MASK);
	    _pci_conf_write(pb->tag, PPB_MEM_REG, memdata);
	}

	_pci_conf_write(pb->tag, PCI_COMMAND_STATUS_REG, cmd);
    }
}


static void
_pci_autoconf (struct pci_attach_args *pa)
{
    extern struct cfdata _pci_cfdata[];
    struct cfdata *cf;
    struct cfdriver *cd;
    struct device *dv;

    for (cf = _pci_cfdata; cd = cf->cf_driver; cf++) {
	if (cf->cf_fstate == FSTATE_FOUND)
	    continue;

	if ((*cd->cd_match) (NULL, cf, pa)) {
	    
	    dv = MALLOC (cd->cd_devsize); 
	    if (!dv) {
		PRINTF ("_pci_autoconf: no memory when allocating device\n");
		continue;
	    }
	    
	    memset (dv, 0, cd->cd_devsize);
	    dv->dv_class = cd->cd_class;
	    dv->dv_unit = cf->cf_unit;
#ifdef IN_PMON
	    sprintf (dv->dv_xname, "%s%d", cd->cd_name, cf->cf_unit);
#else
	    {
		char *s;
		strcpy (dv->dv_xname, cd->cd_name);
		s = dv->dv_xname + strlen (dv->dv_xname);
	        if (cf->cf_unit >= 10)
		    *s++ = '0' + (cf->cf_unit / 10);
		*s++ = '0' + (cf->cf_unit % 10);
		*s++ = '\0';
	    }
#endif
	    dv->dv_parent = NULL;
	    
	    if (dv->dv_unit >= cd->cd_ndevs) {
		int old = cd->cd_ndevs, new;
		void **nsp;
		new = (old != 0) ? old : 1;
		while (new <= dv->dv_unit)
		    new *= 2;
		nsp = MALLOC(new * sizeof(void *));
		if (!nsp) {
		    FREE (dv); 
		    PRINTF ("_pci_autoconf: no memory when %sing device array\n",
				 old != 0 ? "expand" : "creat");
		    continue;
		}
		cd->cd_ndevs = new;
		memset(nsp + old, 0, (new - old) * sizeof(void *));
		if (old != 0) {
		    memcpy(nsp, cd->cd_devs, old * sizeof(void *));
		    FREE (cd->cd_devs);
		}
		cd->cd_devs = nsp;
	    }
		
	    if (cd->cd_devs[dv->dv_unit]) {
		PRINTF ("_pci_autoconf: duplicate device %s\n", dv->dv_xname);
		FREE (dv);
		continue;
	    }
	    cd->cd_devs[dv->dv_unit] = dv;
	    
	    PRINTF (dv->dv_xname);
	    (*cd->cd_attach) (NULL, dv, pa);
	    cf->cf_fstate = FSTATE_FOUND;
	    return;
	}
    }
}


#pragma weak _pci_cacheline_log2
int
_pci_cacheline_log2 (void)
{
    /* default to 8 words == 2^3 */
    return 3;
}


#pragma weak _pci_maxburst_log2
int
_pci_maxburst_log2 (void)
{
    return 32;			/* no limit */
}


static void
_pci_setup_devices (int initialise)
{
    struct pcidev *pd;

    if (initialise) {
	/* Enable each PCI interface */
	for (pd = pcidev; pd < &pcidev[pcindev]; pd++) {
	    struct pci_bus *pb = &_pci_bus[pd->pa.pa_bus];
	    pcitag_t tag = pd->pa.pa_tag;
	    pcireg_t cmd, misc;
	    unsigned int ltim;
	    
	    cmd = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);
	    cmd |= PCI_COMMAND_MASTER_ENABLE 
		| PCI_COMMAND_SERR_ENABLE 
		| PCI_COMMAND_PARITY_ENABLE;
	    /* always enable i/o & memory space, in case this card is
	       just snarfing space from the fixed ISA block and doesn't
	       declare separate PCI space. */
	    cmd |= PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE;
	    if (pb->fast_b2b)
		cmd |= PCI_COMMAND_BACKTOBACK_ENABLE;
	    _pci_conf_write(tag, PCI_COMMAND_STATUS_REG, cmd);

	    ltim = pd->min_gnt * (pb->freq66 ? 66 : 33) / 4;
	    ltim = MIN (MAX (pb->def_ltim, ltim), pb->max_ltim);
	
	    misc = _pci_conf_read (tag, PCI_MISC_REG);
	    PCI_MISC_LTIM_SET (misc, ltim);
	    PCI_MISC_CLSZ_SET (misc, 1 << _pci_cacheline_log2());
	    _pci_conf_write (tag, PCI_MISC_REG, misc);
	}
    }

    /* Call the driver initialisation for each enabled device */
    for (pd = pcidev; pd < &pcidev[pcindev]; pd++) {
	pcitag_t tag = pd->pa.pa_tag;
	pcireg_t cmd;

	cmd = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);
	if (!initialise) {
	    /* Wait for i/o & mem enable to be set (we may be in a race
	       with the system controller setting up the devices. */
	    unsigned int retry = 100000;
	    while (!(cmd & (PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE))) {
		if (--retry == 0)
		    break;
		cmd = _pci_conf_read(tag, PCI_COMMAND_STATUS_REG);
	    }
	}

	if (!(cmd & (PCI_COMMAND_IO_ENABLE | PCI_COMMAND_MEM_ENABLE))) {
	    if (_pciverbose >= 2)
		_pci_tagprintf (tag, "no memory or i/o - not configured\n");
	    continue;
	}

	/* configure device drivers - NetBSD style */
	_pci_autoconf (&pd->pa);
    }
}



/*
 * Initialise the pci-pci bus hierarchy
 */
static void
_pci_ppbinit (void)
{
    struct pci_bus *ppri;
    int bus, device;
    pcitag_t tag;
    pcireg_t id, class, data;

    if (_pci_nbus == _pci_maxbus)
	return;
    
    bus = _pci_nbus - 1;
    ppri = &_pci_bus[bus];
    ppri->min_io_addr = 0xffffffff;
    ppri->max_io_addr = 0;
    ppri->min_mem_addr = 0xffffffff;
    ppri->max_mem_addr = 0;

    for (device = 0; device <= PCI_DEVMAX; device++) {
	tag = _pci_make_tag(bus, device, 0);
	if (!_pci_canscan (tag))
	    continue;
	id = _pci_conf_read(tag, PCI_ID_REG);
	if (id != 0 && id != 0xffffffff) {
	    class = _pci_conf_read(tag, PCI_CLASS_REG);
	    if (PCI_CLASS(class) == PCI_CLASS_BRIDGE &&
		PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI) {
		struct pci_bus *psec = &_pci_bus[_pci_nbus];

		psec->tag = tag;
		psec->primary = bus;
		
		/*
		 * set primary to bus
		 * set secondary to _pci_nbus
		 * set subordinate to max possible bus number
		 */
		data = (PCI_BUSMAX << 16) | (_pci_nbus << 8) | bus;
		_pci_conf_write(tag, PPB_BUS_REG, data);

		_pci_nbus++;

		/* scan new bus for PCI-PCI bridges and initialize */
		_pci_ppbinit();

		/* reset subordinate bus number */
		data = (data & 0xff00ffff) | ((_pci_nbus - 1) << 16);
		_pci_conf_write(tag, PPB_BUS_REG, data);
	    }
	}
    }
}


/*
 * Probe an existing pci-pci bus hierarchy
 */
static void
_pci_ppbfind (int bus)
{
    int device;
    pcitag_t tag;
    pcireg_t id, class, data;

    for (device = 0; device <= PCI_DEVMAX; device++) {
	tag = _pci_make_tag(bus, device, 0);
	if (!_pci_canscan (tag))
	    continue;
	id = _pci_conf_read(tag, PCI_ID_REG);
	if (id != 0 && id != 0xffffffff) {
	    class = _pci_conf_read(tag, PCI_CLASS_REG);
	    if (PCI_CLASS(class) == PCI_CLASS_BRIDGE &&
		PCI_SUBCLASS(class) == PCI_SUBCLASS_BRIDGE_PCI) {
		struct pci_bus *psec;
		int sbus;

		/* read secondary bus number */
		data = _pci_conf_read(tag, PPB_BUS_REG);
		sbus = (data >> 8) & 0xff;
		if (sbus == 0 || sbus >= _pci_maxbus)
		    continue;

		psec = &_pci_bus[sbus];
		psec->tag = tag;
		psec->primary = bus;
		if (sbus + 1 > _pci_nbus)
		    _pci_nbus = sbus + 1;

		/* recursively scan new bus */
		_pci_ppbfind (sbus);
	    }
	}
    }
}



void
_pci_configure (int initialise)
{
    int bus, device;

#ifdef IN_PMON
    {
	extern int _pciverbose;
	char *v = getenv("pciverbose");
	if (v)
	    _pciverbose = atol (v);
    }
#endif
#if _PCI_VERBOSE > 0
    _pci_devinfo_func = _pci_devinfo;
#endif

    /* initialise the PCI bridge */
    SBD_DISPLAY ("PCIH", CHKPNT_PCIH);
    initialise = _pci_hwinit (initialise);
    if (initialise < 0)
	return;

    /* for backwards compatibilty */
    if (_pci_nbus == 0)
	_pci_nbus = 1;

    /* initialize any PCI-PCI bridges */
    if (initialise)
	_pci_ppbinit();
    else
	_pci_ppbfind(0);

    pcindev = pcinmemwin = pciniowin = 0;

    /* scan configuration space of all devices */
    SBD_DISPLAY ("PCIS", CHKPNT_PCIS);
    for (bus = 0; bus < _pci_nbus; bus++) {
	struct pci_bus *pb = &_pci_bus[bus];
	unsigned int def_ltim, max_ltim;

	for (device = 0; device <= PCI_DEVMAX; device++)
	    _pci_query_dev (bus, device, initialise);

	if (pb->ndev == 0)
	    continue;

	if (initialise) {
	    /* convert largest minimum grant time to cycle count */
	    max_ltim = pb->min_gnt * (pb->freq66 ? 66 : 33) / 4;

	    /* now see how much bandwidth is left to distribute */
	    if (pb->bandwidth <= 0) {
		_pci_bdfprintf (bus, -1, -1, "warning: total bandwidth exceeded\n");
		def_ltim = 1;
	    }
	    else {
		/* calculate a fair share for each device */
		def_ltim = pb->bandwidth / pb->ndev;
		if (def_ltim > pb->max_lat)
		    /* that would exceed critical time for some device */
		    def_ltim = pb->max_lat;
		/* convert to cycle count */
		def_ltim = def_ltim * (pb->freq66 ? 66 : 33) / 4;
	    }
	    /* most devices don't implement bottom three bits, so round up */
	    def_ltim = (def_ltim + 7) & ~7;
	    max_ltim = (max_ltim + 7) & ~7;

	    pb->def_ltim = MIN (def_ltim, 255);
	    pb->max_ltim = MIN (MAX (max_ltim, def_ltim), 255);
	}
    }

    if (initialise) {
	/* alter PCI bridge parameters based on query data */
	SBD_DISPLAY ("PCIR", CHKPNT_PCIR);
	_pci_hwreinit ();

	/* setup the individual device windows */
	SBD_DISPLAY ("PCIW", CHKPNT_PCIW);
	_pci_setup_iowins ();
	_pci_setup_memwins ();
	if (_pci_nbus > 1)
	    _pci_setup_ppb ();
    }

    /* enable each device and call its initialisation routine */
    SBD_DISPLAY ("PCID", CHKPNT_PCID);
    _pci_setup_devices (initialise);
    _pci_enumerated = 1;
}


#ifndef IN_PMON

#include <sys/init.h>

void
_pci_init (void)
{
    {
	char *sp = _sbd_getenv ("pciverbose");
	if (sp) {
	    char *ep;
	    _pciverbose = strtol (sp, &ep, 10);
	    if (sp == ep || _pciverbose < 0 || _pciverbose > 3)
		_pciverbose = 3;
	}
    }

    if (!_pci_enumerated) {
	extern int _ram_based;
    	_pci_configure (!_ram_based);
    }
}
#endif

