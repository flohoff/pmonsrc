# $Id: targets.mk,v 1.2 1996/01/16 14:23:49 chris Exp $ 
#
# Targets.mk for PMON on Mannesman RIP-CPU 2.1
#

CPU	=R4000
include $(LSIPKG)/$(CPU).mk

# start address for text and data (cached rom)
FTEXT	=9fc00000
FDATA	=80800200

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0800200

# start address for text (ram)
RTEXT	=80820000
#RTEXT	=a0820000

CLIENTPC=0x80820000

PROMFMT =stagbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=

# Machine type(s)
MACHINE =$(CPUFLGS) -DMANN -DSROM -DITBASE=0xbfc70000 -DCLIENTPC=0x80820000 -DLOCAL_MEM=0x00800000 -DZ8530
#MACHINE =$(CPUFLGS) -DMANN -DSROM -DCLIENTPC=0x80820000 -DLOCAL_MEM=0x00800000 -DZ8530

MACHDIRS=$(SRC)/mann

MACHINC =-I$(SRC)/mann

NETDRV	=

MACHOBJ	=sbd.o sbdreset.o m82510.o z8530.o pport.o

# Target type(s)
TARGETS = BG

