# $Id: targets.mk,v 1.2 1999/04/22 16:34:47 chris Exp $ 
#
# Targets.mk for PMON on hurricane
#
# a6 20/01/1999 04:30p rh  -remove -DV3DBG from MACHINE.
# a5 20/01/1999 04:00p rh  -add -DV3DBG to MACHINE for debugging.
# a4 21/10/1998 01:00p rh  -change PROMFMT =s3
# a3 21/10/1998 11:00A rh  -change P4032 to $(PMONDIRS) in MACHINE, MACHDIRS, MACHINC
# a2 20/10/1998  5:00p rh  -change P4032 to hurrican in MACHINE, MACHDIRS, MACHINC
# a1 22/09/1998  2:30p rh  -add v3mipusc.o v3mipi2c.o to MACHOBJ
# a0 08/09/1998 09:40a rh	-rid of DINET, DRTC
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

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)$(ISA)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)$(ISA)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)$(ISA)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)$(ISA)/$(ELIB)/libc.a

#NETDRV	=if_de

# Machine type(s)
MACHINE =$(CPUFLGS) -DHURRICANE -DTLBREGISTERS

MACHDIRS=$(SRC)/hurricane:$(SRC)/pci

MACHINC =-I$(SRC)/hurricane -I.

MACHOBJ	=sbd.o sbdreset.o v3mipusc.o v3mipi2c.o ns16550.o
#MACHOBJ += sbdflash.o flash.o
#MACHOBJ += pciconf.o pcidevs.o pci_machdep.o
#MACHOBJ += sbdnet.o $(NETDRV).o

override CONVFLAGBG=
override CONVFLAGLG=

# Target
ALL = upmon pmon pram

# Target type(s)
ifndef TARGETS
TARGETS = LG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@
