/*
 * NEC41XX/from.h: onboard FROM support for NEC Vr41xx eval board
 * Copyright (c) 1998 Algorithmics Ltd.
 *
 * This board is 32-bits wide with four 16-bit wide 28F032 chips, 
 * organised as two 32-bit wide interleaved words.  The chip 
 * itself is in fact two 28F016 chips one after the other.
 */

#define FROM_AUTOSELECT 	0x90
#define FROM_READXSR		0x71
#define FROM_PAGEBUFSWAP	0x72
#define FROM_PAGEBUFREAD	0x75
#define FROM_SINGLELOAD		0x74
#define FROM_SEQLOAD		0xe0
#define FROM_PAGEBUFWRITE	0x0c
#define FROM_PAGEBUFWRITEOFFS	0xa0
#define FROM_BLOCKERASE		0x20
#define FROM_BLOCKERASEOFFS	0xba
#define FROM_CONFIRM		0xd0
#define FROM_BLOCKLOCK		0x20
#define FROM_BLOCKLOCKOFFS	0xba
#define FROM_UPLOADSTATUS	0x97
#define FROM_UPLOADINFO		0x99
#define FROM_ERASECHIP		0xa7
#define FROM_RDY		0x96
#define FROM_RDY_LEVEL		 0x01
#define FROM_RDY_PULSE_WRITE	 0x02
#define FROM_RDY_PULSE_ERASE	 0x03
#define FROM_SLEEP		0xf0
#define FROM_ABORT		0x80

/* Manufacturer codes */
#define MAN_AMD			0x01
#define MAN_FUJITSU		0x04
#define MAN_MICRON		0x2c
#define MAN_INTEL		0x89
#define MAN_SHARP		0xb0

/* 28F016 organisation 2MB each, handled 4 chips at a time */
#define FROMMAXSIZE		(0x200000*4)	/* 2MB max flash size */
#define FROMMAXSECSIZE		(0x010000*4)	/* 64KB sector size */
#define FROMMAXSECTORS		32

/* all half-word wide */
typedef unsigned short from_t;
