# $Id: targets.mk,v 1.1 1996/06/17 15:50:15 chris Exp $ 
#
# Targets.mk for PMON on MIDAS Arcade
#

ALL	=pmon pram

CPU	=R4000
include $(LSIPKG)/$(CPU).mk

# start address for text and data (cached rom)
FTEXT	=9fc00000
FDATA	=80000200

# start address for text and data (uncached rom)
UFTEXT	=unusable	#bfc00000
UFDATA	=unusable	#a0000200

# start address for text (ram)
#RTEXT	=80020000
RTEXT	=a0020000

PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a

# Machine type(s)
# MIDASPABUG	no partial word read accesses to even word on SCBUS
# MIDASWRBUG	ethernet writes corrupted by PROM read
# MIDASRABUG	enabling DRAM readaheah makes PMON fail
BUGS= -DMIDASPABUG -DMIDASWRBUG -DMIDASRABUG
MACHINE =$(CPUFLGS) $(BUGS) -DMIDAS -DINET -DEENVRAM -DHAVETIMESOURCE # -DITBASE=0xbfc70000 


MACHDIRS=$(SRC)/midas

MACHINC =-I$(SRC)/midas

NETDRV	=if_fe

MACHOBJ	=ns16550.o eeprom.o sbd.o sbdreset.o sbdenv.o sbdnet.o $(NETDRV).o

# Target type(s)
ifndef TARGETS
TARGETS = LG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -DPROM -DINET -c $< -o $@
