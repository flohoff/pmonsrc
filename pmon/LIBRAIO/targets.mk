# $Id: targets.mk,v 1.1 1996/06/28 12:29:29 nigel Exp $ 
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
MACHINE =$(CPUFLGS) -DLIBRA -DR3081 -DZ8530 -DMHZ=25

MACHDIRS=$(SRC)/libraio

MACHINC =-I$(SRC)/libraio

NETLIBS =
NETDRV	=

MACHOBJ	=sbd.o sbdreset.o z8530.o centronics.o

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif
