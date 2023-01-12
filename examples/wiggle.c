/* $Id: wiggle.c,v 1.2 1996/01/16 14:17:10 chris Exp $ */
#include "stdio.h"
#include "termio.h"
#include "math.h"

/*************************************************************
*  wiggle.c
*	Displays a lissagous figure on a standard crt using text 
*	graphics.
*/

#ifdef FLOAT
#define double float
#endif

typedef int fix;

typedef struct dotentry {
	struct dotentry *next;
	int x;
	int y;
	int fade;
	} dotentry;

dotentry *head,*freelst;
double tab1[360];
int persist;

main()
{
int i,n,x,y,c;
fix angle_x,angle_y; 
fix x_inc,y_inc;	
double mag_x,mag_y;
double d;

persist = 120;
printf("building table...");
/* initialize table */
for (n=0,d=0;n<360;n++,d += ((2*3.142)/360.0)) {
	tab1[n] = sin(d);
	}

angle_x = 0;
angle_y = 0;
x_inc = 300; /*  */
y_inc = 620; /*  */

ioctl(STDIN,CBREAK,NULL);
if (ttctl(STDOUT,TT_CLR) == -1) {
	printf("terminal type not set\n");
	exit(1);
	}

for (;;) {
	ioctl(STDIN,FIONREAD,&n);
	if (n > 0) {
		c = getchar();
		switch (c) {
		    case 'u' : y_inc += 10; break;
		    case 'd' : y_inc -= 10; break;
		    case 's' : if (persist > 10) persist -= 10; break;
		    case 'l' : persist += 10; break;
			}
		}
	n = angle_x/100;
	mag_x = tab1[n];
	n = angle_y/100;
	mag_y = tab1[n];
/*	printf("%d=%e %d=%e\n",angle_x,mag_x,angle_y,mag_y); */
	angle_x += x_inc;
	if (angle_x >= 36000) angle_x -= 36000;
	angle_y += y_inc;
	if (angle_y >= 36000) angle_y -= 36000;
	x = ((mag_x*10)+12)*2;
	y = (mag_y*10)+12;
	putdot(x,y);
	fadedots();
	}
}

putdot(x,y)
int x,y;
{
dotentry *p;
int i;

for (p = head;p != 0 ;p = p->next) {
	if (p->x == x && p->y == y) {
		p->fade = persist;
		return;
		}
	}

/* create a new entry */
p = freelst;
if (p == 0) {
	p = (dotentry *)malloc(sizeof(dotentry));
	if (p == 0) printf("Out of memory\n");
	}
else freelst = p->next;
p->x = x;
p->y = y;
p->fade = persist;

/* add it to the chain */
p->next = head;
head = p;

/* write it to the screen */
ttctl(STDOUT,TT_CM,x,y); 
write(STDOUT,"x",1);
}

fadedots()
{
dotentry *p,*prev;
int i;

prev = 0;
for (p = head;p != 0 ;) {
	p->fade--;
	if (p->fade <= 0) {
		/* remove it from the screen */
		ttctl(STDOUT,TT_CM,p->x,p->y);
		write(STDOUT," ",1);

		if (prev == 0) { /* 1st item in chain */
			/* remove it from the list */
			head = p->next; 

			/* put it back on the freelist */
			p->next = freelst;
			freelst = p;

			/* move to the next item */
			p = head;
			}
		else { /* Nth item in chain */
			/* remove it from the list */
			prev->next = p->next;

			/* put it back on the freelist */
			p->next = freelst;
			freelst = p;

			/* move to the next item */
			p = prev->next;
			}
		}
	else {
		prev = p;
		p = p->next;
		}
	}
}

