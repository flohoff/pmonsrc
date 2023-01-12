# $Id: Makefile,v 1.2 1996/01/16 14:19:28 chris Exp $ 
include ../defines.mk

NETDIRS = sys libc cmd

all: $(NETDIRS)

.PHONY:	$(NETDIRS)

$(NETDIRS):
	@echo "====> $@ <===="
	@$(MAKE) -C $@ $(ACTION)

clean:
	@$(MAKE) ACTION=clean

revision:
	@$(MAKE) ACTION=revision
