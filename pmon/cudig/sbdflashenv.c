/* 
 * CUDIG/sbdflashenv.c: board-specific environment in Flash
 * Copyright (c) 1999  Algorithmics Ltd
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

flashcookie_t
_sbd_flashenv_open (unsigned int *env_offs, unsigned int *env_maxsize)
{
    flashcookie_t cookie;
    struct flashinfo info;
    unsigned int eoffs, esize;

    cookie = flash_open (0);
    if (!cookie)
	return (flashcookie_t) 0;
 
    /* find flash size */
    if (flash_info (cookie, 0, &info) != FLASHDEV_OK)
	return (flashcookie_t) 0;

    /* find at least 512 bytes at top of flash */
    eoffs = info.size; esize = 0; 
    while (esize < 512) {
	flash_info (cookie, eoffs - 1, &info);
	eoffs = info.soffs;
	if (info.sprot) /* must be writable */
	    esize = 0;
	else
	    esize += info.ssize;
    }
    
    *env_offs = eoffs;
    *env_maxsize = esize;
    return cookie;
}
