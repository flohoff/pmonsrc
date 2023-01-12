/* $Id: fpanel.h,v 1.1 1996/12/10 11:58:15 nigel Exp $ */
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
