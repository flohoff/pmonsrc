/* $Id: malloc.c,v 1.2 1996/01/16 14:18:12 chris Exp $ */
typedef double  ALIGN;

union header {
    struct {
	union header   *ptr;
	unsigned        size;
    } s;
    ALIGN           x;
};

typedef union header HEADER;

static HEADER   base;
static HEADER  *allocp;		/* K&R called allocp, freep */

#define NULL   0
#define NALLOC 128

/* The rest of this file is from pages 185-189 of K & R Edition 2 */

static HEADER *morecore ();

void           *
malloc (nbytes)
     unsigned long nbytes;
{
    register HEADER *p, *q;	/* K&R called q, prevp */
    register unsigned nunits;

    nunits = (nbytes + sizeof (HEADER) - 1) / sizeof (HEADER) + 1;
    if ((q = allocp) == NULL) {	/* no free list yet */
	base.s.ptr = allocp = q = &base;
	base.s.size = 0;
    }
    for (p = q->s.ptr;; q = p, p = p->s.ptr) {
	if (p->s.size >= nunits) {	/* big enough */
	    if (p->s.size == nunits)	/* exactly */
		q->s.ptr = p->s.ptr;
	    else {		/* allocate tail end */
		p->s.size -= nunits;
		p += p->s.size;
		p->s.size = nunits;
	    }
	    allocp = q;
	    return ((char *)(p + 1));
	}
	if (p == allocp)
	    if ((p = morecore (nunits)) == NULL)
		return (NULL);
    }
}

static HEADER  *
morecore (nu)
     unsigned        nu;
{
    char           *sbrk ();
    register char  *cp;
    register HEADER *up;
    register int    rnu;

    rnu = NALLOC * ((nu + NALLOC - 1) / NALLOC);
    cp = sbrk (rnu * sizeof (HEADER));
    if ((int)cp == NULL)
	return (NULL);
    up = (HEADER *) cp;
    up->s.size = rnu;
    free ((char *)(up + 1));
    return (allocp);
}


free (ap)
     char           *ap;
{
    register HEADER *p, *q;

    p = (HEADER *) ap - 1;
    for (q = allocp; !(p > q && p < q->s.ptr); q = q->s.ptr)
	if (q >= q->s.ptr && (p > q || p < q->s.ptr))
	    break;
    if (p + p->s.size == q->s.ptr) {
	p->s.size += q->s.ptr->s.size;
	p->s.ptr = q->s.ptr->s.ptr;
    } else
	p->s.ptr = q->s.ptr;
    if (q + q->s.size == p) {
	q->s.size += p->s.size;
	q->s.ptr = p->s.ptr;
    } else
	q->s.ptr = p;
    allocp = q;
}

allocsize (ap)
     char           *ap;
{
    register HEADER *p;

    p = (HEADER *) ap - 1;
    return (p->s.size * sizeof (HEADER));
}
