/*	$NetBSD: pcivar.h,v 1.8 1995/06/18 01:26:50 cgd Exp $	*/

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
 * Definitions for PCI autoconfiguration.
 *
 * This file describes types and functions which are used for PCI
 * configuration.  Some of this information is machine-specific, and is
 * separated into pci_machdep.h.
 */

#include <sys/types.h>

#if !defined(KERNEL) && !defined(__MACHINE_TYPES_H_)
typedef u_int32_t		vm_offset_t;
typedef __typeof(sizeof(int))	vm_size_t;
#endif

#ifndef NULL
#define NULL	(void *)0
#endif

#ifndef __P
#ifdef __STDC__
#define __P(args)	args
#else
#define __P(args)	()
#endif
#endif

#include "pci_machdep.h"

/* PCI interrupt levels; system interrupt levels for PCI bus use */
typedef enum {
	PCI_IPL_NONE,		/* block only the interrupt's IRQ*/
	PCI_IPL_BIO,		/* block I/O interrupts */
	PCI_IPL_NET,		/* network */
	PCI_IPL_TTY,		/* terminal */
	PCI_IPL_CLOCK,		/* clock */
} pci_intrlevel;

struct pci_attach_args {
	int pa_bus;
	int pa_device;
	int pa_function;
	pcitag_t pa_tag;
	pcireg_t pa_id, pa_class;
};

struct pci_match {
    pcireg_t	class, classmask;
    pcireg_t	id, idmask;
};

int	 _pci_hwinit __P((int));
void	 _pci_hwreinit __P((void));
pcitag_t _pci_make_tag __P((int, int, int));
void	 _pci_break_tag __P((pcitag_t, int *, int *, int *));
pcireg_t _pci_conf_read8 __P((pcitag_t, int));
void	 _pci_conf_write8 __P((pcitag_t, int, pcireg_t));
pcireg_t _pci_conf_read16 __P((pcitag_t, int));
void	 _pci_conf_write16 __P((pcitag_t, int, pcireg_t));
pcireg_t _pci_conf_read __P((pcitag_t, int));
void	 _pci_conf_write __P((pcitag_t, int, pcireg_t));
#define _pci_conf_read32 	_pci_conf_read
#define _pci_conf_write32	_pci_conf_write
pcireg_t _pci_conf_read __P((pcitag_t, int));
void	 _pci_conf_write __P((pcitag_t, int, pcireg_t));
int	 _pci_canscan __P((pcitag_t));
void	 _pci_flush __P((void));
int	 _pci_map_io __P((pcitag_t, int, vm_offset_t *, vm_offset_t *));
int	 _pci_map_port __P((pcitag_t, int, unsigned int *));
int	 _pci_map_mem __P((pcitag_t, int, vm_offset_t *, vm_offset_t *));
void	*_pci_map_int __P((pcitag_t, pci_intrlevel, int (*)(void *), void *));
void	 _pci_devinfo __P((pcireg_t, pcireg_t, char *, int *));
pcireg_t _pci_allocate_mem __P((pcitag_t, vm_size_t));
pcireg_t _pci_allocate_io __P((pcitag_t, vm_size_t));
void	 _pci_configure __P((int));
void	 _pci_init __P((void));
pcitag_t _pci_find __P((const struct pci_match *, unsigned int));
vm_offset_t _pci_dmamap (vm_offset_t va, unsigned int len);
vm_offset_t _pci_cpumap (vm_offset_t pcia, unsigned int len);
int	 _pci_cacheline_log2 (void);
int	 _pci_maxburst_log2 (void);

void	 _pci_bdfprintf (int bus, int device, int function, const char *fmt, ...);
void	 _pci_tagprintf (pcitag_t tag, const char *fmt, ...);

/* PCI bus is often accompanied by ISA bus */
void *	_isa_map_io (unsigned int port);
void *	_isa_map_mem (vm_offset_t addr);

#if 0
int	 pci_attach_subdev __P((int, int));
#endif

#ifdef IN_PMON
/* sigh... compatibility */
#define pci_hwinit _pci_hwinit
#define pci_hwreinit _pci_hwreinit
#define pci_make_tag _pci_make_tag
#define pci_break_tag _pci_break_tag
#define pci_conf_read _pci_conf_read
#define pci_conf_write _pci_conf_write
#define pci_map_port _pci_map_port
#define pci_map_io _pci_map_io
#define pci_map_mem _pci_map_mem
#define pci_map_int _pci_map_int
#define pci_devinfo _pci_devinfo
#define pci_configure _pci_configure
#define pci_allocate_mem _pci_allocate_mem
#define pci_allocate_io _pci_allocate_io
#endif

/* PCI bus parameters */
struct pci_bus {
    unsigned char	min_gnt;	/* largest min grant */
    unsigned char	max_lat;	/* smallest max latency */
    unsigned char	devsel;		/* slowest devsel */
    char		fast_b2b;	/* support fast b2b */
    char		prefetch;	/* support prefetch */
    char		freq66;		/* support 66MHz */
    char		width64;	/* 64 bit bus */
    int			bandwidth;	/* # of .25us ticks/sec @ 33MHz */
    unsigned char	ndev;		/* # devices on bus */
    unsigned char	def_ltim;	/* default ltim counter */
    unsigned char	max_ltim;	/* maximum ltim counter */
    u_int8_t		primary;	/* primary bus number */
    pcitag_t		tag;		/* tag for this bus */
    u_int32_t		min_io_addr;	/* min I/O address allocated to bus */
    u_int32_t		max_io_addr;	/* max I/O address allocated to bus */
    u_int32_t 		min_mem_addr;	/* min mem address allocated to bus */
    u_int32_t 		max_mem_addr;	/* max mem address allocated to bus */
};

extern struct pci_bus _pci_bus[];
extern int _pci_nbus;
extern const int _pci_maxbus;
