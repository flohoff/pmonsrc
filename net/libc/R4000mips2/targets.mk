# $Id: targets.mk,v 1.2 1996/01/16 14:20:46 chris Exp $ 
#
# targets.mk for Network Commands Library (netcmd.a)
#

CPU	=R4000
include	$(LSIPKG)/$(CPU)mips2.mk

# Machine type(s)
MACHINE =$(CPUFLGS)

# Target type(s)
ifndef TARGETS
TARGETS = BG LG
endif
