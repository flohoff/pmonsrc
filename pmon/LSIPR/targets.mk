# $Id: targets.mk,v 1.2 1996/01/16 14:23:44 chris Exp $ 
#
# Targets.mk for PMON on P4000
#

CPU	=LR33K
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

PROMFMT =s3

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=

# Machine type(s)
MACHINE =$(CPUFLGS) -DLSIPR

MACHDIRS=$(SRC)/lsipr

MACHINC =-I$(SRC)/lsipr

NETDRV	=

MACHOBJ	=sbd.o sbdreset.o p2681.o

# Target type(s)
TARGETS = BG
