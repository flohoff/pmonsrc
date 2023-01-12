/* 
 * p6032/ics9148.h: ICS9148 clock synthesiser
 *
 * Copyright (c) 2000-2001, Algorithmics Ltd.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the "Free MIPS" License Agreement, a copy of 
 * which is available at:
 *
 *  http://www.algor.co.uk/ftp/pub/doc/freemips-license.txt
 *
 * You may not, however, modify or remove any part of this copyright 
 * message if this program is redistributed or reused in whole or in
 * part.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * "Free MIPS" License for more details.  
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

#define ICS_I2C_BASE_ADDRESS	0xd2

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
