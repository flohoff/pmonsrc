# $Id: targets.mk,v 1.1 1996/12/09 20:10:51 nigel Exp $ 
#
# Targets.mk for Network Kernel library (libnet.a)
#

CPU	=R3000
include $(LSIPKG)/$(CPU).mk

# Machine type(s)
MACHINE =$(CPUFLGS)

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
