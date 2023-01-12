# $Id: targets.mk,v 1.2 1999/06/07 14:19:49 nigel Exp $ 
#
# Targets.mk for PMON on NEC DDB Vrc5074 eval board
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
RTEXT	=80080000
#RTEXT	=a0080000

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
MACHINE =$(CPUFLGS) -DNEC5074 -DNS16550 -DMHZ=160 -DFLASH -DRTC -DNVENV -DTLBREGISTERS -DINET #-DTULIP_DEBUG #-DTULIP_MEGADEBUG

MACHDIRS=$(SRC)/nec5074:$(SRC)/pci

MACHINC =-I$(SRC)/nec5074

MACHOBJ	=sbd.o sbdreset.o ns16550.o ns16550i.o
MACHOBJ	+=sbdfreq.o sbdtod.o
MACHOBJ +=sbdfrom.o flashrom.o flash.o
MACHOBJ +=sbdnvram.o nvenv.o 
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
