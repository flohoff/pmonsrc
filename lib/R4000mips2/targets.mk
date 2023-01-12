# $Id: targets.mk,v 1.3 1998/12/28 14:12:40 nigel Exp $ 
CPU	=R4000

include $(LSIPKG)/$(CPU)mips2.mk

# fp emulation type NONE
SWFP	=-DFLOATINGPT -DNEWFP

# Machine type(s)
MACHINE =$(CPUFLGS) \
	'-Acache(r4k)' '-Acache(r5k)' '-Acache(rm7k)' '-Acache(r5400)'

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
