# $Id: subsys.mk,v 1.6 1998/12/28 14:13:34 nigel Exp $ 
# This file makes all target directories that exist if they also
# appear in the variable TARGETS defined in defines.mk

SRC = ..

all: pmon

include $(LSIPKG)/defines.mk
include ../files.mk
include ./targets.mk

FILES = $(CFILES:.c=) $(ASFILES:.S=)

.PHONY: pmon $(TARGETS)

pmon: $(TARGETS)

#$(TARGETS)::
#	+-@mkdir $@

$(TARGETS)::
	for DIR in $@ ; \
	do \
		if test ! -d $$DIR ; then \
		mkdir $$DIR; \
		fi ; \
	done

BG:: vers.c
	$(MAKE) -C $@ -f ../../target.mk \
		TFLAG="-EB -G 0" ELIB=BG \
		CONVFLAG=$(CONVFLAGBG)

# For SDE-MIPS 3.0 we could have LFLAG=-EL but that wouldn't be
# backwards compatible with SDE-MIPS 2
LG:: vers.c
	$(MAKE) -C $@ -f ../../target.mk \
		TFLAG="-EL -G 0" ELIB=LG \
		LFLAG="-oformat elf32-littlemips -b elf32-littlemips" \
		CONVFLAG=$(CONVFLAGLG)

SABLE:: vers.c
	$(MAKE) -C $@ -f ../../target.mk \
		TFLAG="-EB -G 0 -DSABLE" ELIB=BG

vers.c : FRC
	sh $(LSIPKG)/tools/newvers >vers.c

FRC :

tape :
	@for i in $(CFILES) $(ASFILES) $(OTHERS) ; do \
		echo $(DIR)/$$i ; \
	done

revision :
	@putv $(REV) $(OTHERS) $(CFILES) $(ASFILES) date.c vers.c
	@for i in $(TARGETS) ; do \
	  if [ -d $$i ] ; then \
		cp $$i/pmon VC/pmon.$$i.`getv -l Makefile`; \
	  fi \
	done
	@rm -f vers.c
	@date=`date|awk '{print $$2,$$3,$$6}'` ; \
	rev=`getv -l Makefile` ; \
	rm -f README ; \
	sed -e s/%DATE%/"$$date"/g -e s/%VERS%/$$rev/g README.sh > README ; \
	chmod -w README

include $(LSIPKG)/stdrules.mk
