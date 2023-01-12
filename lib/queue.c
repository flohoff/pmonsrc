/* $Id: queue.c,v 1.2 1996/01/16 14:18:19 chris Exp $ */
#include "queue.h"

/*************************************************************
 *  These queue manipulation routines are used by PMON
 */

#ifdef TEST
/*************************************************************
 *  main() 
 *      This is a simple test program for the queue routines
 */
main ()
{
    char            buf[20];
    int             size, msg;
    Queue          *p;

    printf ("Enter Queue size >");
    gets (buf);
    sscanf (buf, "%d", &size);
    p = Qcreate (size);

    for (;;) {
	printf ("inp=%d outp=%d\n", p->inp, p->outp);
	printf ("Qfull=%d Qempty=%d Qsize=%d\n", Qfull (p), Qempty (p), Qinquiry (p));
	printf ("g or p >");
	gets (buf);
	if (buf[0] == 'g') {
	    msg = (int)Qget (p);
	    printf ("msg=%d\n", msg);
	} else if (buf[0] == 'p') {
	    sscanf (buf, "p %d", &msg);
	    Qput (p, (Msg) msg);
	}
    }
}

reschedule ()
{
}
#endif

/*************************************************************
 *  Queue *Qcreate(size) creates a queue of specified size
 */
Queue          *
Qcreate (size)
     int             size;
{
    int             max;
    Queue          *q;

    if (size < 0 || (size & (size - 1))) {
	printf ("Qcreate: size %d not a power of two\n");
	return 0;
    }

    q = (Queue *) malloc (sizeof (Queue) + ((size - 1) * sizeof (Msg)));
    if (!q)
	return (0);
    q->limit = size - 1;
    q->first = q->count = 0;
    return (q);
}

/*************************************************************
 *  Qput(q,msg) adds msg to queue
 */
Qput (q, msg)
     Queue          *q;
     Msg             msg;
{
    while (Qfull (q))
	reschedule ();

    q->dat[(q->first + q->count) & q->limit] = msg;
    q->count++;
}

/*************************************************************
 *  Msg Qget(q) removes a msg from a queue
 */
Msg 
Qget (q)
     Queue          *q;
{
    Msg             msg;

    while (Qempty (q))
	reschedule ();

    msg = q->dat[q->first];
    q->first = (q->first + 1) & q->limit;
    q->count--;
    return (msg);
}

/** Qfull(q) returns 1 if queue is full (macro) */
/** Qempty(q) returns 1 if queue is empty (macro) */

/*************************************************************
 *  Qinquiry(q,op) inquire about queue SIZE, USED, SPACE
 */
Qinquiry (q, op)
     Queue          *q;
     int             op;
{
    switch (op) {
    case Q_SIZE:		/* what's the capacity of the queue? */
	return Qsize (q);
    case Q_USED:		/* how much space is used? */
	return Qused (q);
    case Q_SPACE:		/* how much space is left? */
	return Qspace (q);
    }
    return -1;
}

/*************************************************************
 *  Msg Qread(q,n) read msg n from queue (non destructive)
 */
Msg 
Qread (q, n)
     Queue          *q;
     int             n;
{
    if (n >= q->count)
      return 0;
    return q->dat[(q->first + n) & q->limit];
}
