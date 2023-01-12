/* $Id: makebits.c,v 1.2 1996/01/16 14:23:57 chris Exp $ */
/* makebits.c - generate VME-4000 bootmode bits */

/* Copyright 1993,1994 Algorithmics Ltd. */

/*
SYNOPSIS

makebits [-2] [-3] [-t] [-a] [-l]

DESCRIPTION
This program generates the R4000 bootmode configuration bits required
by the VME-4000.  When the VME-4000 is reset, the R4000 processor
configures itself by reading the bootmode stream from the top
locations in the bootrom.  This program generates a Motorola hex
format (S-record) file that should be downloaded alomg with the
vxWorks bootrom hex file.  Normally S1 records will be generated for
addresses less than 64K; S2 records will be generated for address
greater than 64K and less the 16M; and S3 records will be generated
for greater than that.

OPTIONS:
-l generate only S2 records.
-2 divide-by-two bus clock
-3 divide-by-three bus clock  (default)
-t enable R4000 timer interrupt
-a ascii dump

*/

#include <stdio.h>


/* forward declarations */
#if __STDC__
void dump_ascii (void);
void leftrotate (int n);
void dump_hex (int lflag);
unsigned char dobyte (int sba, int startbit);
void hexInitialise (unsigned int addr, int lflag);
void hexWrite (unsigned char *buffer, int totalBytes);
void hexTerminate (void);
#else
void dump_ascii ();
void leftrotate ();
void dump_hex ();
unsigned char dobyte ();
void hexInitialise ();
void hexWrite ();
void hexTerminate ();
#endif

#define NMODES 256
#define NBITS 256
unsigned char mode[NMODES][NBITS];

#define min(a,b) ((a)<(b) ? (a) : (b))

#ifdef __STDC__
void main (int argc, char **argv)
#else
void main (argc, argv)
    int argc;
    char **argv;
#endif
    {
    int c, dmode;
    int sba;
    int i;
    int aflag;
    int lflag;
    int tflag;

    aflag = 0;
    lflag = 0;
    tflag = 1;
    dmode = 3;
    while ((c = getopt (argc, argv, "l23at")) != EOF)
	{
	switch (c)
	    {
	  case 'l':
	    lflag = 1;		/* generate S3 type records  */
	    break;
	  case '2':
	    dmode = 2;		/* divide by 2 clock */
	    break;
	  case '3':
	    dmode = 3;		/* divide by 3 clock */
	    break;
	  case 'a':
	    aflag = 1;		/* ascii output mode */
	    break;
	  case 't':
	    tflag = 0;		/* enable timer interrupt */
	    break;
	    }
	}

    /* generate the various confiuration bootmode bit strings */
    
    for (sba = 0; sba < NMODES; sba++)
	{
	mode[sba][0] = 1;	/* BlkOrder: scache sub-block ordering */
	mode[sba][1] = 1;	/* ElBParMode: byte parity */
				/* EndBlt: configurable endianess */
	mode[sba][2] = (sba & 1) ? 0 : 1;
	mode[sba][3] = 1;	/* DShMdDis: Dirty shared mode diabled */
	mode[sba][4] = 1;	/* NoScMode: R4000PC */

	mode[sba][5] = 0;	/* SysPort: 64bits */
	mode[sba][6] = 0;

	mode[sba][7] = 0;	/* SC64BitMd: secondary cache 128bits */
	mode[sba][8] = 0;	/* ElSpltMd: unified secondary cache */

	mode[sba][9] = 1;	/* SCBlkSz: 8 word scache linesize */
	mode[sba][10] = 0;

				/* XmitDatPat: configurable data rate */
	switch ((sba>>1) & 7)
	    {
	  case 0:
	    mode[sba][11] = 0;	/* D */
	    mode[sba][12] = 0;
	    mode[sba][13] = 0;
	    mode[sba][14] = 0;
	    break;
	  case 1:
	    mode[sba][11] = 1;	/* DDx */
	    mode[sba][12] = 0;
	    mode[sba][13] = 0;
	    mode[sba][14] = 0;
	    break;
	  case 2:
	    mode[sba][11] = 0;	/* DDxx */
	    mode[sba][12] = 1;
	    mode[sba][13] = 0;
	    mode[sba][14] = 0;
	    break;
	  case 3:
	    mode[sba][11] = 1;	/* DxDx */
	    mode[sba][12] = 1;
	    mode[sba][13] = 0;
	    mode[sba][14] = 0;
	    break;
	  case  4:
	    mode[sba][11] = 0;	/* DDxxx */
	    mode[sba][12] = 0;
	    mode[sba][13] = 1;
	    mode[sba][14] = 0;
	    break;
	  case 5:
	    mode[sba][11] = 1;	/* DDxxxx */
	    mode[sba][12] = 0;
	    mode[sba][13] = 1;
	    mode[sba][14] = 0;
	    break;
	  case 6:
	    mode[sba][11] = 0;	/* DxxDxx */
	    mode[sba][12] = 1;
	    mode[sba][13] = 1;
	    mode[sba][14] = 0;
	    break;
	  case 7:
	    mode[sba][11] = 0;	/* DxxxDxxxx */
	    mode[sba][12] = 0;
	    mode[sba][13] = 0;
	    mode[sba][14] = 1;
	    break;
	    }
				/* SysClkRatio: configurable */
	switch ((sba >> 4) & 3)
	    {
	  case 0:
	    if (dmode != 2)
		{
		mode[sba][15] = 1; /* divide by 3 */
		mode[sba][16] = 0;
		mode[sba][17] = 0;
		}
	    else
		{
		mode[sba][15] = 0; /* divide by 2 */
		mode[sba][16] = 0;
		mode[sba][17] = 0;
		}
	    break;
	  case 1:
	    if (dmode == 2)
		{
		mode[sba][15] = 1; /* divide by 3 */
		mode[sba][16] = 0;
		mode[sba][17] = 0;
		}
	    else
		{
		mode[sba][15] = 0; /* divide by 2 */
		mode[sba][16] = 0;
		mode[sba][17] = 0;
		}
	    break;
	  case 2:
	    mode[sba][15] = 0;	/* divide by 4 */
	    mode[sba][16] = 1;
	    mode[sba][17] = 0;
	    break;
	  case 3:
	    mode[sba][15] = 1;	/* Reserved! */
	    mode[sba][16] = 1;
	    mode[sba][17] = 0;
	    break;
	    }
	mode[sba][18] = 0;	/* Reserved */
	mode[sba][19] = tflag;	/* TimIntDis: timer interrupt disabled */
	mode[sba][20] = 1;	/* PotUpdDis: potential updates disabled */
	mode[sba][21] = 0;	/* TWrSUp: scache write deassertion delay */
	mode[sba][22] = 0;
	mode[sba][23] = 0;
	mode[sba][24] = 0;
	mode[sba][25] = 0;	/* TWr2Dly: scache write assertion delay2 */
	mode[sba][26] = 0;
	mode[sba][27] = 0;	/* TWr1Dly: scache write assertion delay1 */
	mode[sba][28] = 0;
	mode[sba][29] = 0;	/* TWrRCk: scache write recovery time */
	mode[sba][30] = 0;	/* TDis: scache disable time */
	mode[sba][31] = 0;
	mode[sba][32] = 0;
	mode[sba][33] = 0;	/* TRd2Cyc: scache read cycle time */
	mode[sba][34] = 0;
	mode[sba][35] = 0;
	mode[sba][36] = 0;
	mode[sba][37] = 0;	/* TRd1Cyc: scache read cycle time */
	mode[sba][38] = 0;
	mode[sba][39] = 0;
	mode[sba][40] = 0;
	mode[sba][41] = 0;	/* Reserved */
	mode[sba][42] = 0;	/* Reserved */
	mode[sba][43] = 0;	/* Reserved */
	mode[sba][44] = 0;	/* Reserved */
	mode[sba][45] = 0;	/* Reserved */
	mode[sba][46] = 1;	/* Pkg179: R4000PC */
	mode[sba][47] = 0;	/* CycDivisor: divide by 2 in RP mode */
	mode[sba][48] = 0;
	mode[sba][49] = 0;
	mode[sba][50] = 0;	/* Drv0_50: disabled */
	mode[sba][51] = 0;	/* Drv0_75: disabled */
	mode[sba][52] = 1;	/* Drv1_00: enabled */
	mode[sba][53] = 0;	/* InitP: intermediate pull down rate */
	mode[sba][54] = 0;    
	mode[sba][55] = 0;    
	mode[sba][56] = 1;    
	mode[sba][57] = 0;	/* InitN: intermediate pull up rate */
	mode[sba][58] = 0;
	mode[sba][59] = 0;
	mode[sba][60] = 1;
	mode[sba][61] = 0;	/* EnblDPLLR: disabled */
				/* EnblDPLL: configurable */
	mode[sba][62] = (sba & 0x40) ? 1 : 0;
				/* DsblPll: configurable */
	mode[sba][63] = (sba & 0x80) ? 0 : 1;
	mode[sba][64] = 1;	/* SRTristate: tristate during reset */
	for (i = 65; i < NBITS; i++)
	    mode[sba][i] = 0;	/* Reserved */
	}
    
    if (aflag)
	dump_ascii ();
    else
	dump_hex (lflag);

    exit (0);
    }

#ifdef __STDC__
void dump_ascii (void)
#else
void dump_ascii ()
#endif
    {
    int sba,i;

    puts ("   666666555555555544444444443333333333222222222211111111110000000000");
    puts ("   543210987654321098765432109876543210987654321098765432109876543210");
    for (sba = 0; sba < NMODES; sba++)
	{
	printf ("%02x ", sba);
	for (i = 65; i >= 0; i--)
	    putchar (mode[sba][i] ? '@' : '.');
	putchar ('\n');
	}
    }

/* rotate mode bits by an arbitrary amount */
#ifdef __STDC__
void leftrotate (int n)
#else
void leftrotate (n)
    int n;
#endif
    {
    int save;
    int i, j;

    while (n--)
	{
	for (i = 0; i < NMODES; i++){
	    save = mode[i][NBITS-1];
	    for (j = NBITS-1; j > 0; j--)
		mode[i][j] = mode[i][j-1];
	    mode[i][0] = save;
	    }
	}
    }

#ifdef __STDC__
void dump_hex (int lflag)
#else
void dump_hex (lflag)
int lflag;
#endif
    {
    unsigned char buffer[NBITS/8];
    int sba, i;

    leftrotate (1);

    hexInitialise (0x7e000, lflag);
    for (sba = 0; sba < NMODES; sba++)
	{
	for (i = 0; i < NBITS/8; i++)
	    buffer[i] = dobyte (sba, i*8);
	hexWrite (buffer, NBITS/8);
	}
    hexTerminate ();
    }

#ifdef __STDC__
unsigned char dobyte (int sba, int startbit)
#else
unsigned char dobyte (sba, startbit)
    int sba;
    int startbit;
#endif
    {
    unsigned char byte = 0;
    int i;
    for (i = 0; i < 8; i++, startbit++)
	{
	if (mode[sba][startbit])
	    byte |= (1 << i);
	}
    return (byte);
    }

/* Motorola S-Record output generator */
static unsigned int address;
static int recType;

#ifdef __STDC__
void hexInitialise (unsigned int addr, int lflag)
#else
void hexInitialise (addr, lflag)
unsigned int addr;
int lflag;
#endif
    {
    address = addr;
    recType = lflag ? 3 : 1;
    }

#ifdef __STDC__
void hexWrite (unsigned char *buffer, int totalBytes)
#else
void hexWrite (buffer, totalBytes)
unsigned char *buffer;
int totalBytes;
#endif
    {
    int nBytes;
    int bytesWritten;
    int i;
    unsigned int checkSum;
    unsigned int dataByte;


    bytesWritten = 0;

    while (bytesWritten < totalBytes)
	{
	nBytes = min ((totalBytes - bytesWritten), 16);

	/* write out record type, length, & address fields */

	if (address < 0x10000 && recType <= 1)
	    {
	    recType = 1;
	    if ((address + nBytes) > 0x10000)
		nBytes = 0x10000 - address;
	    printf ("S1%02X%04X", 2 + nBytes + 1, address);
	    checkSum = 2 + nBytes + 1 +			/* length */
		((address >> 8) & 0xff) +		/* adrs msb */
		    (address & 0xff);			/* adrs lsb */
	    }
	else if (((address > 0x0000ffff) && (address < 0x1000000)) && recType <=2)
	    {
	    recType = 2;
	    if ((address + nBytes) > 0x1000000)
		nBytes = 0x1000000 - address;
	    printf ("S2%02X%06X", 3 + nBytes + 1, address);
	    checkSum = 3 + nBytes + 1 +			/* length */
		((address >> 16) & 0xff) +		/* adrs msb */
		    ((address >> 8) & 0xff) +		/* adrs middle byte */
			(address & 0xff); 		/* adrs lsb */
	    }
	else
	    {
	    recType = 3;
	    printf ("S3%02X%08X", 4 + nBytes + 1, address);
	    checkSum = 4 + nBytes + 1 +			/* length */
		((address >> 24) & 0xff) +		/* bits 24-31 */
		    ((address >> 16) & 0xff) +		/* bits 16-23 */
			((address >> 8) & 0xff) +	/* bits 8-15 */
			    (address & 0xff);		/* bits 0-7 */
	    }

	/* read in a data byte, add it to the checksum, and print it out */

	for (i = 0; i < nBytes; i++)
	    {
	    dataByte = *buffer++;
	    checkSum += dataByte;
	    printf ("%02X", dataByte);
	    }

	/* print out the ones complement of the checksum */

	printf("%02X\n", (~checkSum) & 0xff);

	bytesWritten += nBytes;
	address      += nBytes;
	}
    }


#ifdef __STDC__
void hexTerminate (void)
#else
void hexTerminate ()
#endif
    {
    /*
     * S1 records are to be terminated by S9 record;
     * S2 records are to be terminated by S8 record.
     */
    switch (recType)
	{
      case 1:
	printf("S9030000FC\n");
	break;
      case 2:
	printf("S804000000FB\n");
	break;
      default:
      case 3:
	printf("S70500000000FA\n");
	}
    }
