# $Id: targets.mk,v 1.1 1997/02/20 15:01:59 nigel Exp $ 
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

#PROMFMT =s3
PROMFMT =sbin

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

NETDRV	=if_fe

# Machine type(s)
MACHINE =$(CPUFLGS) -DR4300 -DFNC6 -DFLASH -DINET #-DITBASE=0xbfc70000

MACHDIRS=$(SRC)/fnc6

MACHINC =-I$(SRC)/fnc6 -I.

MACHOBJ	=sbd.o sbdreset.o eeprom.o ns16550.o display.o
MACHOBJ +=sbdflash.o flash.o
MACHOBJ +=sbdnet.o $(NETDRV).o

ifeq ($(ELIB),BG)
 override CONVFLAG=-b3,2,1,0
else
 override CONVFLAG=
endif

# Target
ALL = upmon pmon pram

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

$(MACHOBJ): $(SRC)/fnc6/sbd.h
eeprom.o: $(SRC)/fnc6/ecp.h
ns16550.o: $(SRC)/fnc6/ns16550.h
sbdflash.o flash.o: $(SRC)/fnc6/flash.h
