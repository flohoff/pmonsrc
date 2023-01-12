/* $Id: proc.h,v 1.3 1998/06/17 00:49:57 chris Exp $ */
/*
 * proc.h: pretend process structure for IDT/sim
 */

#ifndef _PROC_H_
#define _PROC_H_

#include <setjmp.h>

struct proc {
    pid_t	p_pid;
    void	*p_ucred;
    void	*p_cred;
    struct	filedesc *p_fd;
    struct	pstats *p_stats;	/* accounting/statistics (PROC ONLY) */
    u_short 	p_flag;
    u_short 	p_acflag;
    int 	p_stat;
    int 	p_pri;
    caddr_t	p_wchan;
    char	*p_wmesg;
    char	*p_comm;	/* process name */
    struct	itimerval p_realtimer;	/* alarm timer */
    int		p_sig;		/* set of pending signals */
    sigset_t	p_sigmask;	/* current signal mask */
    sigset_t	p_sigignore;	/* signals being ignored */
    sigset_t	p_sigcatch;	/* signals being caught by user */
    struct sigacts *p_sigacts;
    jmp_buf	p_exitbuf;
};

/* p_flag */
#define SSEL	0x1
#define STIMO	0x2
#define SSINTR	0x4

/* p_stat */
#define SZOMB	0		/* zombie process */
#define SSLEEP	1		/* process is sleeping */
#define SNOTKERN 2		/* running outside net kernel */
#define SRUN	3		/* running inside net kernel */

struct proc *curproc;
struct proc *pfind();
extern struct proc proc[];
int nprocs;
int maxproc;

#ifdef PROM
#ifdef __STDC__
#define SYSCALL(name) sys_##name
#else
#define SYSCALL(name) sys_/**/name
#endif
#else
#define SYSCALL(name) name
#endif

#define crhold(credp)
#define crfree(credp)

#endif /* !_PROC_H_ */
