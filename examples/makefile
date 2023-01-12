# $Id: makefile,v 1.2 1996/01/16 14:17:00 chris Exp $ 

####################################################################
#     New modules should be added the appropriate section below    #
####################################################################

CFILES = acker.c bubble.c dhry.c dry1.c fibo.c game.c hanoi.c hello.c \
	 puzzle.c qsort.c queens.c sieve.c sinewave.c sort.c timetst.c \
	 whetd.c whets.c wiggle.c filetst.c fmain.c onintr.c keybd.c

ASFILES = e4isr.s ftest.s 

OTHERS = Makefile files.mk infile


######################################################################
# This file contains information about your configuration.
######################################################################

# system type ALGDOSSDE
AR = ar crs

# fp emulation type NONE

# Machine type(s)
MACHINE = -DLR33050 -DLR33000

# Target type(s)
TARGETS = BG BO

OPT = -O
FLAGS = $(OPT) -DPMCC $(MACHINE) -DALGSDE -nostdinc -I../include
CFLAGS = $(FLAGS) -DLANGUAGE_C
ASFLAGS = $(FLAGS) -DLANGUAGE_ASSEMBLY

.SUFFIXES: .srec

.c.o:
	>opts $(CFLAGS) -c $<
	$(CC) @opts

.S.o:
	>opts $(ASFLAGS) -c $<
	$(CC) @opts

.s.o:
	>opts $(ASFLAGS) -c $<
	$(CC) @opts

LIBC=../lib/bg
LIBM=../lib/bg

.c.srec:
	>cmd -DPMCC $(CFLAGS) -N -nostdinc -I../include -nostdlib -Ttext 80020000 -o $* -L$(LIBC) -L$(LIBM) $(LIBC)/crt0.o $< $(LIBC)/crtn.o -lc -lm
	$(CC) @cmd
	mkrom -o r.out $*
	srec -o $*.srec r.out
	del r.out

PROGS = acker.srec bubble.srec dhry.srec dry1.srec fibo.srec game.srec hanoi.srec \
	hello.srec puzzle.srec qsort.srec queens.srec sieve.srec sinewave.srec \
	sort.srec timetst.srec whetd.srec whets.srec wiggle.srec filetst.srec \
	fmain.srec onintr.srec keybd.srec

all : $(PROGS)

clean :
	del $(PROGS) *.rec *.o r.out s.out files.c

tape :
	@for i in $(CFILES) $(ASFILES) $(OTHERS) ; do \
		echo $(DIR)/$$i ; \
	done

revision :
	@putv $(REV) $(CFILES) $(ASFILES) $(OTHERS)

onintr.srec: onintr.o e4isr.o
	>cmd $(FLAGS) -N -nostdlib -Ttext 80020000 -o $* -L$(LIBC) -L$(LIBM) $(LIBC)/crt0.o onintr.o e4isr.o $(LIBC)/crtn.o -lc -lm
	$(CC) @cmd
	mkrom -o r.out $*
	srec -o $*.srec r.out
	del r.out

fmain.srec: ftest.o fmain.o
	>cmd $(FLAGS) -N -nostdlib -Ttext 80020000 -o $* -L$(LIBC) -L$(LIBM) $(LIBC)/crt0.o fmain.o ftest.o $(LIBC)/crtn.o -lc -lm
	$(CC) @cmd
	mkrom -o r.out $*
	srec -o $*.srec r.out
	del r.out
