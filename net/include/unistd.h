/* $Id: unistd.h,v 1.2 1996/01/16 14:20:01 chris Exp $ */
/*
 * unistd.h : POSIX definitions
 * Copyright (c) 1992 Algorithmics Ltd.
 */
#ifndef __UNISTD_H
#define __UNISTD_H

/* --- Required -- */
#include <stddef.h>
#include <sys/types.h>
#include <sys/cdefs.h>

#define _POSIX_VERSION	198808L	/* hmm */

#ifdef IDTSIM
extern int stdin, stdout;
#define STDIN_FILENO	stdin
#define STDOUT_FILENO	stdout
#define STDERR_FILENO	stdout
#else
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2
#endif

/* --- For access(2) --- */
#define     R_OK     	4
#define     W_OK     	2
#define     X_OK     	1
#define     F_OK     	0

/* --- For lockf(2) --- */
#define     F_ULOCK	0
#define     F_LOCK	1
#define     F_TLOCK	2
#define     F_TEST	3

/* --- For lseek(2) --- */
#define     SEEK_SET	0
#define     SEEK_CUR	1
#define     SEEK_END	2

/* --- For sysconf() --- */
#define _SC_ARG_MAX	1
#define _SC_CHILD_MAX	2
#define _SC_CLK_TCK	3
#define _SC_NGROUPS_MAX	4
#define _SC_OPEN_MAX	5
#define _SC_JOB_CONTROL	6
#define _SC_SAVED_IDS	7
#define _SC_VERSION	8

/* --- For pathconf() --- */
#define _PC_CHOWN_RESTRICTED	1
#define _PC_MAX_CANNON		2
#define _PC_NAX_INPUT		3
#define _PC_NAME_MAX		4
#define _PC_NO_TRUNC		5
#define _PC_PATH_MAX		6
#define _PC_PIPE_BUF		7
#define _PC_VDISABLE		8

/* --- Prototypes --- */
  /* --- Process creation and execution --- */
int	spawn __P((const char *, int(*)(int, char **), int, char **));

  /* --- Process termination --- */
void	exit __P((int));
void	_exit __P((int));

  /* --- Timer operations --- */
unsigned int alarm __P((unsigned int));

  /* --- Process identification --- */
pid_t	getpid __P((void));
pid_t	getppid __P((void));

  /* --- User identification --- */
uid_t	getuid __P((void));
uid_t	geteuid __P((void));
gid_t	getgid __P((void));
gid_t	getegid __P((void));
int	setuid __P((uid_t));
int	setgid __P((gid_t));

  /* --- Process groups --- */
pid_t	getpgrp __P((void));

  /* --- File descriptor manipulation  POSIX, use fcntl --- */
int	dup __P((int));
int	dup2 __P((int, int));

  /* --- File descriptor deassignment --- */
int	close __P((int));

  /* --- Input and output --- */
size_t	read __P((int, void  *, size_t));
size_t	write __P((int fd, const void *, size_t));

  /* --- Control operations on files --- */
off_t	lseek __P((int, off_t, int));

#endif /* !defined __UNISTD_H */
