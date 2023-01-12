/* 
 * pio.h: abstracted parallel i/o for Laserfold boards
 */

/* bits are identified by an abstract id */
enum pioport {PIO_SW=0,
	      PIO_LCD_BUSY,
	      PIO_LCD_DATA,
	      PIO_LCD_RW,
	      PIO_LCD_E,
	      PIO_LCD_RS,
	      PIO_LED_NREADY,
	      PIO_LED_NDATA,
	      PIO_LED_NONLINE,
	      PIO_E2_DIN,
	      PIO_E2_SK,
	      PIO_E2_CS,
	      PIO_E2_DOUT,
	      PIO_TEMP_2,
	      PIO_TEMP_1,
	      PIO_TEMP_0,
	      PIO_PP_NSEL2,
	      PIO_PP_NSEL1,
	      PIO_ENGSEL};

/* switches are read as a whole byte: these are the individual bits */
#define	PIO_SW_UP	0x40
#define	PIO_SW_MENU	0x20
#define	PIO_SW_ONLINE	0x10
#define	PIO_SW_DOWN	0x08
#define	PIO_SW_STORE	0x04
#define	PIO_SW_FF	0x02
#define	PIO_SW_RESET	0x01

/* button bit numbers */
#define	SW_UP		6
#define	SW_MENU		5
#define	SW_ONLINE	4
#define	SW_DOWN		3
#define	SW_STORE	2
#define	SW_FF		1
#define	SW_RESET	0
#define NBUTTONS	7

#ifndef __ASSEMBLER__
extern void		pio_init (void);
extern void 		pio_acquire (void);
extern void 		pio_release (void);
extern unsigned int	pio_put (enum pioport, unsigned int val);
extern unsigned int	pio_get (enum pioport);
extern unsigned int	pio_bis (enum pioport);
extern unsigned int	pio_bic (enum pioport);
#endif


