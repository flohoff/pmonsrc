## $Id: targets.mk,v 1.8 2001/05/11 12:10:58 chris Exp $
#
# Targets.mk for PMON on Algorithmics P6032 board
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

PROMFMT =bin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP
#OPT	=-O1 -g

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_le_pci
#NETDRV	=if_de

# Machine type(s)
MACHINE =$(CPUFLGS) -DP6032 -DMHZ=120 -DTLBREGISTERS -DFLASH -DRTC -DINET -DBOOTPKG

MACHDIRS=$(SRC)/p6032:$(SRC)/pci

MACHINC =-I$(SRC)/p6032 -I$(SRC)/pci

MACHOBJ	=sbd.o sbdreset.o display.o
MACHOBJ +=sbdrtc.o 
MACHOBJ +=ns16550.o mon.o
MACHOBJ	+=sbdfreq.o 
MACHOBJ +=sbdflash.o sbduflash.o sbdbflash.o flash.o
MACHOBJ +=sbdflashenv.o sbdenv.o sbdtod.o sbdapc.o
MACHOBJ +=pciconf.o pcidevs.o pci_machdep.o sbdnet.o
MACHOBJ +=sbdflash8.o sbdflash16.o
MACHOBJ += $(NETDRV).o

override CONVFLAGBG="-b 3,2,1,0"
override CONVFLAGLG=

# Target
ALL = pmon upmon pram

# Target type(s)
ifndef TARGETS
TARGETS = LG BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

if_le_pci.o: $(LSIPKG)/net/sys/drivers/am7990.c
sbdenv.o: flashenv.c nvenv.c
sbdflash8.o: flashrom.c
sbdflash16.o: flashrom.c
