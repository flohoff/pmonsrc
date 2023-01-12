/* $Id: i8255.h,v 1.1 1996/12/10 11:58:18 nigel Exp $ */
/* 
 * i8255.h: definitions for Citadel i8255 parallel i/o port
 */

#define I8255_PA	0
#define I8255_PB	1
#define I8255_PC	2
#define I8255_CTL	3

#define I8255_MODE_SET		0x80
#define I8255_GRP1_MODE0	0x00
#define I8255_GRP1_MODE1	0x20
#define I8255_GRP1_MODE2	0x40
#define I8255_A_IN		0x10
#define I8255_A_OUT		0x00
#define I8255_CU_IN		0x08
#define I8255_CU_OUT		0x00
#define I8255_GRP2_MODE0	0x00
#define I8255_GRP2_MODE1	0x04
#define I8255_B_IN		0x02
#define I8255_B_OUT		0x00
#define I8255_CL_IN		0x01
#define I8255_CL_OUT		0x00

#define I8255_MODE_A_OUT	(I8255_MODE_SET | \
				 I8255_GRP1_MODE0 | I8255_GRP2_MODE0 | \
				 I8255_A_OUT | I8255_B_OUT | \
				 I8255_CU_OUT | I8255_CL_IN)

#define I8255_MODE_A_IN		(I8255_MODE_SET | \
				 I8255_GRP1_MODE0 | I8255_GRP2_MODE0 | \
				 I8255_A_IN | I8255_B_OUT | \
				 I8255_CU_OUT | I8255_CL_IN)

/* Port A: input mode */
#define	PA_LCD_BUSY	0x80
#define	PA_SW_UP	0x40
#define	PA_SW_MENU	0x20
#define	PA_SW_ONLINE	0x10
#define	PA_SW_DOWN	0x08
#define	PA_SW_STORE	0x04
#define	PA_SW_FF	0x02
#define	PA_SW_RESET	0x01

/* Port B: output */
/* note that eeprom and lcd control share some pins */
#define PB_E2_CS	0x40
#define PB_E2_SK	0x20	/* (LCD_RW) */
#define PB_E2_DIN	0x08	/* (LCD_RS) */

#define PB_LCD_RW	0x20	/* (E2_SK) */
#define PB_LCD_E	0x10
#define PB_LCD_RS	0x08	/* (E2_DIN) */

#define PB_TEMP_0	0x80
#define PB_LED_NREADY	0x04
#define PB_LED_NDATA	0x02
#define PB_LED_NONLINE	0x01

/* Port C (upper): output */
#define PC_TEMP_2	0x80
#define PC_TEMP_1	0x40
#define PC_PP_NSEL2	0x20	/* parallel port */
#define PC_PP_NSEL1	0x10	/* parallel port */

/* Port C (lower): input */
#define PC_ENGSEL	0x02
#define PC_E2_DOUT	0x01
