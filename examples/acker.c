/* $Id: acker.c,v 1.2 1996/01/16 14:16:50 chris Exp $ */
#ifndef LOOPS
#define LOOPS 1
#endif

/*************************************************************
*  acker
*	The standard Ackerman's function benchmark
*/


main()
{
int i;

    for (i=0;i<LOOPS;i++) acker(3,6);
    exit(0);
}

acker(x,y)
{
    if (x==0)
	return(y+1);
    else if (y==0)
	return(acker(x-1,1));
    else
	return(acker(x-1, acker(x, y-1)));
}
