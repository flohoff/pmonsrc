#undef LEDEBUG
#define DWIO
/*	$OpenBSD: if_le_pci.c,v 1.12 1999/08/10 08:10:35 deraadt Exp $	*/
/*	$NetBSD: if_le_pci.c,v 1.13 1996/10/25 21:33:32 cgd Exp $	*/

/*-
 * Copyright (c) 1995 Charles M. Hannum.  All rights reserved.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Ralph Campbell and Rick Macklem.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)if_le.c	8.2 (Berkeley) 11/16/93
 */

#include "bpfilter.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/syslog.h>
#include <sys/socket.h>
#if defined(PROM)
#include <pci/device.h>
#else
#include <sys/device.h>
#endif

#include <vm/vm.h>
#include <vm/vm_kern.h>
#include <vm/vm_param.h>

#include <net/if.h>
#include <net/if_media.h>

#ifdef INET
#include <netinet/in.h>
#include <netinet/if_ether.h>
#endif

#include <vm/vm.h>

#include <machine/cpu.h>

#ifdef PROM

#include <pci/pcireg.h>
#include <pci/pcivar.h>
#include <pci/pcidevs.h>
#include "am7990reg.h"
#include "am7990var.h"

#include "if_levar.h"

#else

#include <machine/bus.h>
#include <machine/intr.h>

#include <dev/pci/pcireg.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcidevs.h>
#include <dev/ic/am7990reg.h>
#include <dev/ic/am7990var.h>

#include <dev/pci/if_levar.h>
#endif

#ifdef __alpha__			/* XXX */
/* XXX XXX NEED REAL DMA MAPPING SUPPORT XXX XXX */ 
#undef vtophys
#define	vtophys(va)	alpha_XXX_dmamap((vm_offset_t)(va))
#endif

int le_pci_match __P((struct device *, void *, void *));
void le_pci_attach __P((struct device *, struct device *, void *));

#ifdef PROM

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

#ifdef P6032
#define htorl(x) x
#define rtohl(x) x
#define htors(x) x
#define rtohs(x) x
#else
#define htorl(x) htoll(x)
#define rtohl(x) ltohl(x)
#define htors(x) htols(x)
#define rtohs(x) ltohs(x)
#endif

#define htoml(x) htoll(x)
#define mtohl(x) ltohl(x)
#define htoms(x) htols(x)
#define mtohs(x) ltohs(x)

hide void le_pci_wrcsr __P((struct am7990_softc *, u_int16_t, u_int16_t));
hide u_int16_t le_pci_rdcsr __P((struct am7990_softc *, u_int16_t));
hide void le_pci_wrbcr __P((struct am7990_softc *, u_int16_t, u_int16_t));
hide u_int16_t le_pci_rdbcr __P((struct am7990_softc *, u_int16_t));
hide void le_pci_wranr __P((struct am7990_softc *, u_int16_t, u_int16_t, u_int16_t));
hide u_int16_t le_pci_rdanr __P((struct am7990_softc *, u_int16_t, u_int16_t));

#include "am7990.c"

#undef vtophys
#define	vtophys(va)	_pci_dmamap ((vm_offset_t)(va), 0)

#else
struct cfattach le_pci_ca = {
	sizeof(struct le_softc), le_pci_match, le_pci_attach
};
#endif

/*
 * PCI constants.
 * XXX These should be in a common file!
 */
#define PCI_CBIO	0x10		/* Configuration Base IO Address */

hide void
le_pci_wrcsr(sc, port, val)
	struct am7990_softc *sc;
	u_int16_t port, val;
{
	struct le_softc *lesc = (struct le_softc *)sc;
#ifdef PROM
#ifdef DWIO
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rap) = htorl(port);
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rdp) = htorl(val);
#else
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rap) = htors(port);
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rdp) = htors(val);
#endif
#else
	bus_space_tag_t iot = lesc->sc_iot;
	bus_space_handle_t ioh = lesc->sc_ioh;

	bus_space_write_2(iot, ioh, lesc->sc_rap, port);
	bus_space_write_2(iot, ioh, lesc->sc_rdp, val);
#endif
}

hide u_int16_t
le_pci_rdcsr(sc, port)
	struct am7990_softc *sc;
	u_int16_t port;
{
	struct le_softc *lesc = (struct le_softc *)sc;
#ifdef PROM
	u_int16_t val;
#ifdef DWIO
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rap) = htorl(port);
	val = rtohl(*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rdp));
#else
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rap) = htors(port);
	val = rtohs(*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rdp));
#endif
#else
	bus_space_tag_t iot = lesc->sc_iot;
	bus_space_handle_t ioh = lesc->sc_ioh;
	u_int16_t val;

	bus_space_write_2(iot, ioh, lesc->sc_rap, port);
	val = bus_space_read_2(iot, ioh, lesc->sc_rdp);
#endif
	return (val);
}

hide void
le_pci_wrbcr(sc, port, val)
	struct am7990_softc *sc;
	u_int16_t port, val;
{
	struct le_softc *lesc = (struct le_softc *)sc;
#ifdef PROM
#ifdef DWIO
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rap) = htorl(port);
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_bdp) = htorl(val);
#else
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rap) = htors(port);
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_bdp) = htors(val);
#endif
#else
	bus_space_tag_t iot = lesc->sc_iot;
	bus_space_handle_t ioh = lesc->sc_ioh;

	bus_space_write_2(iot, ioh, lesc->sc_rap, port);
	bus_space_write_2(iot, ioh, lesc->sc_rbp, val);
#endif
}

hide u_int16_t
le_pci_rdbcr(sc, port)
	struct am7990_softc *sc;
	u_int16_t port;
{
	struct le_softc *lesc = (struct le_softc *)sc;
#ifdef PROM
	u_int16_t val;
#ifdef DWIO
	*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_rap) = htorl(port);
	val = rtohl(*(volatile u_int32_t *)(lesc->sc_iobase + lesc->sc_bdp));
#else
	*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_rap) = htors(port);
	val = rtohs(*(volatile u_int16_t *)(lesc->sc_iobase + lesc->sc_bdp));
#endif
#else
	bus_space_tag_t iot = lesc->sc_iot;
	bus_space_handle_t ioh = lesc->sc_ioh;
	u_int16_t val;

	bus_space_write_2(iot, ioh, lesc->sc_rap, port);
	val = bus_space_read_2(iot, ioh, lesc->sc_rbp);
#endif
	return (val);
}

hide void
le_pci_wranr(sc, phy, port, val)
	struct am7990_softc *sc;
	u_int16_t phy, port, val;
{
	le_pci_wrbcr (sc, 33, (phy << 5) | port);
	le_pci_wrbcr (sc, 34, val);
}

hide u_int16_t
le_pci_rdanr(sc, phy, port)
	struct am7990_softc *sc;
	u_int16_t phy, port;
{	
	le_pci_wrbcr (sc, 33, (phy << 5) | port);
	return le_pci_rdbcr (sc, 34);
}

int
le_pci_match(parent, match, aux)
	struct device *parent;
	void *match, *aux;
{
	struct pci_attach_args *pa = aux;

	if (PCI_VENDOR(pa->pa_id) != PCI_VENDOR_AMD)
		return (0);

	switch (PCI_PRODUCT(pa->pa_id)) {
	case PCI_PRODUCT_AMD_PCNET_PCI:
	case PCI_PRODUCT_AMD_PCHOME_PCI:
		return (1);
	}

	return (0);
}

void
le_pci_attach(parent, self, aux)
	struct device *parent, *self;
	void *aux;
{
	struct le_softc *lesc = (void *)self;
	struct am7990_softc *sc = &lesc->sc_am7990;
	struct pci_attach_args *pa = aux;
#ifdef PROM
	vm_offset_t iobase;
	vm_offset_t paiobase;
#else
	pci_intr_handle_t ih;
	bus_addr_t iobase;
	bus_size_t iosize;
	bus_space_handle_t ioh;
	bus_space_tag_t iot = pa->pa_iot;
	pci_chipset_tag_t pc = pa->pa_pc;
#endif
	pcireg_t csr;
	int i;
	const char *intrstr;

	switch (PCI_PRODUCT(pa->pa_id)) {
	case PCI_PRODUCT_AMD_PCNET_PCI:
		lesc->sc_rap = PCNET_PCI_RAP;
		lesc->sc_rdp = PCNET_PCI_RDP;
		lesc->sc_bdp = PCNET_PCI_BDP;
		break;
	}

#ifdef PROM
	if (pci_map_io (pa->pa_tag, PCI_CBIO, &iobase, &paiobase)) {
		printf(": can't find I/O base\n");
		return;
	}
#else
	if (pci_io_find(pc, pa->pa_tag, PCI_CBIO, &iobase, &iosize)) {
		printf(": can't find I/O base\n");
		return;
	}
	if (bus_space_map(iot, iobase, iosize, 0, &ioh)) {
		printf(": can't map I/O space\n");
		return;
	}
#endif

#ifdef PROM	
	lesc->sc_iobase = iobase;
#ifdef DWIO
	/* do switch to 32-bit mode (for 5230!) */
	le_pci_wrcsr (sc, 0, 0);
#endif
	if ((le_pci_rdbcr (sc, 19) & ((1<<15)|(1<<14))) == 0) {
	    /* These are the bits that should be in EEROM */
	    unsigned short bits[64] = {
		0x4000, 0x05bc, 0xffff,	/* MAC address */
		0x0000,		/* CSR116 (On Now Misc. Configuration) */
		0x1100,		/* Hardware ID (MB 0x11); Reserved (MBZ) */
		0x1111,		/* Scratch */
		0x0270,		/* csum bytes 00-0b & 0e-0f */
		0x5757,		/* MB 0x5757 */
		0x0000,		/* BCR2 (Miscellaneous Configuration */
		0x00c0,		/* BCR4 Link Status LED = link up */
		0x0084,		/* BCR5 LED1 = Rx Activity */
		0x1080,		/* BCR6 LED2 = 100Mbs */
		0x0090,		/* BCR7 LED3 = Tx Activity */
		0x0000,		/* BCR9 Full-Duplex Control */
		0x9061,		/* BCR18 Burst & Bus Control */
		0x1818,		/* BCR22 PCI Latency */
		0x1022,		/* BCR23 PCI subsystem vendor */
		0x2000,		/* BCR24 PCI subsystem ID */
		0x0017,		/* BCR25 SRAM size */
		0x0008,		/* BCR26 SRAM boundary */
		0x0000,		/* BCR27 SRAM control */
		0x0000,		/* BCR32 MII CSR */
		0x03c0,		/* BCR33 MII address  */
		0x1022,		/* BCR35 PCI Vendor ID */
		0xfe11,		/* BCR36 PCI PMC */
		0x0114,		/* BCR37 PCI Data1  */
		0x010f,		/* BCR38 PCI Data2 */
		0x010f,		/* BCR39 PCI Data3 */
		0x010f,		/* BCR40 PCI Data4 */
		0x0114,		/* BCR41 PCI Data5 */
		0x010f,		/* BCR42 PCI Data6 */
		0x010f,		/* BCR43 PCI Data7 */
		0x010f,		/* BCR44 PCI Data8 */
		0x0000,		/* BCR48 MBZ */
		0x0000,		/* BCR49 MBZ */
		0x0000,		/* BCR50 MBZ */
		0x0000,		/* BCR51 MBZ */
		0x0000,		/* BCR52 MBZ */
		0x0000,		/* BCR53 MBZ */
		0x0000,		/* BCR54 MBZ */

		0x1900,		/* Checksum adjust byte; MBZ */

		/* Rest not used */
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
		0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
	    };

	    /* patch ethaddr */
	    sbdethaddr (&bits[0]);

	    /* fix the checksum */
	    {
		int csum = 0;
		for (i = 0; i < 8; i++)
		{
		    if (i == 6)
			continue;
		    csum += bits[i] & 0xff;
		    csum += (bits[i] >> 8 ) & 0xff;
		}
		bits[6] = csum;
	    }
	    /* enable writes to LED regs & shadow RAM */
	    le_pci_wrbcr (sc, 2, (1<<12) | (1<<8));

	    /* copy first 16 bytes to address PROM space */
#ifdef DWIO
	    {
		volatile unsigned int *iob = (unsigned int *)iobase;
		unsigned int *bitsp = (unsigned int *)bits;
		for (i = 0; i < 4; i++) {
		    iob[i] = htorl(bitsp[i]);
		}
	    }
	    {
		unsigned short *iob = (unsigned short *)iobase;
		for (i = 0; i < 3; i++)
		    le_pci_wrcsr (sc, 12+i, bits[i]);
	    }
#else
	    {
		unsigned short *iob = (unsigned short *)iobase;
		for (i = 0; i < 8; i++) {
		    iob[i] = htols(bits[i]);
		    if (i < 3)
			le_pci_wrcsr (sc, 12+i, bits[i]);
		}
	    }
#endif

	    /* back to regular programming mode */
	    le_pci_wrcsr (sc, 116, bits[0x03]);
	    le_pci_wrbcr (sc, 4, bits[0x09]);
	    le_pci_wrbcr (sc, 5, bits[0x0a]);
	    le_pci_wrbcr (sc, 6, bits[0x0b]);
	    le_pci_wrbcr (sc, 7, bits[0x0c]);

	    le_pci_wrbcr (sc, 9, bits[0x0d]);

	    le_pci_wrbcr (sc, 18, bits[0x0e]);

	    le_pci_wrbcr (sc, 22, bits[0x0f]);
	    le_pci_wrbcr (sc, 23, bits[0x10]);
	    le_pci_wrbcr (sc, 24, bits[0x11]);
	    le_pci_wrbcr (sc, 25, bits[0x12]);
	    le_pci_wrbcr (sc, 26, bits[0x13]);
	    le_pci_wrbcr (sc, 27, bits[0x14]);

	    le_pci_wrbcr (sc, 32, bits[0x15]);
	    le_pci_wrbcr (sc, 33, bits[0x16]);

	    le_pci_wrbcr (sc, 35, bits[0x17]);
	    le_pci_wrbcr (sc, 36, bits[0x18]);
	    le_pci_wrbcr (sc, 37, bits[0x19]);
	    le_pci_wrbcr (sc, 38, bits[0x1a]);
	    le_pci_wrbcr (sc, 39, bits[0x1b]);
	    le_pci_wrbcr (sc, 40, bits[0x1c]);
	    le_pci_wrbcr (sc, 41, bits[0x1d]);
	    le_pci_wrbcr (sc, 42, bits[0x1e]);
	    le_pci_wrbcr (sc, 43, bits[0x1f]);
	    le_pci_wrbcr (sc, 44, bits[0x20]);

	    le_pci_wrbcr (sc, 48, bits[0x21]);
	    le_pci_wrbcr (sc, 49, bits[0x22]);
	    le_pci_wrbcr (sc, 50, bits[0x23]);
	    le_pci_wrbcr (sc, 51, bits[0x24]);
	    le_pci_wrbcr (sc, 52, bits[0x25]);
	    le_pci_wrbcr (sc, 53, bits[0x26]);
	    le_pci_wrbcr (sc, 54, bits[0x27]);
	    
	    /* finally write BCR2 */
	    le_pci_wrbcr (sc, 2, bits[0x08]);
		
	}
#endif

	/*
	 * Extract the physical MAC address from the ROM.
	 */
#ifdef PROM
	{
	    unsigned char *iob = (unsigned char *)iobase;
#if #endian(big) && defined(P6032)
	    for (i = 0; i < sizeof(sc->sc_arpcom.ac_enaddr); i++)
		sc->sc_arpcom.ac_enaddr[i] = iob[i^3];
#else
	    for (i = 0; i < sizeof(sc->sc_arpcom.ac_enaddr); i++)
		sc->sc_arpcom.ac_enaddr[i] = iob[i];
#endif
	}
#else
	for (i = 0; i < sizeof(sc->sc_arpcom.ac_enaddr); i++)
		sc->sc_arpcom.ac_enaddr[i] = bus_space_read_1(iot, ioh, i);
#endif

	sc->sc_mem = malloc(16384, M_DEVBUF, M_NOWAIT);
	if (sc->sc_mem == 0) {
		printf(": couldn't allocate memory for card\n");
		return;
	}
#ifdef __mips__
	sc->sc_mem = PA_TO_KVA1(KVA_TO_PA(sc->sc_mem));
#endif

#ifndef PROM
	printf("\n");

	lesc->sc_iot = iot;
	lesc->sc_ioh = ioh;
#endif

	sc->sc_conf3 = 0;
#ifdef PROM
	sc->sc_conf3 |= 1 << 6;		/* set DXSUFLO */
#endif
	sc->sc_addr = vtophys(sc->sc_mem);	/* XXX XXX XXX */
	sc->sc_memsize = 16384;
	sc->sc_copytodesc = am7990_copytodesc_contig;
	sc->sc_copyfromdesc = am7990_copyfromdesc_contig;
	sc->sc_copytobuf = am7990_copytobuf_contig;
	sc->sc_copyfrombuf = am7990_copyfrombuf_contig;
	sc->sc_zerobuf = am7990_zerobuf_contig;

	sc->sc_rdcsr = le_pci_rdcsr;
	sc->sc_wrcsr = le_pci_wrcsr;
	sc->sc_hwreset = NULL;
	sc->sc_hwinit = NULL;

#ifndef PROM
	printf("%s", sc->sc_dev.dv_xname);
#endif
	am7990_config(sc);

#ifdef PROM
	/* Enable the card. */
	csr = pci_conf_read(pa->pa_tag, PCI_COMMAND_STATUS_REG);
	pci_conf_write(pa->pa_tag, PCI_COMMAND_STATUS_REG,
	    csr | PCI_COMMAND_MASTER_ENABLE);
#else
	/* Enable the card. */
	csr = pci_conf_read(pc, pa->pa_tag,
	    PCI_COMMAND_STATUS_REG);
	pci_conf_write(pc, pa->pa_tag, PCI_COMMAND_STATUS_REG,
	    csr | PCI_COMMAND_MASTER_ENABLE);

	/* Map and establish the interrupt. */
	if (pci_intr_map(pc, pa->pa_intrtag, pa->pa_intrpin,
	    pa->pa_intrline, &ih)) {
		printf("%s: couldn't map interrupt\n", sc->sc_dev.dv_xname);
		return;
	}
	intrstr = pci_intr_string(pc, ih);
	lesc->sc_ih = pci_intr_establish(pc, ih, IPL_NET, am7990_intr, sc,
	    sc->sc_dev.dv_xname);
	if (lesc->sc_ih == NULL) {
		printf("%s: couldn't establish interrupt",
		    sc->sc_dev.dv_xname);
		if (intrstr != NULL)
			printf(" at %s", intrstr);
		printf("\n");
		return;
	}
	printf("%s: interrupting at %s\n", sc->sc_dev.dv_xname, intrstr);
#endif
}

