/*
 * devaz/sbdflash.c: Flash support for Algorithmics DEVA-0
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
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
