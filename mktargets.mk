# $Id: mktargets.mk,v 1.2 1996/01/16 14:16:47 chris Exp $ 
# This file makes all target directories that exist if they also
# appear in the variable TARGETS defined in defines.mk

include $(LSIPKG)/defines.mk
include ../files.mk
include ./targets.mk

FILES = $(CFILES:.c=) $(ASFILES:.s=)
OBJ_FILES = $(OFILES:.O=)

# make each target directory
all: rules.mk
	@for i in $(TARGETS) ; do \
	  if [ ! -d $$i ] ; then \
	    case $$i in  \
	      BG) mkdir BG ;;\
	      BO) mkdir BO ;;\
	      LG) mkdir LG ;;\
	      LO) mkdir LO ;;\
	    esac ;\
	  fi ; \
	  if [ -d $$i ] ; then \
	    echo "======> $$i <====" ; \
	    case $$i in  \
	      BG) (cd $$i;$(MAKE) -f ../../target.mk "TFLAG= -EB -G 0" ) ;; \
	      BO) (cd $$i;$(MAKE) -f ../../target.mk "TFLAG= -EB -G 8" ) ;; \
	      LG) (cd $$i;$(MAKE) -f ../../target.mk "TFLAG= -EL -G 0" ) ;; \
	      LO) (cd $$i;$(MAKE) -f ../../target.mk "TFLAG= -EL -G 8" ) ;; \
	    esac ; \
	  fi ; \
	done

tape :
	@for i in $(CFILES) $(ASFILES) $(OTHERS) ; do \
		echo $(DIR)/$$i ; \
	done
	@for i in BG BO LG LO ; do \
		for j in $(OFILES) ; do \
			echo $(DIR)/$$i/$$j ; \
		done ; \
	done

revision :
	@putv $(REV) $(OTHERS) $(CFILES) $(ASFILES) $(PFILES)

include $(LSIPKG)/stdrules.mk
