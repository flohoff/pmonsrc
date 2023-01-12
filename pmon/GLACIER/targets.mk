# $Id: targets.mk,v 1.1 1996/09/28 23:42:06 chris Exp $ 
#
# Targets.mk for PMON on GLACIER
#

CPU	=R4000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

FTEXT	=8fc00000
FDATA	=80000200

# start address for text and data (uncached rom)
UFTEXT	=afc00000
UFDATA	=a0000200

# start address for text (ram)
RTEXT	=80020000
#RTEXT	=a0020000

PROMFMT =s3

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

#NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
#	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
#	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
#	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
#	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a
NELLIBS =
NETDRV	=

# Machine type(s)
MACHINE =$(CPUFLGS) -DGLACIER -DTLBREGISTERS

MACHDIRS=$(SRC)/glacier:$(SRC)/pci

MACHINC =-I$(SRC)/glacier -I.

MACHOBJ	=sbd.o sbdreset.o sbdenv.o gser.o gpar.o
MACHOBJ +=pciconf.o pcidevs.o pci_machdep.o

override CONVFLAG=

# Target
ALL = upmon pmon pram

# Target type(s)
ifndef TARGETS
TARGETS = LG
endif

#gser.o: gser.c
#	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -DPROM -DLANGUAGE_C -c $< -o $@

#gpar.o: gpar.c
#	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -DPROM -DLANGUAGE_C -c $< -o $@
