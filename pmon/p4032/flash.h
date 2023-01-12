/*
 * p4032/flash.h: onboard FLASH support for P4032 board
 * Copyright (c) 1996 Algorithmics Ltd.
 */

#define UNLOCK1		0xaaaa
#define UNLOCK2		0x5555

#define FLASH_RESET	0xf0
#define FLASH_AUTOSELECT 0x90
#define FLASH_PROGRAM	0xa0
#define FLASH_ERASE	0x80
#define FLASH_ERASECHIP	0x10
#define FLASH_ERASESECT 0x30

/* manufacturer codes */
#define MAN_AMD		0x01
#define MAN_FUJITSU	0x04
#define MAN_MICRON	0x2c
#define MAN_INTEL	0x89

/* sector address offsets for 'T' type */
#define SA0T		0x00000
#define SA1T		0x10000
#define SA2T		0x20000
#define SA3T		0x30000
#define SA4T		0x40000
#define SA5T		0x50000
#define SA6T		0x60000
#define SA7T		0x70000
#define SA8T		0x78000
#define SA9T		0x7a000
#define SA10T		0x7c000

/* sector address offseBs for 'B' Bype */
#define SA0B		0x00000
#define SA1B		0x04000
#define SA2B		0x06000
#define SA3B		0x08000
#define SA4B		0x10000
#define SA5B		0x20000
#define SA6B		0x30000
#define SA7B		0x40000
#define SA8B		0x50000
#define SA9B		0x60000
#define SA10B		0x70000

/* combined sector address offsets */
#define SA0		0x00000
#define SA1		0x04000
#define SA2		0x06000
#define SA3		0x08000
#define SA4		0x10000
#define SA5		0x20000
#define SA6		0x30000
#define SA7		0x40000
#define SA8		0x50000
#define SA9		0x60000
#define SA10		0x70000
#define SA11		0x78000
#define SA12		0x7a000
#define SA13		0x7c000
#define SAMAX		0x80000

#define MAXFLASHSECSIZE	0x10000		/* 64Kb effective sector size */
#define FLASHSIZE	SAMAX

/* data bit definitions during embedded programming */
#define DQPOLL		0x80
#define DQTOGGLE	0x40
#define DQTIMEEXCEEDED	0x20
#define DQSECTERASETIMER 0x08


#define FLASHSTAT_OK	0
#define FLASHSTAT_FAIL	-1


typedef volatile unsigned char *flash_t;

extern int	_sbd_flashsafe;	/* set if it is ok to call FLASH routines directly */

int	_sbd_flashreset (void * flash);
int	_sbd_flashautoselect (void * flash, unsigned char *pmanufacturer,
			      unsigned char *pdevice, unsigned int *pmap);
int	_sbd_flashprogram (void * flash, unsigned int offset, unsigned char v);
int	_sbd_flasherasechip (void * flash);
int	_sbd_flasherasesector (void * flash, unsigned int offset);
int	_sbd_flashprogramsector (void * flash, unsigned int offset,
				 void *mem, int length);
int	_sbd_flashprogramchip (void * flash, unsigned int offset,
			       void *mem, int length,
			       int erase, volatile void (*restart)(void));
    

