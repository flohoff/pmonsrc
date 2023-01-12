# $Id: targets.mk,v 1.2 1996/01/16 14:23:36 chris Exp $ 
#
# Targets.mk for PMON on IDT RS385
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

PROMFMT =stagbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) -DIDT385 -DR3081 -DR3041 #-DM82510

MACHDIRS=$(SRC)/idt385

MACHINC =-I$(SRC)/idt385

MACHOBJ	=sbd.o sbdreset.o p2681.o m82510.o 

NETLIBS	=
NETDRV	=

# Target type(s)
TARGETS = BG
