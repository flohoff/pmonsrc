/*
 * GAL9/from.h: onboard FROM support for Galileo-9
 * Copyright (c) 1997 Algorithmics Ltd.
 */

/* magic addresses in word mode A14..A0 */
#define UNLOCK1_OFFS		0x5555
#define UNLOCK1_DATA		0xaa
#define UNLOCK2_OFFS		0x2aaa
#define UNLOCK2_DATA		0x55

/* command codes are the same for all */
#define FROM_RESET		0xf0
#define FROM_AUTOSELECT 	0x90
#define FROM_PROGRAM		0xa0
#define FROM_ERASE		0x80
#define FROM_ERASECHIP		0x10
#define FROM_ERASESECT		0x30

/* Manufacturer codes */
#define MAN_AMD			0x01
#define MAN_FUJITSU		0x04
#define MAN_MICRON		0x2c
#define MAN_INTEL		0x89

/* data bit definitions for "embedded programming algorithm" */
#define DQPOLL			0x80
#define DQTOGGLE		0x40
#define DQTIMEEXCEEDED		0x20
#define DQSECTERASETIMER 	0x08

#define FROMMAXSIZE		(0x080000*4)	/* 512KB max flash size */
#define FROMMAXSECSIZE		(0x010000*4)	/* 64KB max sector size */
#define FROMMAXSECTORS		11

/* GAL9 flash roms are all half-word wide */
typedef unsigned short from_t;
