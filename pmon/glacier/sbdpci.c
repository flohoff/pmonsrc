


extern struct cfdriver decd;

#define NORM FSTATE_NOTFOUND
#define STAR FSTATE_STAR

struct cfdata cfdata[] = {
	/* driver     unit state    loc     flags parents ivstubs */
/*  0: de0 */
	{&decd,	 	0, NORM,    0,      0, 	  0, 	  0},
	{0}
};

