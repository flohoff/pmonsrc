# $Id: targets.mk,v 1.4 1996/09/10 13:11:08 chris Exp $ 
#
# Targets.mk for PMON on P4000
#

CPU	=R4000
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

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a

# Machine type(s)
MACHINE =$(CPUFLGS) -DP4000 -DINET -DRTC -DTLBREGISTERS -DITBASE=0xbfc70000

MACHDIRS=$(SRC)/p4000:$(SRC)/algcommon

MACHINC =-I$(SRC)/p4000 -I$(SRC)/algcommon

NETDRV	=if_sonic

MACHOBJ	=mk48t02.o mpsc.o sbd.o sbdreset.o sbdnet.o $(NETDRV).o \
	#/usr/algor/chris/IT/bt/btpkg.o \
	#/usr/algor/chris/IT/P4000i/it.o


# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif

sbdnet.o: $(SRC)/algcommon/sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -DPROM -DINET -c $< -o $@
