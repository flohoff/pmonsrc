# $Id: targets.mk,v 1.3 1998/12/28 14:12:32 nigel Exp $ 
CPU	=R3000

include $(LSIPKG)/$(CPU).mk

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) '-Acache(r3k)'

# Target type(s)
TARGETS = BG

