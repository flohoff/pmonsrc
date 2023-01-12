# $Id: targets.mk,v 1.2 1999/06/23 13:14:07 nigel Exp $ 
#
# Targets.mk for PMON on Algorithmics/Temic module
#

CPU	=R4000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

#FTEXT	=9fc00000
#FDATA	=80000280
FTEXT	=80001000
DBASE	=

# start address for text and data (uncached rom)
#UFTEXT	=bfc00000
#UFDATA	=a0000280
UFTEXT	=a0001000
DBASE	=

# start address for text (ram)
RTEXT	=80080000
#RTEXT	=a0020000

PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP
OPT	=-O1 -g

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_de

# Machine type(s)
MACHINE =$(CPUFLGS) -DTEMIC -DNS16550 -DMHZ=60 -DCLIENTPC=0x80080000 -DTLBREGISTERS -DINET -DFLASH #-DTULIP_DEBUG #-DTULIP_MEGADEBUG

MACHDIRS=$(SRC)/temic:$(SRC)/pci

MACHINC =-I$(SRC)/temic

MACHOBJ	=sbd.o sbdreset.o ns16550.o 
MACHOBJ	+=sbdfreq.o sbdics.o
MACHOBJ +=sbdfrom.o sbdflashenv.o flashrom.o flash.o flashenv.o
MACHOBJ +=sbdnet.o pciconf.o pcidevs.o pci_machdep.o $(NETDRV).o

override CONVFLAGBG=
override CONVFLAGLG=

# Target
ALL = pmon upmon pram

# Target type(s)
override TARGETS = LG

ns16550.o: ns16550.h sbd.h

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@
