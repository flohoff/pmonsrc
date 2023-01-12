# $Id: targets.mk,v 1.2 1996/01/16 14:23:58 chris Exp $ 
#
# Targets.mk for PMON on VME4000
#

CPU	=R4000
include $(LSIPKG)/$(CPU).mk

# start address for text and data (rom & ram)
FTEXT = 9fc00000
FDATA = 80000200

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

#LIBNET	=$(LSIPKG)/net/$(CPU)/$(ELIB)/libnet.a
NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a

# Machine type(s)
MACHINE =$(CPUFLGS) -DVME4000 -DRTC -DINET

MACHDIRS=$(SRC)/algvme:$(SRC)/algcommon

MACHINC =-I$(SRC)/algvme -I$(SRC)/algcommon

NETDRV	=if_sonic

MACHOBJ	=mk48t02.o vacser.o sbd.o sbdreset.o sbdnet.o $(NETDRV).o

# Target type(s)
TARGETS = BG

sbdnet.o: $(SRC)/algcommon/sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -DPROM -DINET -c $< -o $@
