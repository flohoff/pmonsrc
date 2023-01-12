# $Id: targets.mk,v 1.2 1996/01/16 13:40:47 chris Exp $ 
#
# Targets.mk for PMON on AXON2
#

CPU	=R3000
include $(LSIPKG)/$(CPU).mk

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000200
FDATA	=a0000200

PROMFMT =s3

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) -DAXON2 -DR3041 -DMHZ=25

MACHDIRS=$(SRC)/axon2

MACHINC =-I$(SRC)/axon2

MACHOBJ	=sbd.o sbdreset.o m82510.o nonet.o

NETLIBS	=
NETDRV	=

# Target
ALL = upmon

# Target type(s)
TARGETS = BG

