# $Id: files.mk,v 1.10 2000/03/28 00:20:40 nigel Exp $ 
####################################################################
#     New modules should be added the appropriate section below    #
####################################################################


ASFILES = clock.S longjmp.S cache.S r3kcache.S lr30cache.S \
	  cwcache.S r4kcache.S r5kcache.S r54cache.S rm7kcache.S \
	  setbs.S haswhat.S abort.S fabs.S fpstate.S cp2supp.S \
	  cp0supp.S strcmp.S memcpy.S memset.S
# modf.S

CFILES = argvize.c atob.c bcopy.c cc2str.c \
	 errno.c exit.c getword.c index.c lseek.c malloc.c \
	 printf.c queue.c rindex.c sbrk.c sizemem.c stdio.c \
	 str2cc.c str_fmt.c strbalp.c strbequ.c strcat.c strccat.c \
	 strchr.c strcspn.c strdchr.c strempty.c \
	 strequ.c strichr.c striequ.c stristr.c strlen.c strmerge.c \
	 strncat.c strnchr.c strncmp.c strncpy.c strnwrd.c strpat.c strpbrk.c \
	 strposn.c strrchr.c strrpset.c strrrot.c strrset.c strset.c \
	 strsort.c strspn.c strstr.c strtok.c strtol.c strtoupp.c \
	 termio.c terms.c time.c toupper.c vsprintf.c \
	 atod.c read.c bzero.c calloc.c atof.c atol.c rand.c \
	 open.c close.c write.c getmach.c atoi.c realloc.c \
	 fileno.c scanf.c files.c feof.c signal.c \
	 eraline.c getchar.c getc.c ungetc.c putchar.c \
	 puts.c putc.c fgets.c fputs.c fflush.c sprintf.c fprintf.c \
	 vfprintf.c fwrite.c fread.c fseek.c isalnum.c isalpha.c \
	 iscntrl.c isdigit.c islower.c isprint.c isspace.c isxdigit.c \
	 fclose.c gets.c fgetc.c strcpy.c qsort.c getopt.c memcmp.c \
	 modf.c

#OFILES = dtoa.O

#PFILES = dtoa.S

OTHERS = Makefile files.mk target.mk crt1.S crtn.S
