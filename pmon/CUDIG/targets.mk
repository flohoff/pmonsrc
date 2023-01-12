# $Id: targets.mk,v 1.2 1999/02/23 14:06:46 nigel Exp $ 
#
# Targets.mk for PMON on Telegate CUDIG
#

CPU	=R4000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

FTEXT	=9fc00000
FDATA	=80000280

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000280

# start address for text (ram)
RTEXT	=80020000
#RTEXT	=a0020000

PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP
#OPT	=-O1 -g

# Machine type(s)
MACHINE =$(CPUFLGS) -DCUDIG -DMHZ=67 -DFLASH -DITBASE=0xbfc68000

MACHDIRS=$(SRC)/cudig

MACHINC =-I$(SRC)/cudig

MACHOBJ	=sbd.o sbdreset.o ns16550.o
MACHOBJ +=sbdfrom.o flashrom.o sbdflashenv.o flashenv.o flash.o

override CONVFLAGBG=
override CONVFLAGLG=

# Target
ALL = upmon pram

# Target type(s)
override TARGETS = BG

ns16550.o: ns16550.h sbd.h
