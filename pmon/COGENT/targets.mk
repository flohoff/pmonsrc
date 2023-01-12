# $Id: targets.mk,v 1.2 1996/01/16 14:23:32 chris Exp $ 
#
# Targets.mk for PMON on COGENT
#

CPU	=R4000
ISA	=mips2

include $(LSIPKG)/$(CPU)$(ISA).mk

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000200
FDATA	=a0000200

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

# Machine type(s)
MACHINE =$(CPUFLGS) -DR4300 -DCOGENT -DINET -DRTC -DNS16550 -DMHZ=33

MACHDIRS=$(SRC)/cogent

MACHINC =-I$(SRC)/cogent

NETDRV	=if_fe

MACHOBJ	=sbd.o sbdreset.o sbdnet.o $(NETDRV).o mk48t02.o ns16550.o cm1629.o


# Target
ALL = upmon pram

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@
