# $Id: defines.mk,v 1.4 1997/09/12 14:53:49 chris Exp $ 
######################################################################
# This file contains information about your configuration.
######################################################################

# system type Algorithmics SDE
CC	=sde-gcc
AS	=sde-gcc
AR	=sde-ar crs
LD	=sde-ld -n
CONV	=sde-conv

# these are the historical defaults
CONVFLAGBG = 
CONVFLAGLG = "-b3,2,1,0"

TBASE	=-Ttext
DBASE	=-Tdata

NOSTDINC=-nostdinc
#OPT	=-O2 -g
OPT	=-O2
#OPT	=-O3
