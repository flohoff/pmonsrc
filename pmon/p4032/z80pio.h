/*
 * z80pio.h: Z80 parallel i/o chip register definitions
 */

#ifndef __ASSEMBLER__
typedef struct {
    unsigned int	a_dat;
    unsigned int	b_dat;
    unsigned int	a_ctl;
    unsigned int	b_ctl;
} zpiodev;
#endif

/* register offsets */
#define ZPIO_A_DAT	0
#define ZPIO_B_DAT	4
#define ZPIO_A_CTL	8
#define ZPIO_B_CTL	12

#define ZPIO_MODE_OUT	0x0f
#define ZPIO_MODE_IN	0x4f
#define ZPIO_MODE_BIDIR	0x8f
#define ZPIO_MODE_CTRL	0xcf

#define ZPIO_ICW_ENABLE	0x80
#define ZPIO_ICW_AND	0x40
#define ZPIO_ICW_HIGH	0x40
#define ZPIO_ICW_MASK	0x40
#define ZPIO_ICW_SET	0x07

#define ZPIO_INT_ENB	0x83
#define ZPIO_INT_DIS	0x03

#define ZPIOB_E2_DO	0x80	/* in:  EEPROM data out */
#define ZPIOB_E2_SK	0x40	/* out: EEPROM clock */
#define ZPIOB_E2_CS	0x20	/* out: EEPROM chip select */
#define ZPIOB_E2_DI	0x10	/* out: EEPROM data in */
#define ZPIOB_FPGA_DMASK 0x0c	/* out: FPGA JTAG data mask */
#define ZPIOB_FPGA_DSHFT 2	/* out: FPGA JTAG data shift */
#define ZPIOB_FPGA_TCK	0x02	/* out: FPGA JTAG clock */
#define ZPIOB_NFAULT	0x01	/* out: Centronics peripheral mode nFault */
