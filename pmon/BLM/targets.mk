# $Id: targets.mk,v 1.2 1999/04/02 18:21:37 nigel Exp $ 
#
# Targets.mk for PMON on Siemens Atea BLM-RISX
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

PROMFMT =s3

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP
#OPT	=-O1 -g

# Machine type(s)
MACHINE =$(CPUFLGS) -DBLM -DZ8530 -DMHZ=180 -DFLASH #-DTLBREGISTERS 

MACHDIRS=$(SRC)/blm

MACHINC =-I$(SRC)/blm

MACHOBJ	=sbd.o sbdreset.o z8530.o
MACHOBJ +=sbdfrom.o flashrom.o sbdflashenv.o flashenv.o flash.o

override CONVFLAGBG=
override CONVFLAGLG=

# Target
ALL = upmon pram

# Target type(s)
override TARGETS = BG

z8530.o: z8530.h sbd.h

