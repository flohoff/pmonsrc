/*
 * nvenv.c: generic Algo-style environment in r/w non-volatile memory.
 *
 * Copyright (c) 1997-1999 Algorithmics Ltd - all rights reserved.
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

/*
 * See flashenv.c for Flash ROM environment support.
 */

#ifdef IN_PMON
#include <stddef.h>
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include "kit_impl.h"
#endif

#include <errno.h>
#include "nvenv.h"

#ifdef _SBD_RTCENV
static int		env_state = ENVSTATE_UNKNOWN;
static int		env_init_state;
static unsigned int	env_offs;
static unsigned int	env_maxsize;


static __inline__ unsigned char 
env_getbyte (unsigned int offs)
{
    return _sbd_nvram_getbyte (env_offs + offs);
}


static __inline__ void
env_setbyte (unsigned int offs, unsigned char val)
{
    _sbd_nvram_setbyte (env_offs + offs, val);
}


static __inline__ unsigned short
env_getshort (unsigned int offs)
{
    return _sbd_nvram_getshort (env_offs + offs);
}


static __inline__ void
env_setshort (unsigned int offs, unsigned short val)
{
    _sbd_nvram_setshort (env_offs + offs, val);
}

static void
env_setstate (int state)
{
    env_setbyte (ENVOFF_STATE, env_state = state);
}


static __inline__ void
env_setrecoverable (void)
{
    env_setstate (ENVSTATE_RECOVERABLE);
}


static __inline__ void
env_setunrecoverable (void)
{
    env_setstate (ENVSTATE_UNRECOVERABLE);
}



/*
 * calculate environment checksum
 */
static unsigned short
env_calcsum (void)
{
    unsigned short sum = ENV_MAGIC;
    unsigned int i;
    
    for (i = ENVOFF_BASE; i < env_maxsize; i += 2)
	sum += env_getshort(i);
    return (sum);
}


/*
 * update the checksum
 */
static void
env_updatesum (void)
{
    env_setshort (ENVOFF_CSUM, env_calcsum ());
    env_setstate (ENVSTATE_OK);
    _sbd_nvram_flush (-1);
}


/*
 * check that the environment structure is valid
 */
static int
env_structcheck ()
{
    unsigned int envsize, envp, top;
    
    envsize = env_getshort (ENVOFF_SIZE);
    if (envsize > env_maxsize - ENVOFF_BASE)
	return (0);

    envp = ENVOFF_BASE; 
    top = ENVOFF_BASE + envsize;

    while (envp < top) {
	unsigned int varsize = env_getbyte (envp);
	unsigned int i;
	
	if (varsize == 0 || envp + varsize > top) {
#ifdef NVENV_DEBUG
	    _mon_printf ("nvenv: bad variable size %d at offs 0x%x\n", 
			 varsize, envp);
#endif
	    return (0);		/* sanity test */
	}
	
	for (i = envp + 1; i < envp + varsize; i++) {
	    unsigned char c = env_getbyte (i);
	    if (c == '=')
		break;		/* end of name */
	    if (c < ' ' || c >= 0x7f) {
#ifdef NVENV_DEBUG
		_mon_printf ("nvenv: bad char in name at offs 0x%x\n", envp);
#endif
		return (0);	/* name must be printable */
	    }
	}

	envp += varsize;
    }

    return (1);
}


/*
 * completely reset the environment to be empty and valid
 */
static void
env_reset (void)
{
    env_setshort (ENVOFF_MAGIC, ENV_MAGIC);
    env_setshort (ENVOFF_SIZE, 0);
    env_updatesum ();
}


#pragma weak _sbd_env_force_reset=_stub_sbd_env_force_reset
int _stub_sbd_env_force_reset (void);
int _stub_sbd_env_force_reset ()
{
    return 0;
}


/*
 * load env and check its validity.
 */
static int
env_invalid (void)
{
    if (env_state == ENVSTATE_UNKNOWN) {

	/* find out how much space is available to us */
	(void) _sbd_nvram_size (&env_offs, &env_maxsize);

	/* must be word aligned */
	if (env_offs & 1)
	    ++env_offs, --env_maxsize;
	if (env_maxsize & 1)
	    --env_maxsize;

	/* initialise env and get initial state */
	env_init_state = env_getbyte (ENVOFF_STATE);
	if (_sbd_env_force_reset ())
	    /* user requested reset */
	    env_init_state = ENVSTATE_FORCERESET;
	else if (env_getshort (ENVOFF_MAGIC) != ENV_MAGIC)
	    /* bad magic: must be uninitialised */
	    env_init_state = ENVSTATE_UNINITIALISED;
	else if (env_getshort (ENVOFF_SIZE) > env_maxsize - ENVOFF_BASE) {
	    /* bad size: must be corrupt */
#ifdef NVENV_DEBUG
	    _mon_printf ("nvenv: size %d > max %d\n", 
			 env_getshort (ENVOFF_SIZE), 
			 env_maxsize - ENVOFF_BASE);
#endif
	    env_init_state = ENVSTATE_UNRECOVERABLE;
	}
	else if (env_calcsum () != env_getshort (ENVOFF_CSUM)) {
	    /* bad checksum: may be recoverable */
	    if (env_init_state != ENVSTATE_RECOVERABLE) {
		/* but wasn't! */
#ifdef NVENV_DEBUG
		_mon_printf ("nvenv: sum 0x%04x != stored sum 0x%04x\n", 
			     env_calcsum (),
			     env_getshort (ENVOFF_CSUM));
#endif
		env_init_state = ENVSTATE_UNRECOVERABLE;
	    }
	}

	env_state = env_init_state;
      
	/* take recovery action if necessary */
	switch (env_init_state) {

	default:
	    /* may be an old env with uninitialised state, reset to OK */
	    env_init_state = ENVSTATE_OK;
	    /* drop through */

	case ENVSTATE_RECOVERABLE:
	    /* just recalculate the checksum */
	    env_updatesum ();
	    /* drop through */

	case ENVSTATE_OK:
	    /* seems ok, but do a final check of the structure */
	    if (env_structcheck ())
		/* ok, we pass */
		break;

	    /* oh dear, we have to completely reset it */
	    env_init_state = ENVSTATE_UNRECOVERABLE;
	    /* drop through */

	case ENVSTATE_UNRECOVERABLE:
	case ENVSTATE_UNINITIALISED:
#ifdef NVENV_DEBUG
	    break;
#endif

	case ENVSTATE_FORCERESET:
	    /* reset to a blank environment */
	    env_reset ();
	    break;
	}
    }

    return env_state;
}



/* 
 * return offset of named environment entry
 */

static int
env_find (const char *name)
{
    unsigned int envsize, envp, top, namelen, varsize, i;

    envsize = env_getshort (ENVOFF_SIZE);
    if (envsize > env_maxsize - ENVOFF_BASE)
	return (0);			/* sanity */

    namelen = strlen (name);
    if (name == 0 || namelen >= 255)
	return (0);

    envp = ENVOFF_BASE; 
    top = ENVOFF_BASE + envsize;

    while (envp < top) {

	varsize = env_getbyte (envp);
	if (varsize == 0 || envp + varsize > top)
	    break;		/* sanity */

	--varsize; ++envp;
	if (env_getbyte (envp) != '=' && varsize >= namelen) {

	    /* find length of match */
	    for (i = 0; i < namelen; i++)
		if (env_getbyte (envp + i) != (unsigned char)name[i])
		    break;

	    if (i == namelen &&
		(i == varsize ||			/* match on boolean */
		 env_getbyte (envp + i) == '='))	/* match on variable */
		return (envp - 1);	
	}

	envp += varsize;
    }

    return (0);
}



/*
 * delete named environment entry
 */

static int
env_delete (const char *name)
{
    unsigned int nenvp, envp, envsize, varsize, nremain;
  
    envp = env_find (name);
    if (envp == 0)
	return 0;

    envsize = env_getshort (ENVOFF_SIZE); 
    varsize = env_getbyte (envp);

    /* unrecoverable because we're changing the layout */
    env_setunrecoverable ();

    /* splice out this entry by moving higher entries down */
    nenvp = envp + varsize;			 /* next entry */
    nremain = envsize - (nenvp - ENVOFF_BASE); /* # bytes after this */
    while (nremain--)
	env_setbyte (envp++, env_getbyte (nenvp++));

    env_setshort (ENVOFF_SIZE, envp - ENVOFF_BASE);
    env_updatesum ();
    return 1;
}



int
_sbd_setenv (const char *name, const char *val, int overwrite)
{
    unsigned int namelen, vallen, total, envsize;
    unsigned int oldsize = 0; 
    unsigned int envp;

    if (env_invalid ()) {
	errno = ENODEV;
	return -1;
    }

    namelen = name ? strlen (name) : 0;
    if (namelen == 0) {
	errno = EINVAL;
	return -1;
    }

    total = 1 + namelen;		/* size . "name" */

    vallen = val ? strlen (val) : 0; 
    if (vallen)
	total += 1 + vallen;		/* . '=' . "value" */
    if (total > 255) {			/* max variable size */
	errno = ENAMETOOLONG;
	return -1;
    }

    envp = env_find (name);

    if (envp) {

	if (!overwrite)
	    return 0;

	oldsize = env_getbyte (envp);

	if (oldsize == total) {
	    /* same size: replace value in place */
	    if (vallen) {
		/* recoverable because we do not move any other variables */
		env_setrecoverable ();
		envp += 1 + namelen + 1;
		while (vallen--)
		    env_setbyte (envp++, *val++);
		env_updatesum ();
	    }
	    return 0;
	}
    }


    envsize = env_getshort (ENVOFF_SIZE);
    if (envsize - oldsize + total > env_maxsize - ENVOFF_BASE) {
	/* not enough room */
	errno = ENOSPC;
	return -1;
    }

    if (envp) {
	/* delete any old entry */
	env_delete (name);
	envsize = env_getshort (ENVOFF_SIZE); /* envsize has changed */
    }

    /* recoverable because we don't update the size until we finish */
    env_setrecoverable ();
    envp = ENVOFF_BASE + envsize;
    env_setbyte (envp++, (unsigned char) total);
    while (namelen--)
	env_setbyte (envp++, *name++);
    if (vallen) {
	env_setbyte (envp++, '=');
	while (vallen--)
	    env_setbyte (envp++, *val++);
    }

    env_setshort (ENVOFF_SIZE, envp - ENVOFF_BASE);
    env_updatesum ();
    return 0;
}


char *
_sbd_getenv (const char *name)
{
    unsigned int envp, namelen, varlen;
    static char buf[256];		/* FIXME: this cannot be static */
    char *s = buf;

    if (env_invalid ()) {
	errno = ENODEV;
	return ((char *)0);
    }

    envp = env_find (name);
    if (envp == 0) {
	errno = ENOENT;
	return ((char *)0);
    }

    namelen = strlen (name);
    varlen = env_getbyte (envp); 
    if (varlen > namelen + 2) { 	/* !boolean */
	int n = varlen - (namelen + 2);
	envp += namelen + 2;
	while (n--)
	    *s++ = env_getbyte (envp++);
    }
    *s = '\0';
    return (buf);
}


void
_sbd_unsetenv (const char *s)
{
    if (!env_invalid ())
	(void) env_delete (s);
}


#ifdef IN_PMON
#define _sbd_mapenv	sbd_mapenv
#endif

/*
 * apply func to each string in environment
 */
void
_sbd_mapenv (int (*func)(const char *, const char *))
{
    unsigned int envsize, envp, top;
    char buf[256];
    
    if (env_invalid ())
	return;

    envsize = env_getshort(ENVOFF_SIZE);
    if (envsize > env_maxsize - ENVOFF_BASE)
	return;			/* sanity */

    envp = ENVOFF_BASE;
    top = ENVOFF_BASE + envsize;

    while (envp < top) {
	char *s = buf; 
	char *val = 0;
	int varlen = env_getbyte (envp);

	if (varlen == 0 || envp + varlen > top)
	    break;		/* sanity clause */

	++envp;
	while (--varlen) {
	    char c = env_getbyte (envp++);
	    if ((c == '=') && !val) {
		*s++ = '\0';
		val = s;
	    }
	    else
		*s++ = c;
	}
	*s = '\0';

	if (!val)
	    val = s;

	(*func) (buf, val);
    }
}

#ifdef IN_PMON
int sbd_setenv (const char *name, const char *val)
{
    return (_sbd_setenv (name, val, 1) == 0);
}


int sbd_unsetenv (const char *name)
{
    _sbd_unsetenv (name);
    return 1;
}
#endif



const char *
_sbd_envinit (void)
{
    const char *err;

    /* load and check env */
    (void) env_invalid ();

    if (env_init_state == ENVSTATE_OK)
	return NULL;

    /* report any initial failure */
    switch (env_init_state) {
    case ENVSTATE_UNINITIALISED:
	err = "uninitialised";
	break;
    case ENVSTATE_UNRECOVERABLE:
	err = "unrecoverable error";
	break;
    case ENVSTATE_RECOVERABLE:
	err = "recoverable checksum error";
	break;
    case ENVSTATE_FORCERESET:
	err = "reset forced by user";
	break;
    default:
	err = "bad state";
	break;
    }
    env_init_state = ENVSTATE_OK;

    if (!_sbd_nvram_hwtest ())
	err = "hardware test failure";

    return err;
}
#endif
