/* $Id: fibo.c,v 1.2 1996/01/16 14:16:53 chris Exp $ */
#include <stdio.h>

/*************************************************************
*  fib
*	The standard benchmark that calculates the Fibonacci series
*/

#ifndef LOOPS
#define LOOPS 1
#endif
#define NUMBER 24

main()
        {
        int i;
        unsigned value, fib();
        
        /*printf("%d iterations:",LOOPS);*/
        for (i=1;i<=LOOPS;i++)
                value=fib(NUMBER);
        /*printf("fibonacci(%d)=%u.\n",NUMBER,value);*/
        exit(0);
        }

unsigned fib(x)
        int x;
        {
        if(x>2)
                return(fib(x-1)+fib(x-2));
        else
        return(1);
        }      

