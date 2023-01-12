/*
 * p6032/sbdflash.c: Flash support for Algorithmics P-6032
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

#ifdef IN_PMON
#include <sys/types.h>
#include <mips/cpu.h>
#include "pmon.h"
#else
#include <sys/types.h>
#include <mips/cpu.h>
#include <stdlib.h>
#include <assert.h>
#include "kit_impl.h"
#endif
#include "flashdev.h"
#include "sbd.h"

/* 
 * Indirect calls to device-specific modules.
 */

flashcookie_t
_sbd_flashopen (paddr_t devaddr)
{
    flashcookie_t cp;
    if (devaddr == 0)
	cp = _sbd_uflashopen (0);
    else if (devaddr == 1)
	cp = _sbd_bflashopen (0);
    else {
	cp = _sbd_uflashopen (devaddr);
	if (cp == FLASH_MISSING || cp == FLASH_UNKNOWN)
	    cp = _sbd_bflashopen (devaddr);
    }
    return cp;
}
