# $Id: target.mk,v 1.2 1996/01/16 14:20:54 chris Exp $ 
# This file is used to build each of the target directories.
# The variable TFLAG is used by the parent makefile to define target 
# specific options.

NET	=$(LSIPKG)/net
SRC	=$(NET)/sys
VPATH	=$(SRC)/kern:$(SRC)/machine:$(SRC)/net:$(SRC)/netinet

include $(LSIPKG)/defines.mk
include $(SRC)/files.mk
include ../targets.mk

OBJ	=$(ASFILES:.S=.o) $(CFILES:.c=.o) $(OFILES)
 
NETDEFS	=-DPROM -DINET -DKERNEL
INC	=-I$(SRC) -I$(SRC)/sys -I$(NET)/include -I$(LSIPKG)/include 
FLAGS	=$(NOSTDINC) $(INC) $(OPT) $(MACHINE) $(NETDEFS)
CFLAGS	=$(TFLAG) $(FLAGS) -DLANGUAGE_C
ASFLAGS	=$(TFLAG) $(FLAGS)
DEST	=netsys

$(DEST): $(DEST).a
	@echo $(DEST) is now up to date; touch $(DEST)

$(DEST).a: $(OBJ)
	rm -f $(DEST).a
	$(AR) $(DEST).a $(OBJ)
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
