/*
 * p4032/sbdflash.c: onboard FLASH support for P4032 board
 * Copyright (c) 1996 Algorithmics Ltd.
 */


/*
 * sbdflash.c: SBD flash code
 */

#include "pmon.h"

#include "flash.h"

#ifdef PMON

#ifdef __GNUC__
typedef __typeof(sizeof(int))	size_t;   /* sizeof() */
#else
typedef unsigned int		size_t;
#endif
typedef unsigned int		vaddr_t;

#undef QUICKFLASH
#undef FLASHUSEALLOCA
#undef FLASHUSEMALLOC
#define FLASHUSEMALLOC

#endif

#define CLEANCACHE(s,n) mips_clean_cache ((vaddr_t)(s), (size_t)n)

#undef DBGDBG
#ifdef DBGDBG
#define DBG(fmt, args...) \
	_dbg_printf (fmt , ## args);
#else
#define DBG(fmt, args...)
#endif

#define UNLOCK(f) \
	*((f)+UNLOCK1) = 0xaa; \
	*((f)+UNLOCK2) = 0x55;

#define CMD(f,cmd) \
	UNLOCK(f); \
	*((f)+UNLOCK1) = cmd;

#ifdef QUICKFLASH

#define flashreset		int	_sbd_flashreset
#define flashautoselect		int	_sbd_flashautoselect
#define flashprogram		int	_sbd_flashprogram
#define flasherasechip		int	_sbd_flasherasechip
#define flasherasesector	int	_sbd_flasherasesector
#define flashprogramsector	int	_sbd_flashprogramsector
#define flashprogramchip	int	_sbd_flashprogramchip

#define SAFEROUTINE(name,protoargs,callargs)

#else

#define flashreset		static int	_flashreset
#define flashautoselect		static int	_flashautoselect
#define flashprogram		static int	_flashprogram
#define flasherasechip		static int	_flasherasechip
#define flasherasesector	static int	_flasherasesector
#define flashprogramsector	static int	_flashprogramsector
#define flashprogramchip	static int	_flashprogramchip

#if !defined(FLASHUSEALLOCA) && !defined(FLASHUSEMALLOC)
#define FLASHUSEALLOCA
#endif

#ifdef FLASHUSEALLOCA
#define CODECREATE(name, size) \
    void *code; \
    if (_sbd_flashsafe) { \
        code = name; \
    } \
    else { \
        code = alloca (size); \
        memcpy (code, name, size); \
	CLEANCACHE (code, size); \
    }
#endif

#ifdef FLASHUSEMALLOC
#define CODECREATE(name, size) \
    void *code; \
    static void *codep; \
    if (_sbd_flashsafe) \
        code = name; \
    else { \
       if (codep == 0) { \
           codep = (void *)malloc (size); \
           memcpy (codep, name, size); \
           CLEANCACHE (codep, size); \
       } \
       code = codep; \
    }
#endif

#define SAFEROUTINE(name,protoargs,callargs) \
int _sbd_##name protoargs \
{ \
    int (*fn)(); \
    const size_t codesize = (void *)_sbd_##name - (void *)_##name; \
    CODECREATE(_##name, codesize); \
    fn = (int (*)())code; \
    return (*fn) callargs; \
}

#endif

int	_sbd_flashsafe;

flashreset (void *flash)
{
    CMD((flash_t)flash, FLASH_RESET);
    return FLASHSTAT_OK;
}


SAFEROUTINE (flashreset, (void *flash), (flash))

flashautoselect (void *flash,
		     unsigned char *pmanufacturer,
		     unsigned char *pdevice,
		     unsigned int *pmap
		    )
{
    unsigned int map;
    
    CMD((flash_t)flash, FLASH_AUTOSELECT);

    *pmanufacturer = *(flash_t)(flash + 0*2);
    *pdevice = *(flash_t)(flash + 1*2);
	
    map = 0;
#define ADDMAP(sector) \
    map <<= 1; \
    map |= *(flash_t)(flash + (sector) + 2*2) & 1;

    ADDMAP(SA13);
    ADDMAP(SA12);
    ADDMAP(SA11);
    ADDMAP(SA10);
    ADDMAP(SA9);
    ADDMAP(SA8);
    ADDMAP(SA7);
    ADDMAP(SA6);
    ADDMAP(SA5);
    ADDMAP(SA4);
    ADDMAP(SA3);
    ADDMAP(SA2);
    ADDMAP(SA1);
    ADDMAP(SA0);

    CMD((flash_t)flash, FLASH_RESET);

    return FLASHSTAT_OK;
}

SAFEROUTINE(flashautoselect,
	    (void *flash, unsigned char *pmanufacturer,
	     unsigned char *pdevice, unsigned int *pmap),
	    (flash, pmanufacturer, pdevice, pmap))

flashprogram (void *flash, unsigned int offset, unsigned char v)
{
    unsigned char poll;
    flash_t fp = flash + offset;

    if (*fp == v)
	return FLASHSTAT_OK;

    CMD((flash_t)flash, FLASH_PROGRAM);

    *fp = v;
    
    for (;;) {
	poll = *fp;
	if (((poll ^ v) & DQPOLL) == 0)
	    return (FLASHSTAT_OK);
	if (poll & DQTIMEEXCEEDED) {
	    poll = *fp;
	    if (((poll ^ v) & DQPOLL) == 0)
		return (FLASHSTAT_OK);
	    return (FLASHSTAT_FAIL);
	}
    }
}

SAFEROUTINE(flashprogram,
	    (void *flash, unsigned int offset, unsigned char v),
	    (flash, offset, v))
	    

flasherasechip (void *flash)
{
    unsigned char poll;

    CMD((flash_t)flash, FLASH_ERASE);
    CMD((flash_t)flash, FLASH_ERASECHIP);
    
    for (;;) {
	poll = *(flash_t)flash;
	if (poll & DQPOLL)
	    return (FLASHSTAT_OK);
	if (poll & DQTIMEEXCEEDED) {
	    poll = *(flash_t)flash;
	    if (poll & DQPOLL)
		return (FLASHSTAT_OK);
	    return (FLASHSTAT_FAIL);
	}
    }
}

SAFEROUTINE(flasherasechip, (void *flash), (flash))

flasherasesector (void *flash, unsigned int sector)
{
    unsigned char poll;
    flash_t fp = flash + sector;

    CMD((flash_t)flash,FLASH_ERASE);
    UNLOCK((flash_t)flash);
    *fp = FLASH_ERASESECT;
    
    for (;;) {
	poll = *fp;
	if (poll & DQPOLL)
	    return (FLASHSTAT_OK);
	if (poll & DQTIMEEXCEEDED) {
	    poll = *fp;
	    if (poll & DQPOLL)
		return (FLASHSTAT_OK);
	    return (FLASHSTAT_FAIL);
	}
    }
}

SAFEROUTINE(flasherasesector,
	    (void *flash, unsigned int sector),
	    (flash, sector))

flashprogramsector (void *flash, unsigned int offset,
			 void *mem, int length)
{
    unsigned char poll;
    flash_t fp = flash + offset;
    unsigned char *mp = mem;
    int status;
    int i;

    status = FLASHSTAT_OK;
    
    /* check to see if the sector must be erased */ 
    for (i = 0; i < length; i++) {
	if (mp[i] & ~fp[i]) {
	    DBG ("mem[%d]=%02x fp[%d]=%02x -> erasing sector\n",
			 i, mp[i], i, fp[i]);
	    /* 
	     * some of the changing bits are not set in the flash
	     * so erase is necessary
	     */
	    CMD((flash_t)flash,FLASH_ERASE);
	    UNLOCK((flash_t)flash);
	    *fp = FLASH_ERASESECT;
    
	    for (;;) {
		poll = *fp;
		if (poll & DQPOLL)
		    break;
		if (poll & DQTIMEEXCEEDED) {
		    poll = *fp;
		    if ((poll & DQPOLL) == 0)
			status = FLASHSTAT_FAIL;
		    break;
		}
	    }
	    break;
	}
    }

    DBG ("starting program...");
    /* attempt to program even if erase failed */
    for (i = 0; i < length; i++) {
	if (fp[i] == mp[i])
	    continue;

	CMD((flash_t)flash, FLASH_PROGRAM);
	fp[i] = mp[i];
    
	for (;;) {
	    poll = fp[i];
	    if (((poll ^ mp[i]) & DQPOLL) == 0)
		break;
	    if (poll & DQTIMEEXCEEDED) {
		poll = fp[i];
		if (((poll ^ mp[i]) & DQPOLL) != 0)
		    status = FLASHSTAT_FAIL;
		break;
	    }
	}
    }

    DBG ("done\n");
    return status;
}

SAFEROUTINE(flashprogramsector,
	    (void *flash, unsigned int offset, void *mem, int length),
	    (flash, offset, mem, length))

flashprogramchip (void *flash, unsigned int offset,
		  void *mem, int length,
		  int erase, void (*restart)(void))
{
    unsigned char poll;
    flash_t fp = flash + offset;
    unsigned char *mp = mem;
    int status;
    int i;

    status = FLASHSTAT_OK;
    
    /* erase chip if required */ 
    if (erase) {
	DBG ("erasing chip...");
	CMD((flash_t)flash,FLASH_ERASE);
	CMD((flash_t)flash,FLASH_ERASECHIP);
    
	for (;;) {
	    poll = *fp;
	    if (poll & DQPOLL)
		break;
	    if (poll & DQTIMEEXCEEDED) {
		poll = *fp;
		if ((poll & DQPOLL) == 0)
		    status = FLASHSTAT_FAIL;
		break;
	    }
	}
    }

    DBG ("starting program...");

    /* attempt to program even if erase failed */
    for (i = 0; i < length; i++) {
	if (fp[i] == mp[i])
	    continue;

	CMD((flash_t)flash, FLASH_PROGRAM);
	fp[i] = mp[i];
    
	for (;;) {
	    poll = fp[i];
	    if (((poll ^ mp[i]) & DQPOLL) == 0)
		break;
	    if (poll & DQTIMEEXCEEDED) {
		poll = fp[i];
		if (((poll ^ mp[i]) & DQPOLL) != 0)
		    status = FLASHSTAT_FAIL;
		break;
	    }
	}
    }

    DBG ("done\n");

    if (restart) {
	DBG("rebooting\n");
	(*restart)();
    }

    return status;
}

SAFEROUTINE(flashprogramchip,
	    (void *flash, unsigned int offset, void *mem, int length, int erase, volatile void (*restart)(void)),
	    (flash, offset, mem, length, erase, restart))
