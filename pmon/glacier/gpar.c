/*
 * gpar.c: GRIN parallel interface driver
 *
 */

#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>

#include <gpar.h>


#define NGPAR	1

static void *gparbase[NGPAR];

static int
gparinit (void *dp)
{
    pGPAR(dp);

    GPAR_PRESCALE = GPAR_PRESCALE_ENABLE | GPARPRESCALE;
    GPAR_CONTROL = GPAR_CONTROL_nFAULT | GPAR_CONTROL_STRBSEL | GPAR_CONTROL_PLH;
    GPAR_500NS = GPAR500NS << GPAR_500NS_SHIFT;
    
    return 0;
}


gpar (int op, char *dat, int chan, int data)
{
    void *dp;

    switch (op) {
    case OP_INIT:
	return gparinit (dp);

    case OP_RESET:
	break;

    case OP_TXRDY:
	return 1;		/* always ready */

    case OP_TX:
	break;			/* discard transmit data */

    case OP_RXRDY:
        {
	    pGPAR(dp);
	    return (GPAR_STATUS & GPAR_STATUS_RXREADY);
	}

    case OP_RX:
        {
	    pGPAR(dp);
	    return (GPAR_DATA & 0xff);
	}

    case OP_RXSTOP:
	break;

    case OP_FLUSH:
	if (data & 1) {
	    pGPAR(dp);
	    while (GPAR_STATUS & GPAR_STATUS_RXREADY)
		(void)GPAR_DATA;
	}
	break;
    }

    return 0;
}


/* PCI autoconf */


#include "pci/pcivar.h"
#include "pci/pcireg.h"

#include "pci/device.h"
#include "pci/pcidevs.h"

static int gpar_probe (struct device *parent, void *match, void *aux);
static void gpar_attach (struct device * const parent,
			struct device * const self, void * const aux);

typedef struct gpar_softc {
    struct device gpar_dev;		/* base device */
    void *csr;
} gpar_softc_t;


struct cfdriver gparcd = {
    0, "gpar", gpar_probe, gpar_attach, DV_TTY, sizeof(gpar_softc_t), 0, 0
};


static int
gpar_probe (struct device *parent, void *match, void *aux)
{
    struct pci_attach_args *pa = (struct pci_attach_args *) aux;

#if 0
    _mon_printf ("gpar_probe 0x%x\r\n", pa->pa_id);
#endif

    if (PCI_VENDOR(pa->pa_id) != PCI_VENDOR_HP)
	return 0;
    if (PCI_PRODUCT(pa->pa_id) != PCI_PRODUCT_HP_GRINSER)
	return 0;

    return 1;
}

static void
gpar_attach (struct device * const parent,
	     struct device * const self, void * const aux)
{
    gpar_softc_t * const sc = (gpar_softc_t *) self;
    struct pci_attach_args * const pa = (struct pci_attach_args *) aux;
    int unit = sc->gpar_dev.dv_unit;
    vm_offset_t vap, pap;
    struct sbdser const *ss;

    if (unit > NGPAR)
	return;

    if (pci_map_mem(pa->pa_tag, PCI_MAP_REG_START, &vap, &pap) != 0)
	return;

    sc->csr = (void *)vap;
    gparbase[unit] = sc->csr;
}

