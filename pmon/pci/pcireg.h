/*	$NetBSD: pcireg.h,v 1.4 1995/07/27 00:29:02 mycroft Exp $	*/

/*
 * Copyright (c) 1995 Nigel Stephens.  All rights reserved.
 * Copyright (c) 1995 Christopher G. Demetriou.  All rights reserved.
 * Copyright (c) 1994 Charles Hannum.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Charles Hannum.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Standardized PCI configuration information
 *
 * XXX This is not complete.
 */

/* some PCI bus constants */

#define PCI_BUSMAX	255
#define PCI_DEVMAX	31
#define PCI_FUNCMAX	7
#define PCI_REGMAX	255

/*
 * Device identification register; contains a vendor ID and a device ID.
 */
#define	PCI_ID_REG			0x00

#ifndef __ASSEMBLER__
typedef u_int16_t pci_vendor_id_t;
typedef u_int16_t pci_product_id_t;
#endif

#define	PCI_VENDOR_SHIFT			0
#define	PCI_VENDOR_MASK				0xffff
#define	PCI_VENDOR(id) \
	    (((id) >> PCI_VENDOR_SHIFT) & PCI_VENDOR_MASK)

#define	PCI_PRODUCT_SHIFT			16
#define	PCI_PRODUCT_MASK			0xffff
#define	PCI_PRODUCT(id) \
	    (((id) >> PCI_PRODUCT_SHIFT) & PCI_PRODUCT_MASK)

/*
 * Command and status register.
 */
#define	PCI_COMMAND_STATUS_REG			0x04

#define	PCI_COMMAND_IO_ENABLE			0x00000001
#define	PCI_COMMAND_MEM_ENABLE			0x00000002
#define	PCI_COMMAND_MASTER_ENABLE		0x00000004
#define	PCI_COMMAND_SPECIAL_ENABLE		0x00000008
#define	PCI_COMMAND_INVALIDATE_ENABLE		0x00000010
#define	PCI_COMMAND_PALETTE_ENABLE		0x00000020
#define	PCI_COMMAND_PARITY_ENABLE		0x00000040
#define	PCI_COMMAND_STEPPING_ENABLE		0x00000080
#define	PCI_COMMAND_SERR_ENABLE			0x00000100
#define	PCI_COMMAND_BACKTOBACK_ENABLE		0x00000200

#define	PCI_STATUS_66MHZ_OKAY			0x00200000
#define	PCI_STATUS_UDF_SUPPORTED		0x00400000
#define	PCI_STATUS_BACKTOBACK_OKAY		0x00800000
#define	PCI_STATUS_PARITY_ERROR			0x01000000
#define	PCI_STATUS_DEVSEL_FAST			0x00000000
#define	PCI_STATUS_DEVSEL_MEDIUM		0x02000000
#define	PCI_STATUS_DEVSEL_SLOW			0x04000000
#define	PCI_STATUS_DEVSEL_MASK			0x06000000
#define	PCI_STATUS_DEVSEL_SHIFT			25
#define	PCI_STATUS_TARGET_TARGET_ABORT		0x08000000
#define	PCI_STATUS_MASTER_TARGET_ABORT		0x10000000
#define	PCI_STATUS_MASTER_ABORT			0x20000000
#define	PCI_STATUS_SPECIAL_ERROR		0x40000000
#define	PCI_STATUS_PARITY_DETECT		0x80000000

/*
 * PCI Class and Revision Register; defines type and revision of device.
 */
#define	PCI_CLASS_REG			0x08

#ifndef __ASSEMBLER__
typedef u_int8_t pci_class_t;
typedef u_int8_t pci_subclass_t;
typedef u_int8_t pci_interface_t;
typedef u_int8_t pci_revision_t;
#endif

#define	PCI_CLASS_SHIFT				24
#define	PCI_CLASS_MASK				0xff
#define	PCI_CLASS(cr) \
	    (((cr) >> PCI_CLASS_SHIFT) & PCI_CLASS_MASK)

#define	PCI_SUBCLASS_SHIFT			16
#define	PCI_SUBCLASS_MASK			0xff
#define	PCI_SUBCLASS(cr) \
	    (((cr) >> PCI_SUBCLASS_SHIFT) & PCI_SUBCLASS_MASK)

#define	PCI_INTERFACE_SHIFT			8
#define	PCI_INTERFACE_MASK			0xff
#define	PCI_INTERFACE(cr) \
	    (((cr) >> PCI_INTERFACE_SHIFT) & PCI_INTERFACE_MASK)

#define	PCI_REVISION_SHIFT			0
#define	PCI_REVISION_MASK			0xff
#define	PCI_REVISION(cr) \
	    (((cr) >> PCI_REVISION_SHIFT) & PCI_REVISION_MASK)

/* base classes */
#define	PCI_CLASS_PREHISTORIC			0x00
#define	PCI_CLASS_MASS_STORAGE			0x01
#define	PCI_CLASS_NETWORK			0x02
#define	PCI_CLASS_DISPLAY			0x03
#define	PCI_CLASS_MULTIMEDIA			0x04
#define	PCI_CLASS_MEMORY			0x05
#define	PCI_CLASS_BRIDGE			0x06
#define	PCI_CLASS_COMMUNICATION			0x07
#define	PCI_CLASS_PERIPHERAL			0x08
#define	PCI_CLASS_INPUT				0x09
#define	PCI_CLASS_DOCKING			0x0a
#define	PCI_CLASS_PROCESSOR			0x0b
#define	PCI_CLASS_SERIALBUS			0x0c
#define	PCI_CLASS_UNDEFINED			0xff

/* 0x00 prehistoric subclasses */
#define	PCI_SUBCLASS_PREHISTORIC_MISC		0x00
#define	PCI_SUBCLASS_PREHISTORIC_VGA		0x01

/* 0x01 mass storage subclasses */
#define	PCI_SUBCLASS_MASS_STORAGE_SCSI		0x00
#define	PCI_SUBCLASS_MASS_STORAGE_IDE		0x01
#define	PCI_SUBCLASS_MASS_STORAGE_FLOPPY	0x02
#define	PCI_SUBCLASS_MASS_STORAGE_IPI		0x03
#define	PCI_SUBCLASS_MASS_STORAGE_RAID		0x04
#define	PCI_SUBCLASS_MASS_STORAGE_MISC		0x80

/* 0x02 network subclasses */
#define	PCI_SUBCLASS_NETWORK_ETHERNET		0x00
#define	PCI_SUBCLASS_NETWORK_TOKENRING		0x01
#define	PCI_SUBCLASS_NETWORK_FDDI		0x02
#define	PCI_SUBCLASS_NETWORK_ATM		0x03
#define	PCI_SUBCLASS_NETWORK_MISC		0x80

/* 0x03 display subclasses */
#define	PCI_SUBCLASS_DISPLAY_VGA		0x00
#define	PCI_SUBCLASS_DISPLAY_XGA		0x01
#define	PCI_SUBCLASS_DISPLAY_MISC		0x80

/* 0x04 multimedia subclasses */
#define	PCI_SUBCLASS_MULTIMEDIA_VIDEO		0x00
#define	PCI_SUBCLASS_MULTIMEDIA_AUDIO		0x01
#define	PCI_SUBCLASS_MULTIMEDIA_MISC		0x80

/* 0x05 memory subclasses */
#define	PCI_SUBCLASS_MEMORY_RAM			0x00
#define	PCI_SUBCLASS_MEMORY_FLASH		0x01
#define	PCI_SUBCLASS_MEMORY_MISC		0x80

/* 0x06 bridge subclasses */
#define	PCI_SUBCLASS_BRIDGE_HOST		0x00
#define	PCI_SUBCLASS_BRIDGE_ISA			0x01
#define	PCI_SUBCLASS_BRIDGE_EISA		0x02
#define	PCI_SUBCLASS_BRIDGE_MC			0x03
#define	PCI_SUBCLASS_BRIDGE_PCI			0x04
#define	PCI_SUBCLASS_BRIDGE_PCMCIA		0x05
#define	PCI_SUBCLASS_BRIDGE_NUBUS		0x06
#define	PCI_SUBCLASS_BRIDGE_CARDBUS		0x07
#define	PCI_SUBCLASS_BRIDGE_MISC		0x80

/* 0x07 communication subclasses */
#define	PCI_SUBCLASS_COMMUNICATION_SERIAL	0x00
#define	PCI_SUBCLASS_COMMUNICATION_PARALLEL	0x01
#define	PCI_SUBCLASS_COMMUNICATION_MISC		0x80

/* 0x08 peripheral subclasses */
#define PCI_SUBCLASS_PERIPHERAL_PIC		0x00
#define PCI_SUBCLASS_PERIPHERAL_DMA		0x01
#define PCI_SUBCLASS_PERIPHERAL_TIMER		0x02
#define PCI_SUBCLASS_PERIPHERAL_RTC		0x03
#define PCI_SUBCLASS_PERIPHERAL_MISC		0x80

/* 0x09 input subclasses */
#define PCI_SUBCLASS_INPUT_KEYBOARD		0x00
#define PCI_SUBCLASS_INPUT_DIGITISER		0x01
#define PCI_SUBCLASS_INPUT_MOUSE		0x02
#define PCI_SUBCLASS_INPUT_MISC			0x80

/* 0x0a docking subclasses */
#define PCI_SUBCLASS_DOCKING_GENERIC		0x00
#define PCI_SUBCLASS_DOCKING_MISC		0x80

/* 0x0b processor subclases */
#define PCI_SUBCLASS_PROCESSOR_386		0x00
#define PCI_SUBCLASS_PROCESSOR_486		0x01
#define PCI_SUBCLASS_PROCESSOR_PENTIUM		0x02
#define PCI_SUBCLASS_PROCESSOR_ALPHA		0x10
#define PCI_SUBCLASS_PROCESSOR_POWERPC		0x20
#define PCI_SUBCLASS_PROCESSOR_COPROCESSOR	0x40

/* 0x0c serial bus subclasses */
#define PCI_SUBCLASS_SERIALBUS_FIREWIRE		0x00
#define PCI_SUBCLASS_SERIALBUS_ACCESSS		0x01
#define PCI_SUBCLASS_SERIALBUS_SSA		0x02
#define PCI_SUBCLASS_SERIALBUS_USB		0x03
#define PCI_SUBCLASS_SERIALBUS_FIBRECHANNEL	0x04

#define	PCI_MISC_REG			0x0c

#define	PCI_MISC_HDRTYPE_SHIFT			16
#define	PCI_MISC_HDRTYPE_MASK			0xff
#define	PCI_MISC_HDRTYPE(mr) \
	    (((mr) >> PCI_MISC_HDRTYPE_SHIFT) & PCI_MISC_HDRTYPE_MASK)
#define PCI_MISC_HDRTYPE_MULTI			0x80
#define PCI_MISC_HDRTYPE_TYPEMASK		0x7f
#define PCI_MISC_HDRTYPE_DEVICE			0
#define PCI_MISC_HDRTYPE_BRIDGE			1

#define	PCI_MISC_LTIM_SHIFT			8
#define	PCI_MISC_LTIM_MASK			0xff
#define	PCI_MISC_LTIM(mr) \
	    (((mr) >> PCI_MISC_LTIM_SHIFT) & PCI_MISC_LTIM_MASK)
#define	PCI_MISC_LTIM_SET(mr,v) \
	    (mr) = ((mr) & ~(PCI_MISC_LTIM_MASK << PCI_MISC_LTIM_SHIFT)) | \
		((v) << PCI_MISC_LTIM_SHIFT)

#define	PCI_MISC_CLSZ_SHIFT			0
#define	PCI_MISC_CLSZ_MASK			0xff
#define	PCI_MISC_CLSZ(mr) \
	    (((mr) >> PCI_MISC_CLSZ_SHIFT) & PCI_MISC_CLSZ_MASK) 
#define	PCI_MISC_CLSZ_SET(mr,v) \
	    (mr) = ((mr) & ~(PCI_MISC_CLSZ_MASK << PCI_MISC_CLSZ_SHIFT)) | \
		((v) << PCI_MISC_CLSZ_SHIFT)

/*
 * Mapping registers
 */
#define	PCI_MAP_REG_START		0x10
#define	PCI_MAP_REG_END			0x28
#define	PCI_MAP_REG_ROM			0x30

#define	PCI_MAP_MEMORY				0x00000000
#define	PCI_MAP_MEMORY_TYPE_32BIT		0x00000000
#define	PCI_MAP_MEMORY_TYPE_32BIT_1M		0x00000002
#define	PCI_MAP_MEMORY_TYPE_64BIT		0x00000004
#define	PCI_MAP_MEMORY_TYPE_MASK		0x00000006
#define	PCI_MAP_MEMORY_CACHABLE			0x00000008
#define	PCI_MAP_MEMORY_PREFETCHABLE		0x00000008
#define	PCI_MAP_MEMORY_ADDRESS_MASK		0xfffffff0

#define	PCI_MAP_IO				0x00000001
#define	PCI_MAP_IO_ADDRESS_MASK			0xfffffffc

#define	PCI_MAP_ROM				0x00000001
#define	PCI_MAP_ROM_ADDRESS_MASK		0xfffff800

/*
 * Interrupt Configuration Register; contains interrupt pin and line.
 */
#define	PCI_INTERRUPT_REG		0x3c

#ifndef __ASSEMBLER__
typedef u_int8_t pci_intr_pin_t;
typedef u_int8_t pci_intr_line_t;
#endif

#define	PCI_INTERRUPT_PIN_SHIFT			8
#define	PCI_INTERRUPT_PIN_MASK			0xff
#define	PCI_INTERRUPT_PIN(icr) \
	    (((icr) >> PCI_INTERRUPT_PIN_SHIFT) & PCI_INTERRUPT_PIN_MASK)

#define	PCI_INTERRUPT_LINE_SHIFT		0
#define	PCI_INTERRUPT_LINE_MASK			0xff
#define	PCI_INTERRUPT_LINE(icr) \
	    (((icr) >> PCI_INTERRUPT_LINE_SHIFT) & PCI_INTERRUPT_LINE_MASK)

#define	PCI_INTERRUPT_PIN_NONE			0x00
#define	PCI_INTERRUPT_PIN_A			0x01
#define	PCI_INTERRUPT_PIN_B			0x02
#define	PCI_INTERRUPT_PIN_C			0x03
#define	PCI_INTERRUPT_PIN_D			0x04

#define	PCI_BPARAM_REG 			0x3c

#define	PCI_BPARAM_MAX_LAT_SHIFT		24
#define	PCI_BPARAM_MAX_LAT_MASK			0xff
#define	PCI_BPARAM_MAX_LAT(bp) \
	    (((bp) >> PCI_BPARAM_MAX_LAT_SHIFT) & PCI_BPARAM_MAX_LAT_MASK)

#define	PCI_BPARAM_MIN_GNT_SHIFT		16
#define	PCI_BPARAM_MIN_GNT_MASK			0xff
#define	PCI_BPARAM_MIN_GNT(bp) \
	    (((bp) >> PCI_BPARAM_MIN_GNT_SHIFT) & PCI_BPARAM_MIN_GNT_MASK)

/*
 * PCI to PCI Bridge registers
 */
#define PPB_BUS_REG 0x18

#define	PPB_IO_REG	0x1C
#define		PPB_IO_BASE_MASK	0x000000ff
#define		PPB_IO_LIMIT_MASK	0x0000ff00

#define	PPB_MEM_REG	0x20
#define		PPB_MEM_BASE_MASK	0x0000ffff
#define		PPB_MEM_LIMIT_MASK	0xffff0000
