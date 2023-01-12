/* 
 * devaz/sbd.h: Algorithmics DEVAZ board definition header file
 * Copyright (c) 1999 Algorithmics Ltd.
 */

#ifndef __SBD_H__
#define __SBD_H__

#ifndef MHZ
/* fastest possible pipeline clock */
#define MHZ		200
#endif

#define RAMCYCLE	60			/* ~60ns dram cycle */
#define ROMCYCLE	800			/* ~1500ns rom cycle */
#define CACHECYCLE	(1000/MHZ) 		/* pipeline clock */
#define CYCLETIME	CACHECYCLE
#define CACHEMISS	(CYCLETIME * 6)

/*
 * rough scaling factors for 2 instruction DELAY loop to get 1ms and 1us delays
 */
#define ASMDELAY(ns,icycle)	\
	(((ns) + (icycle)) / ((icycle) * 2))

#define CACHENS(ns)	ASMDELAY((ns), CACHECYCLE)
#define RAMNS(ns)	ASMDELAY((ns), CACHEMISS+RAMCYCLE)
#define ROMNS(ns)	ASMDELAY((ns), CACHEMISS+ROMCYCLE)
#define CACHEUS(us)	ASMDELAY((us)*1000, CACHECYCLE)
#define RAMUS(us)	ASMDELAY((us)*1000, CACHEMISS+RAMCYCLE)
#define ROMUS(us)	ASMDELAY((us)*1000, CACHEMISS+ROMCYCLE)
#define CACHEMS(ms)	((ms) * ASMDELAY(1000000, CACHECYCLE))
#define RAMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+RAMCYCLE))
#define ROMMS(ms)	((ms) * ASMDELAY(1000000, CACHEMISS+ROMCYCLE))

#ifndef __ASSEMBLER__
extern void _sbd_nsdelay (unsigned long ns);
#define nsdelay(ns)	_sbd_nsdelay (ns)
#define usdelay(us)	_sbd_nsdelay ((us) * 1000)
#define msdelay(ms)	_sbd_nsdelay ((ms) * 1000000)
#endif

#include "bonito.h"


#define PCI_MEM_SPACE	(BONITO_PCILO_BASE+0x00000000)	/* 192MB */
#define PCI_MEM_SPACE_SIZE	BONITO_PCILO_SIZE
#define PCI_IO_SPACE	BONITO_PCIIO_BASE	/* 1MB */
#define PCI_IO_SPACE_SIZE	BONITO_PCIIO_SIZE
#define PCI_CFG_SPACE	BONITO_PCICFG_BASE		/* 512KB */
#define PCI_CFG_SPACE_SIZE	BONITO_PCICFG_SIZE
#define BOOTPROM_BASE	BONITO_BOOT_BASE
#define BONITO_BASE	BONITO_REG_BASE
#define UART0_BASE	(BONITO_DEV_BASE+0x40020) /* IOCS1 */
#define UART1_BASE	(BONITO_DEV_BASE+0x40000) /* IOCS1 */
#define IDE0_BASE	(BONITO_DEV_BASE+0x80000) /* IOCS2 */
#define IDE1_BASE	(BONITO_DEV_BASE+0xc0000) /* IOCS3 */
#define FLASH_BASE	BONITO_FLASH_BASE
#define FLASH_SIZE	BONITO_FLASH_SIZE
#define BOOT_BASE	BONITO_BOOT_BASE
#define BOOT_SIZE	BONITO_BOOT_SIZE
#define SOCKET_BASE	BONITO_SOCKET_BASE
#define SOCKET_SIZE	BONITO_SOCKET_SIZE

/* Define UART baud rate and register layout */
#define NS16550_HZ	18432000	/* 18.432MHz */
#ifdef __ASSEMBLER__
#define NSREG(x)	((x)*4)
#else
#define nsreg(x)	unsigned int x
#endif

/* GPIO definitions */
#define PIO_I2C_DIMMSDAW BONITO_GPIO_IOW(0)	/* I2C SODIMM data */
#define PIO_I2C_SCL	BONITO_GPIO_IOW(1)	/* I2C clock */
#define PIO_I2C_DBGSDAW	BONITO_GPIO_IOW(2)	/* I2C DBG data */
#define PIO_UART_RESET	BONITO_GPIO_IOW(3)	/* Active high */
#define PIO_ICS_SCL	BONITO_GPIO_IOW(4)	/* ICS synth serial clock */
#define PIO_ICS_SDAW	BONITO_GPIO_IOW(5)	/* ICS synth serial data */
#define PIO_IDE_RESET	BONITO_GPIO_IOW(7)	/* Active Low */
#define PIO_I2C_DIMMSDAR BONITO_GPIO_IOR(0)	/* I2C SODIMM data */
#define PIO_I2C_DBGSDAR BONITO_GPIO_IOR(2)	/* I2C DBG data */
#define PIO_ICS_SDAR	BONITO_GPIO_IOR(5)	/* ICS synth serial data */
#define PIO_IDE_INTR	BONITO_GPIO_INR(0)
#define PIO_UART0_INTR	BONITO_GPIO_INR(1)
#define PIO_UART1_INTR	BONITO_GPIO_INR(2)
#define PIO_IDE_ASP	BONITO_GPIO_INR(3)
#define PIO_IDE_PDIAG	BONITO_GPIO_INR(4)
#define PIO_DBG_INTR	BONITO_GPIO_INR(5)

/* ICU masks */
#define ICU_IDE_INTR	BONITO_ICU_GPIN(0)
#define ICU_UART0_INTR	BONITO_ICU_GPIN(1)
#define ICU_UART1_INTR	BONITO_ICU_GPIN(2)
#define ICU_DRAMPERR	BONITO_ICU_DRAMPERR
#define ICU_CPUPERR	BONITO_ICU_CPUPERR
#define ICU_IDEDMA	BONITO_ICU_IDEDMA
#define ICU_PCICOPIER	BONITO_ICU_PCICOPIER
#define ICU_POSTEDRD	BONITO_ICU_POSTEDRD
#define ICU_PCIIRQ	BONITO_ICU_PCIIRQ
#define ICU_MASTERERR	BONITO_ICU_MASTERERR
#define ICU_SYSTEMERR	BONITO_ICU_SYSTEMERR

/* default PIO input enable */
#define PIO_IE		(~(PIO_IDE_RESET|PIO_UART_RESET|PIO_I2C_SCL|PIO_ICS_SCL))

/* Store environment in flash #0 */
#define _SBD_FLASHENV	0

#ifndef __ASSEMBLER__

/* prototypes for board specific functions */

#ifdef FLASHDEV_OK
extern flashcookie_t	_sbd_bflashopen (paddr_t);
extern flashcookie_t	_sbd_uflashopen (paddr_t);
#endif

extern void _bonito_clean_dcache (void *va, unsigned long nb);
extern void _bonito_inval_dcache (void *va, unsigned long nb);

#endif

/* divert device drivers to Bonito-specific cache cleaning code */
#define sbd_clean_dcache	_bonito_clean_dcache
#define sbd_inval_dcache	_bonito_inval_dcache

#endif /* __SBD_H__ */
