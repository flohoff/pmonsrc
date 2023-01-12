# $Id: files.mk,v 1.5 2000/03/28 00:20:53 nigel Exp $ 
####################################################################
#     New modules should be added the appropriate section below    #
####################################################################

CFILES = main.c commands.c dis.c hist.c machine.c regs.c sym.c \
	 set.c stty.c more.c load.c loadfn.c go.c debug.c regdefs.c \
	 memtst.c cmdtable.c time.c sbrk.c demo.c \
	 watch.c r4kwatch.c r5kwatch.c rm7kwatch.c r4650watch.c

ASFILES = mips.S 

OTHERS = Makefile README files.mk target.mk set.h pmon.h vers.c
