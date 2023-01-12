/*
 * p5064/from.h: onboard FLASH ROM support for P5064 board
 * Copyright (c) 1997 Algorithmics Ltd.
 */

/* The sequence of addresses used to control the devices changes
 * slightly between the 4Mb and 8Mb devices 
 */
#define UNLOCK1_4Mb_OFFS	0xaaaa
#define UNLOCK1_8Mb_OFFS	0x5555
#define UNLOCK1_DATA		0xaa

#define UNLOCK2_4Mb_OFFS	0x5555
#define UNLOCK2_8Mb_OFFS	0x2aaa
#define UNLOCK2_DATA		0x55

/* Command codes are the same for all */
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

#define FROMMAXSIZE		(0x100000)	/* 1MB max flash size */
#define FROMMAXSECSIZE		(0x010000)	/* 64KB max sector size */
#define FROMMAXSECTORS		19

/* P5064 flash roms are all byte wide */
typedef unsigned char from_t;
