/* $Id: sinewave.c,v 1.2 1996/01/16 14:17:07 chris Exp $ */
#include "math.h"

/*************************************************************
*  sinewave.c
*	Displays a number of overlapping sinewaves on the screen,
*	doesn't not require PMON to know about terminal type.
*/

#define STEPSZ 10		/* degrees per step */
#define WAVES 3			/* number of sine waves */
#define DEGS_TO_RADS(x)		((2*3.142)/(360/(double)(x)))
#define SCALE(x)		((int)(((x)*40)+40))

#ifdef RAMFP
typedef int iFunc();
int cop1();
iFunc *c1dat[] = {0,cop1};
#endif

main()
{
int n,i;
double rads;
char t1[100],t2[100];

#ifdef RAMFP
onintr(11,c1dat);
#endif

for (;;) 
	for (n=0;n<360;n+=STEPSZ) { 
		sprintf(t1,"%*c",SCALE(sin(DEGS_TO_RADS(n))),'x');
		for (i=1;i<WAVES;i++) {
			rads = DEGS_TO_RADS((n+((360/WAVES)*i)%360));
			sprintf(t2,"%*c",SCALE(sin(rads)),'x');
			strmerge(t1,t2);
			}
		printf("%s\n",t1);
		}
}

