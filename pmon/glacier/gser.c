#include <mips.h>
#include <termio.h>
#include <pmon.h>
#include <sbd.h>

#include <gser.h>

#define NGSER	1

static void *gserbase[NGSER];

static const unsigned short gserprescale[] = {
    -1,
    GSERPRESCALE (50),
    GSERPRESCALE (75),
    GSERPRESCALE (110),
    GSERPRESCALE (134),
    GSERPRESCALE (150),
    GSERPRESCALE (200),
    GSERPRESCALE (300),
    GSERPRESCALE (600),
    GSERPRESCALE (1200),
    GSERPRESCALE (1800),
    GSERPRESCALE (2400),
    GSERPRESCALE (4800),
    GSERPRESCALE (9600),
    GSERPRESCALE (19200),
    GSERPRESCALE (38400)};

static int
gserinit (void *dp, int chan)
{
    pGSER(dp);
    return 0;
}


static int
gserprogram (void *dp, int chan, int baudrate)
{
    pGSER(dp);
    unsigned short brtc;
    int timeout;

    /* wait for Tx fifo to completely drain */
    timeout = 10000;
    while ((GSER_TXCSR & GSER_TXCSR_EMPTY) == 0)
	if (--timeout == 0)
	    break;

    baudrate &= CBAUD;
    if (baudrate == 0)
	return 1;

    GSER_PRESCALE = gserprescale[baudrate];
    GSER_CFG = GSER_CFG_IDTR | GSER_CFG_5WIRE | GSER_CFG_8BIT;
    GSER_RXCSR = GSER_RXCSR_SIE;
    GSER_TXCSR = GSER_TXCSR_XON | GSER_TXCSR_SOE;

    return 0;
}


gser (int op, char *dat, int chan, int data)
{
    void *dp;
    
    if (chan > NGSER)
	return 1;
    
    switch (op) {
    case OP_INIT:
	return gserinit (dp, chan);

    case OP_BAUD:
	return gserprogram (dp, chan, data);

    case OP_TXRDY:
        {
	    pGSER(dp);
	    return (GSER_TXCSR & GSER_TXCSR_EMPTY);
	}

    case OP_TX:
        {
	    pGSER(dp);
	    GSER_DATA = data; wbflush ();
	}
        break;

    case OP_RXRDY:
        {
	    pGSER(dp);
	    return (GSER_RXCSR & GSER_RXCSR_FULL);
	}

    case OP_RX:
        {
	    pGSER(dp);
	    return GSER_DATA & 0xff;
	}

    case OP_RXSTOP:
        {
	    pGSER(dp);
	    if (data)
		GSER_TXCSR |= GSER_TXCSR_XOFF;	    /* disable Rx */
	    else
		GSER_TXCSR |= GSER_TXCSR_XON;	    /* enable Rx */
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

static int gser_probe (struct device *parent, void *match, void *aux);
static void gser_attach (struct device * const parent,
			struct device * const self, void * const aux);

typedef struct gser_softc {
    struct device gser_dev;		/* base device */
    void *csr;
} gser_softc_t;


struct cfdriver gsercd = {
    0, "gser", gser_probe, gser_attach, DV_TTY, sizeof(gser_softc_t), 0, 0
};


static int
gser_probe (struct device *parent, void *match, void *aux)
{
    struct pci_attach_args *pa = (struct pci_attach_args *) aux;

#if 0
    _mon_printf ("gser_probe 0x%x\r\n", pa->pa_id);
#endif

    if (PCI_VENDOR(pa->pa_id) != PCI_VENDOR_HP)
	return 0;
    if (PCI_PRODUCT(pa->pa_id) != PCI_PRODUCT_HP_GRINSER)
	return 0;

    return 1;
}

static void
gser_attach (struct device * const parent,
	     struct device * const self, void * const aux)
{
    gser_softc_t * const sc = (gser_softc_t *) self;
    struct pci_attach_args * const pa = (struct pci_attach_args *) aux;
    int unit = sc->gser_dev.dv_unit;
    vm_offset_t vap, pap;
    struct sbdser const *ss;

    if (unit > NGSER)
	return;

    if (pci_map_mem(pa->pa_tag, PCI_MAP_REG_START, &vap, &pap) != 0)
	return;

    sc->csr = (void *)vap;
    gserbase[unit] = sc->csr;
}

