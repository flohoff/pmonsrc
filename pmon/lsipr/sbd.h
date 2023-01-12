/* $Id: sbd.h,v 1.3 1997/01/20 18:18:14 chris Exp $ */
/* stuff for LEDs on Pocket Rocket and RacerX boards */

#ifdef MIPSEB
#define SIOBASE 0xbe000003
#else
#define SIOBASE 0xbe000000
#endif

#ifdef RACERX
#define RED_LED	0x20
#define GRN_LED 0x40
#else
#define RED_LED	0x02
#define GRN_LED 0x01
#endif

#define LOCAL_MEM_SIZE	0x400000	/* Max mem (4Mb) */

#ifdef LANGUAGE_C
#define LED_OFF (*((volatile unsigned char *)SIOBASE+0x38))
#define LED_ON (*((volatile unsigned char *)SIOBASE+0x3c))
#else
#define LED_OFF SIOBASE+0x38
#define LED_ON  SIOBASE+0x3c
#endif
