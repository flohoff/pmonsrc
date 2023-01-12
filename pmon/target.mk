# $Id: target.mk,v 1.8 2000/08/03 22:34:32 chris Exp $ 
# This file is used to build each of the target directories.
# The variables TFLAG & ELIB are used to define target specific options.

SRC	=$(LSIPKG)/pmon

All: all

include $(LSIPKG)/defines.mk
include $(SRC)/files.mk
include ../targets.mk

ifndef ALL
 ALL=pmon upmon pram
endif

all: $(ALL)

VPATH  	=$(SRC):$(MACHDIRS)

OBJ	=$(CFILES:.c=.o) $(MACHOBJ) 

CLIB	=$(LSIPKG)/lib/$(CPU)/$(ELIB)
LIBS	=$(CLIB)/lib$(NMOD)c.a #$(FLIB)/lib$(NMOD)f.a

INC	=-I$(LSIPKG)/include -I$(SRC) $(MACHINC)
FLAGS	=$(SWFP) $(ENDIAN) $(HWBS) -DPMON -DIN_PMON -D$(CPU) $(MACHINE) $(OPT) $(NOSTDINC) $(INC) -DFDATA=0x$(FDATA)
CFLAGS	=$(TFLAG) $(FLAGS)
ASFLAGS	=$(TFLAG) $(FLAGS) $(CPUSFLGS)

LDFLAGS	=$(LFLAG) $(TBASE) $(FTEXT) $(DBASE) $(FDATA) 
ULDFLAGS=$(LFLAG) $(TBASE) $(UFTEXT) $(DBASE) $(UFDATA) 
RLDFLAGS=$(LFLAG) $(TBASE) $(RTEXT)

pmon: mips.o $(OBJ) $(LIBS) $(NETLIBS) date.o vers.o
	rm -f $@
	$(LD) $(LDFLAGS) -o $@ mips.o $(OBJ) date.o vers.o $(NETLIBS) $(LIBS)
	ln $@ $@-`cat ../version`
	$(CONV) -vp $(CONVFLAG) -f $(PROMFMT) -o $@.$(PROMFMT) $@ 

upmon: mips.o $(OBJ) $(LIBS) $(NETLIBS) date.o vers.o
	rm -f $@
	$(LD) $(ULDFLAGS) -o $@ mips.o $(OBJ) date.o vers.o $(NETLIBS) $(LIBS)
	ln $@ $@-`cat ../version`
	$(CONV) -vp $(CONVFLAG) -f $(PROMFMT) -o $@.$(PROMFMT) $@ 

pram: mipsram.o $(OBJ) $(LIBS) $(NETLIBS) date.o vers.o
	$(LD) $(RLDFLAGS) -o $@ mipsram.o $(OBJ) date.o vers.o $(NETLIBS) $(LIBS)


itromhi.s3: itromhi
	sde-conv -vp $(CONVFLAG) -f s3 -o $@ $<

pmon.s3: pmon
	sde-conv -vp $(CONVFLAG) -f s3 -o $@ $<

upmon.s3: upmon
	sde-conv -vp $(CONVFLAG) -f s3 -o $@ $<

pmonitrom.s3: itromhi.s3 pmon.s3 
	s3merge -k -o pmonitrom.s3 itromhi.s3 pmon.s3

date.o: $(OBJ) $(LIBS) $(NETLIBS)
	$(CC) $(CFLAGS) -c ../../date.c -o $@

vers.o: ../vers.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f *.o pmon pmon.* upmon upmon.* pram

-include ../rules.mk

$(NETDRV).o: $(LSIPKG)/net/sys/drivers/$(NETDRV).c
	$(CC) $(CFLAGS) -I$(LSIPKG)/net/sys $(MACHINC) -DKERNEL -D_KERNEL -DPROM -DLANGUAGE_C -DINET -c $< -o $@
