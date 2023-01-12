/*
 * r5kwatch.c: R5000 family watchpoint support for SDE-MIPS kit
 *
 * Copyright (c) 1998-2000 Algorithmics Ltd - all rights reserved.
 * 
 * This program is NOT free software, it is supplied under the terms
 * of the SDE-MIPS License Agreement, a copy of which is available at:
 *
 *  http://www.algor.co.uk/algor/info/sde-license.pdf
 *
 * Any company which has obtained and signed a valid SDE-MIPS license
 * may use and modify this software internally and use (without
 * restrictions) any derived binary.  You may not, however,
 * redistribute this in whole or in part as source code, nor may you
 * modify or remove any part of this copyright message.
 */

#ifdef R4000
#include <sys/types.h>
#include <mips/r5kc0.h>
#include <mips/cpu.h>
#include <mips/prid.h>

#include "watchpoint.h"

/*
 * R5000-family CPUs do not support watchpoints!
 */

static int	r5kwp_init (unsigned int);

const struct mips_watchpoint_funcs _r5k_watchpoint_funcs = {
    r5kwp_init
};


static int	
r5kwp_init (unsigned int prid)
{
    switch ((prid >> 8) & 0xff) {
    case PRID_R5000:
    case PRID_RM52XX:
    case PRID_RC6457X:
	/* recognised, but don't support watchpoints */
	return 0;
    default:
	return -1;
    }
}
#endif
