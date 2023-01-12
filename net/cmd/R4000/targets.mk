# $Id: targets.mk,v 1.2 1996/01/16 14:19:45 chris Exp $ 
#
# targets.mk for Network Commands Library (netcmd.a)
#

CPU	=R4000
include $(LSIPKG)/$(CPU).mk

# Machine type(s)
MACHINE =$(CPUFLGS)

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
