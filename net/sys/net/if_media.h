/*	$OpenBSD: if_media.h,v 1.2 1999/07/21 19:55:59 jason Exp $	*/
/*	$NetBSD: if_media.h,v 1.11 1998/08/12 23:23:29 thorpej Exp $	*/

/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Jason R. Thorpe of the Numerical Aerospace Simulation Facility,
 * NASA Ames Research Center.
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
 *	This product includes software developed by the NetBSD
 *	Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Copyright (c) 1997
 *	Jonathan Stone and Jason R. Thorpe.  All rights reserved.
 *
 * This software is derived from information provided by Matt Thomas.
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
 *	This product includes software developed by Jonathan Stone
 *	and Jason R. Thorpe for the NetBSD Project.
 * 4. The names of the authors may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _NET_IF_MEDIA_H_
#define _NET_IF_MEDIA_H_

/*
 * Prototypes and definitions for BSD/OS-compatible network interface
 * media selection.
 *
 * Where it is safe to do so, this code strays slightly from the BSD/OS
 * design.  Software which uses the API (device drivers, basically)
 * shouldn't notice any difference.
 *
 * Many thanks to Matt Thomas for providing the information necessary
 * to implement this interface.
 */

#ifdef _KERNEL

#include <sys/queue.h>

/*
 * Driver callbacks for media status and change requests.
 */
typedef	int (*ifm_change_cb_t) __P((struct ifnet *ifp));
typedef	void (*ifm_stat_cb_t) __P((struct ifnet *ifp, struct ifmediareq *req));


/*
 * In-kernel representation of a single supported media type.
 */
struct ifmedia_entry {
	LIST_ENTRY(ifmedia_entry) ifm_list;
	int	ifm_media;	/* description of this media attachment */
	int	ifm_data;	/* for driver-specific use */
	void	*ifm_aux;	/* for driver-specific use */
};

/*
 * One of these goes into a network interface's softc structure.
 * It is used to keep general media state.
 */
struct ifmedia {
	int	ifm_mask;	/* mask of changes we don't care about */
	int	ifm_media;	/* current user-set media word */
	struct ifmedia_entry *ifm_cur;	/* currently selected media */
	LIST_HEAD(, ifmedia_entry) ifm_list; /* list of all supported media */
	ifm_change_cb_t	ifm_change;	/* media change driver callback */
	ifm_stat_cb_t	ifm_status;	/* media status driver callback */
};

/* Initialize an interface's struct if_media field. */
void	ifmedia_init __P((struct ifmedia *ifm, int dontcare_mask,
	    ifm_change_cb_t change_callback, ifm_stat_cb_t status_callback));

/* Add one supported medium to a struct ifmedia. */
void	ifmedia_add __P((struct ifmedia *ifm, int mword, int data, void *aux));

/* Add an array (of ifmedia_entry) media to a struct ifmedia. */
void	ifmedia_list_add(struct ifmedia *mp, struct ifmedia_entry *lp,
	    int count);

/* Set default media type on initialization. */
void	ifmedia_set __P((struct ifmedia *ifm, int mword));

/* Common ioctl function for getting/setting media, called by driver. */
int	ifmedia_ioctl __P((struct ifnet *ifp, struct ifreq *ifr,
	    struct ifmedia *ifm, u_long cmd));

#endif /*_KERNEL */

/*
 * if_media Options word:
 *	Bits	Use
 *	----	-------
 *	0-3	Media variant
 *	4	RFU
 *	5-7	Media type
 *	8-15	Type specific options
 *	16-19	RFU
 *	20-27	Shared (global) options
 *	28-31	Instance
 */

/*
 * Ethernet
 */
#define IFM_ETHER	0x00000020
#define	IFM_10_T	3		/* 10BaseT - RJ45 */
#define	IFM_10_2	4		/* 10Base2 - Thinnet */
#define	IFM_10_5	5		/* 10Base5 - AUI */
#define	IFM_100_TX	6		/* 100BaseTX - RJ45 */
#define	IFM_100_FX	7		/* 100BaseFX - Fiber */
#define	IFM_100_T4	8		/* 100BaseT4 - 4 pair cat 3 */
#define	IFM_100_VG	9		/* 100VG-AnyLAN */
#define	IFM_100_T2	10		/* 100BaseT2 */
#define	IFM_1000_FX	11		/* 1000BaseFX - gigabit over fiber */
#define	IFM_10_STP	12		/* 10BaseT over shielded TP */
#define	IFM_10_FL	13		/* 10BaseFL - Fiber */
#define	IFM_1000_SX	14		/* 1000baseSX Multi-mode Fiber */
#define	IFM_1000_LX	15		/* 1000baseLX Single-mode Fiber */
#define	IFM_1000_CX	16		/* 1000baseCX 150ohm STP */
#define	IFM_1000_TX	17		/* 1000baseTX 4 pair cat 5 */

/*
 * Token ring
 */
#define	IFM_TOKEN	0x00000040
#define	IFM_TOK_STP4	3		/* Shielded twisted pair 4m - DB9 */
#define	IFM_TOK_STP16	4		/* Shielded twisted pair 16m - DB9 */
#define	IFM_TOK_UTP4	5		/* Unshielded twisted pair 4m - RJ45 */
#define	IFM_TOK_UTP16	6		/* Unshielded twisted pair 16m - RJ45 */
#define	IFM_TOK_ETR	0x00000200	/* Early token release */
#define	IFM_TOK_SRCRT	0x00000400	/* Enable source routing features */
#define	IFM_TOK_ALLR	0x00000800	/* All routes / Single route bcast */

/*
 * FDDI
 */
#define	IFM_FDDI	0x00000060
#define	IFM_FDDI_SMF	3		/* Single-mode fiber */
#define	IFM_FDDI_MMF	4		/* Multi-mode fiber */
#define IFM_FDDI_UTP	5		/* CDDI / UTP */
#define IFM_FDDI_DA	0x00000100	/* Dual attach / single attach */

/*
 * Shared media sub-types
 */
#define	IFM_AUTO	0		/* Autoselect best media */
#define	IFM_MANUAL	1		/* Jumper/dipswitch selects media */
#define	IFM_NONE	2		/* Deselect all media */

/*
 * Shared options
 */
#define IFM_FDX		0x00100000	/* Force full duplex */
#define	IFM_HDX		0x00200000	/* Force half duplex */
#define IFM_FLAG0	0x01000000	/* Driver defined flag */
#define IFM_FLAG1	0x02000000	/* Driver defined flag */
#define IFM_FLAG2	0x04000000	/* Driver defined flag */
#define	IFM_LOOP	0x08000000	/* Put hardware in loopback */

/*
 * Masks
 */
#define	IFM_NMASK	0x000000e0	/* Network type */
#define	IFM_TMASK	0x0000000f	/* Media sub-type */
#define	IFM_IMASK	0xf0000000	/* Instance */
#define	IFM_ISHIFT	28		/* Instance shift */
#define	IFM_OMASK	0x0000ff00	/* Type specific options */
#define	IFM_GMASK	0x0ff00000	/* Global options */

#define	IFM_NMIN	IFM_ETHER	/* lowest Network type */
#define	IFM_NMAX	IFM_NMASK	/* highest Network type */

/*
 * Status bits
 */
#define	IFM_AVALID	0x00000001	/* Active bit valid */
#define	IFM_ACTIVE	0x00000002	/* Interface attached to working net */

/*
 * Macros to extract various bits of information from the media word.
 */
#define	IFM_TYPE(x)	((x) & IFM_NMASK)
#define	IFM_SUBTYPE(x)	((x) & IFM_TMASK)
#define	IFM_INST(x)	(((x) & IFM_IMASK) >> IFM_ISHIFT)
#define	IFM_OPTIONS(x)	((x) & (IFM_OMASK|IFM_GMASK))

#define	IFM_INST_MAX	IFM_INST(IFM_IMASK)

/*
 * Macro to create a media word.
 */
#define	IFM_MAKEWORD(type, subtype, options, instance)			\
    ((type) | (subtype) | (options) | ((instance) << IFM_ISHIFT))

/*
 * NetBSD extension not defined in the BSDI API.  This is used in various
 * places to get the canonical description for a given type/subtype.
 *
 * In the subtype and mediaopt descriptions, the valid TYPE bits are OR'd
 * in to indicate which TYPE the subtype/option corresponds to.  If no
 * TYPE is present, it is a shared media/mediaopt.
 *
 * Note that these are parsed case-insensitive.
 *
 * Order is important.  The first matching entry is the canonical name
 * for a media type; subsequent matches are aliases.
 */
struct ifmedia_description {
	int	ifmt_word;		/* word value; may be masked */
	const char *ifmt_string;	/* description */
};

#define	IFM_TYPE_DESCRIPTIONS {						\
	{ IFM_ETHER,			"Ethernet" },			\
	{ IFM_ETHER,			"ether" },			\
	{ IFM_TOKEN,			"TokenRing" },			\
	{ IFM_TOKEN,			"token" },			\
	{ IFM_FDDI,			"FDDI" },			\
	{ 0, NULL },							\
}

#define	IFM_TYPE_MATCH(dt, t)						\
	(IFM_TYPE((dt)) == 0 || IFM_TYPE((dt)) == IFM_TYPE((t)))

#define	IFM_SUBTYPE_DESCRIPTIONS {					\
	{ IFM_AUTO,			"autoselect" },			\
	{ IFM_AUTO,			"auto" },			\
	{ IFM_MANUAL,			"manual" },			\
	{ IFM_NONE,			"none" },			\
									\
	{ IFM_ETHER|IFM_10_T,		"10baseT" },			\
	{ IFM_ETHER|IFM_10_T,		"10baseT/UTP" },		\
	{ IFM_ETHER|IFM_10_T,		"UTP" },			\
	{ IFM_ETHER|IFM_10_T,		"10UTP" },			\
	{ IFM_ETHER|IFM_10_2,		"10base2" },			\
	{ IFM_ETHER|IFM_10_2,		"10base2/BNC" },		\
	{ IFM_ETHER|IFM_10_2,		"BNC" },			\
	{ IFM_ETHER|IFM_10_2,		"10BNC" },			\
	{ IFM_ETHER|IFM_10_5,		"10base5" },			\
	{ IFM_ETHER|IFM_10_5,		"10base5/AUI" },		\
	{ IFM_ETHER|IFM_10_5,		"AUI" },			\
	{ IFM_ETHER|IFM_10_5,		"10AUI" },			\
	{ IFM_ETHER|IFM_100_TX,		"100baseTX" },			\
	{ IFM_ETHER|IFM_100_TX,		"100TX" },			\
	{ IFM_ETHER|IFM_100_FX,		"100baseFX" },			\
	{ IFM_ETHER|IFM_100_FX,		"100FX" },			\
	{ IFM_ETHER|IFM_100_T4,		"100baseT4" },			\
	{ IFM_ETHER|IFM_100_T4,		"100T4" },			\
	{ IFM_ETHER|IFM_100_VG,		"100baseVG" },			\
	{ IFM_ETHER|IFM_100_VG,		"100VG" },			\
	{ IFM_ETHER|IFM_100_T2,		"100baseT2" },			\
	{ IFM_ETHER|IFM_100_T2,		"100T2" },			\
	{ IFM_ETHER|IFM_1000_FX,	"1000baseFX" },			\
	{ IFM_ETHER|IFM_1000_FX,	"1000FX" },			\
	{ IFM_ETHER|IFM_10_STP,		"10baseSTP" },			\
	{ IFM_ETHER|IFM_10_STP,		"STP" },			\
	{ IFM_ETHER|IFM_10_STP,		"10STP" },			\
	{ IFM_ETHER|IFM_10_FL,		"10baseFL" },			\
	{ IFM_ETHER|IFM_10_FL,		"FL" },				\
	{ IFM_ETHER|IFM_10_FL,		"10FL" },			\
	{ IFM_ETHER|IFM_1000_SX,	"1000baseSX" },			\
	{ IFM_ETHER|IFM_1000_LX,	"1000baseLX" },			\
	{ IFM_ETHER|IFM_1000_CX,	"1000baseCX" },			\
	{ IFM_ETHER|IFM_1000_TX,	"1000baseTX" },			\
									\
	{ IFM_TOKEN|IFM_TOK_STP4,	"DB9/4Mbit" },			\
	{ IFM_TOKEN|IFM_TOK_STP4,	"4STP" },			\
	{ IFM_TOKEN|IFM_TOK_STP16,	"DB9/16Mbit" },			\
	{ IFM_TOKEN|IFM_TOK_STP16,	"16STP" },			\
	{ IFM_TOKEN|IFM_TOK_UTP4,	"UTP/4Mbit" },			\
	{ IFM_TOKEN|IFM_TOK_UTP4,	"4UTP" },			\
	{ IFM_TOKEN|IFM_TOK_UTP16,	"UTP/16Mbit" },			\
	{ IFM_TOKEN|IFM_TOK_UTP16,	"16UTP" },			\
									\
	{ IFM_FDDI|IFM_FDDI_SMF,	"Single-mode" },		\
	{ IFM_FDDI|IFM_FDDI_SMF,	"SMF" },			\
	{ IFM_FDDI|IFM_FDDI_MMF,	"Multi-mode" },			\
	{ IFM_FDDI|IFM_FDDI_MMF,	"MMF" },			\
	{ IFM_FDDI|IFM_FDDI_UTP,	"UTP" },			\
	{ IFM_FDDI|IFM_FDDI_UTP,	"CDDI" },			\
									\
	{ 0, NULL },							\
}

#define	IFM_OPTION_DESCRIPTIONS {					\
	{ IFM_FDX,			"full-duplex" },		\
	{ IFM_FDX,			"fdx" },			\
	{ IFM_HDX,			"half-duplex" },		\
	{ IFM_HDX,			"hdx" },			\
	{ IFM_FLAG0,			"flag0" },			\
	{ IFM_FLAG1,			"flag1" },			\
	{ IFM_FLAG2,			"flag2" },			\
	{ IFM_LOOP,			"loopback" },			\
	{ IFM_LOOP,			"hw-loopback"},			\
	{ IFM_LOOP,			"loop" },			\
									\
	{ IFM_TOKEN|IFM_TOK_ETR,	"EarlyTokenRelease" },		\
	{ IFM_TOKEN|IFM_TOK_ETR,	"ETR" },			\
	{ IFM_TOKEN|IFM_TOK_SRCRT,	"SourceRouting" },		\
	{ IFM_TOKEN|IFM_TOK_SRCRT,	"SRCRT" },			\
	{ IFM_TOKEN|IFM_TOK_ALLR,	"AllRoutes" },			\
	{ IFM_TOKEN|IFM_TOK_ALLR,	"ALLR" },			\
									\
	{ IFM_FDDI|IFM_FDDI_DA,		"dual-attach" },		\
	{ IFM_FDDI|IFM_FDDI_DA,		"das" },			\
									\
	{ 0, NULL },							\
}

#endif	/* _NET_IF_MEDIA_H_ */
