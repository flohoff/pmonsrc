/*
 * watchpoint.h: PMON h/w watchpoint and debug support
 *
 * Copyright (c) 1998-2000, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
 */


#define WATCHPOINT_MAX	8	/* max # of watchpoints for all cpus */

struct mips_watchpoint {
    int			num;
    int			capabilities;
    void		*private;
    unsigned short	type;
    short		asid;
    vaddr_t		va;
    paddr_t		pa;
    size_t		len;
    vaddr_t		mask;
};

/* Per-cpu watchpoint support functions */
struct mips_watchpoint_funcs {
    int		(*wp_init) (unsigned int);
    void	(*wp_setup) (struct mips_watchpoint *);
    int		(*wp_hit) (vaddr_t *, size_t *);
    int		(*wp_set) (struct mips_watchpoint *);
    int		(*wp_clear) (struct mips_watchpoint *);
    void	(*wp_remove) (void);
    void	(*wp_insert) (void);
    void	(*wp_reset) (void);
};


/* watchpoint and other debug capabilities (returned) */
#define MIPS_WATCHPOINT_INEXACT	0x8000	/* inexact (unmatched) watchpoint */
#define MIPS_WATCHPOINT_SSTEP	0x1000	/* single-step supported */
#define MIPS_WATCHPOINT_VALUE	0x0400	/* data value match supported */
#define MIPS_WATCHPOINT_ASID	0x0200	/* ASID match supported */
#define MIPS_WATCHPOINT_VADDR	0x0100	/* virtual address (not physical) */
#define MIPS_WATCHPOINT_RANGE	0x0080	/* supports an address range */
#define MIPS_WATCHPOINT_MASK	0x0040	/* supports an address mask */
#define MIPS_WATCHPOINT_DWORD	0x0020	/* dword alignment (8 bytes) */
#define MIPS_WATCHPOINT_WORD	0x0010	/* word alignment (4 bytes) */

/* watchpoint capabilities and type (when setting) */
#define MIPS_WATCHPOINT_X	0x0004	/* instruction fetch wp */
#define MIPS_WATCHPOINT_R	0x0002	/* data read wp */
#define MIPS_WATCHPOINT_W	0x0001	/* data write wp */

/* watchpoint support functions */
int	_mips_watchpoint_init (void);
int	_mips_watchpoint_howmany (void);
int	_mips_watchpoint_capabilities (int);
int	_mips_watchpoint_hit (vaddr_t *, size_t *);
int	_mips_watchpoint_set (int, int, vaddr_t, paddr_t, size_t);
int	_mips_watchpoint_clear (int, int, vaddr_t, size_t);
void	_mips_watchpoint_remove (void);
void	_mips_watchpoint_insert (void);
void	_mips_watchpoint_reset (void);

/* internal utility functions for watchpoint code */
unsigned long	_mips_watchpoint_calc_mask (vaddr_t, size_t);
int		_mips_watchpoint_address (int, vaddr_t *, size_t *);
int		_mips_watchpoint_set_callback (int, vaddr_t, size_t);

/* return codes from set/clear */
#define MIPS_WP_OK		0
#define MIPS_WP_NONE		1
#define MIPS_WP_NOTSUP		2
#define MIPS_WP_INUSE		3
#define MIPS_WP_NOMATCH		4
#define MIPS_WP_OVERLAP		5
#define MIPS_WP_BADADDR		6

/* in exception handler */
#ifdef IN_PMON
#include <pmon.h>
#define _mips_watchlo	DBGREG[R_WATCHLO]
#define _mips_watchhi	DBGREG[R_WATCHHI]
#define _mips_watchmask	DBGREG[R_WATCHMASK]
#else
extern reg_t _mips_watchlo, _mips_watchhi;
#endif
