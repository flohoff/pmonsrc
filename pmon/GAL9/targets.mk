# $Id: targets.mk,v 1.8 1998/10/28 14:57:42 nigel Exp $ 
#
# Targets.mk for PMON on Galileo-9
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

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_de

# Machine type(s)
MACHINE =$(CPUFLGS) -DGAL9 -DTLBREGISTERS -DINET -DFLASH -DENVOFF_BASE=10 -DZ8530 -DMHZ=100 -DITBASE=0xbfc68000

MACHDIRS=$(SRC)/gal9:$(SRC)/pci

MACHINC =-I$(SRC)/gal9

MACHOBJ	=sbd.o sbdreset.o z8530.o
MACHOBJ +=sbdfrom.o sbdflashenv.o flashenv.o flash.o
MACHOBJ +=pciconf.o pcidevs.o sbdpci.o sbdnet.o $(NETDRV).o

override CONVFLAGBG=
override CONVFLAGLG=

# Target
ALL = upmon pram

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif

z8530.o: z8530.h sbd.h

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@
