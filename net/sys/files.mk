# $Id: files.mk,v 1.2 1996/01/16 14:20:51 chris Exp $ 
####################################################################
#     New modules should be added the appropriate section below    #
####################################################################


ASFILES	= 

CFILES	=init_main.c kern_proc.c kern_syscalls.c kern_glue.c kern_malloc.c \
	 kern_sig.c kern_synch.c kern_clock.c kern_time.c kern_descrip.c \
	 sys_generic.c sys_socket.c \
	 uipc_syscalls.c uipc_socket.c uipc_socket2.c \
	 uipc_mbuf.c uipc_domain.c \
	 raw_cb.c raw_usrreq.c \
	 rtsock.c route.c radix.c \
	 if_ether.c in.c in_cksum.c in_pcb.c in_proto.c \
	 ip_icmp.c ip_input.c ip_output.c raw_ip.c \
	 af.c if.c if_ethersubr.c if_loop.c \
	 udp_usrreq.c param.c machdep.c


OTHERS	=Makefile files.mk target.mk
