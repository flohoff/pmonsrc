/* 
 * P5064/sbdflashenv.c: board-specific environment in Flash
 * Copyright (c) 1997  Algorithmics Ltd
 */

#ifdef IN_PMON
#include "pmon.h"
#include "sde-compat.h"
#else
#include <stddef.h>
#include <mips/cpu.h>
#include <kit_impl.h>
#endif
#include <flashdev.h>
#include <sbd.h>

flashcookie_t
_sbd_flashenv_open (unsigned int *env_offs, unsigned int *env_maxsize)
{
    flashcookie_t cookie;
    struct flashinfo info;

    /* XXX board specific address - wired-down flash prom */
    cookie = flash_open (FLASH_BASE);
    if (!cookie)
	return (flashcookie_t) 0;

    /* find flash size */
    if (flash_info (cookie, 0, &info) != FLASHDEV_OK)
	return (flashcookie_t) 0;

    /* find last sector at least 512 bytes down from top of flash */
    flash_info (cookie, info.size - 512, &info);

    if (info.sprot)
	/* must be writable */
	return (flashcookie_t) 0;
    
    *env_offs = info.soffs;
    *env_maxsize = info.ssize;

    return cookie;
}
