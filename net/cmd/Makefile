# $Id: Makefile,v 1.2 1996/01/16 14:19:30 chris Exp $ 
include $(LSIPKG)/defines.mk
include files.mk

ifndef LIBDIRS
LIBDIRS = R4000 R3000
endif

all: $(LIBDIRS)

.PHONY:	$(LIBDIRS)

$(LIBDIRS):
	@echo "====> $@ <===="
	$(MAKE) -C $@ $(ACTION)

clean:
	@$(MAKE) ACTION=clean

revision:
	@putv $(REV) $(OTHERS)
	@$(MAKE) ACTION=revision
