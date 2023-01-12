# $Id: targets.mk,v 1.4 1996/02/12 12:46:30 chris Exp $ 
#
# Targets.mk for PMON on Terma XDS
#

CPU	=R4000
ISA	=mips3

include $(LSIPKG)/$(CPU).mk

# start address for text and data (cached rom)
FTEXT	=9fc00000
FDATA	=80000200

# start address for text and data (uncached rom)
UFTEXT	=bfc00000
UFDATA	=a0000200

# start address for text (ram)
#RTEXT	=80020000
RTEXT	=a0020000

PROMFMT =bin 

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

NETLIBS	=$(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/net/libc/$(CPU)/$(ELIB)/netlibc.a \
	 $(LSIPKG)/net/sys/$(CPU)/$(ELIB)/netsys.a \
	 $(LSIPKG)/net/cmd/$(CPU)/$(ELIB)/netcmd.a \
	 $(LSIPKG)/lib/$(CPU)/$(ELIB)/libc.a

# Machine type(s)
MACHINE =$(CPUFLGS) -DXDS -DINET -DPTTY -DSTANDALONEVTTY -DNOSRAMPARTIALACCESS -DMHZ=33 -DXDSSONICBUG -DSWENV -DMAIN=sbd_main -DITBASE=0xbfc70000 -DPRINTMESSAGE

MACHDIRS=$(SRC)/xds

MACHINC =-I$(SRC)/xds

NETDRV	=if_xdssonic

MACHOBJ	=sbd.o sbdreset.o sbdnet.o sbdenv.o sbdmenu.o vtty.o ptty.o $(NETDRV).o


# Target
ALL = pmon upmon pram

# Target type(s)
ifndef TARGETS
TARGETS = BG
endif

sbdnet.o: sbdnet.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DINET -DKERNEL -DPROM -c $< -o $@

lclxxx.o: lclxxx.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys -I$(LSIPKG)/net/sys/sys -I$(LSIPKG)/net/include -I$(LSIPKG)/include -DINET -DKERNEL -DPROM -DLANGUAGE_C -c $< -o $@


sbdmenu.o: sbdmenu.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys -I$(LSIPKG)/net/sys/sys -I$(LSIPKG)/net/include -I$(LSIPKG)/include -DINET -DKERNEL -DPROM -DLANGUAGE_C -c $< -o $@

vtty.o: vtty.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -c $< -o $@

ptty.o: ptty.c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -c $< -o $@

