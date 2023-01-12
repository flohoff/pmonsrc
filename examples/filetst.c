/* $Id: filetst.c,v 1.2 1996/01/16 14:16:55 chris Exp $ */
#include <stdio.h>
#include <unistd.h>

/*
 * This is an example showing how to use the fileIO facility.
 * To compile and execute type: 
 *	% mkfiles files infile			-- creates files.c & files.coff
 *	% pmcc -o filetst filetst.c files.c 	-- compile and link
 *	% genFrec files.coff > files.rec 	-- convert to fast recs
 *	PMON> load -t;load;g			-- prepare to downnload
 *	% edown < files.rec;edown < filetst.rec -- download
 */

#define LNMAX 100
char filename[] = "infile";

main()
{
FILE *fp;
char buf[LNMAX];

fp = fopen(filename,"r");
if (fp == 0) {
	fprintf(stderr,"Can't open %s for reading\n",filename);
	exit(-1);
	}

/* print the file to stdout */
while (fgets(buf,LNMAX,fp)) printf("%s",buf);

/* move back to the beginning */
if (fseek(fp,0L,SEEK_SET) == -1) {
	fprintf(stderr,"Bad SEEK_CUR\n");
	exit(-1);
	}

/* print the 1st line again */
if (!fgets(buf,LNMAX,fp)) {
	fprintf(stderr,"Unexpected EOF\n");
	exit(-1);
	}
printf("%s",buf);

/* move forward 20 bytes */
if (fseek(fp,20L,SEEK_CUR) == -1) {
	fprintf(stderr,"Bad SEEK_CUR\n");
	exit(-1);
	}

/* and print */
if (!fgets(buf,LNMAX,fp)) {
	fprintf(stderr,"Unexpected EOF\n");
	exit(-1);
	}
printf("%s",buf);

/* move to EOF-20 */
if (fseek(fp,-20L,SEEK_END) == -1) {
	fprintf(stderr,"Bad SEEK_END\n");
	exit(-1);
	}

/* and print */
if (!fgets(buf,LNMAX,fp)) {
	fprintf(stderr,"Unexpected EOF\n");
	exit(-1);
	}
printf("%s",buf);
}

