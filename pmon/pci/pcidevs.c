/*
 * pcidevs.c
 *
 * Derived from NetBSD kernel with following copyright:
 *
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

#ifdef IN_PMON
#include "mips.h"
#include "pmon.h"
#include "stdio.h"
#else
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include "pcivar.h"
#include "pcireg.h"
#include "pcidevs.h"

extern int _pciverbose;

/*
 * Descriptions of known PCI classes and subclasses.
 *
 * Subclasses are described in the same way as classes, but have a
 * NULL subclass pointer.
 */
struct pci_class {
        const char *const name;
	int		val;		/* as wide as pci_{,sub}class_t */
	const struct pci_class *const subclasses;
};

static const struct pci_class pci_subclass_prehistoric[] = {
	{ "miscellaneous",	PCI_SUBCLASS_PREHISTORIC_MISC,		},
	{ "VGA",		PCI_SUBCLASS_PREHISTORIC_VGA,		},
	{ 0 }
};

static const struct pci_class pci_subclass_mass_storage[] = {
	{ "SCSI",		PCI_SUBCLASS_MASS_STORAGE_SCSI,		},
	{ "IDE",		PCI_SUBCLASS_MASS_STORAGE_IDE,		},
	{ "floppy",		PCI_SUBCLASS_MASS_STORAGE_FLOPPY,	},
	{ "IPI",		PCI_SUBCLASS_MASS_STORAGE_IPI,		},
	{ "RAID",		PCI_SUBCLASS_MASS_STORAGE_RAID,		},
	{ "miscellaneous",	PCI_SUBCLASS_MASS_STORAGE_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_network[] = {
	{ "ethernet",		PCI_SUBCLASS_NETWORK_ETHERNET,		},
	{ "token ring",		PCI_SUBCLASS_NETWORK_TOKENRING,		},
	{ "FDDI",		PCI_SUBCLASS_NETWORK_FDDI,		},
	{ "ATM",		PCI_SUBCLASS_NETWORK_ATM,		},
	{ "miscellaneous",	PCI_SUBCLASS_NETWORK_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_display[] = {
	{ "VGA",		PCI_SUBCLASS_DISPLAY_VGA,		},
	{ "XGA",		PCI_SUBCLASS_DISPLAY_XGA,		},
	{ "miscellaneous",	PCI_SUBCLASS_DISPLAY_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_multimedia[] = {
	{ "video",		PCI_SUBCLASS_MULTIMEDIA_VIDEO,		},
	{ "audio",		PCI_SUBCLASS_MULTIMEDIA_AUDIO,		},
	{ "miscellaneous",	PCI_SUBCLASS_MULTIMEDIA_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_memory[] = {
	{ "RAM",		PCI_SUBCLASS_MEMORY_RAM,		},
	{ "flash",		PCI_SUBCLASS_MEMORY_FLASH,		},
	{ "miscellaneous",	PCI_SUBCLASS_MEMORY_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_bridge[] = {
	{ "host",		PCI_SUBCLASS_BRIDGE_HOST,		},
	{ "ISA",		PCI_SUBCLASS_BRIDGE_ISA,		},
	{ "EISA",		PCI_SUBCLASS_BRIDGE_EISA,		},
	{ "MicroChannel",	PCI_SUBCLASS_BRIDGE_MC,			},
	{ "PCI",		PCI_SUBCLASS_BRIDGE_PCI,		},
	{ "PCMCIA",		PCI_SUBCLASS_BRIDGE_PCMCIA,		},
	{ "NuBus",		PCI_SUBCLASS_BRIDGE_NUBUS,		},
	{ "CardBus",		PCI_SUBCLASS_BRIDGE_CARDBUS,		},
	{ "miscellaneous",	PCI_SUBCLASS_BRIDGE_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_communication[] = {
	{ "serial",		PCI_SUBCLASS_COMMUNICATION_SERIAL,	},
	{ "parallel",		PCI_SUBCLASS_COMMUNICATION_PARALLEL,	},
	{ "miscellaneous",	PCI_SUBCLASS_COMMUNICATION_MISC,	},
	{ 0 },
};

static const struct pci_class pci_subclass_peripheral[] = {
	{ "PIC",		PCI_SUBCLASS_PERIPHERAL_PIC,		},
	{ "DMA",		PCI_SUBCLASS_PERIPHERAL_DMA,		},
	{ "timer",		PCI_SUBCLASS_PERIPHERAL_TIMER,		},
	{ "RTC",		PCI_SUBCLASS_PERIPHERAL_RTC,		},
	{ "miscellaneous",	PCI_SUBCLASS_PERIPHERAL_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_input[] = {
	{ "keyboard",		PCI_SUBCLASS_INPUT_KEYBOARD,		},
	{ "digitiser",		PCI_SUBCLASS_INPUT_DIGITISER,		},
	{ "mouse",		PCI_SUBCLASS_INPUT_MOUSE,		},
	{ "miscellaneous",	PCI_SUBCLASS_INPUT_MISC,		},
	{ 0 },
};

static const struct pci_class pci_subclass_docking[] = {
	{ "generic",		PCI_SUBCLASS_DOCKING_GENERIC,		},
	{ "miscellaneous",	PCI_SUBCLASS_DOCKING_MISC,		},
	{ 0 },
};

const struct pci_class pci_subclass_processor[] = {
	{ "386",		PCI_SUBCLASS_PROCESSOR_386,		},
	{ "486",		PCI_SUBCLASS_PROCESSOR_486,		},
	{ "Pentium",		PCI_SUBCLASS_PROCESSOR_PENTIUM,		},
	{ "Alpha",		PCI_SUBCLASS_PROCESSOR_ALPHA,		},
	{ "PowerPC",		PCI_SUBCLASS_PROCESSOR_POWERPC,		},
	{ "coprocessor",	PCI_SUBCLASS_PROCESSOR_COPROCESSOR,	},
	{ 0 },
};

const struct pci_class pci_subclass_serialbus[] = {
	{ "FireWire",		PCI_SUBCLASS_SERIALBUS_FIREWIRE,	},
	{ "Access",		PCI_SUBCLASS_SERIALBUS_ACCESSS,		},
	{ "SSA",		PCI_SUBCLASS_SERIALBUS_SSA,		},
	{ "USB",		PCI_SUBCLASS_SERIALBUS_USB,		},
	{ "FibreChannel",	PCI_SUBCLASS_SERIALBUS_FIBRECHANNEL,	},
	{ 0 },
};

static const struct pci_class pci_class[] = {
	{ "prehistoric",	PCI_CLASS_PREHISTORIC,
	    pci_subclass_prehistoric,				},
	{ "mass storage",	PCI_CLASS_MASS_STORAGE,
	    pci_subclass_mass_storage,				},
	{ "network",		PCI_CLASS_NETWORK,
	    pci_subclass_network,				},
	{ "display",		PCI_CLASS_DISPLAY,
	    pci_subclass_display,				},
	{ "multimedia",		PCI_CLASS_MULTIMEDIA,
	    pci_subclass_multimedia,				},
	{ "memory",		PCI_CLASS_MEMORY,
	    pci_subclass_memory,				},
	{ "bridge",		PCI_CLASS_BRIDGE,
	    pci_subclass_bridge,				},
	{ "communication",	PCI_CLASS_COMMUNICATION,
	    pci_subclass_communication,				},
	{ "peripheral",		PCI_CLASS_PERIPHERAL,
	    pci_subclass_peripheral,				},
	{ "input",		PCI_CLASS_INPUT,
	    pci_subclass_input,					},
	{ "docking",		PCI_CLASS_DOCKING,
	    pci_subclass_docking,				},
	{ "processor",		PCI_CLASS_PROCESSOR,
	    pci_subclass_processor,				},
	{ "serialbus",		PCI_CLASS_SERIALBUS,
	    pci_subclass_serialbus,				},
	{ "undefined",		PCI_CLASS_UNDEFINED,
	    0,							},
	{ 0 },
};

/*
 * Descriptions of of known vendors and devices ("products").
 */
struct pci_knowndev {
	pci_vendor_id_t		vendor;
	pci_product_id_t	product;
	int			flags;
        const char		*const vendorname, *const productname;
};
#define	PCI_KNOWNDEV_UNSUPP	0x01		/* unsupported device */
#define	PCI_KNOWNDEV_NOPROD	0x02		/* match on vendor only */

#include "pcidevs_data.h"


void
_pci_devinfo(pcireg_t id_reg, pcireg_t class_reg, char *cp, int *supp)
{
	pci_vendor_id_t vendor;
	pci_product_id_t product;
	pci_class_t class;
	pci_subclass_t subclass;
	pci_interface_t interface;
	pci_revision_t revision;
	const char *vendor_namep, *product_namep;
	const struct pci_class *classp;
	const struct pci_class *subclassp;
	const struct pci_knowndev *kdp;

	vendor = PCI_VENDOR(id_reg);
	product = PCI_PRODUCT(id_reg);

	class = PCI_CLASS(class_reg);
	subclass = PCI_SUBCLASS(class_reg);
	interface = PCI_INTERFACE(class_reg);
	revision = PCI_REVISION(class_reg);

	kdp = pci_knowndevs;
        while (kdp->vendorname != NULL) {	/* all have vendor name */
                if (kdp->vendor == vendor && (kdp->product == product ||
		    (kdp->flags & PCI_KNOWNDEV_NOPROD) != 0))
                        break;
		kdp++;
	}
        if (kdp->vendorname == NULL) {
		vendor_namep = product_namep = NULL;
		if (supp != NULL)
			*supp = 0;
        } else {
		vendor_namep = kdp->vendorname;
		product_namep = (kdp->flags & PCI_KNOWNDEV_NOPROD) == 0 ?
		    kdp->productname : NULL;
		if (supp != NULL)
			*supp = (kdp->flags & PCI_KNOWNDEV_UNSUPP) == 0;
        }

	classp = pci_class;
	while (classp->name != NULL) {
		if (class == classp->val)
			break;
		classp++;
	}

	subclassp = (classp->name != NULL) ? classp->subclasses : NULL;
	while (subclassp && subclassp->name != NULL) {
		if (subclass == subclassp->val)
			break;
		subclassp++;
	}

	if (vendor_namep == NULL)
		cp += sprintf(cp, "vendor/product: 0x%04x/0x%04x",
		    vendor, product);
	else if (product_namep != NULL)
		cp += sprintf(cp, "%s %s", vendor_namep, product_namep);
	else
		cp += sprintf(cp, "%s, product: 0x%x",
		    vendor_namep, product);
	cp += sprintf(cp, " (");
	if (classp->name == NULL)
		cp += sprintf(cp, "class/subclass: 0x%02x/0x%02x",
		    class, subclass);
	else {
		cp += sprintf(cp, "%s, ", classp->name);
 		if (subclassp == NULL || subclassp->name == NULL)
		    cp += sprintf(cp, "subclass: 0x%02x",
				  subclass);
		else
		    cp += sprintf(cp, "%s", subclassp->name);
	}
	if (_pciverbose >= 2) {
	    cp += sprintf(cp, ", interface: 0x%02x", interface);
	    cp += sprintf(cp, ", revision: 0x%02x", revision);
	}
	strcpy (cp, ")");
}
