/* $Id: bubble.c,v 1.2 1996/01/16 14:16:51 chris Exp $ */
/*************************************************************
*  BUBBLE.C: Bubble sort of doubly linked list
*	Useful for playing with dbx
*/

#define LSTSZ 10

typedef struct TCB {
	struct TCB *forw;
	struct TCB *back;
	unsigned char tid;
	unsigned short pri;
	unsigned int status;
	} TCB;

TCB *headchn,*tailchn;
int ran[LSTSZ] = {0,2,9,4,1,8,6,3,7,5};	
char *malloc();

main()
{

init();
print("Unsorted list: ");
sort();
print("Sorted list:   ");
}

init()
{
TCB *p;
int i;

headchn = 0;
tailchn = 0;

for (i=0;i<LSTSZ;i++) {
	p = (TCB *)malloc(sizeof(TCB));	
	p->tid = i;
	p->pri = ran[i];
	p->status = 0;
	p->forw = 0;
	p->back = 0;
	insert(p);
	}
}

insert(p)
TCB *p;
{

if (tailchn == 0) { /* chain is empty */
	tailchn = p;
	headchn = p;
	}
else {
	tailchn->back = p;
	p->forw = tailchn;
	tailchn = p;
	}
}

sort()
{
int flag;
TCB *p,*q;

printf("Sorting..");
if (tailchn == 0) return;
while (1) {
	flag = 0;
	p = tailchn;
	while (1) {
		q = p->forw;
		if (q == 0) break;
		if (p->pri > q->pri) {
			swap(p,q);
			flag = 1;
			}
		else p = q;
		}
	if (flag == 0) break;
	}
printf("\n");
}

swap(p,q)
TCB *p,*q;
{

putchar('.');
if (p->back == 0) {      /* first two in chain */
	tailchn = q;
	q->forw->back = p;
	}
else if (q->forw == 0) { /* last two in chain */
	p->back->forw = q;
	headchn = p;
	}
else { 			 /* middle of chain */
	p->back->forw = q;
	q->forw->back = p;
	}
	
p->forw = q->forw;
q->forw = p;
#ifdef BUG
p->back = q;
q->back = p->back;
#else
q->back = p->back;
p->back = q;
#endif
}

print(mess)
char *mess;
{
TCB *p;

printf("%s",mess);
p = tailchn;
while (p != 0) {
	printf("%d ",p->pri);
	p = p->forw;
	}
printf("\n");
}

prtcb(p)
TCB *p;
{
printf("%6s  %08x\n","forw",p->forw);
printf("%6s  %08x\n","back",p->back);
printf("%6s  %d\n","tid",p->tid);
printf("%6s  %d\n","pri",p->pri);
printf("%6s  %08x\n","status",p->status);
}

prtcbs(p)
TCB *p;
{

printf("%8s  %~8s %~8s %~12s %~12s %~8s\n",
	"","forw","back","tid","pri","status");
while (p != 0) {
	printf("%08x: %08x %08x %12d %12d %08x\n",
		p,p->forw,p->back,p->tid,p->pri,p->status);
	p = p->forw;
	}
}

