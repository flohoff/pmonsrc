/* $Id: cm1629.h,v 1.2 1996/01/16 14:24:32 chris Exp $ */
/*
 * CM1629 2x16 LCD
 */

#ifdef MIPSEB
#define lcdreg(x)	unsigned :32; unsigned :24; unsigned x:8;
#define LCDREG(x)	((8*(x))+7)
#endif
#ifdef MIPSEL
#define lcdreg(x)	unsigned x:8; unsigned :24; unsigned :32; 
#define LCDREG(x)	(8*(x))
#endif

#ifndef __ASSEMBLER__
typedef struct {
    lcdreg(lcd_data);		/* data register */
    lcdreg(lcd_cmd);		/* command register */
} cm1629dev;
#define lcd_stat lcd_cmd
#endif

#define LCD_DATA	LCDREG(0)
#define LCD_CMD		LCDREG(1)
#define LCD_STAT	LCD_CMD

/* status register */
#define	LCD_STAT_BUSY	0x80	/* display busy */
#define LCD_ADD_MASK	0x7f	/* current display address */

/* command register */

#define LCD_NOOP	0x00
#define LCD_CLEAR	0x01
#define LCD_HOME	0x02
#define LCD_MODE	0x04
# define LCD_MODE_INCR	  0x02
# define LCD_MODE_DECR	  0x00
# define LCD_MODE_SHIFT	  0x01
#define LCD_CTRL	0x08
# define LCD_CTRL_DISPLAY 0x04
# define LCD_CTRL_CURSOR  0x02
# define LCD_CTRL_BLINK   0x01
#define LCD_CSHIFT	0x10
#define LCD_DSHIFT	0x18
# define LCD_SHIFT_L	  0x04
# define LCD_SHIFT_R	  0x00
#define LCD_FUNC	0x20
# define LCD_FUNC_8BIT	  0x10
# define LCD_FUNC_4BIT	  0x00
# define LCD_FUNC_2LINE   0x08
# define LCD_FUNC_1LINE   0x00
# define LCD_FUNC_5x10	  0x04
# define LCD_FUNC_5x7	  0x00
#define LCD_SET_CGADDR	0x40
#define LCD_SET_DDADDR	0x80
