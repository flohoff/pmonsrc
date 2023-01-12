# $Id: targets.mk,v 1.6 1999/07/12 17:22:36 chris Exp $
#
# Targets.mk for PMON on Algorithmics DEVA-Z board
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
MACHINE =$(CPUFLGS) -DDEVAZ -DNS16550 -DMHZ=120 -DTLBREGISTERS -DFLASH -DINET # -DFROM_DEBUG=2 -DTULIP_DEBUG -DTULIP_MEGADEBUG

MACHDIRS=$(SRC)/devaz:$(SRC)/pci

MACHINC =-I$(SRC)/devaz -I$(SRC)/pci

MACHOBJ	=sbd.o sbdreset.o ns16550.o
MACHOBJ	+=sbdfreq.o sbdics.o
MACHOBJ +=sbdflashenv.o sbdflash.o sbduflash.o sbdbflash.o flash.o flashenv.o
MACHOBJ +=pciconf.o pcidevs.o pci_machdep.o sbdnet.o
MACHOBJ += $(NETDRV).o

override CONVFLAGBG="-b 3,2,1,0"
override CONVFLAGLG=

# Target
ALL = pmon upmon pram

# Target type(s)
override TARGETS = LG BG

ns16550.o: ns16550.h sbd.h

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

sbdbflash.o: sbdbflash.c flashrom.c flashrom.h
sbduflash.o: sbduflash.c flashrom.c flashrom.h
