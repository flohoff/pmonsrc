/* $Id: qsort.c,v 1.2 1996/01/16 14:17:04 chris Exp $ */
#include <stdio.h>
#define MAXNUM 200 
#ifndef LOOPS
#define LOOPS 10
#endif
#define MODULUS ((long) 0x20000)
#define C 13849L
#define A 25173L

/*************************************************************
*  qsort.c
*	The standard quicksort benchmark
*/

static long seed = 7L;
       long random();
       long buffer[MAXNUM] = {0};

main()
{
int i,j;
long temp;
/* printf("Filling array and sorting %d times \n",LOOPS); */
for(i=0;i<LOOPS;++i)
	{
	for(j=0;j<MAXNUM;++j)
		{
		temp=random(MODULUS);
		if (temp<0L)
			temp=(-temp);
		buffer[j]=temp;
		}
	/* printf("Buffer full,iteration %d\n",i); */
	quick(0,MAXNUM-1,buffer);
	}
/* printf("done\n"); */
	exit(0);
}


quick(lo,hi,base)
int lo,hi;
long base[];

{
int i,j;
long pivot,temp;

if(lo<hi)
	{
	for(i=lo,j=hi,pivot=base[hi];i<j;)
		{
		while(i<j && base[i]<=pivot)
			++i;
		while(j>i && base[j]>=pivot)
			--j;
		if(i<j)
			{
			temp = base[i];
			base[i]=base[j];
			base[j]=temp;
			}
		}
	quick(lo,i-1,base);
	quick(i+1,hi,base);
	}
}

long random(size)
long size;
{
seed=seed *A+C;
return(seed%size);
}
