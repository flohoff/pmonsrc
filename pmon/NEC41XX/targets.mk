# $Id: targets.mk,v 1.1 1998/12/28 14:13:43 nigel Exp $ 
#
# Targets.mk for PMON on P4032
#

CPU	=R4000
ISA	=
include $(LSIPKG)/$(CPU)$(ISA).mk

FTEXT	=9fc00000
FDATA	=80000200

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000200

# start address for text (ram)
RTEXT	=80020000
#RTEXT	=a0020000

PROMFMT =s3

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_ne

# Machine type(s)
MACHINE =$(CPUFLGS) -DR41XX -DTLBREGISTERS -DRTC #-DFLASH #-DINET

MACHDIRS=$(SRC)/nec41xx

MACHINC =-I$(SRC)/nec41xx -I.

MACHOBJ	=sbd.o rtc.o ns16550.o sbdreset.o 
#MACHOBJ +=sbdnet.o $(NETDRV).o
#MACHOBJ +=sbdflash.o flash.o

override LFLAG += -Map $@.map
override OPT=-O2 -g

#override CONVFLAGBG="-b 3,2,1,0"
override CONVFLAGLG=

# Target
ALL = upmon pmon pram

# Target type(s)
# only little-endian currently
override TARGETS = LG

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@
