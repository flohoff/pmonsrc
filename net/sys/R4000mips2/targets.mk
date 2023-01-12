# $Id: targets.mk,v 1.2 1996/01/16 14:21:00 chris Exp $ 
#
# Targets.mk for Network Kernel library (libnet.a)
#

CPU	=R4000
include $(LSIPKG)/$(CPU)mips2.mk

# Machine type(s)
MACHINE =$(CPUFLGS)

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
