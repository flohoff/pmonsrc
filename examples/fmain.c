/* $Id: fmain.c,v 1.2 1996/01/16 14:16:55 chris Exp $ */
char ticks[] = "|/-\\";

/*************************************************************
*  fmain
*	The main program for an example that executes all
*	FPU instructions. However, it doesn't check the
*	results.
*/

#ifdef PMCC
main()
#else
ftest()
#endif
{
int i;
char *tick;

tick = ticks;
for (i=0;;i++) {
	if ((i%40000) == 0) {
		printf("\r %c ",*tick);
		tick++;
		if (*tick == 0) tick = ticks;
		}
	_ftest();
	}
}
