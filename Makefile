# $Id: Makefile,v 1.6 2000/08/23 14:34:14 chris Exp $ 
include defines.mk
include files.mk

ifndef LSIPKG
 LSIPKG		=$(shell pwd)
endif

ifndef PMONDIRS
 PMONDIRS	=P4000
endif

ifndef LIBDIRS
 LIBDIRS	=R4000
endif

ifndef TARGETS
 TARGETS	=BG LG
endif

export LSIPKG
export PMONDIRS
export LIBDIRS
export TARGETS

#DIRS = fpem fpem.new lib net tools pmon 
DIRS = lib net tools pmon 

all: $(DIRS)

.PHONY:	$(DIRS)

$(DIRS):
	@echo "====> $@ <===="
	$(MAKE) -C $@ $(ACTION)

clean:
	@$(MAKE) ACTION=clean

revision:
	@putv $(REV) $(OTHERS)
	@$(MAKE) ACTION=revision


.PHONY: TAGS
TAGS:
	cp /dev/null $(LSIPKG)/TAGS
	find -L $(LSIPKG) -name "*.[chsS]" -print | xargs etags -a -o $(LSIPKG)/TAGS
