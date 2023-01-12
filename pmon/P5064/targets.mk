# $Id: targets.mk,v 1.8 2001/05/11 12:10:53 chris Exp $ 
#
# Targets.mk for PMON on P5064
#

CPU	=R4000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

FTEXT	=9fc00000
FDATA	=80000400

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000400

# start address for text (ram)
RTEXT	=80020000
#RTEXT	=a0020000

PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS= $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_de

# Machine type(s)
MACHINE =$(CPUFLGS) -DP5064 -DTLBREGISTERS -DINET -DFLASH -DRTC -DBOOTPKG

MACHDIRS=$(SRC)/p5064:$(SRC)/pci

MACHINC =-I$(SRC)/p5064 -I.

MACHOBJ	= sbd.o sbdreset.o display.o
MACHOBJ += p2681.o
MACHOBJ += rtc.o 
MACHOBJ += ns16550.o centronics.o 
MACHOBJ += sbdfrom.o sbdflashenv.o flashenv.o flash.o
MACHOBJ += pciconf.o pcidevs.o pci_machdep.o
MACHOBJ += sbdnet.o $(NETDRV).o

override CONVFLAGBG="-b 3,2,1,0"
override CONVFLAGLG=

# Target
ALL = upmon pmon pram

# Target type(s)
ifndef TARGETS
TARGETS = LG BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

mips.o: ../targets.mk
