/* $Id: getmach.c,v 1.2 1996/01/16 14:18:01 chris Exp $ */
/*
 * ** Return machine type: 3000, 3010, 33000, 33050, 33020.
 */

#ifndef FLOATINGPT
#define hasFPU()	0
#endif

getmachtype ()
{
#ifdef R4000
    return (4000);
#else
    if (hasTLB ()) {
	if (hasFPU ())
	    return (3010);
	else
	    return (3000);
    } else {
	if (hasFPU ())
	    return (33050);
	else if (hasCP2 ())
	    return (33020);
	else
	    return (33000);
    }
#endif
}
