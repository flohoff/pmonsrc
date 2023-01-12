/* 		pci_machdep.c 				   */
/*			 Update:    19/01/1999  rh */
/*			  Start:	04/12/1998  rh */

/* Very loosely based on: */
/*	$NetBSD: pci_machdep.c,v 1.17 1995/07/27 21:39:59 cgd Exp $	*/
/* 
 * a08 21/jan/1999  3:30p rh -minor cleanup.
 * a07 20/jan/1999  9:30p rh -move code for V3USC_LB_PCI_BASE0 from conf read/write to hwinit.
 * a06 19/jan/1999  3:30p rh -major clean.
 * a05 12/jan/1999 12:30p rh -EDIT _pci_hwinit(), add in init for V3USC_PCI_I2O_MAP.
 * a04 11/jan/1999  6:00p rh -EDIT _pci_hwinit(), set up PCI/LOCAL apertures on USC.
 * a03 08/jan/1999 10:30a rh -Clean up; comment out printf statements for DEBUG.
 * a01 23/dec/1998  1:00p rh -debugging.
 */

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

typedef unsigned long DWORD;

#include "mips.h"
#include "pmon.h"
#include "stdio.h"

#include "sbd.h"
#include "v3uscreg.h"		

#include "pci/pcivar.h"
#include "pci/pcireg.h"

#if #endian(little)
#define V3USC_SWAP	V3USC_SWAP_NONE
#endif
#if #endian(big)
#define V3USC_SWAP	V3USC_SWAP_8BIT
#endif

/* host <-> little endian conversion */
#if #endian(little)
#define ltohl(x) (x)
#define ltohs(x) (x)
#define htoll(x) (x)
#define htols(x) (x)
#else
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

/* PCI regions in PCI space */
#define PCI_IO_SPACE_PCI_BASE	0x00000000
#define PCI_MEM_SPACE_PCI_BASE	0x10000000

static pcireg_t pci_io_space_pci_base = PCI_IO_SPACE_PCI_BASE;
static pcireg_t pci_mem_space_pci_base = PCI_IO_SPACE_PCI_BASE;

static pcireg_t nextpciioaddr = PCI_IO_SPACE_PCI_BASE;
static pcireg_t nextpcimemaddr = PCI_MEM_SPACE_PCI_BASE;

static const struct pci_bus v3usc_pci_bus = {
	0,		/* minimum grant */
	255,	/* maximum latency */
	0,		/* devsel time = fast */
	1,		/* we support fast back-to-back */
	1,		/* we support prefetch */
	0,		/* we don't support 66 MHz */
	0,		/* we don't support 64 bits */
	4000000,	/* bandwidth: in 0.25us cycles / sec */
	1		/* initially one device on bus (i.e. us) */
};

#define MAXBUS	1
const int _pci_maxbus = MAXBUS;
struct pci_bus _pci_bus[MAXBUS];

static unsigned char v3usc_vrev;   
static unsigned long pci_mem_space_size;


/*
 * Called to initialise the bridge at the beginning of time
 */
void
_pci_hwinit (void)
{
    volatile p4032icu * const icu = PA_TO_KVA1 (ICU_BASE);
    char * const _v3uscp = PA_TO_KVA1(V3USC_BASE);
    int tries;
    extern int _pmon_in_ram;
    u_int32_t savereg3;

    if (!_pmon_in_ram) 
    {
#define MAXTRIES 100
		for (tries = 0; tries < MAXTRIES; tries++) 
		{
		    mips_wbflush ();

		    /* check that it responds when we read its revision id */
			v3usc_vrev	= V3USC_PCI_CC_REV & PCI_CC_REV_VREV_MASK;
		    if (icu->irr.err & INTR_ERR_BUSERR) 
		    {
				icu->ctrl.clear = INTR_ERR_BUSERR;
				continue;
		    }
		    break;
		} /* end of for(tries = 0; tries < MAXTRIES; tries++) */
    
		if (tries >= MAXTRIES) 
		{
		    printf ("ERROR: V3USCC not responding after reset\n");
		    while (1)
			continue;
		}
		else if (tries > 1) 
		{
	    	printf ("V3USC reset after %d attempts\n", tries);
		}
    } /* end of if (!_pmon_in_ram) */ 
    else 
    {
		v3usc_vrev = V3USC_PCI_CC_REV & PCI_CC_REV_VREV_MASK;
    }

	/* printf("I am in _pci_hwinit\n");	*/		

    /* reset the PCI bus */
    V3USC_SYSTEM_B	&= ~SYSTEM_B_RST_OUT;

    /* enable bridge to PCI and PCI memory accesses, plus error handling */
	V3USC_PCI_CMD_W = PCI_CMD_W_MASTER_EN
	| PCI_CMD_W_MEM_EN
	| PCI_CMD_W_SERR_EN
	| PCI_CMD_W_PAR_EN;

    /* clear errors and say we do fast back-to-back transfers */
	V3USC_PCI_STAT_W = PCI_STAT_W_PAR_ERR
	| PCI_STAT_W_SYS_ERR	
	| PCI_STAT_W_M_ABORT	
	| PCI_STAT_W_T_ABORT	
	| PCI_STAT_W_PAR_REP	
	| PCI_STAT_W_FAST_BACK;


	/* Local to PCI aptr 0 -Local:0x1c000000 -> PCI CONF_SPACE:0x00000000(16MB)	*/

	V3USC_LB_PCI_BASE0 =
	PCI_CONF_SPACE					/* 0x1c000000 : bits 31-24; Base address,in sbd.h */
									/* 0x00000000 : bitS 23-16; Map address */
	|LB_PCI_BASEX_CONFIG			/* 0x0000a000 : bits 15-13; PCI_CMD, PCI command,Configuration read/write */
									/*          0 : bit  12; reserved */
  									/* 0x00000000 : bit  11; COMBINE,Burst write combined is OFF */
									/*          0 : bit  10; reserved */
	|LB_PCI_BASEX_BYTE_SWAP_NO		/*			0 : bits 9-8; SWAP,Byte Swap control,no swap 32 bit */
									/*          0 : bit  7; reserved */
	|LB_PCI_BASEX_SIZE_16MB;		/* 0x00000010 : bits 6-4; SIZE,aperture size 16MB*/
  									/* 0x00000000 : bit  3; Prefetchable is OFF */
  									/* 0x00000000 : bit  2; Error Report Enable is OFF */
									/*  		0 : bits 1,0; PCI Address Low */


	savereg3 = V3USC_LB_BUS_CFG;	/* Enable Bus Watch Timer with 64 clocks */
	savereg3 &= 0xfffff00f;
	savereg3 |= 0x00000400;
	V3USC_LB_BUS_CFG = savereg3;


	/* Local to PCI aptr 1 -Local:0x10000000  -> PCI MEM_SPACE:0x10000000 (128MB)	*/

	V3USC_LB_PCI_BASE1 =
	PCI_MEM_SPACE					/* 0x10000000 : bits 31-24; Base address,in sbd.h */
	|(PCI_MEM_SPACE_PCI_BASE >> 8)	/* 0x00100000 : bitS 23-16; Map address */
	|LB_PCI_BASEX_MEMORY			/* 0x00006000 : bits 15-13; PCI_CMD, PCI command,read/write */
									/*          0 : bit  12; reserved */
    								/* 0x00000000 : bit  11; COMBINE,Burst write combined is OFF */
									/*          0 : bit  10; reserved */
	|LB_PCI_BASEX_BYTE_SWAP_NO		/*			0 : bits 9-8; SWAP,Byte Swap control,no swap 32 bit */
									/*          0 : bit  7; reserved */
	|LB_PCI_BASEX_SIZE_128MB;		/* 0x00000040 : bits 6-4; SIZE,aperture size 128MB*/
  									/* 0x00000000 : bit  3; Prefetchable is OFF */
  									/* 0x00000000 : bit  2; Error Report Enable is OFF */
									/*  		0 : bits 1,0; PCI Address Low */


	/* PCI I2O AND SDRAM aptr base address - PCI:C0000000-D0000000 -> LOCAL:00000000 */
#define PCI_LOCAL_BASE	0xC0000000

	V3USC_PCI_I2O_BASE =
	PCI_LOCAL_BASE					/* 0xC0000000 : bits 31-20; Base address */
									/*          0 : bit 19-10; reserved */
	|PCI_I2O_BASE_PREFETCH			/* 0x00000008 : bit 3; Prefetchable - no effect */
	|PCI_I2O_BASE_TYPE_MASK;		/* 0x00000006 : Bits 2,1; Address range */
									/*  		0 : bit 0; As PCI Memory space */

	/* PCI I2O address map -PCI:C0000000-D0000000 -> LOCAL:00000000 	*/
	V3USC_PCI_I2O_MAP =	 
									/*          0 : bits 29-20; Map address */
									/*          0 : bits 19-17; reserved */
  									/* 0x00000000 : bit  16; W_FLUSH,no flush of prefetched reads */
	PCI_I2O_PCI_WR_MB_00			/*          0 : bits 15-14; PCI_WR_MB,write FIFO drain strategy */
	|PCI_I2O_PCI_RD_MB_10			/* 0x00002000 : bits 13-12; PCI_RD_MB,read FIFO drain strategy */
									/*          0 : bits 11-10; reserved */
  									/*          0 : bits  9-8; SWAP,no swap,32bit */
    |PCI_I2O_MAP_SIZE_32MB			/* 0x00000050 : bits  7-4; SIZE,32MB aperture */
  									/* 0x00000000 : bits  3; RD_POST_INH,Read posting inhit disabled */
  									/* 0x00000000 : bits  2; I20_MODE,disabled */
	|PCI_I2O_MAP_REG_EN				/* 0x00000002 : bits  1; REG_EN,PCI_I2O_BASE enabled */
	|PCI_I2O_MAP_ENABLE;			/* 0x00000001 : bits  0; ENABLE,PCI_I2O_BASE will respond to PCI cycles */


	/* PCI Access to Internal USC Registers disabled -PCI:USC 	*/
	V3USC_PCI_REG_BASE =
	0xffffff00						/*            : bits 31-9; Base address */
									/*			0 : bit 7; REG_EN is disabled */
									/*			0 : bit 6; decode ENABLE is disabled */
									/*          0 : bit 5,4; reserved */
	|PCI_REG_BASE_PREFETCH			/* 0x00000008 : bit 3; Prefetchable - no effect */
	|PCI_REG_BASE_TYPE_MASK			/* 0x00000006: bits 2-1; Address range */												
	|PCI_REG_BASE_IO;				/* 0x00000001: bit 0; As PCI I/O space */


    /* finally unreset the PCI bus and the onboard PCI devices */
    V3USC_SYSTEM_B |= SYSTEM_B_RST_OUT;
    sbddelay (1);

    /* initialise global data */
    {
	char *s;
	word v;
	if ((s = getenv ("pciiobase")) && atob (&v, s, 0))
	    pci_io_space_pci_base = v;
	else
	    pci_io_space_pci_base = PCI_IO_SPACE_PCI_BASE;
	if ((s = getenv ("pcimemase")) && atob (&v, s, 0))
	    pci_mem_space_pci_base = v;
	else
	    pci_mem_space_pci_base = PCI_MEM_SPACE_PCI_BASE;

	nextpciioaddr = pci_io_space_pci_base;
	nextpcimemaddr = pci_mem_space_pci_base;
    }
    _pci_bus[0] = v3usc_pci_bus;
}


/*
 * Called to reinitialise the bridge after we've scanned each PCI device
 * and know what is possible.
 */
void
_pci_hwreinit (void)
{
    char * const _v3uscp = PA_TO_KVA1(V3USC_BASE);

    if (_pci_bus[0].fast_b2b)
	/* fast back-to-back is supported by all devices */
	V3USC_PCI_CMD_W |= PCI_CMD_W_FBB_EN;

    /* set maximum bus latency timer */
    V3USC_PCI_HDR_CFG = 0xff << PCI_HDR_CFG_LT_SHIFT;

	V3USC_PCI_HDR_CFG = _pci_bus[0].def_ltim << PCI_HDR_CFG_LT_SHIFT;



#define V3USC_LB_PCI_BASEX_PREFETCH 	0x08

    if (_pci_bus[0].prefetch)
	/* we can safely prefetch from all pci devices */
	V3USC_LB_PCI_BASE1 |= V3USC_LB_PCI_BASEX_PREFETCH;
    /* clear latched PCI interrupts */
    V3USC_INT_STAT = 0;		


    /* enable V3USC interrupt */
    imask_bis (INTR_DEV_V3USC);
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

int
_pci_canscan(pcitag_t tag)
{
    return (1);
}

pcireg_t
_pci_conf_read(pcitag_t tag, int reg)
{
    char * const _v3uscp = PA_TO_KVA1(V3USC_BASE);
    u_int32_t addr, savereg, savereg2;
    pcireg_t data;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x100) {
	printf ("_pci_conf_read: bad reg %x\n", reg);
	return ~0;
    }

    pci_break_tag (tag, &bus, &device, &function); 
    if (bus != 0 || device > 20)
	return ~0;	/* device out of range */

	/* This next line assumes the a PCI backplane that connects the  */
	/* IDSEL line for each slot to a Address line with a small value */
	/* resistor.  Note: The PCI spec. suggest several options here   */
	/* This code assumes the following IDSEL to Address line mapping */

	/* Slot  Address Line */
	/*  0  - AD11 */
	/*  1  - AD12 */
	/*  2  - AD13 */
	/*  3  - AD14 */
	/*  4  - AD15 */

	/* 0x800 is AD11 set, shift left by the slot number (device) */
    addr = (0x800 << device) | (function << 8) | reg;

	/* printf("In _pci_conf_read ; addr = %08x \n", addr );	*/ 

    /* clear aborts */
	V3USC_PCI_STAT_W |=	PCI_STAT_W_M_ABORT | PCI_STAT_W_T_ABORT;

    /* low 20 bits of address are in the actual address */
    data = *(volatile pcireg_t *) PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xffff));

	/* printf("In _pci_conf_read ; data = %08x \n", data );	*/ 

    if (V3USC_PCI_STAT_W & PCI_STAT_W_M_ABORT) {
	V3USC_PCI_STAT_W |= PCI_STAT_W_M_ABORT;
#if 0
	printf ("device %d: master abort\n", device);
#endif
	return ~0;
    }

    if (V3USC_PCI_STAT_W & PCI_STAT_W_T_ABORT) {
	V3USC_PCI_STAT_W |= PCI_STAT_W_T_ABORT;
	printf ("PCI slot %d: target abort!\n", device);
	return ~0;
    }

	/* printf("In _pci_conf_read ; leaving function\n" ); */	

    return ltohl(data);
}


void
_pci_conf_write(pcitag_t tag, int reg, pcireg_t data)
{
    char * const _v3uscp = PA_TO_KVA1(V3USC_BASE);
    u_int32_t addr,savereg;
    int bus, device, function;

    if (reg & 3 || reg < 0 || reg >= 0x80) {
	printf ("_pci_conf_read: bad reg %x\n", reg);
	return;
    }

    pci_break_tag (tag, &bus, &device, &function);

    if (bus != 0 || device > 20)
	return;	/* device out of range */

	/* This next line assumes the a PCI backplane that connects the  */
	/* IDSEL line for each slot to a Address line with a small value */
	/* resistor.  Note: The PCI spec. suggest several options here   */
	/* This code assumes the following IDSEL to Address line mapping */

	/* Slot  Address Line */
	/*  0  - AD11 */
	/*  1  - AD12 */
	/*  2  - AD13 */
	/*  3  - AD14 */
	/*  4  - AD15 */

	/* 0x800 is AD11 set, shift left by the slot number (device) */
    addr = (0x800 << device) | (function << 8) | reg;

    /* clear aborts */
	V3USC_PCI_STAT_W |=	PCI_STAT_W_M_ABORT | PCI_STAT_W_T_ABORT;


    /* low 20 bits of address are in the actual address */
    *(volatile pcireg_t *) PA_TO_KVA1(PCI_CONF_SPACE + (addr&0xfffff)) = htoll(data);

    /* wait for write FIFO to empty */
    if (V3USC_PCI_STAT_W & PCI_STAT_W_M_ABORT) {
	V3USC_PCI_STAT_W |= PCI_STAT_W_M_ABORT;
	printf ("PCI slot %d: conf_write: master abort\n", device);
    }

    if (V3USC_PCI_STAT_W & PCI_STAT_W_T_ABORT) {
	V3USC_PCI_STAT_W |= PCI_STAT_W_T_ABORT;
	printf ("PCI slot %d: conf_write: target abort!\n", device);
    }
}


int
pci_map_io(pcitag_t tag, int reg, vm_offset_t *vap, vm_offset_t *pap)
{
    panic("pci_map_io: can't map i/o");
}


int
_pci_map_mem(tag, reg, vap, pap)
	pcitag_t tag;
	int reg;
	vm_offset_t *vap;
	vm_offset_t *pap;
{
	pcireg_t address;
	vm_offset_t pa;

	if (reg < PCI_MAP_REG_START || reg >= PCI_MAP_REG_END || (reg & 3))
		panic("pci_map_mem: bad request");

	address = _pci_conf_read(tag, reg);

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

	data = _pci_conf_read(tag, PCI_INTERRUPT_REG);

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
_pci_allocate_mem (pcitag_t tag, size_t size)
{
    pcireg_t address;

    address = (nextpcimemaddr + size - 1) & ~(size - 1);
    nextpcimemaddr = address + size;
    if (nextpcimemaddr > pci_mem_space_pci_base + PCI_MEM_SPACE_SIZE)
	panic ("not enough PCI memory space");
    return address;
}


pcireg_t
_pci_allocate_io (pcitag_t tag, size_t size)
{
    pcireg_t address;

    address = (nextpciioaddr + size - 1) & ~(size - 1);
    nextpciioaddr = address + size;
    if (nextpciioaddr > pci_io_space_pci_base + PCI_IO_SPACE_SIZE)
	panic ("not enough PCI i/o space");
    return address;
}
