# $Id: subsys.mk,v 1.3 1996/04/23 09:51:04 chris Exp $ 
# This file makes all target directories that exist if they also
# appear in the variable TARGETS defined in defines.mk

SRC	=..

include $(LSIPKG)/defines.mk
include ../files.mk
include ./targets.mk

FILES	=$(CFILES:.c=) $(ASFILES:.s=)
OBJ_FILES=$(OFILES:.O=)

all: $(TARGETS)

.PHONY:	$(TARGETS)

#$(TARGETS)::
#	+-@mkdir $@

$(TARGETS)::
	for DIR in $@ ; \
	do \
		if test ! -d $$DIR ; then \
		mkdir $$DIR; \
		fi ; \
	done

BG:: 
	@echo "====> $@ <===="
	$(MAKE) -C $@ -f ../../target.mk TFLAG="-EB -G 0"

BO:: 
	@echo "====> $@ <===="
	$(MAKE) -C $@ -f ../../target.mk TFLAG="-EB -G 8"

LG:: 
	@echo "====> $@ <===="
	$(MAKE) -C $@ -f ../../target.mk TFLAG="-EL -G 0"

LO:: 
	@echo "====> $@ <===="
	$(MAKE) -C $@ -f ../../target.mk TFLAG="-EL -G 8"

revision:
	@putv $(REV) $(OTHERS) $(CFILES) $(ASFILES) $(PFILES)

include $(LSIPKG)/stdrules.mk
