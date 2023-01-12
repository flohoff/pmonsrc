/* $Id: sort.c,v 1.2 1996/01/16 14:17:08 chris Exp $ */
/*************************************************************
*  sort.c
*       The standard bubble sort benchmark. This demonstrates 
*	some of the compiler's common-subexpression elimination
*	capabilities.  For example, inspect the code generated for
*	procedure Sort_array.
*/

#ifndef LOOPS
#define LOOPS 10
#endif

typedef unsigned char boolean;

void Sort_array(Tab,Last) int Tab[]; int Last; {
   boolean Swap;
   int Temp,I;
   do {
      Swap = 0;
      for (I = 0; I<Last; I++)
	 if (Tab[I] > Tab[I+1]) {
	    Temp = Tab[I];
	    Tab[I] = Tab[I+1];
	    Tab[I+1] = Temp;
	    Swap = 1;
	    }
      }
   while (Swap);
}

int Tab[100];

void Print_array() {
   int I,J;
   printf("\nArray Contents:\n");
   for (I=0; I<=9; I++) {
      printf("%5d:",10*I);
      for (J=0; J<=9; J++) printf("%5d",Tab[10*I+J]);
      printf("\n");
      }
}

main () {
   int I,J,K,L;

for (L = 0; L < LOOPS; L++) {
	/* Initialize the table that will be sorted. */
   	K = 0;
   	for (I = 9; I >= 0; I--)
      		for (J = I*10; J < (I+1)*10; J++)
			 Tab[K++] = J&1 ? J+1 : J-1;

	/* Print_array(); */
	Sort_array(Tab,99);	   /* Sort it. */
	/* Print_array(); */
	}
exit(0);
}
