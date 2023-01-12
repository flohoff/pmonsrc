# $Id: targets.mk,v 1.4 1996/12/10 12:19:16 nigel Exp $ 
#
# Targets.mk for PMON on LIBRA
#

CPU	=R3000
include $(LSIPKG)/$(CPU).mk

# start address for text and data (cached rom)
FTEXT	=9fc00000
FDATA	=80000200

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000200

# start address for text (ram)
RTEXT	=80020000
#RTEXT	=a0020000

PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) -DR3081 -DLASERFOLD -DEENVRAM -DMHZ=25

MACHDIRS=$(SRC)/laserfold

MACHINC =-I$(SRC)/laserfold

NETLIBS =
NETDRV	=

MACHOBJ	=sbd.o sbdreset.o eeprom.o fpanel.o pio.o centronics.o ns16550.o

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif
