/* $Id: rand.c,v 1.2 1996/01/16 14:18:22 chris Exp $ */

unsigned long int rand_next = 1;

/*************************************************************
 *  int rand()
 *      Taken from the K&R C programming language book. Page 46.
 *      returns a pseudo-random integer of 0..32767. Note that
 *      this is compatible with the System V function rand(), not
 *      with the bsd function rand() that returns 0..(2**31)-1.
 */
int 
rand ()
{

    rand_next = rand_next * 1103515245 + 12345;
    return ((unsigned int)(rand_next / 65536) % 32768);
}

/*************************************************************
 *  srand(seed)
 *      companion routine to rand(). Initializes the seed.
 */
srand (seed)
     unsigned int    seed;
{
    rand_next = seed;
}

/*************************************************************
 *  randn(limit)
 *      returns a pseudo-random number in the range 0..limit.
 *      Assumes that limit is less than 32000.
 */
randn (limit)
     int             limit;
{
    return ((rand () * (limit + 1)) / 32768);
}
