/* $Id: kern_glue.c,v 1.5 1998/12/28 14:13:06 nigel Exp $ */
#include "param.h"
#include "proc.h"
#include "map.h"
#include "kernel.h"
#include "systm.h"
#include "malloc.h"
#include "syslog.h"
#include "vm/vm.h"
#include "vm/vm_kern.h"

#include "machine/stdarg.h"
#include "mips.h"

int 
suser (credp, flagp)
{
    return 1;
}


int
subyte (base, byte)
    void *base;
    int byte;
{
    *(unsigned char *)base = byte;
}

int
suword (base, word)
    void *base;
    int word;
{
    *(unsigned int *)base = word;
}

int
fubyte (base)
    const void *base;
{
    return *(unsigned char *)base;
}

int
fuword (base)
    const void *base;
{
    return *(unsigned int *)base;
}

void
bcopy (src, dst, cnt)
    const void *src;
    void *dst;
    u_int cnt;
{
    if (dst >= src && dst < src + cnt) {
	src += cnt;
	dst += cnt;
	while (cnt--)
	  *(char *)--dst = *(const char *)--src;
    } else {
	memcpy (dst, src, cnt);
    }
}

void
ovbcopy (src, dst, cnt)
    const void *src;
    void *dst;
    u_int cnt;
{
    bcopy (src, dst, cnt);
}

bcmp (b1, b2, cnt)
    const void *b1, *b2;
    u_int cnt;
{
    while (cnt--) 
      if (*(const char *)b1++ != *(const char *)b2++)
	return 1;
    return 0;
}

#if 0
/* in PMON library */
void
bzero (dst, cnt)
    void *dst;
    u_int cnt;
{
    memset (dst, 0, cnt);
}
#endif

int
copyin (src, dst, cnt)
    const void *src;
    void *dst;
    u_int cnt;
{
    memcpy (dst, src, cnt);
    return 0;
}

copyout (src, dst, cnt)
    const void *src;
    void *dst;
    u_int cnt;
{
    memcpy (dst, src, cnt);
    return 0;
}

uiomove(cp, n, uio)
    register caddr_t cp;
    register int n;
    register struct uio *uio;
{
    register struct iovec *iov;
    u_int cnt;
    
    while (n > 0 && uio->uio_resid) {
	iov = uio->uio_iov;
	cnt = iov->iov_len;
	if (cnt == 0) {
	    uio->uio_iov++;
	    uio->uio_iovcnt--;
	    continue;
	}
	if (cnt > n)
	  cnt = n;
	if (uio->uio_rw == UIO_READ)
	  memcpy (iov->iov_base, cp, cnt);
	else
	  memcpy (cp, iov->iov_base, cnt);
	iov->iov_base += cnt;
	iov->iov_len -= cnt;
	uio->uio_resid -= cnt;
	uio->uio_offset += cnt;
	cp += cnt;
	n -= cnt;
    }
    return 0;
}

int
min (a, b)
{
    return a < b ? a : b;
}

int
imin (a, b)
{
    return a < b ? a : b;
}

int
max (a, b)
{
    return a > b ? a : b;
}


int
imax (a, b)
{
    return a > b ? a : b;
}

static u_char *kmem;
static vm_offset_t kmem_offs;


vminit ()
{
    if (!kmem) {
	extern int memorysize;
	/* grab a chunk at the top of memory */
	if (memorysize < VM_KMEM_SIZE * 2)
	  panic ("not enough memory for network");
	memorysize = (memorysize - VM_KMEM_SIZE) & ~PGOFSET;
	if (IS_K0SEG (&kmem)) 
	  /* if linked for data in kseg0, keep kmem there too */
	  kmem = (u_char *) PHYS_TO_K0 (memorysize);
	else
	  kmem = (u_char *) PHYS_TO_K1 (memorysize);
    }
}


vm_offset_t
kmem_malloc (map, size, canwait)
    vm_map_t map;
    vm_size_t size;
{
    vm_offset_t p;

    size = (size + PGOFSET) & ~PGOFSET;
    if (kmem_offs + size > VM_KMEM_SIZE) {
	log (LOG_DEBUG, "kmem_malloc: request for %d bytes fails\n", size);
	return 0;
    }
    p = (vm_offset_t) &kmem[kmem_offs];
    kmem_offs += size;
    return p;
}


vm_offset_t
kmem_alloc (map, size)
    vm_map_t map;
    vm_size_t size;
{
    return kmem_malloc (map, size, 0);
}


vm_map_t
kmem_suballoc (map, base, lim, size, canwait)
    vm_map_t map;
    vm_offset_t *base, *lim;
    vm_size_t size;
{
    if (size > VM_KMEM_SIZE)
      panic ("kmem_suballoc");
    *base = (vm_offset_t) kmem;
    *lim = (vm_offset_t)  kmem + VM_KMEM_SIZE;
    return map;
}

void
kmem_free (map, addr, size)
    vm_map_t map;
    vm_offset_t addr;
    vm_size_t size;
{
    panic ("kmem_free");
}

struct qelem {
    struct    qelem *q_forw;
    struct    qelem *q_back;
};


_insque (elem, pred)
    struct qelem *elem, *pred;
{
    elem->q_forw = pred->q_forw;
    elem->q_back = pred;
    pred->q_forw->q_back = elem;
    pred->q_forw = elem;
}

_remque (elem)
    struct qelem *elem;
{
    elem->q_back->q_forw = elem->q_forw;
    elem->q_forw->q_back = elem->q_back;
    elem->q_back = 0;
}

ffs(mask)
	register long mask;
{
	register int bit;

	if (!mask)
		return(0);
	for (bit = 1;; ++bit) {
		if (mask&0x01)
			return(bit);
		mask >>= 1;
	}
}

extern int sysloglevel;

static const char *const priname[] = {
    "\nEMERG",
    "\nALERT",
    "\nCRIT",
    "\nERROR",
    "\nWARNING",
    "\nNOTICE",
    "INFO",
    "DEBUG",
};


void
log (int kind, const char *fmt, ...)
{
    va_list arg;
    int pri = kind & LOG_PRIMASK;

    if (pri > sysloglevel)
      return;

    printf ("%s: ", priname[pri]);
    va_start(arg, fmt);
    vprintf (fmt, arg);
    va_end(arg);
}


void
panic (str)
    const char *str;
{
    int s = splhigh ();
    printf ("\n\nPANIC: %s\n\n\n", str);
#ifdef IDTSIM
    prominit ();
#else
    { 
	extern void start(void) __attribute__((noreturn));
	start ();
    }
#endif
}

void
tablefull (name)
    const char *name;
{
    log (LOG_ERR, "%s table is full\n", name);
}
