/* $Id: sieve.c,v 1.2 1996/01/16 14:17:06 chris Exp $ */
#ifndef LOOPS
#define LOOPS 10
#endif
#define TRUE 1
#define FALSE 0
#define SIZE 8190

/*************************************************************
*  sieve.c
*	The standard sieve benchmark
*/

char flags[SIZE+1];

main()
{
	register int i, prime, k, count, iter;
	for (iter=1;iter<=LOOPS;iter++) {
		count=0;
		for (i=0;i<=SIZE;i++)
			flags[i]=TRUE;
		for (i=0;i<=SIZE;i++) {
			if (flags[i]) {
				prime=i+i+3;
				for (k=i+prime;k<=SIZE;k+=prime)
					flags[k]=FALSE;
				count++;
			}
		}
	}
	exit(0);
}

