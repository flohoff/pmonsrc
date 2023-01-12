/* $Id: timetst.c,v 1.2 1996/01/16 14:17:08 chris Exp $ */
/*************************************************************
*  timetst.c
*	An example showing how to use the function 'time'. The
*	Default CPU clock frequency is defined in lib/time.c
*	You can override this by setting clkfreq. For example,
*	to compile this program for a 40 MHz board use:
*
*		pmcc -DCLKFREQ=40 timetst.c
*/

extern int clkfreq; /* import the global variable defined in lib/time.c */

main()
{
int c,t1,t2;

#if CLKFREQ
clkfreq = CLKFREQ;
#endif

t1 = time(0L);
for (;;) {
	t2 = time(0L);
	if (t2-t1 >= 1) {
		t1 = t2;
		printf("\r%5d %5d %5d ",t2,clock(),cycles());
		}
	}
}
