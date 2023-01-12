/*
 * flashenv.c: generic Algo-style environment in Flash memory.
 * Copyright (c) 1997  Algorithmics Ltd
 */

#ifdef IN_PMON
#include <pmon.h>
#include <sde-compat.h>
#else
#include <stddef.h>
#include <mips/cpu.h>
#include <kit_impl.h>
#endif
#include <flashdev.h>
#include <flashenv.h>
#include <errno.h>

static int		env_state = ENVSTATE_UNKNOWN;
static int		env_init_state;
static flashcookie_t	env_cookie;	/* handle on flash */
static unsigned int	env_offs; 	/* base offset of env in flash */
static unsigned int	env_maxsize; 	/* maximum hiwater offset */
#ifdef IN_PMON
static unsigned char	*env_compbuf; 	/* compression sector buffer */
#endif
static int		envsize;	/* current hiwater in flash */
static int		envfree;	/* total unused bytes */

static __inline__ void
env_flush (void)
{
    /* null pointer causes flush */
    (void) flash_programbytes (env_cookie, 0, 0, 0, 0);
}


static __inline__ void
env_getbytes (unsigned int offs, unsigned char *bp, unsigned int nb)
{
    (void) flash_readbytes (env_cookie, env_offs + offs, bp, nb, 
			    FLASHDEV_READ_STREAM);
}


static __inline__ void
env_setbytes (unsigned int offs, const unsigned char *bp, unsigned int nb)
{
    (void) flash_programbytes (env_cookie, env_offs + offs, bp, nb, 
			       FLASHDEV_PROG_STREAM);
}


static __inline__ unsigned char 
env_getbyte (unsigned int offs)
{
    unsigned char v;
    env_getbytes (offs, &v, 1);
    return v;
}

static __inline__ void
env_setbyte (unsigned int offs, unsigned char val)
{
    unsigned char v = val;
    env_setbytes (offs, &v, 1);
}


/*
 * check that the environment structure is valid
 */
static int
env_structcheck ()
{
    unsigned int envp, top;
    unsigned char c;
    
    envp = ENVOFF_BASE; 
    top = env_maxsize;
    envfree = 0;

    while (envp < top) {
	unsigned int varsize = env_getbyte (envp);
	unsigned int i;
	
	if (varsize == 0xff)
	    break;		/* end of allocated area */

	if (varsize <= 1 || envp + varsize > top)
	    return 0;		/* sanity test */
	
	/* get first byte of name */
	c = env_getbyte (envp+1);
	if (c != 0x00 && c != 0xff) { 
	    /* valid, active entry */
	    if (c == '=')
		return 0;		/* first char can't be '=' */
	    for (i = envp + 1; i < envp + varsize; i++) {
		c = env_getbyte (i);
		if (c == '=')
		    break;		/* end of name (don't check value) */
		if (c < ' ' || c >= 0x7f)
		    return 0;		/* name must be printable ascii */
	    }
	}
	else {
	    /* deleted or partially written entry */
	    envfree += varsize;
	}

	/* advance to next entry */
	envp += varsize;
    }

    /* remember current hiwater */
    envsize = envp - ENVOFF_BASE;

    /* check that the remainder is uninitialised */
    while (envp < top) {
	if (env_getbyte (envp++) != 0xff)
	    return 0;
	envfree++;
    }

    return 1;
}


/*
 * completely reset the environment to be empty and valid
 */
static void
env_reset (void)
{
    static const unsigned char magic[2] = {ENV_MAGIC_0, ENV_MAGIC_1};

    (void) flash_erasesector (env_cookie, env_offs);
    env_setbytes (ENVOFF_MAGIC, magic, 2);
    env_flush ();

    envsize = 0;
    envfree = env_maxsize - ENVOFF_BASE;
    env_state = ENVSTATE_OK;
}


/*
 * load env and check its validity.
 */
static int
env_invalid (void)
{
    extern flashcookie_t _sbd_flashenv_open (unsigned int *, unsigned int *);
    unsigned char magic[2];
    int ok;

    if (env_state != ENVSTATE_UNKNOWN)
	return env_state;

    /* find out how much space is available to us */
    env_cookie = _sbd_flashenv_open (&env_offs, &env_maxsize);
    
    /* have we got enough/any space? */
    if (!env_cookie || env_maxsize < ENVOFF_BASE + 64) {
	env_state = env_init_state = ENVSTATE_MISSING;
	return env_state;
    }
    
    /* offset and size should be word aligned */
    if (env_offs & 1)
	++env_offs, --env_maxsize;
    if (env_maxsize & 1)
	--env_maxsize;
    
    /* check magic number */
    env_getbytes (ENVOFF_MAGIC, magic, 2);
    if (magic[0] != ENV_MAGIC_0 || magic[1] != ENV_MAGIC_1)
	env_init_state = ENVSTATE_UNINITIALISED;
    else
	env_init_state = ENVSTATE_OK;
    
    /* take recovery action if necessary */
    switch (env_state = env_init_state) {
	
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
	/* reset to a blank environment */
	env_reset ();
	break;
    }

#ifdef IN_PMON
    /* PMON: grab compress buffer from top of memory */
    memorysize = (memorysize - env_maxsize) & ~31;
    if (IS_KVA0 (&env_compbuf))
	env_compbuf = PA_TO_KVA0 (memorysize);
    else
	env_compbuf = PA_TO_KVA1 (memorysize);
#endif

    return env_state;
}



/* 
 * return offset of named environment entry
 */
static int
env_find (const char *name)
{
    unsigned int envp, top, namelen, varsize, i;
    unsigned char c;

    namelen = strlen (name);
    if (name == 0 || namelen >= 254)
	return (0);

    envp = ENVOFF_BASE; 
    top = ENVOFF_BASE + envsize;

    while (envp < top) {

	varsize = env_getbyte (envp);
	    
	if (varsize <= 1 || varsize == 0xff || envp + varsize > top)
	    break;		/* sanity */

	--varsize; ++envp;
	c = env_getbyte (envp);
	if (c != 0x00 && c != 0xff && c != '=' && varsize >= namelen) {

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
 * delete environment entry
 */
static void
env_delete (unsigned int envp)
{
    envfree += env_getbyte (envp);
    env_setbyte (envp+1, 0);	/* simply mark as deleted */
    env_flush ();
}


/*
 * compress environment by removing deleted entries
 */
static void
env_compress (void)
{
#ifdef IN_PMON
    unsigned char *cbuf = env_compbuf;
#else
    extern unsigned char _sbd_flash_secbuf[];
    unsigned char *cbuf = _sbd_flash_secbuf;
#endif
    unsigned char *cp = cbuf;
    unsigned int envp, top;

    if (!cbuf)
	return;

    envp = ENVOFF_BASE;
    top = ENVOFF_BASE + envsize;

    while (envp < top) {
	unsigned int varlen = env_getbyte (envp);
	unsigned char c;

	if (varlen <= 1 || varlen == 0xff || envp + varlen > top)
	    return;		/* sanity clause */

	/* check first byte of name */
	c = env_getbyte (envp + 1);
	if (c != 0x00 && c != 0xff) {
	    /* valid, active entry - copy it */
	    env_getbytes (envp, cp, varlen);
	    cp += varlen;
	}

	envp += varlen;
    }

    env_reset ();

    envsize = cp - cbuf;
    envfree -= envsize;

    /* hope we don't get a power failure while writing this block! */
    env_setbytes (ENVOFF_BASE, cbuf, envsize);
    env_flush ();
}



#ifdef IN_PMON
#define _sbd_mapenv	sbd_mapenv
#endif

int
_sbd_setenv (const char *name, const char *val, int overwrite)
{
    unsigned int namelen, vallen, total, oldlen;
    const char *cp;
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

    envp = env_find (name);
    if (envp && !overwrite)
	return 0;
    oldlen = envp ? env_getbyte (envp) : 0;

    total = 1 + namelen;		/* size . "name" */

    vallen = val ? strlen (val) : 0; 
    if (vallen)
	total += 1 + vallen;		/* . '=' . "value" */
    if (total >= 255) {			/* max variable size */
	errno = ENAMETOOLONG;
	return -1;
    }

    if (envfree + oldlen < total) {
	/* there can never be enough space for this variable, 
	   even after deleting the old value */
	errno = ENOSPC;
	return -1;
    }

    if (envp)
	env_delete (envp);	/* delete the old entry */

    /* not enough room to append at end - compress */
    if (ENVOFF_BASE + envsize + total > env_maxsize) {
	env_compress ();
	if (ENVOFF_BASE + envsize + total > env_maxsize) {
	    /* XXX shouldn't happen */
	    errno = ENOSPC;
	    return -1;
	}
    }

    /* 
     * By writing the first byte of the name last, we allow 
     * env_structcheck() to detect a partially written value
     * and skip it.
     *
     * Of course if the power fails or we are reset when halfway 
     * through writing the size byte then there's nothing we can do,
     * but at least that reduces the danger window to 1 byte.
     */

    envp = ENVOFF_BASE + envsize;

    /* store size first, so we can skip a partially written entry */
    env_setbyte (envp++, (unsigned char) total);
    env_flush ();		/* make sure it's written */

    /* store all but first byte of name */
    if (namelen > 1)
	env_setbytes (envp + 1, name + 1, namelen - 1);
    envp += namelen;

    /* store value */
    if (vallen) {
	env_setbyte (envp++, '=');
	env_setbytes (envp, val, vallen);
    }

    /* finally store first byte of name */
    env_setbyte (ENVOFF_BASE + envsize + 1, name[0]);
    env_flush ();

    envsize += total;
    envfree -= total;

    return 0;
}


char *
_sbd_getenv (const char *name)
{
    unsigned int envp, namelen, varlen;
    static char buf[256];		/* FIXME: this should not be static */
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
    if (!env_invalid ()) {
	unsigned int envp = env_find (s);
	if (envp != 0)
	    env_delete (envp);
    }
}


/*
 * apply func to each string in environment
 */
void
_sbd_mapenv (int (*func)(const char *, const char *))
{
    unsigned int envp, top;
    unsigned char c;
    char buf[256];

    if (env_invalid ())
	return;

    envp = ENVOFF_BASE;
    top = ENVOFF_BASE + envsize;

    while (envp < top) {
	char *s = buf; 
	char *val = 0;
	int varlen = env_getbyte (envp);

	if (varlen <= 1 || varlen == 0xff || envp + varlen > top)
	    break;		/* sanity clause */

	/* get first byte of name */
	c = env_getbyte (++envp);
	if (c == 0x00 || c == 0xff) {
	    /* deleted or partially written entry */
	    envp += varlen - 1;
	}
	else {
	    /* valid entry - copy to buffer */
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
	return 0;

    /* report any initial failure */
    switch (env_init_state) {
    case ENVSTATE_UNINITIALISED:
	err = "uninitialised";
	break;
    case ENVSTATE_MISSING:
	err = "no flash device found";
	break;
    case ENVSTATE_UNRECOVERABLE:
	err = "unrecoverable error";
	break;
    default:
	err = "bad state";
	break;
    }
    env_init_state = ENVSTATE_OK;

    return err;
}
