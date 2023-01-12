# $Id: files.mk,v 1.2 1996/01/16 14:20:05 chris Exp $ 
####################################################################
#     New modules should be added the appropriate section below    #
####################################################################


ASFILES	= 

CFILES	=inet_addr.c inet_lnaof.c inet_makeaddr.c inet_netof.c \
	 inet_network.c inet_ntoa.c \
	 gethostnamadr.c getnetbyname.c getprotoname.c \
	 getservbyname.c getservent.c \
	 netorder.c \
	 res_comp.c res_init.c res_mkquery.c res_query.c res_send.c \
	 send.c recv.c herror.c libc.c

OTHERS	=Makefile files.mk target.mk
