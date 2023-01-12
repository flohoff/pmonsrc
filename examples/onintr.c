/* $Id: onintr.c,v 1.2 1996/01/16 14:17:02 chris Exp $ */
/*************************************************************
*  onintr.c
*	An example program showing how to use PMON's onintr
*	function to transfer control to a user defined exception
*	handler. For another example see lib/clock.s.
*	In this example, control is passed to the handler e4isr()
*	when a hardware interrupt occurs. The function onintr()
*	builds a linked list of user-installed handlers for each
*	exception type (in this case 0, HWINT). e4isr must pass
*	control to the next handler in the chain if the exception
*	is not for it, i.e., INT1. The function e4isr is located
*	in the file e4isr.s. To compile, type:
*	
*		pmcc onintr.c e4isr.s
*/

/*
 * Pocket Rocket Hardware Interrupts
 *
 *	 SR    CAUSE	BIT	 PIN	Description
 *     -------------------------------------------------
 *	IBIT1	SW1	 8	 -	Software int 0
 *	IBIT2	SW2	 9	 -	Software int 1
 *	IBIT3	IP3	10 	INT0	Timer 1
 *	IBIT4	IP4	11	INT1	Timer 2
 *	IBIT5	IP5	12	INT2	Duart
 *	IBIT6	IP6	13	INT3	FPU
 *	IBIT7	IP7	14	INT4	-
 *	IBIT8	IP8	15	INT5	-
 */

#include <mips.h>

typedef int iFunc();
int e4isr();
iFunc *e4dat[] = {0,e4isr};

int ticks;

main()
{
int t1,t2;

/* set up timer2 to generate ints every 500ms */
onintr(0,e4dat);	/* install interrupt handler */
TIC2 = 12500000;	/* 500ms if CPU clock is 25MHz */
TC2 = (TC_CE|TC_IE);	/* timer enable, interrupt enable */
enabint(SR_IBIT4);	/* enable interrupts */

t1 = ticks;
for (;;) {
	t2 = ticks;
	if (t2-t1 >= 1) {
		t1 = t2;
		printf("\r%5d ",t2);
		}
	}
}

