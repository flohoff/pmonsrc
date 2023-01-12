/*
 * sbdnvenv.c: optional Algo-style environment in r/w non-volatile memory.
 *
 * Copyright (c) 2000 Algorithmics Ltd - all rights reserved.
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

#include <sys/types.h>
#include "sbd.h"
#if defined(IN_PMON)
#include <mips/cpu.h>
#endif

#if defined(_SBD_NVENV)
#if defined(IN_PMON)
#include "nvenv.c"
#else
#include "../share/nvenv.c"
#endif
#endif

#if defined(_SBD_FLASHENV)
#if defined(IN_PMON)
#include "flashenv.c"
#else
#include "../share/flashenv.c"
#endif
#endif
