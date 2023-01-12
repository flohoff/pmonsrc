# $Id: targets.mk,v 1.3 1998/12/28 14:12:28 nigel Exp $ 
CPU	=LR33K

include $(LSIPKG)/$(CPU).mk

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) '-Acache(lr30)'

# Target type(s)
TARGETS = BG

