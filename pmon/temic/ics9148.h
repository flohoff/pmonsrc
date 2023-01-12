/* 
 * TEMIC/ics9148.c: ICS9148 clock synthesiser for Algor/Temic module
 * Copyright (c) 1999 Algorithmics Ltd.
 */

struct ics9148reg {
    unsigned char	freq;
    unsigned char	cpu;
    unsigned char	pci;
    unsigned char	sdraml;
    unsigned char	sdramh;
    unsigned char	periph;
    unsigned char	res;
};

#define ICS_FREQ_SPREAD_0_6	0x80	/* 0.6% spread spectrum */
#define ICS_FREQ_MASK		0x70
#define  ICS_FREQ_100_333_666	 0x70
#define  ICS_FREQ_95_318_635	 0x60
#define  ICS_FREQ_83_333_666	 0x50
#define  ICS_FREQ_75_300_600	 0x40
#define  ICS_FREQ_75_375_750	 0x30
#define  ICS_FREQ_69_343_685	 0x20
#define  ICS_FREQ_67_334_668	 0x10
#define  ICS_FREQ_60_300_600	 0x00
#define ICS_FREQ_USEREG		0x08
#define ICS_FREQ_SPREAD_DOWN	0x04
#define ICS_FREQ_SPREAD_ENABLE	0x02
#define ICS_FREQ_TRISTATE	0x01

#define ICS_CPU_SDRAM12		0x10
#define ICS_CPU_CPUCLK2		0x04
#define ICS_CPU_CPUCLK1		0x02
#define ICS_CPU_CPUCLK0		0x01

#define ICS_PCI_PCICLKF		0x40
#define ICS_PCI_PCICLK4		0x10
#define ICS_PCI_PCICLK3		0x08
#define ICS_PCI_PCICLK2		0x04
#define ICS_PCI_PCICLK1		0x02
#define ICS_PCI_PCICLK0		0x01

#define ICS_SDRAML_SDRAM7	0x80
#define ICS_SDRAML_SDRAM6	0x40
#define ICS_SDRAML_SDRAM5	0x20
#define ICS_SDRAML_SDRAM4	0x10
#define ICS_SDRAML_SDRAM3	0x08
#define ICS_SDRAML_SDRAM2	0x04
#define ICS_SDRAML_SDRAM1	0x02
#define ICS_SDRAML_SDRAM0	0x01

#define ICS_SDRAMH_AGP0		0x80
#define ICS_SDRAMH_SDRAM11	0x08
#define ICS_SDRAMH_SDRAM10	0x04
#define ICS_SDRAMH_SDRAM9	0x02
#define ICS_SDRAMH_SDRAM8	0x01

#define ICS_PERIPH_AGP1		0x10
#define ICS_PERIPH_AGP2		0x02
#define ICS_PERIPH_REF0		0x01

extern int	_sbd_icsread (void *, int);
extern int	_sbd_icswrite (const void *, int);
