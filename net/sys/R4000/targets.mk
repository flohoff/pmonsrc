# $Id: targets.mk,v 1.2 1996/01/16 14:20:56 chris Exp $ 
#
# Targets.mk for Network Kernel library (libnet.a)
#

CPU	=R4000
include $(LSIPKG)/$(CPU).mk

# Machine type(s)
MACHINE =$(CPUFLGS)

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
