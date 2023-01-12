/* $Id: queue.h,v 1.2 1996/01/16 14:17:22 chris Exp $ */
#ifndef _QUEUE_
#define _QUEUE_

typedef unsigned char Msg;

typedef struct Queue {
	unsigned short first;
	unsigned short count;
	unsigned short limit;
	Msg dat[1];
	} Queue;

/* Qinquiry operations */
#define Q_SIZE 1	/* Queue capacity */
#define Q_USED 2	/* space used */
#define Q_SPACE 3	/* space remaining */

void *malloc();
Queue *Qcreate();
Msg Qget();

#define Qfull(x)	((x)->count == (x)->limit)
#define Qempty(x)	((x)->count == 0)
#define Qsize(x)	((x)->limit)
#define Qused(x)	((x)->count)
#define Qspace(x)	((x)->limit - (x)->count)

#endif /* _QUEUE_ */
