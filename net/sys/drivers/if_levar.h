/*	$OpenBSD: if_levar.h,v 1.4 1997/11/07 08:07:29 niklas Exp $	*/
/*	$NetBSD: if_levar.h,v 1.3 1996/10/21 22:56:46 thorpej Exp $	*/

/*
 * LANCE Ethernet driver header file
 *
 * Copyright (c) 1994, 1995 Charles M. Hannum.  All rights reserved.
 *
 * Copyright (C) 1993, Paul Richards. This software may be used, modified,
 *   copied, distributed, and sold, in both source and binary form provided
 *   that the above copyright and these terms are retained. Under no
 *   circumstances is the author responsible for the proper functioning
 *   of this software, nor does the author assume any responsibility
 *   for damages incurred with its use.
 */

#ifdef DWIO
#define	PCNET_PCI_RDP	0x10
#define	PCNET_PCI_RAP	0x14
#define	PCNET_PCI_BDP	0x1c
#else
#define	PCNET_PCI_RDP	0x10
#define	PCNET_PCI_RAP	0x12
#define	PCNET_PCI_BDP	0x16
#endif

/*
 * Ethernet software status per interface.
 *
 * Each interface is referenced by a network interface structure,
 * arpcom.ac_if, which the routing code uses to locate the interface.
 * This structure contains the output queue for the interface, its address, ...
 */
struct le_softc {
	struct	am7990_softc sc_am7990;	/* glue to MI code */

	void	*sc_ih;
#ifdef PROM
	vm_offset_t sc_iobase;
#else
	bus_space_tag_t sc_iot;		/* space cookie */
	bus_space_handle_t sc_ioh;	/* bus space handle */
#endif
	int	sc_rap, sc_rdp, sc_bdp;		/* offsets to LANCE registers */
};
