# $Id: targets.mk,v 1.1 1998/06/26 13:02:22 chris Exp $ 
#
# Targets.mk for PMON on CCUBE
# Copyright (c) 1998 Algorithmics Ltd
#

CPU	=R3000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

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
#SWFP	=

# Machine type(s)
MACHINE =$(CPUFLGS) -DCCUBE -DR3041 -DFLASH

MACHDIRS=$(SRC)/ccube

MACHINC =-I$(SRC)/ccube -I.

MACHOBJ	= sbd.o sbdreset.o
MACHOBJ += ns16550.o
#MACHOBJ += sbdfrom.o sbdflashenv.o flashenv.o flash.o

# Target
ALL = upmon pmon pram

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif

