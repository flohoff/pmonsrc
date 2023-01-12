# $Id: targets.mk,v 1.2 1997/02/20 15:02:07 nigel Exp $ 
#
# Targets.mk for PMON on UBI pablo
#

CPU	=R3000
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

PROMFMT =sbin
override CONVFLAG=

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) -DR3081 -DPABLO -DEENVRAM -DINET -DMHZ=33

MACHDIRS=$(SRC)/pablo

MACHINC =-I$(SRC)/pablo

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_fe

MACHOBJ	=sbd.o sbdreset.o sbdnet.o centronics.o ns16550.o eeprom.o $(NETDRV).o

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

$(MACHOBJ): sbd.h
