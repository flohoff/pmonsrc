# $Id: Makefile,v 1.2 1996/01/16 14:22:57 chris Exp $ 
include ../defines.mk
include files.mk

ifndef PMONDIRS
PMONDIRS = P4000 LSIPR CIRRMS IDT385 VME4000 MANN
endif

.PHONY: $(PMONDIRS)

all: $(PMONDIRS)

$(PMONDIRS):
	@echo "====> $@ <===="
	$(MAKE) -C $@ $(ACTION)

clean:
	@$(MAKE) ACTION=clean

revision:
	@putv $(REV) $(OTHERS)
	@$(MAKE) ACTION=revision
