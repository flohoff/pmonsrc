/* 
 * npio.h: new controller parallel i/o
 */

#define NPIO_PA		0
#define NPIO_PB		1
#define NPIO_PC		2

#define NPA_ENGSEL	0x80	/* when E2_CS=0 */
#define NPA_E2_DOUT	0x80	/* when E2_CS=1 */
#define	NPA_SW_UP	0x40
#define	NPA_SW_MENU	0x20
#define	NPA_SW_ONLINE	0x10
#define	NPA_SW_DOWN	0x08
#define	NPA_SW_STORE	0x04
#define	NPA_SW_FF	0x02
#define	NPA_SW_RESET	0x01	/* when LCD_IENABLE=0 */
#define	NPA_LCD_BUSY	0x01	/* when LCD_IENABLE=1 */

#define NPA_LCD_ODATA	0xff

#define	NPB_LCD_NOUTPUT	0x40
#define	NPB_LCD_RW	0x20
#define	NPB_LCD_E	0x10
#define	NPB_LCD_RS	0x08
#define NPB_LED_NREADY	0x04
#define NPB_LED_NDATA	0x02
#define NPB_LED_NONLINE	0x01

#define NPC_TEMP_2	0x80
#define NPC_TEMP_1	0x40
#define NPC_TEMP_0	0x20
#define NPC_PP_NSEL2	0x10
#define NPC_PP_NSEL1	0x08
#define NPC_E2_DIN	0x04
#define NPC_E2_SK	0x02
#define NPC_E2_CS	0x01
