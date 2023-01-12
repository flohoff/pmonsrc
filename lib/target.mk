# $Id: target.mk,v 1.4 1998/10/06 14:17:02 nigel Exp $ 
# This file is used to build each of the target directories.
# The variable TFLAG is used by the parent makefile to define target 
# specific options.

SRC		=../..
VPATH	  	=$(SRC)

include $(LSIPKG)/defines.mk
include $(SRC)/files.mk
include ../targets.mk

OBJ = $(ASFILES:.S=.o) $(CFILES:.c=.o) $(OFILES)

INC = $(LSIPKG)/include
FLAGS = $(NOSTDINC) -I$(INC) -I$(SRC) $(SWFP) $(OPT) $(MACHINE) -DIN_PMON
CFLAGS = $(TFLAG) $(FLAGS)
ASFLAGS = $(TFLAG) $(FLAGS)
OBJS = libc.a crt1.o crtn.o crt0.o crt1r3000.o crtnr3000.o libr3000c.a

libc: $(OBJS)
	@echo libc is now up to date;echo "" > libc

libc.a: $(OBJ)
	rm -f libc.a
	$(AR) libc.a $(OBJ)

clean:
	@rm -f $(ASFILES:.S=.o) $(CFILES:.c=.o) $(OBJS)
	@for i in $(OFILES:.O=) XX ; do \
		if [ -r $(SRC)/$$i.S ] ; then \
			rm -f $$i.O ; \
		elif [ -r $(SRC)/$$i.c ] ; then \
			rm -f $$i.O ; \
		fi \
	done

-include ../rules.mk

# for MIPS hosted tools
crt1.o : $(SRC)/crt1.S
	$(CC) -c $(ASFLAGS) -o crt1.o $(SRC)/crt1.S
crtn.o : $(SRC)/crtn.S
	$(CC) -c $(ASFLAGS) -o crtn.o $(SRC)/crtn.S

# for Dec hosted tools
crt0.o : crt1.o
	cp crt1.o crt0.o

# for Sun hosted tools
crt1r3000.o : crt1.o
	cp crt1.o crt1r3000.o
crtnr3000.o : crtn.o
	cp crtn.o crtnr3000.o
libr3000c.a : libc.a
	cp libc.a libr3000c.a
